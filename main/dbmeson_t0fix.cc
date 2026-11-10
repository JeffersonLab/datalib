/*! \file
 * \brief Copy meson elementals, wrapping time slices into the lattice.
 *
 * Usage: dbmeson_t0fix <new db> <old db>
 * Each t_slice becomes (old_time + 2*Nt) % Nt, where Nt is the lattice
 * extent along decay_dir in the input metadata. Values are unchanged.
 * The output must not already exist; wrapped key collisions are errors.
 *
 * StorageMeson is a reader interface, not a single on-disk format:
 *   - Without USE_SUPERBBLAS: FileDB input and FileDB output only.
 *   - With USE_SUPERBBLAS: FileDB or superbblas tensor input; output uses
 *     the same storage family as the input (no automatic conversion).
 * Thus FileDB files such as meson.sdb do not require superbblas support.
 * The filename extension does not select the storage format.
 *
 * FileDB records retain their displacement and momentum fields. Their values
 * and all metadata are copied byte-for-byte; only the leading time
 * field of each key changes. The superbblas path preserves the
 * seven-dimensional tensor order, displacements, momenta, phasings, and
 * derivative settings.
 */
#include <algorithm>
#include <datalib/codec/binary.h>
#include <datalib/storage/dbtype.h>
#include <datalib/storage/filedb.h>
#include <datalib/superb/meson_superb.h>
#include <filesystem>
#include <set>
#include <stdexcept>

using namespace Hadron;

namespace {
  // Keep the complete serialized records, including any optional key fields.
  struct RawKey : Datalib::Storage::SerialDBKey<KeyMesonElementalOperator_t> {
    std::string bytes;
    void readObject(const std::string &input) { bytes = input; }
    void writeObject(std::string &output) const { output = bytes; }
  };
  struct RawValue
      : Datalib::Storage::SerialDBData<ValMesonElementalOperator_t> {
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
    // Use exactly the FileDB backend used by StorageMeson, retaining each meson
    // record.
    Datalib::TensorStorage::StorageMeson storage(false);
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
      // Both old and current meson key layouts start with t_slice.
      Datalib::Binary::BinaryBufferReader reader(key.bytes);
      int t_slice;
      read(reader, t_slice);
      Datalib::Binary::BinaryBufferWriter writer;
      write(writer, wrapTime(t_slice, Nt));
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
namespace {
  // Tensor-format input is opened through StorageMeson, as in dbutil.cc.
  // Keep its dictionaries and tensor order; only the time coordinate changes.
  void copyTensor(const std::string &output, const std::string &input) {
    using K = KeyMesonElementalOperator_t;
    using V = ValMesonElementalOperator_t;
    using T = Datalib::TensorStorage::StorageMeson::T;
    constexpr std::size_t N = Datalib::TensorStorage::StorageMeson::N;
    if (std::filesystem::exists(output))
      throw std::runtime_error("output already exists: " + output);
    Datalib::TensorStorage::StorageMeson dbin(false);
    if (dbin.open({input}) != 0 || dbin.sb_stores.size() != 1 ||
        !dbin.db_stores.empty())
      throw std::runtime_error("expected one meson tensor input");
    const auto &store = dbin.sb_stores.front();
    const std::string meta = getMetaData(input);
    const auto layout = getDBLayoutFromMetaData(meta);
    if (layout.decay_dir < 0 || layout.decay_dir >= layout.latt_size.size())
      throw std::runtime_error("invalid decay_dir");
    const int Nt = layout.latt_size[layout.decay_dir];
    if (Nt <= 0)
      throw std::runtime_error("invalid lattice time extent");
    const int n = store.dim.at('i');
    if (n <= 0 || store.dim.at('j') != n)
      throw std::runtime_error("invalid meson matrix dimensions");
    auto dims = store.dim;
    dims.at('t') = Nt;
    const auto dim =
        Datalib::TensorStorage::detail::kvcoors<N>(store.order, dims);
    const auto size = Datalib::TensorStorage::detail::kvcoors<N>(
        store.order, {{'i', n}, {'j', n}}, 1);
    std::vector<K> keys;
    dbin.keys(keys);
    std::vector<superbblas::Coor<N>> destinations;
    std::set<superbblas::Coor<N>> unique;
    for (const auto &old_key : keys) {
      K key = old_key;
      key.t_slice = wrapTime(old_key.t_slice, Nt);
      const int d =
          std::find(store.disps.begin(), store.disps.end(), key.displacement) -
          store.disps.begin();
      const int m = std::find(store.moms.begin(), store.moms.end(),
                              Datalib::TensorStorage::detail::toCoor(key.mom)) -
                    store.moms.begin();
      const int h = std::find(store.phasings.begin(), store.phasings.end(),
                              Datalib::TensorStorage::detail::toCoor(
                                  key.phasing_source)) -
                    store.phasings.begin();
      // StorageMeson stores the negative of the key's sink phasing.
      const int H =
          std::find(
              store.phasings.begin(), store.phasings.end(),
              Datalib::TensorStorage::detail::negate(
                  Datalib::TensorStorage::detail::toCoor(key.phasing_sink))) -
          store.phasings.begin();
      if (d == store.disps.size() || m == store.moms.size() ||
          h == store.phasings.size() || H == store.phasings.size())
        throw std::runtime_error("key not present in tensor dictionaries");
      const auto from = Datalib::TensorStorage::detail::kvcoors<N>(
          store.order,
          {{'t', key.t_slice}, {'d', d}, {'m', m}, {'h', h}, {'H', H}});
      if (!unique.insert(from).second)
        throw std::runtime_error(
            "wrapping produces duplicate keys; refusing to overwrite values");
      destinations.push_back(from);
    }
    superbblas::Storage_handle stoh;
    superbblas::create_storage<N, T>(
        dim, superbblas::FastToSlow, output.c_str(), meta.c_str(), meta.size(),
        superbblas::checksum_type::BlockChecksum, &stoh);
    try {
      const auto ctx = superbblas::createCpuContext();
      const superbblas::Coor<2> value_size{n, n};
      const superbblas::PartitionItem<2> part{superbblas::Coor<2>{},
                                              value_size};
      for (std::size_t i = 0; i < keys.size(); ++i) {
        V value;
        if (dbin.get(keys[i], value) != 0)
          throw std::runtime_error("cannot read meson value");
        if (value.op.nrows() != n || value.op.ncols() != n)
          throw std::runtime_error("inconsistent meson matrix dimensions");
        const auto &from = destinations[i];
        const superbblas::PartitionItem<N> block{from, size};
        superbblas::append_blocks<N, N, T>(&block, 1, store.order.c_str(), {},
                                           dim, dim, store.order.c_str(), {},
                                           stoh, superbblas::FastToSlow);
        const T *ptr = (const T *)value.op;
        // Match the ij memory order used by StorageMeson::get_sb.
        superbblas::save<2, N, T, T>(
            T{1}, &part, 1, "ij", {}, value_size, value_size, &ptr, &ctx,
            store.order.c_str(), from, stoh, superbblas::FastToSlow);
      }
    } catch (...) {
      superbblas::close_storage<N, T>(stoh);
      throw;
    }
    superbblas::close_storage<N, T>(stoh);
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
    // Preserve the input storage family; the extension does not select it.
    Datalib::TensorStorage::StorageMeson input(false);
    input.open({argv[2]});
    const bool is_filedb = !input.db_stores.empty();
    input.close();
    if (is_filedb)
      copyFileDB(argv[1], argv[2]);
    else
      copyTensor(argv[1], argv[2]);
#else
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
