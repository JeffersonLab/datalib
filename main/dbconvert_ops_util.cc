/*! \file
 * \brief Convert hadron ops from SU(2) to SU(N) format
 */

#include <datalib/graph/filenames.h>
#include <datalib/graph/naming.h>
#include <datalib/io/serial_xml.h>
#include <datalib/legacy/hadron_convert_corr_util.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>

// opsxml input structure
struct opsxml_t {
  std::string opnamekey; // operator name
  Hadron::KeyHadronNPartIrrepOp_t op;
};

void read(Datalib::Serial::XMLReader &xml, const std::string &path,
          opsxml_t &param) {
  Datalib::Serial::XMLReader paramtop(xml, path);
  read(paramtop, "OpNameKey", param.opnamekey);
  read(paramtop, "Irrep", param.op);
}

using namespace Hadron;

int main(int argc, char **argv) {
  // Parse the arguments
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0]
              << " <new opslist> <new opsxml> <old opsxml1> .. <old opsxmlN>"
              << std::endl;
    return -1;
  }

  try {
    // Get keys, convert them
    std::map<std::string, KeyHadronSUNClassNPartIrrepOp_t> ops_map;

    // Read old ops xmls
    for (int n = 3; n < argc; ++n) {
      std::vector<opsxml_t> opsxml;

      std::cout << "Read old ops = " << argv[n] << std::endl;

      // Bung into a reader
      Datalib::Serial::XMLReader xml_mm(argv[n]);
      Datalib::Serial::XMLReader xml_in(xml_mm, "/");

      read(xml_in, "OpsList", opsxml);

      for (auto key = opsxml.begin(); key != opsxml.end(); ++key) {
        KeyHadronSUNClassNPartIrrepOp_t op_xml = convertHadronNPart(key->op);
        std::string op_key = ensemFileName(op_xml);

        ops_map.insert(std::make_pair(op_key, op_xml));
      }
    }

    // Write out new ops xml
    std::cout << "Write new ops" << std::endl;
    Datalib::Serial::XMLFileWriter new_ops_xml(argv[2]);

    write(new_ops_xml, "OpsList", ops_map);

    // close it
    new_ops_xml.close();

    // Write out new ops list
    std::cout << "Write new list" << std::endl;
    std::ofstream new_ops_list(argv[1]);

    for (auto p = ops_map.begin(); p != ops_map.end(); ++p) {
      std::string tmp = p->first;

      std::string mom_type;
      std::size_t pos = tmp.rfind("_");
      if (pos != std::string::npos) {
        mom_type = tmp.substr(pos + 1);
        tmp = tmp.substr(0, pos - 1);
      } else {
        std::cerr << "Could not parse mom_type for op= " << tmp << std::endl;
        exit(1);
      }

      // Sadly, this is not great

      // Get the irrep for this op
      std::string ir =
          removeIrrepLG(removeHelicity(Hadron::getSingleHadronOpIrrep(tmp)));

      // Remove any possible embedding info
      pos = ir.rfind(",");
      if (pos != std::string::npos) {
        ir = ir.substr(0, pos);
      }

      // Generate in form   100_A1P
      std::string f = mom_type + "_" + ir;

      new_ops_list << f << " " << p->first << "\n";
    }

  } catch (const std::string &e) {
    std::cerr << argv[0] << ": error message: " << e << std::endl;
    exit(1);
  }

  return 0;
}
