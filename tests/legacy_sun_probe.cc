#include <datalib/legacy/hadron_convert_corr_util.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
std::string bytes(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in)
    throw std::runtime_error("Missing fixture: " + path);
  return {std::istreambuf_iterator<char>(in), {}};
}
template <class T> std::string encode(const T &value) {
  Datalib::Binary::BinaryBufferWriter out;
  write(out, value);
  return out.str();
}
template <class T> T check(const std::string &stem) {
  T value{};
  Datalib::Binary::BinaryBufferReader in(bytes(stem + ".bin"));
  read(in, value);
  if (encode(value) != bytes(stem + ".bin"))
    throw std::runtime_error("Binary codec changed: " + stem);
  std::istringstream stream(bytes(stem + ".xml"));
  Datalib::Serial::XMLReader xml(stream);
  T from_xml{};
  read(xml, "/Key", from_xml);
  if (encode(from_xml) != encode(value))
    throw std::runtime_error("XML contents changed: " + stem);
  Datalib::Serial::XMLBufferWriter writer;
  write(writer, "Key", value);
  if (writer.str() != bytes(stem + ".xml"))
    throw std::runtime_error("XML codec changed: " + stem);
  return value;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  const std::string dir = argv[1];
  const auto sun =
      check<Hadron::KeyHadronSUNNPartNPtCorr_t>(dir + "/legacy-sun");
  const auto su2 =
      check<Hadron::KeyHadronNPartNPtCorr_t>(dir + "/su2-populated");
  const auto converted =
      check<Hadron::KeyHadronSUNClassNPartNPtCorr_t>(dir + "/converted");
  if (sun.npoint.size() != 2 || sun.npoint[1].irrep.op.CGs.size() != 1 ||
      sun.npoint[1].irrep.op.ops.size() != 2)
    throw std::runtime_error("Empty legacy fixture");
  if (encode(Hadron::convertHadronNPart(su2)) != encode(converted))
    throw std::runtime_error("Legacy SU(2) conversion changed");
}
