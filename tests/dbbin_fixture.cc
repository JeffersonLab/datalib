// Synthetic software fixture: independent standard-complex FILEDB codec.
#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/storage/filedb.h>

#include <complex>
#include <iomanip>
#include <numeric>
#include <stdexcept>

using Key = Datalib::Storage::SerialDBKey<FF::KeyHadron1PtCorr_t>;
using Value = Datalib::Storage::SerialDBData<std::vector<std::complex<double>>>;
using Database = FILEDB::AllConfStoreDB<Key, Value>;

void create(const std::string &path, int count) {
  std::vector<int> configurations(count);
  std::iota(configurations.begin(), configurations.end(), 0);
  Database db(configurations);
  const std::string metadata =
      "<DBMetaData><id>hadron1Pt</id><lattSize>4 4 4 16</lattSize>"
      "<decay_dir>3</decay_dir></DBMetaData>";
  db.setMaxUserInfoLen(metadata.size());
  if (db.open(path, O_RDWR | O_CREAT | O_EXCL, 0600) != 0)
    throw std::runtime_error("create failed");
  db.insertUserdata(metadata);
  for (int channel = 0; channel < 2; ++channel) {
    FF::KeyHadron1PtCorr_t model{};
    model.num_vecs = 2;
    model.creation_op = 1;
    model.name = "channel" + std::to_string(channel);
    model.smear = "fixture";
    model.mom.resize(3);
    model.mom = 0;
    model.mass = "fixture";
    model.ensemble = "software-test";
    Key key(model);
    std::vector<Value> values(count);
    for (int cfg = 0; cfg < count; ++cfg)
      for (int t = 0; t < 3; ++t)
        values[cfg].data().emplace_back(100 * channel + 10 * cfg + t,
                                        -50 * channel + 4 * cfg - 2 * t);
    if (db.insert(key, values) != 0)
      throw std::runtime_error("insert failed");
  }
}

void readback(const std::string &path) {
  Database db;
  if (db.open(path, O_RDONLY, 0400) != 0)
    throw std::runtime_error("read failed");
  std::string metadata;
  db.getUserdata(metadata);
  std::vector<Key> keys;
  db.keys(keys);
  std::cout << "RESULT:{\"configurations\":" << db.size()
            << ",\"metadata\":" << std::quoted(metadata) << ",\"key_bytes\":{";
  bool first_encoding = true;
  for (const auto &key : keys) {
    std::string encoded;
    key.writeObject(encoded);
    std::string hex;
    for (unsigned char byte : encoded) {
      hex += "0123456789abcdef"[byte >> 4];
      hex += "0123456789abcdef"[byte & 15];
    }
    if (!first_encoding)
      std::cout << ',';
    first_encoding = false;
    std::cout << std::quoted(key.key().name) << ':' << std::quoted(hex);
  }
  std::cout << "},\"keys\":{";
  bool first_key = true;
  for (auto &key : keys) {
    std::vector<Value> values;
    if (db.get(key, values) != 0)
      throw std::runtime_error("get failed");
    if (!first_key)
      std::cout << ',';
    first_key = false;
    std::cout << std::quoted(key.key().name) << ":[";
    for (std::size_t cfg = 0; cfg < values.size(); ++cfg) {
      if (cfg)
        std::cout << ',';
      std::cout << '[';
      const auto &row = values[cfg].data();
      for (std::size_t t = 0; t < row.size(); ++t) {
        if (t)
          std::cout << ',';
        std::cout << '[' << std::setprecision(17) << row[t].real() << ','
                  << row[t].imag() << ']';
      }
      std::cout << ']';
    }
    std::cout << ']';
  }
  std::cout << "}}\n";
}

int main(int argc, char **argv) {
  try {
    if (argc == 4 && std::string(argv[1]) == "create")
      create(argv[2], std::stoi(argv[3]));
    else if (argc == 3 && std::string(argv[1]) == "read")
      readback(argv[2]);
    else
      throw std::runtime_error(
          "usage: dbbin_fixture create FILE COUNT | read FILE");
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
