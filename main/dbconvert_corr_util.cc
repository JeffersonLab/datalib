#include <complex>
#include <vector>
/*! \file
 * \brief Convert correlation functions from SU(2) to SU(N) format
 */
#include <datalib/io/serial_xml.h>

#include <datalib/graph/naming.h>
#include <datalib/legacy/hadron_convert_corr_util.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>
#include <datalib/storage/corr_reader.h>

using namespace Hadron;

int main(int argc, char **argv) {
  // Parse the arguments
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <new dbasename> <old db>"
              << std::endl;
    return -1;
  }

  try {
    // Old db
    Datalib::Storage::StorageCorrFunc<KeyHadronNPartNPtCorr_t> dbold;
    dbold.open_for_reading(argv[2]);

    // Get keys, convert them, and write out the correlator
    const auto &old_keys = dbold.keys();

    // Get a new version of the metadata
    std::string new_meta;
    if (1) {
      std::string meta_data = dbold.metadata;

      // Bung into a reader
      std::istringstream xml_s(meta_data);
      Datalib::Serial::XMLReader xml_meta(xml_s);
      Datalib::Serial::XMLReader xml_in(xml_meta, "/DBMetaData");

      XMLArray::Array<int> latt_size;
      read(xml_in, "lattSize", latt_size);

      int decay_dir;
      read(xml_in, "decay_dir", decay_dir);

      Datalib::Serial::XMLBufferWriter xml;
      int version = 4;

      push(xml, "DBMetaData");
      write(xml, "id", "hadronSUNClassNPartNPtCorr");
      write(xml, "lattSize", latt_size);
      write(xml, "decay_dir", decay_dir);
      write(xml, "version", version);
      write(xml, "ensemble", old_keys.at(0).ensemble);
      pop(xml);

      new_meta = xml.str();
    }

    // New db
    Datalib::Storage::StorageCorrFunc<KeyHadronSUNClassNPartNPtCorr_t> corr_db;
    corr_db.open_for_writing(argv[1], dbold.num_configs, new_meta,
                             dbold.NtCorr);

    // Register the complete output key dictionary before writing any values.
    std::vector<Hadron::KeyHadronSUNClassNPartNPtCorr_t> converted_keys;
    converted_keys.reserve(old_keys.size());
    for (const auto &key : old_keys)
      converted_keys.push_back(Hadron::convertHadronNPart(key));
    corr_db.insert_keys(converted_keys);

    // Get keys, convert them, and write out the correlator
    for (const auto key : old_keys) {
      // Get the old value
      std::vector<std::vector<std::complex<double>>> val;
      int ret;

      // std::cout << "key= " << key->key() << "\n";

      if ((ret = dbold.get(key, val)) != 0) {
        std::cerr << __func__ << ": key not found\n";
        exit(1);
      }

      // Convert the key
      KeyHadronSUNClassNPartNPtCorr_t new_key = convertHadronNPart(key);

      // Insert the new key
      if ((ret = corr_db.insert(new_key, val)) != 0) {
        std::cerr << __func__ << ": error inserting key\n";
        exit(1);
      }
    }

    // close it
    corr_db.close();
  } catch (const std::string &e) {
    std::cerr << argv[0] << ": error message: " << e << std::endl;
    exit(1);
  }

  return 0;
}
