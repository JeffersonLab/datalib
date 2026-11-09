// -*- C++ -*-
/*!
 * \brief Get the type of the db
 */

#ifndef __dbtype_h__
#define __dbtype_h__

#include <string>
#include <vector>
#include <datalib/io/serial_xml.h>

namespace Hadron
{
  namespace Layout
  {
    struct Layout_t
    {
      std::string        dbtype;      /*!< Type of db */
      std::vector<int>   latt_size;   /*!< Total lattice size*/
      int                decay_dir;   /*!< The decay direction within the lattice */
    };
  }

  //! Read a layout
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, Layout::Layout_t& layout);

  //! Write a layout
//  void write(XMLWriter& xml, const std::string& path, const Layout::Layout_t& layout);

  //! Get the metadata
  std::string getMetaData(const std::string& dbase);

  //! Get layout and type
  Layout::Layout_t getDBLayoutFromMetaData(const std::string& meta_data);

  // Get layout and type from dbfile
  Layout::Layout_t getDBLayout(const std::string& dbfile);

  //! Read the dbtype from the dbfile
  std::string getDBType(const std::string& dbfile);

  //! Get lattice size
  std::vector<int> getLattSize(const std::string& dbfile);

  //! Get decay dir
  int getDecayDir(const std::string& dbfile);

  //! Get time extent
  int getTimeExtent(const std::string& dbfile);

} // namespace Hadron

#include <datalib/superb/superb_aux.h>
#include <datalib/storage/corr_reader.h>
#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>
#include <sstream>
namespace Hadron
{
  using namespace FILEDB;

  // Get the metadata
  inline std::string getMetaData(const std::string& dbase)
  {
    // Ignore possible suffix in filename
    const auto& [filename, default_phasing] = Datalib::TensorStorage::detail::get_filename_and_phasing(dbase);

#ifdef USE_SUPERBBLAS
    // Try to open superbblas file
    try {
      std::vector<char> metadatav;
      std::vector<superbblas::IndexType> dimv;
      superbblas::values_datatype values_dtype;
      superbblas::read_storage_header(filename.c_str(), superbblas::FastToSlow, values_dtype,
				      metadatav, dimv);
      return std::string(metadatav.begin(), metadatav.end());
    } catch (...) {
    }
#endif

    // Open DB solely to get the user data
    Datalib::Storage::StorageCorrFunc<KeyHadronSUNClassNPartNPtCorr_t> database;
    database.open_for_reading(filename, true /* only read metadata */);
    return database.metadata;
  }


  // Read a layout
  inline void read(Datalib::Serial::XMLReader& xml, const std::string& path, Layout::Layout_t& layout)
  {
    Datalib::Serial::XMLReader inputtop(xml, path);

    read(inputtop, "/DBMetaData/id", layout.dbtype);
    read(inputtop, "/DBMetaData/lattSize", layout.latt_size);
    read(inputtop, "/DBMetaData/decay_dir", layout.decay_dir);

    if (layout.latt_size.size() == 0)
    {
      std::cerr << __func__ << ": layout wrong: latt_size is 0\n";
      exit(1);
    }

    if ((layout.decay_dir < 0) || (layout.decay_dir >= layout.latt_size.size()))
    {
      std::cerr << __func__ << ": layout messed up decay_dir=";
    }
  }


  // Write a layout
  inline void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const Layout::Layout_t& layout)
  {
    push(xml, path);

    write(xml, "id", layout.dbtype);
    write(xml, "lattSize", layout.latt_size);
    write(xml, "decay_dir", layout.decay_dir);

    pop(xml);
  }


  // Get layout and type
  inline Layout::Layout_t getDBLayoutFromMetaData(const std::string& meta_data)
  {
    std::istringstream  xml_s(meta_data);
    Datalib::Serial::XMLReader  xml_meta(xml_s);

    Layout::Layout_t layout;
    read(xml_meta, "/DBMetaData", layout);

    return layout;
  }


  // Get layout and type
  inline Layout::Layout_t getDBLayout(const std::string& dbfile)
  {
    return getDBLayoutFromMetaData(getMetaData(dbfile));
  }


  //! Read the dbtype from the metadata
  inline std::string getDBType(const std::string& dbfile)
  {
    Layout::Layout_t layout(getDBLayout(dbfile));

    return layout.dbtype;
  }


  // Get lattice size
  std::vector<int> getLattSize(const std::string& dbfile)
  {
    Layout::Layout_t layout(getDBLayout(dbfile));

    return layout.latt_size;
  }


  // Get decay dir
  inline int getDecayDir(const std::string& dbfile)
  {
    Layout::Layout_t layout(getDBLayout(dbfile));

    return layout.decay_dir;
  }


  // Get time extent
  inline int getTimeExtent(const std::string& dbfile)
  {
    Layout::Layout_t layout(getDBLayout(dbfile));

    return layout.latt_size[layout.decay_dir];
  }

}

#endif
