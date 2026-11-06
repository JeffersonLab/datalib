// -*- C++ -*-
// $Id: key_val_db.h,v 2.3 2009/03/09 02:41:17 edwards Exp $
/*! \file
 * \brief Key and values for DB
 */

#ifndef DATALIB_STORAGE_FILEDB_H
#define DATALIB_STORAGE_FILEDB_H

#include "datalib/codec/binary.h"
#include "datalib/codec/byteorder.h"

#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <AllConfStoreDB.h>
#include "DBKey.h"
#include "DBData.h"

namespace Datalib::Storage
{
  using Binary::read;
  using Binary::write;

  //---------------------------------------------------------------------
  //! Serializable key harness
  /*! \ingroup ferm */
  template<typename K>
  class SerialDBKey : public FILEDB::DBKey
  {
  public:
    //! Default constructor
    SerialDBKey() {}

    //! Constructor from data
    SerialDBKey(const K& k) : key_(k) {}

    //! Setter
    K& key() {return key_;}

    //! Getter
    const K& key() const {return key_;}

    // Part of Serializable
    unsigned short serialID (void) const {return 456;}

    void writeObject (std::string& output) const {
      Binary::BinaryBufferWriter bin;
      write(bin, key());
      output = bin.str();
    }

    void readObject(const std::string& input)
    {
      // Try several modes
      // NOTE: avoid mode 0, that one reads `phasing' and no keys in filehash are stored with phasing
      for (int mode = 1; mode < Binary::BinaryBufferReader::max_mode; ++mode)
      {
	try
	{
	  Binary::BinaryBufferReader bin(input);
	  bin.getReadingMode() = mode;
	  K k;
	  read(bin, k);
	  key() = k;
	  return;
	} catch (...)
	{
	  if (mode == Binary::BinaryBufferReader::max_mode - 1)
	    throw;
	}
      }
    }

    // Part of DBKey
    int hasHashFunc (void) const {return 0;}
    int hasCompareFunc (void) const {return 0;}

    /**
     * Empty hash and compare functions. We are using default functions.
     */
    static unsigned int hash (const void*, unsigned int) {return 0;}
    static int compare (const FFDB_DBT*, const FFDB_DBT*) {return 0;}

  private:
    K  key_;
  };


  //---------------------------------------------------------------------
  //! Serializable value harness
  /*! \ingroup ferm */
  template<typename D>
  class SerialDBData : public FILEDB::DBData
  {
  public:
    //! Default constructor
    SerialDBData() {}

    //! Constructor from data
    SerialDBData(const D& d) : data_(d) {}

    //! Copy constructor
    SerialDBData(const SerialDBData& d) : data_(d.data_) {}

    //! Setter
    D& data() {return data_;}

    //! Getter
    const D& data() const {return data_;}

    // Part of Serializable
    unsigned short serialID (void) const {return 123;}

    void writeObject (std::string& output) const {
      Binary::BinaryBufferWriter bin;
      write(bin, data());
      Util::n_uint32_t chk(bin.getChecksum());
      write(bin, chk);
      output = bin.str();
    }

    void readObject (const std::string& input) {
      Binary::BinaryBufferReader bin(input);
      read(bin, data());
      // Some historical FILEDB values contain a complete payload without CRC.
      // A present trailer still requires all four checksum bytes.
      if (bin.atEnd())
        return;
      const Util::n_uint32_t expected = bin.getChecksum();
      Util::n_uint32_t stored;
      read(bin, stored);
      if (stored != expected)
      {
        std::cerr << __func__ << ": data checksum mismatch\n";
        std::exit(1);
      }
    }

  private:
    D  data_;
  };

  template<class Key, class Value>
  using FileDBStore = FILEDB::AllConfStoreDB<SerialDBKey<Key>, SerialDBData<Value>>;

  template<class Key, class Value>
  std::pair<FileDBStore<Key, Value>, std::string> openFileDBForReading(const std::string& filename)
  {
    FileDBStore<Key, Value> db;
    db.setCacheSizeMB(10 * 1024);
    if (db.open(filename, O_RDONLY, 0400) != 0)
	throw std::runtime_error(std::string("error opening database file for reading: ") +
				 filename);

    // Get the metadata
    std::string metadata;
    db.getUserdata(metadata);

    return {db, metadata};
  }

  template<class Key, class Value>
  FileDBStore<Key, Value> openFileDBForWriting(const std::string& filename, int num_configs,
                                               const std::string& metadata)
  {
    std::vector<int> configs(num_configs);
    for (std::size_t i = 0; i < configs.size(); ++i)
	configs[i] = (int)i;
    FileDBStore<Key, Value> db(configs);
    db.setCacheSizeMB(10 * 1024);
    db.setMaxUserInfoLen(metadata.size());
    if (db.open(filename, O_RDWR | O_TRUNC | O_CREAT, 0644) != 0)
	throw std::runtime_error(std::string("error opening database file for writing: ") +
				 filename);

    db.insertUserdata(metadata);

    return db;
  }

} // namespace Datalib::Storage

#endif
