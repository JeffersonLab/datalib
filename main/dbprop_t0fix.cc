/*! \file
 * \brief Copy perambulators, wrapping source and sink times into the lattice.
 *
 * Usage: dbprop_t0fix <new db> <old db>
 * Both times become (old_time + 2*Nt) % Nt, where Nt is the lattice
 * extent along decay_dir in the input metadata. Values are unchanged.
 * The output must not already exist; wrapped key collisions are errors.
 *
 * StorageProp4 is a reader interface, not a single on-disk format:
 *   - Without USE_SUPERBBLAS: FileDB input and FileDB output only.
 *   - With USE_SUPERBBLAS: FileDB or superbblas tensor input; output uses
 *     the same storage family as the input (no automatic conversion).
 * Thus FileDB files such as prop.sdb do not require superbblas support.
 * The filename extension does not select the storage format.
 *
 * FileDB records contain individual spin pairs. Their serialized values
 * and all metadata are copied byte-for-byte; only the two leading time
 * fields of each key change. The superbblas path uses Prop4 values that
 * group all spin pairs, writing an eight-dimensional tensor with phasings.
 */
#include <algorithm>
#include <datalib/codec/binary.h>
#include <datalib/io/serial_xml.h>
#include <datalib/storage/dbtype.h>
#include <datalib/storage/filedb.h>
#include <datalib/superb/prop_superb.h>
#include <filesystem>
#include <set>
#include <stdexcept>
#include <tuple>

using namespace Hadron;

namespace {
  // Keep the complete serialized records, including any optional key fields.
  struct RawKey : Datalib::Storage::SerialDBKey<OldKeyPropElementalOperator_t> {
    std::string bytes;
    void readObject(const std::string &input) { bytes = input; }
    void writeObject(std::string &output) const { output = bytes; }
  };
  struct RawValue : Datalib::Storage::SerialDBData<ValPropElementalOperator_t> {
    std::string bytes;
    void readObject(const std::string &input) { bytes = input; }
    void writeObject(std::string &output) const { output = bytes; }
  };

  // Widen before adding; normalize the C++ remainder for very negative times.
  int wrapTime(int t, int Nt) {
    const auto r = (static_cast<long long>(t) + 2LL * Nt) % Nt;
    return static_cast<int>(r < 0 ? r + Nt : r);
  }

  void copyFileDB(const std::string &output, const std::string &input) {
    if (std::filesystem::exists(output))
      throw std::runtime_error("output already exists: " + output);
    // Use exactly the FileDB backend used by StorageProp4, retaining each spin
    // record.
    Datalib::TensorStorage::StorageProp4 storage(false);
    if (storage.open({input}) != 0 || storage.db_stores.size() != 1)
      throw std::runtime_error("expected one FileDB input");
    auto &dbin = storage.db_stores.front().db;
    std::string meta;
    if (dbin.getUserdata(meta) != 0)
      throw std::runtime_error("cannot read metadata");
    const auto layout = getDBLayoutFromMetaData(meta);
    if (layout.decay_dir < 0 || layout.decay_dir >= layout.latt_size.size())
      throw std::runtime_error("invalid decay_dir");
    const int Nt = layout.latt_size[layout.decay_dir];
    if (Nt <= 0)
      throw std::runtime_error("invalid lattice time extent");
    std::vector<RawKey> keys;
    dbin.keys<RawKey, RawValue>(keys);
    std::vector<RawKey> new_keys;
    // Check all destinations before creating output: modulo can merge keys.
    std::set<std::string> destinations;
    for (const auto &key : keys) {
      // Both prop key formats start with t_slice, then t_source.
      Datalib::Binary::BinaryBufferReader reader(key.bytes);
      int t_slice, t_source;
      read(reader, t_slice);
      read(reader, t_source);
      Datalib::Binary::BinaryBufferWriter writer;
      write(writer, wrapTime(t_slice, Nt));
      write(writer, wrapTime(t_source, Nt));
      const auto prefix = writer.str();
      if (key.bytes.size() < prefix.size())
        throw std::runtime_error("truncated key");
      RawKey updated = key;
      updated.bytes.replace(0, prefix.size(), prefix);
      if (!destinations.insert(updated.bytes).second)
        throw std::runtime_error(
            "wrapping produces duplicate keys; refusing to overwrite values");
      new_keys.push_back(updated);
    }
    FILEDB::ConfDataStoreDBSimple dbout;
    dbout.setMaxUserInfoLen(meta.size());
    dbout.setNumberBuckets(keys.size());
    if (dbout.open(output, O_RDWR | O_CREAT | O_EXCL, 0644) != 0)
      throw std::runtime_error("cannot create output: " + output);
    if (dbout.insertUserdata(meta) != 0)
      throw std::runtime_error("cannot write metadata");
    for (std::size_t i = 0; i < keys.size(); ++i) {
      RawValue value;
      if (dbin.get(keys[i], value) != 0)
        throw std::runtime_error("cannot read value");
      if (dbout.insert(new_keys[i], value) != 0)
        throw std::runtime_error("cannot write value");
    }
    dbout.close();
    std::cout << "Copied " << keys.size() << " key-value pairs; Nt=" << Nt
              << '\n';
  }
} // namespace

#ifdef USE_SUPERBBLAS
// Only the tensor-format writer below depends on superbblas.
namespace {
  using K = KeyProp4ElementalOperator_t;
  using V = ValProp4ElementalOperator_t;
  using T = Datalib::TensorStorage::StorageProp4::T;

  // Replace a direct child of DBMetaData while retaining all other metadata.
  void replaceMetadata(std::string &meta, const std::string &name,
                       const std::string &xml) {
    const std::string begin = "<" + name + ">", end = "</" + name + ">";
    // A similarly named element can also appear inside Params.
    std::size_t pos = std::string::npos;
    int depth = 0;
    for (std::size_t i = 0; (i = meta.find('<', i)) != std::string::npos;) {
      const auto end_tag = meta.find('>', i);
      if (end_tag == std::string::npos)
        throw std::runtime_error("invalid XML metadata");
      if (meta.compare(i, begin.size(), begin) == 0 && depth == 1) {
        pos = i;
        break;
      }
      if (meta[i + 1] == '/')
        --depth;
      else if (meta[i + 1] != '?' && meta[i + 1] != '!' &&
               meta[end_tag - 1] != '/')
        ++depth;
      i = end_tag + 1;
    }
    if (pos != std::string::npos) {
      const auto last = meta.find(end, pos);
      if (last == std::string::npos)
        throw std::runtime_error("invalid metadata: " + name);
      meta.replace(pos, last + end.size() - pos, xml);
    } else {
      const auto last = meta.rfind("</DBMetaData>");
      if (last == std::string::npos)
        throw std::runtime_error("missing DBMetaData");
      meta.insert(last, xml);
    }
  }

  int wrap(int t, int Nt) {
    // Widen before adding to avoid signed integer overflow.
    const auto r = (static_cast<long long>(t) + 2LL * Nt) % Nt;
    return static_cast<int>(r < 0 ? r + Nt : r);
  }

  void copyDatabase(const std::string &output, const std::string &input) {
    if (std::filesystem::exists(output))
      throw std::runtime_error("output already exists: " + output);
    std::string meta_str = getMetaData(input);
    Hadron::Layout::Layout_t layout(Hadron::getDBLayoutFromMetaData(meta_str));
    if (layout.decay_dir < 0 || layout.decay_dir >= layout.latt_size.size())
      throw std::runtime_error("invalid decay_dir");
    int Nt = layout.latt_size[layout.decay_dir];
    if (Nt <= 0)
      throw std::runtime_error("invalid lattice time extent");

    Datalib::TensorStorage::StorageProp4 dbin(false);
    if (dbin.open({input}) != 0)
      throw std::runtime_error("cannot open input: " + input);
    std::vector<K> keys;
    dbin.keys(keys);
    if (keys.empty())
      throw std::runtime_error("input has no keys");

    // StorageProp4 stores one mass and a shared list of source/sink phasings.
    std::vector<std::vector<int>> phasings;
    auto phaseIndex = [&phasings](const XMLArray::Array<int> &phase) {
      const std::vector<int> p(phase);
      auto it = std::find(phasings.begin(), phasings.end(), p);
      if (it == phasings.end()) {
        phasings.push_back(p);
        return int(phasings.size() - 1);
      }
      return int(it - phasings.begin());
    };
    std::set<std::tuple<int, int, int, int>> destinations;
    for (const auto &key : keys) {
      if (key.mass_label != keys.front().mass_label)
        throw std::runtime_error(
            "one output StorageProp4 file cannot contain multiple masses");
      const int src = phaseIndex(key.phasing_source);
      const int snk = phaseIndex(-key.phasing_sink);
      if (!destinations
               .emplace(wrap(key.t_source, Nt), wrap(key.t_slice, Nt), src, snk)
               .second)
        throw std::runtime_error(
            "wrapping produces duplicate keys; refusing to overwrite values");
    }

    Datalib::Serial::XMLBufferWriter xml;
    write(xml, "phasings", phasings);
    replaceMetadata(meta_str, "phasings", xml.printRoot());
    Datalib::Serial::XMLBufferWriter mass_xml;
    write(mass_xml, "mass_label", keys.front().mass_label);
    replaceMetadata(meta_str, "mass_label", mass_xml.printRoot());
    replaceMetadata(meta_str, "tensorOrder",
                    "<tensorOrder>sqnNpPhH</tensorOrder>");

    V val;
    if (dbin.get(keys.front(), val) != 0)
      throw std::runtime_error("cannot read first value");
    const int n = val.op.size3();
    const superbblas::Coor<8> dim{Datalib::TensorStorage::spin_count,
                                  Datalib::TensorStorage::spin_count,
                                  n,
                                  n,
                                  Nt,
                                  Nt,
                                  int(phasings.size()),
                                  int(phasings.size())};
    const char *order = "sqnNpPhH";
    superbblas::Storage_handle stoh;
    superbblas::create_storage<8, T>(
        dim, superbblas::FastToSlow, output.c_str(), meta_str.c_str(),
        meta_str.size(), superbblas::checksum_type::BlockChecksum, &stoh);
    try {
      const auto ctx = superbblas::createCpuContext();
      for (const auto &old_key : keys) {
        if (dbin.get(old_key, val) != 0)
          throw std::runtime_error("cannot read value");
        if (val.op.size3() != n || val.op.size4() != n ||
            val.op.size1() != Datalib::TensorStorage::spin_count ||
            val.op.size2() != Datalib::TensorStorage::spin_count)
          throw std::runtime_error("inconsistent value dimensions");
        K key = old_key;
        key.t_source = wrap(old_key.t_source, Nt);
        key.t_slice = wrap(old_key.t_slice, Nt);
        const superbblas::Coor<8> from{0,
                                       0,
                                       0,
                                       0,
                                       key.t_source,
                                       key.t_slice,
                                       phaseIndex(key.phasing_source),
                                       phaseIndex(-key.phasing_sink)};
        const superbblas::Coor<8> size{Datalib::TensorStorage::spin_count,
                                       Datalib::TensorStorage::spin_count,
                                       n,
                                       n,
                                       1,
                                       1,
                                       1,
                                       1};
        const superbblas::PartitionItem<8> block{from, size};
        superbblas::append_blocks<8, 8, T>(&block, 1, order, {}, dim, dim,
                                           order, {}, stoh,
                                           superbblas::FastToSlow);
        const superbblas::Coor<4> value_size{Datalib::TensorStorage::spin_count,
                                             Datalib::TensorStorage::spin_count,
                                             n, n};
        const superbblas::PartitionItem<4> part{superbblas::Coor<4>{},
                                                value_size};
        const T *ptr = (const T *)val.op;
        superbblas::save<4, 8, T, T>(T{1}, &part, 1, "sqnN", {}, value_size,
                                     value_size, &ptr, &ctx, order, from, stoh,
                                     superbblas::FastToSlow);
      }
      superbblas::close_storage<8, T>(stoh);
    } catch (...) {
      superbblas::close_storage<8, T>(stoh);
      throw;
    }
    std::cout << "Copied " << keys.size() << " key-value pairs; Nt=" << Nt
              << '\n';
  }
} // namespace
#endif

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <new db> <old db>\n";
    return 1;
  }
  try {
#ifdef USE_SUPERBBLAS
    // Detect the input backend at runtime and preserve its storage family.
    Datalib::TensorStorage::StorageProp4 input(false);
    input.open({argv[2]});
    const bool is_filedb = !input.db_stores.empty();
    input.close();
    if (is_filedb)
      copyFileDB(argv[1], argv[2]);
    else
      copyDatabase(argv[1], argv[2]);
#else
    // StorageProp4 still reads FileDB when tensor support is compiled out.
    copyFileDB(argv[1], argv[2]);
#endif
  } catch (const std::string &e) {
    std::cerr << argv[0] << ": " << e << '\n';
    return 1;
  } catch (const std::exception &e) {
    std::cerr << argv[0] << ": " << e.what() << '\n';
    return 1;
  }
  return 0;
}
