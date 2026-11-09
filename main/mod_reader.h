// Read-only QDP++ MapObjectDisk inspection. No QDP/Chroma or lattice values.
#ifndef ADAT_DBUTIL_MOD_READER_H
#define ADAT_DBUTIL_MOD_READER_H

#include <datalib/codec/byteorder.h>
#include <datalib/io/serial_xml.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace MODInspect
{
inline const std::string magic = "XXXXQDPLazyDiskMapObjFileXXXX";

// QDP primitives and descriptor lengths are big endian. Positions are a
// single big-endian 128-bit item (including on little-endian writer hosts).
class Reader
{
  std::istream& in;
  uint64_t remaining;
public:
  Datalib::Util::n_uint32_t checksum = 0;
  Reader(std::istream& stream, uint64_t size) : in(stream), remaining(size) {}
  std::string bytes(uint64_t n)
  {
    if (n > remaining || n > uint64_t(std::numeric_limits<std::streamsize>::max()))
      throw std::runtime_error("MOD: truncated or invalid length");
    std::string s(n, '\0');
    if (n && !in.read(&s[0], n)) throw std::runtime_error("MOD: read failed");
    remaining -= n;
    checksum = Datalib::Util::crc32(checksum, s.data(), s.size());
    return s;
  }
  uint64_t number(unsigned n)
  {
    uint64_t v = 0;
    for (unsigned char c : bytes(n)) v = (v << 8) | c;
    return v;
  }
  int32_t integer() { return static_cast<int32_t>(number(4)); }
  std::string descriptor()
  {
    int32_t n = integer();
    if (n < 0) throw std::runtime_error("MOD: negative descriptor length");
    return bytes(n);
  }
  uint64_t position()
  {
    if (number(8) != 0) throw std::runtime_error("MOD: offset exceeds 64 bits");
    return number(8);
  }
  uint64_t left() const { return remaining; }
};

inline bool isMOD(const std::string& path)
{
  std::ifstream f(path, std::ios::binary);
  // Probe a fixed number of bytes, without trusting an arbitrary file's length.
  char prefix[33];
  if (!f.read(prefix, sizeof(prefix))) return false;
  return std::string(prefix, 4) == std::string("\0\0\0\35", 4) &&
         std::string(prefix + 4, 29) == magic;
}

struct File
{
  std::ifstream stream;
  uint64_t size;
  std::string metadata;
  uint64_t header_end;
  explicit File(const std::string& path) : stream(path, std::ios::binary)
  {
    if (!stream) throw std::runtime_error("MOD: cannot open " + path);
    stream.seekg(0, std::ios::end);
    auto end = stream.tellg();
    if (end < 0) throw std::runtime_error("MOD: cannot determine file size");
    size = static_cast<uint64_t>(end);
    stream.seekg(0);
    Reader r(stream, size);
    if (r.descriptor() != magic) throw std::runtime_error("MOD: wrong magic");
    if (r.number(4) != 1) throw std::runtime_error("MOD: unsupported version");
    metadata = r.descriptor();
    header_end = size - r.left();
  }
  std::vector<std::string> keys()
  {
    Reader header(stream, size - header_end);
    uint64_t index = header.position();
    if (index < header_end + 16 || index > size || size - index < 8)
      throw std::runtime_error("MOD: invalid index offset");
    stream.seekg(static_cast<std::streamoff>(index));
    Reader r(stream, size - index);
    uint64_t count = r.number(4);
    if (count > (r.left() - 4) / 20) throw std::runtime_error("MOD: invalid key count");
    std::vector<std::string> result;
    for (uint64_t i = 0; i < count; ++i)
    {
      result.push_back(r.descriptor());
      uint64_t pos = r.position();
      if (pos < header_end + 16 || pos >= index)
        throw std::runtime_error("MOD: invalid value offset");
    }
    auto expected = r.checksum;
    if (r.number(4) != expected) throw std::runtime_error("MOD: index checksum mismatch");
    return result;
  }
};

// adat's XML string writer emits raw text, so escape the mass label here.
inline std::string xmlText(const std::string& text)
{
  std::string out;
  for (char c : text) {
    if (c == '&') out += "&amp;";
    else if (c == '<') out += "&lt;";
    else if (c == '>') out += "&gt;";
    else out += c;
  }
  return out;
}

inline int run(int argc, char** argv)
{
  std::string command = argv[2];
  if (!((command == "meta" || command == "keys") && argc == 3) &&
      !(command == "keysxml" && argc == 4))
    throw std::runtime_error("MOD supports only: meta, keys, keysxml <output_file_xml>");
  File file(argv[1]);
  if (command == "meta") { std::cout << file.metadata << '\n'; return 0; }
  std::istringstream metadata(file.metadata);
  Datalib::Serial::XMLReader xml(metadata);
  std::string type;
  Datalib::Serial::read(xml, "/MODMetaData/id", type);
  bool eigen = type == "eigenVecsTimeSlice";
  bool prop = type == "propDistillation";
  if (!eigen && !prop) throw std::runtime_error("Unsupported MOD key type: " + type);
  auto keys = file.keys();
  // Decode and validate every key before producing output.
  struct Key { int t_source, t_slice, colorvec, spin_src, spin_snk; std::string mass; };
  std::vector<Key> decoded;
  for (const auto& raw : keys)
  {
    std::istringstream input(raw);
    Reader r(input, raw.size());
    Key k{};
    if (prop) k.t_source = r.integer();
    k.t_slice = r.integer(); k.colorvec = r.integer();
    if (prop) { k.spin_src = r.integer(); k.spin_snk = r.integer(); k.mass = r.descriptor(); }
    if (r.left()) throw std::runtime_error("MOD: unexpected bytes in key");
    decoded.push_back(k);
  }
  if (command == "keys")
  {
    for (const auto& k : decoded)
    {
      std::cout << (eigen ? "KeyTimeSliceColorVec_t:" : "KeyPropDistillation_t:")
                << " t_slice= " << k.t_slice;
      if (eigen) std::cout << " colorvec= " << k.colorvec;
      else std::cout << " t_source= " << k.t_source << " spin_snk= " << k.spin_snk
                     << " spin_src= " << k.spin_src << " colorvec_src= " << k.colorvec
                     << " mass= " << k.mass;
      std::cout << '\n';
    }
  }
  else
  {
    Datalib::Serial::XMLFileWriter out(argv[3]);
    Datalib::Serial::push(out, "Keys");
    for (const auto& k : decoded)
    {
      Datalib::Serial::push(out, "elem");
      if (prop) Datalib::Serial::write(out, "t_source", k.t_source);
      Datalib::Serial::write(out, "t_slice", k.t_slice);
      Datalib::Serial::write(out, eigen ? "colorvec" : "colorvec_src", k.colorvec);
      if (prop) {
        Datalib::Serial::write(out, "spin_src", k.spin_src);
        Datalib::Serial::write(out, "spin_snk", k.spin_snk);
        Datalib::Serial::write(out, "mass", xmlText(k.mass));
      }
      Datalib::Serial::pop(out);
    }
    Datalib::Serial::pop(out); out.close();
  }
  return 0;
}
}
#endif
