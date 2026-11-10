// Synthetic software fixtures for legacy tool arithmetic and codec comparison.
#include <complex>
#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/legacy/hadron_2pt_corr.h>
#include <datalib/legacy/hadron_3pt_corr.h>
#include <datalib/legacy/hadron_convert_corr_util.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>
#include <datalib/storage/corr_reader.h>
#include <datalib/storage/dbtype.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <numeric>

using Row = std::vector<std::complex<double>>;
using SU2 = Hadron::KeyHadronNPartNPtCorr_t;
using SUN = Hadron::KeyHadronSUNNPartNPtCorr_t;
XMLArray::Array<int> zeroMomentum() {
  XMLArray::Array<int> value(3);
  value = 0;
  return value;
}
Hadron::KeyParticleOp_t particle(const std::string &name) {
  return {name, "fixture", zeroMomentum()};
}
SU2 su2(int particles) {
  SU2 key{};
  key.ensemble = "software-test";
  for (int point = 0; point < 2; ++point) {
    SU2::NPoint_t n{};
    n.t_slice = point ? 1 : -2;
    n.irrep.creation_op = point;
    n.irrep.smearedP = true;
    n.irrep.row = 1;
    n.irrep.mom = zeroMomentum();
    n.irrep.phasing = zeroMomentum();
    for (int p = 0; p < particles; ++p)
      n.irrep.op.ops.push_back(particle("pion_test_J0_A1"));
    for (int p = 1; p < particles; ++p) {
      Hadron::KeyHadronNPartIrrepOp_t::CGPair_t cg{};
      cg.left = p == 1 ? "1" : "12";
      cg.right = std::to_string(p + 1);
      cg.target.slot = p == 1 ? "12" : "123";
      cg.target.irrep = "A1";
      cg.target.embed = 1;
      cg.target.twoI = 2;
      cg.target.mom_type = zeroMomentum();
      n.irrep.op.CGs.push_back(cg);
    }
    key.npoint.push_back(n);
  }
  return key;
}
SUN sun(int left, int right, int mask) {
  SUN key{};
  for (int point = 0; point < 2; ++point) {
    SUN::NPoint_t n{};
    n.t_slice = point ? 1 : -2;
    n.irrep.creation_op = point;
    n.irrep.smearedP = true;
    n.irrep.flavor = {1, 3, 1};
    n.irrep.irrep_mom = {1, zeroMomentum()};
    n.irrep.op.ops.push_back(particle("Nucleon_test_J1o2_H1o2D2E"));
    n.irrep.op.ops.push_back(particle("pion_test_J0_H0D2A2"));
    Hadron::KeyHadronSUNNPartIrrepOp_t::CGPair_t cg{};
    cg.left = "1";
    cg.right = "2";
    cg.target.slot = "12";
    cg.target.F = "2,1";
    cg.target.irrep = ((mask & (1 << point)) ? "Hg," : "A1,") +
                      std::to_string(point ? right : left);
    cg.target.mom_type = zeroMomentum();
    n.irrep.op.CGs.push_back(cg);
    key.npoint.push_back(n);
  }
  return key;
}
std::string bytes(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in)
    throw std::runtime_error("Missing fixture: " + path);
  return {std::istreambuf_iterator<char>(in), {}};
}
template <class K> K decode(const std::string &path) {
  K key{};
  Datalib::Binary::BinaryBufferReader reader(bytes(path));
  read(reader, key);
  return key;
}
template <class K> void freeze(const std::string &path, const K &key) {
  Datalib::Binary::BinaryBufferWriter binary;
  write(binary, key);
  std::ofstream(path + ".bin", std::ios::binary) << binary.str();
  Datalib::Serial::XMLBufferWriter xml;
  write(xml, "Key", key);
  std::ofstream(path + ".xml") << xml.str();
}
template <class K>
void create(const std::string &path, const std::string &type,
            const std::vector<K> &keys, int count) {
  std::vector<int> configurations(count);
  std::iota(configurations.begin(), configurations.end(), 0);
  Datalib::Storage::FileDBStore<K, Row> db(configurations);
  const std::string metadata =
      "<DBMetaData><id>" + type +
      "</id><lattSize>4 4 4 "
      "16</lattSize><decay_dir>3</decay_dir><ensemble>software-test</"
      "ensemble></DBMetaData>";
  db.setMaxUserInfoLen(metadata.size());
  if (db.open(path, O_RDWR | O_CREAT | O_EXCL, 0600))
    throw std::runtime_error("Create failed");
  db.insertUserdata(metadata);
  int channel = 0;
  for (const auto &key : keys) {
    std::vector<Datalib::Storage::SerialDBData<Row>> values(count);
    for (int cfg = 0; cfg < count; ++cfg)
      for (int t = 0; t < 3; ++t)
        values[cfg].data().emplace_back(1.25 + channel + cfg * 0.5 + t * 0.25,
                                        -0.5 + channel * 0.25 - cfg * 0.125 +
                                            t * 0.5);
    if (db.insert(Datalib::Storage::SerialDBKey<K>(key), values))
      throw std::runtime_error("Insert failed");
    ++channel;
  }
}
std::string hex(const std::string &input) {
  std::string out;
  for (unsigned char c : input) {
    out += "0123456789abcdef"[c >> 4];
    out += "0123456789abcdef"[c & 15];
  }
  return out;
}
std::string jsonString(const std::string &input) {
  std::string out = "\"";
  for (unsigned char c : input) {
    if (c == '"' || c == '\\') {
      out += '\\';
      out += c;
    } else if (c < 32) {
      out += "\\u00";
      out += "0123456789abcdef"[c >> 4];
      out += "0123456789abcdef"[c & 15];
    } else
      out += c;
  }
  return out + '"';
}
template <class K> void dump(const std::string &path) {
  Datalib::Storage::StorageCorrFunc<K> db;
  db.open_for_reading(path);
  std::vector<std::string> keys;
  std::vector<std::vector<Row>> values;
  for (const auto &key : db.keys()) {
    std::string encoded;
    Datalib::Storage::SerialDBKey<K>(key).writeObject(encoded);
    std::vector<Row> rows;
    if (db.get(key, rows))
      throw std::runtime_error("Readback failed");
    keys.push_back(encoded);
    values.push_back(rows);
  }
  std::cout << "RESULT:{\"configurations\":" << db.num_configs
            << ",\"metadata\":" << jsonString(db.metadata) << ",\"records\":{";
  for (std::size_t k = 0; k < keys.size(); ++k) {
    if (k)
      std::cout << ',';
    std::cout << std::quoted(hex(keys[k])) << ":[";
    for (std::size_t c = 0; c < values[k].size(); ++c) {
      if (c)
        std::cout << ',';
      std::cout << '[';
      for (std::size_t t = 0; t < values[k][c].size(); ++t) {
        if (t)
          std::cout << ',';
        auto z = values[k][c][t];
        std::cout << '[' << std::setprecision(17) << z.real() << ',' << z.imag()
                  << ']';
      }
      std::cout << ']';
    }
    std::cout << ']';
  }
  std::cout << "}}\n";
}
int main(int argc, char **argv) {
  try {
    if (argc == 4 && std::string(argv[1]) == "create") {
      const std::string dir = argv[2], fixtures = argv[3];
      std::filesystem::create_directory(dir);
      auto a = decode<FF::KeyHadron1PtCorr_t>(fixtures + "/legacy-1pt.bin");
      auto a2 = a;
      a.ensemble = "one";
      a2.ensemble = "two";
      create(dir + "/one.sdb", "hadron1Pt", std::vector{a, a2}, 5);
      auto b = decode<FF::KeyHadron2PtCorr_t>(fixtures + "/legacy-2pt.bin");
      auto b2 = b;
      b.ensemble = "one";
      b2.ensemble = "two";
      create(dir + "/two.sdb", "hadron2Pt", std::vector{b, b2}, 5);
      auto c = decode<FF::KeyHadron3PtCorr_t>(fixtures + "/legacy-3pt.bin");
      auto c2 = c;
      c.ensemble = "one";
      c2.ensemble = "two";
      create(dir + "/three.sdb", "hadron3Pt", std::vector{c, c2}, 5);
      std::vector<SU2> su2keys{su2(1), su2(2), su2(3)};
      create(dir + "/su2.sdb", "hadronNPartNPtCorr", su2keys, 5);
      std::vector<SUN> sunkeys;
      for (int mask = 0; mask < 4; ++mask)
        for (int left = 1; left <= ((mask & 1) ? 2 : 1); ++left)
          for (int right = 1; right <= ((mask & 2) ? 2 : 1); ++right)
            sunkeys.push_back(sun(left, right, mask));
      create(dir + "/sun.sdb", "hadronSUNNPartNPtCorr", sunkeys, 5);
      create(dir + "/sun-single.sdb", "hadronSUNNPartNPtCorr", sunkeys, 1);
      freeze(dir + "/legacy-sun", sunkeys.back());
      freeze(dir + "/su2-populated", su2keys.back());
      freeze(dir + "/converted", Hadron::convertHadronNPart(su2keys.back()));
    } else if (argc == 3 && std::string(argv[1]) == "dump") {
      auto type = Hadron::getDBType(argv[2]);
      if (type == "hadron1Pt")
        dump<FF::KeyHadron1PtCorr_t>(argv[2]);
      else if (type == "hadron2Pt")
        dump<FF::KeyHadron2PtCorr_t>(argv[2]);
      else if (type == "hadron3Pt")
        dump<FF::KeyHadron3PtCorr_t>(argv[2]);
      else if (type == "hadronNPartNPtCorr")
        dump<SU2>(argv[2]);
      else if (type == "hadronSUNNPartNPtCorr")
        dump<SUN>(argv[2]);
      else if (type == "hadronSUNClassNPartNPtCorr")
        dump<Hadron::KeyHadronSUNClassNPartNPtCorr_t>(argv[2]);
      else
        throw std::runtime_error("Unknown type: " + type);
    } else
      throw std::runtime_error(
          "usage: tools_fixture create NEW_DIR LEGACY_FIXTURES | dump DB");
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
