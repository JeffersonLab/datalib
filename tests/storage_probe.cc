#include <datalib/storage/corr_storage.h>

#include <cerrno>
#include <complex>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using Datalib::Storage::KeyValueDB;

namespace
{
using Matrix = std::vector<std::vector<std::complex<double>>>;

const std::string metadata = "<Fixture><version>4</version></Fixture>";
const std::vector<std::string> tokens = {"p", "ion:a", "xray:b"};
const std::vector<std::string> keys = {"pion:a", "pxray:b"};
const Matrix first = {{{1, -2}, {-3, 4}, {5, 0}},
                      {{-6, 7}, {8, -9}, {0, 10}}};
const Matrix second = {{{0.125, -0.25}, {0.5, 0.75}, {0, 0}},
                       {{-1.25, 1.5}, {2, 2.5}, {3.5, -4}}};

void require(bool condition, const char* message)
{
  if (!condition)
    throw std::runtime_error(message);
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

void emit(const std::string& path,
          const std::vector<std::string>& dictionary = tokens,
          const std::vector<std::string>& keyset = keys)
{
  KeyValueDB output(path, metadata, dictionary, keyset, 2, 3);
  output.write(keyset[1], second);
  output.write(keyset[0], {first[0]});
  output.write(keyset[0], {first[1]}, 1);
}

void check(const std::string& path,
           const std::vector<std::string>& dictionary = tokens,
           const std::vector<std::string>& keyset = keys)
{
  KeyValueDB header(path, true);
  require(header.metadata == metadata, "Metadata changed");
  require(header.num_rows == 2 && header.num_columns == 3, "Shape changed");
  require(header.f == nullptr && header.entries_pos.empty(), "Metadata-only access changed");

  KeyValueDB input(path);
  require(input.entries_pos.size() == keyset.size(), "Key count changed");
  require(input.dictionary == dictionary, "Dictionary changed");
  Matrix value;
  require(input.read(keyset[0], value) && value == first, "First matrix changed");
  require(input.read(keyset[1], value) && value == second, "Second matrix changed");
  require(!input.read("absent", value), "Unknown key was found");
}

template<class Operation>
void requiresFailure(Operation operation)
{
  try
  {
    operation();
  }
  catch (const std::runtime_error&)
  {
    return;
  }
  throw std::runtime_error("Invalid storage was accepted");
}
}

int main(int argc, char** argv)
{
  require(argc == 4, "Pass both endian fixtures and an output directory");
  const std::string little = argv[1];
  const std::string big = argv[2];
  const std::string output = argv[3];
  check(little);
  check(big);
  emit(output + "/roundtrip.bin");
  check(output + "/roundtrip.bin");
  const auto written = load(output + "/roundtrip.bin");
  require(written == load(little) || written == load(big), "Writer bytes changed");

  const std::vector<std::string> mixed_tokens = {"pion:", "a", "b"};
  const std::vector<std::string> mixed_keys = {"pion:a", "pion:b"};
  emit(output + "/mixed-tokens.bin", mixed_tokens, mixed_keys);
  check(output + "/mixed-tokens.bin", mixed_tokens, mixed_keys);

  Datalib::Storage::detail::CompressedDictionary dictionary(tokens);
  requiresFailure([&] { dictionary.get_indices("pi"); });

  auto bytes = load(little);
  bytes.at(20) ^= 1;
  save(output + "/bad-header.bin", bytes);
  requiresFailure([&] { KeyValueDB bad(output + "/bad-header.bin"); });

  bytes = load(little);
  KeyValueDB input(little);
  bytes.at(input.values_base) ^= 1;
  save(output + "/bad-payload.bin", bytes);
  requiresFailure([&] {
    KeyValueDB bad(output + "/bad-payload.bin");
    Matrix value;
    bad.read(keys[0], value);
  });
  return 0;
}
