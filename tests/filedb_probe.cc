#include <datalib/storage/filedb.h>
using Datalib::Storage::SerialDBKey;
using Datalib::Storage::SerialDBData;
using Datalib::Storage::openFileDBForReading;
using Datalib::Storage::openFileDBForWriting;
namespace Codec = Datalib::Binary;

#include <complex>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace fixture
{
struct Key
{
  int id = 0;
  int displacement = -1;
  bool operator==(const Key&) const = default;
};

void read(Codec::BinaryReader& input, Key& key)
{
  Codec::read(input, key.id);
  if (input.getReadingMode() == 1)
    Codec::read(input, key.displacement);
  else if (input.getReadingMode() != 2)
    throw std::runtime_error("Unexpected FILEDB key mode");
}

void write(Codec::BinaryWriter& output, const Key& key)
{
  Codec::write(output, key.id);
  Codec::write(output, key.displacement);
}

using Value = std::vector<std::complex<double>>;
const Key first{3, 7};
const Key second{11, -2};
const Value samples[] = {{{1.5, -0.25}, {-3, 4}}, {{5, 0}, {0.125, -2}},
                         {{-1, 2}, {0.5, 0}}, {{3, 5}, {-0.25, -1}}};
const std::string metadata = "<Fixture>FILEDB reference</Fixture>";

void require(bool ok, const char* message)
{
  if (!ok)
    throw std::runtime_error(message);
}

void writeDB(const std::string& path)
{
  auto db = openFileDBForWriting<Key, Value>(path, 2, metadata);
  require(db.insert(SerialDBKey<Key>(first), {samples[0], samples[1]}) == 0,
          "Cannot insert first key");
  require(db.insert(SerialDBKey<Key>(second), {samples[2], samples[3]}) == 0,
          "Cannot insert second key");
  require(db.close() == 0, "Cannot close database");
}

void readDB(const std::string& path)
{
  auto [db, actual_metadata] = openFileDBForReading<Key, Value>(path);
  require(actual_metadata == metadata && db.size() == 2, "Metadata or configuration count changed");
  std::vector<SerialDBKey<Key>> keys;
  db.keys(keys);
  require(keys.size() == 2, "Key count changed");
  for (int i = 0; i < 2; ++i)
  {
    std::vector<SerialDBData<Value>> values;
    require(db.get(SerialDBKey<Key>(i == 0 ? first : second), values) == 0 &&
            values.size() == 2, "Configuration read failed");
    require(values[0].data() == samples[2 * i] &&
            values[1].data() == samples[2 * i + 1], "Payload changed");
  }
  require(db.close() == 0, "Cannot close reader");
}

std::string load(const std::string& path)
{
  std::ifstream file(path, std::ios::binary);
  require(bool(file), "Cannot open fixture");
  return {std::istreambuf_iterator<char>(file), {}};
}

void save(const std::string& path, const std::string& bytes)
{
  std::ofstream file(path, std::ios::binary);
  file.write(bytes.data(), bytes.size());
  require(bool(file), "Cannot write fixture");
}

void checkBytes(const std::string& directory, bool emit)
{
  SerialDBKey<Key> key(first);
  SerialDBData<Value> data(samples[0]);
  require(key.serialID() == 456 && data.serialID() == 123, "Serial identifiers changed");
  std::string key_bytes;
  std::string value_bytes;
  key.writeObject(key_bytes);
  data.writeObject(value_bytes);
  Codec::BinaryBufferWriter legacy;
  Codec::write(legacy, first.id);
  if (emit)
  {
    save(directory + "/filedb-key.bin", key_bytes);
    save(directory + "/filedb-value.bin", value_bytes);
    save(directory + "/filedb-legacy-key.bin", legacy.str());
  }
  require(key_bytes == load(directory + "/filedb-key.bin"), "Key wire bytes changed");
  require(value_bytes == load(directory + "/filedb-value.bin"), "Value wire bytes changed");
  require(legacy.str() == load(directory + "/filedb-legacy-key.bin"), "Legacy key fixture changed");
  key.readObject(load(directory + "/filedb-key.bin"));
  require(key.key() == first, "Mode-1 key decode changed");
  key.readObject(load(directory + "/filedb-legacy-key.bin"));
  require(key.key() == Key{first.id, -1}, "Mode-2 fallback changed");
  data.readObject(load(directory + "/filedb-value.bin"));
  require(data.data() == samples[0], "Checksum-protected payload changed");
  const auto without_checksum = value_bytes.substr(0, value_bytes.size() - 4);
  data.readObject(without_checksum);
  require(data.data() == samples[0], "Checksum-free legacy value changed");
  for (std::size_t missing = 1; missing < 4; ++missing)
  {
    bool rejected = false;
    try
    {
      data.readObject(value_bytes.substr(0, value_bytes.size() - missing));
    }
    catch (const Codec::BinaryReadException&)
    {
      rejected = true;
    }
    require(rejected, "A partial checksum was accepted");
  }
  bool rejected = false;
  try
  {
    data.readObject(without_checksum.substr(0, without_checksum.size() - 1));
  }
  catch (const Codec::BinaryReadException&)
  {
    rejected = true;
  }
  require(rejected, "A truncated payload was accepted");
}
}

int main(int argc, char** argv)
{
  using namespace fixture;
  require(argc >= 3, "Pass an operation and path");
  const std::string mode = argv[1];
  if (mode == "emit")
  {
    checkBytes(argv[2], true);
    return 0;
  }
  if (mode == "write")
  {
    writeDB(argv[2]);
    return 0;
  }
  if (mode == "read")
  {
    readDB(argv[2]);
    return 0;
  }
  if (mode == "bad-checksum")
  {
    auto bytes = load(std::string(argv[2]) + "/filedb-value.bin");
    bytes.back() ^= 1;
    SerialDBData<Value> data;
    data.readObject(bytes);
    return 0;
  }
  require(mode == "check" && argc == 4, "Pass fixtures and a new database path");
  checkBytes(argv[2], false);
  writeDB(argv[3]);
  readDB(argv[3]);
  return 0;
}
