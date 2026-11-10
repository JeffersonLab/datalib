// Synthetic spin/time fixture for the standalone dbprop_diag_t0 utility.
#include <datalib/storage/filedb.h>
#include <datalib/superb/prop_elem_type.h>
#include <ConfDataStoreDB.h>
#include <cmath>
#include <iomanip>
#include <map>

using Key = Datalib::Storage::SerialDBKey<Hadron::KeyPropElementalOperator_t>;
using Value = Datalib::Storage::SerialDBData<Hadron::ValPropElementalOperator_t>;
using Store = FILEDB::ConfDataStoreDB<Key, Value>;
std::string hex(const std::string &bytes) {
  std::string result;
  for (unsigned char c : bytes) {
    result += "0123456789abcdef"[c >> 4];
    result += "0123456789abcdef"[c & 15];
  }
  return result;
}
void create(const std::string &file) {
  Store store;
  const std::string metadata = "<DBMetaData><id>propElemOp</id><lattSize>2 2 2 2</lattSize><decay_dir>3</decay_dir></DBMetaData>";
  store.setMaxUserInfoLen(metadata.size());
  if (store.open(file, O_RDWR | O_CREAT | O_EXCL, 0600))
    throw std::runtime_error("Create failed");
  store.insertUserdata(metadata);
  for (int sink = 0; sink < 2; ++sink)
    for (int source = 0; source < 2; ++source)
      for (int left = 0; left < 4; ++left)
        for (int right = 0; right < 4; ++right) {
          Hadron::KeyPropElementalOperator_t key{};
          key.t_slice = sink;
          key.t_source = source;
          key.spin_l = left;
          key.spin_r = right;
          key.mass_label = "test";
          Value value;
          value.data().mat.resize(2, 2);
          for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j)
              value.data().mat(i, j) = {2.0 + sink * 10 + source * 3 + left * 0.5 + right * 0.25 + i,
                                      -1.0 + left * 0.125 + right * 0.0625 + j};
          if (store.insert(Key(key), value))
            throw std::runtime_error("Insert failed");
        }
}
void dump(const std::string &file) {
  Store store;
  if (store.open(file, O_RDONLY, 0400))
    throw std::runtime_error("Read failed");
  std::string metadata;
  store.getUserdata(metadata);
  std::vector<Key> keys;
  store.keys(keys);
  std::map<std::string, std::string> all, diagonal;
  for (const auto &key : keys) {
    Value value;
    if (store.get(key, value))
      throw std::runtime_error("Get failed");
    if (value.data().mat.size1() != 2 || value.data().mat.size2() != 2)
      throw std::runtime_error("Matrix shape changed");
    for (int i = 0; i < 2; ++i)
      for (int j = 0; j < 2; ++j) {
        auto z = value.data().mat(i, j);
        if (!std::isfinite(z.real()) || !std::isfinite(z.imag()))
          throw std::runtime_error("Nonfinite matrix value");
      }
    std::string k, v;
    key.writeObject(k);
    value.writeObject(v);
    all.emplace(hex(k), hex(v));
    if (key.key().t_source == key.key().t_slice)
      diagonal.emplace(hex(k), hex(v));
  }
  auto emit = [](const auto &records) {
    bool first = true;
    std::cout << '{';
    for (const auto &[key, value] : records) {
      if (!first)
        std::cout << ',';
      first = false;
      std::cout << std::quoted(key) << ':' << std::quoted(value);
    }
    std::cout << '}';
  };
  std::cout << "RESULT:{\"metadata_hex\":" << std::quoted(hex(metadata)) << ",\"records\":";
  emit(all);
  std::cout << ",\"diagonal\":";
  emit(diagonal);
  std::cout << "}\n";
}
int main(int argc, char **argv) {
  try {
    if (argc != 3)
      return 2;
    if (std::string(argv[1]) == "create")
      create(argv[2]);
    else
      dump(argv[2]);
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
