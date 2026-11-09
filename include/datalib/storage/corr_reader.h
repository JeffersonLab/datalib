// -*- C++ -*-
/*! \file
 * \brief Auxiliary functions for superbblas key-value database
 */

#ifndef DATALIB_CORR_READER_H
#define DATALIB_CORR_READER_H
#include <datalib/codec/binary.h>
#include <datalib/model/arrays.h>

#include "AllConfStoreDB.h"
#include <datalib/storage/corr_storage.h>
#include <datalib/storage/corr_keys.h>

#include <datalib/storage/filedb.h>
#include <datalib/superb/superb_aux.h>
#include <datalib/codec/superb_options.h>

#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>

#include <memory>
#include <unordered_set>

namespace Datalib::Storage
{
  inline std::vector<std::vector<std::complex<double>>> unpackLegacy(const std::vector<SerialDBData<std::vector<std::complex<double>>>>& values) {std::vector<std::vector<std::complex<double>>> result;result.reserve(values.size());for(const auto& v:values)result.push_back(v.data());return result;}
  inline std::vector<SerialDBData<std::vector<std::complex<double>>>> packLegacy(const std::vector<std::vector<std::complex<double>>>& values) {std::vector<SerialDBData<std::vector<std::complex<double>>>> result;result.reserve(values.size());for(const auto& v:values)result.emplace_back(v);return result;}
  /// Tag type to indicate to NOT use the new storage format
  struct DontUseTheNewFormat {
  };

  /// Tag type to indicate to the use of the new storage format
  struct OneCanUseTheNewFormat {
  };

  //! AllConfStoreDB compatible class for storing correlation functions

  template <typename Key, typename IsNewFormatAllowed =
			    std::conditional<SB::has_schema<Key>::value, OneCanUseTheNewFormat,
					     DontUseTheNewFormat>::type>
  class StorageCorrFunc
  {
  public:
    using T = std::complex<double>; // storage number type

    std::string metadata;
    int num_configs;
    int NtCorr;

    using FileHash = Datalib::Storage::FileDBStore<Key, std::vector<std::complex<double>>>;
    struct Storage {
      std::string filename;
      std::shared_ptr<Datalib::Storage::KeyValueDB> db_simple;

      std::shared_ptr<FileHash> db_filehash;

      /// Key in use: 0 for KeyHadronSUNClassNPartNPtCorr_tGen<0>; and 1 for KeyHadronSUNClassNPartNPtCorr_tGen<1>
      int version;

      /// Default phasing (only for old format)
      XMLArray::Array<int> default_phasing;

      Storage() : version(-1)
      {
      }
    };

    std::vector<Storage> stos;

    StorageCorrFunc() : num_configs(0), NtCorr(0)
    {
    }

    ~StorageCorrFunc()
    {
      close();
    }

    /**
     * Open a file for reading
     * @param filename: file path
     */

    void open_for_reading(const std::string& filename, bool read_only_metadata = false)
    {
      open_for_reading(std::vector<std::string>{filename}, read_only_metadata);
    }

    /**
     * Open a bunch of files for reading
     * @param filenames: file paths
     */

    void open_for_reading(const std::vector<std::string>& filenames, bool read_only_metadata = false)
    {
      close();

      bool first_file = true;
      for (const auto& filename : filenames)
      {
	const auto [filename_without_suffix, phasing] =
	  Datalib::TensorStorage::detail::get_filename_and_phasing(filename);
	Storage s;
	s.filename = filename_without_suffix;

	// Try the new format
	bool done = false;
	if (std::is_same<IsNewFormatAllowed, OneCanUseTheNewFormat>::value || read_only_metadata)
	{
	  try
	  {
	    const auto& db_simple =
	      std::make_shared<Datalib::Storage::KeyValueDB>(filename_without_suffix, read_only_metadata);
	    if (first_file)
	    {
	      num_configs = db_simple->num_rows;
	      metadata = db_simple->metadata;
	      NtCorr = db_simple->num_columns;
	    }
	    else if (num_configs != db_simple->num_rows)
	    {
	      throw std::runtime_error(std::string("The file ") + filename_without_suffix +
				       std::string(" has different number of configurations"));
	    }
	    else if (NtCorr != db_simple->num_columns)
	    {
	      throw std::runtime_error(std::string("The file ") + filename_without_suffix +
				       std::string(" has different number of NtCorr"));
	    }
	    if constexpr (std::is_same<Key, Hadron::KeyHadronSUNClassNPartNPtCorr_t>::value)
	    {
	      const auto& ops = SB::getOptionsFromXML(metadata);
	      if (SB::getOption<std::string>(*ops, "DBMetaData/id") ==
		  std::string("hadronSUNClassNPartNPtCorr"))
	      {
		s.version = SB::getOption<int>(*ops, "DBMetaData/version", 3) < 4 ? 0 : 1;
	      }
	      if (!read_only_metadata && phasing.size() > 0)
	      {
		// Transform all keys
		std::unordered_map<std::string, std::size_t> new_entries_pos(
		  db_simple->entries_pos.size() * 2);
		for (const auto& it : db_simple->entries_pos) {
		  const auto& new_key_str = get_db_string_from_key(
		    set_default_phasing(get_key_from_db_string(it.first, s.version), phasing),
		    s.version);
		  new_entries_pos.insert({new_key_str, it.second});
		}
		std::swap(db_simple->entries_pos, new_entries_pos);
	      }
	    }

	    s.db_simple = db_simple;
	    done = true;
	  } catch (...)
	  {
	  }
	}

	// Try the old format
	if (!done)
	{
	  auto db_and_meta = open_db_reading_filehash(filename_without_suffix);
	  s.db_filehash = std::make_shared<FileHash>(std::move(db_and_meta.first));
	  s.default_phasing = (phasing.size() == 0 ? Datalib::Model::zero_mom() : phasing);
	  if (first_file)
	  {
	    num_configs = s.db_filehash->size();
	    metadata = db_and_meta.second;
	  }
	  else if (num_configs != s.db_filehash->size())
	  {
	    throw std::runtime_error(std::string("The file ") + filename +
				     std::string(" has different number of configurations"));
	  }
	  if (!read_only_metadata)
	  {
	    std::vector<Datalib::Storage::SerialDBData<std::vector<std::complex<double>>>> some_data;
	    if constexpr (std::is_same<Key, Hadron::KeyHadronSUNClassNPartNPtCorr_t>::value)
	    {
	      try
	      {
		// Try to read the devel-pdf version of the key
		std::vector<Datalib::Storage::SerialDBKey<Hadron::KeyHadronSUNClassNPartNPtCorr_tGen<
		  Hadron::KeyHadronSUNVersion::DevelPdf>>>
		  some_key;
		s.db_filehash->someKeyAndData(some_key, some_data);
		s.version = Hadron::KeyHadronSUNVersion::DevelPdf;
	      } catch (const Datalib::Binary::BinaryReadException&)
	      {
		// Try to read the devel version of the key
		std::vector<Datalib::Storage::SerialDBKey<
		  Hadron::KeyHadronSUNClassNPartNPtCorr_tGen<Hadron::KeyHadronSUNVersion::Devel>>>
		  some_key;
		s.db_filehash->someKeyAndData(some_key, some_data);
		s.version = Hadron::KeyHadronSUNVersion::Devel;
	      }
	    }
	    else
	    {
	      std::vector<Datalib::Storage::SerialDBKey<Key>> some_key;
	      s.db_filehash->someKeyAndData(some_key, some_data);
	    }
	    int this_NtCorr = some_data.size() == 0 ? 0 : some_data.at(0).data().size();
	    if (first_file)
	    {
	      NtCorr = this_NtCorr;
	    }
	    else if (NtCorr != this_NtCorr)
	    {
	      throw std::runtime_error(std::string("The file ") + filename +
				       std::string(" has different number of NtCorr"));
	    }
	  }
	}

	if (read_only_metadata)
	  return;
	first_file = false;
	stos.push_back(s);
      }
    }

    /**
     * Open a file for writing
     * @param filename: file paths
     * @param num_configs: number of configurations
     *
     * @return 0 on success, -1 on failure
     *
     */

    void open_for_writing(
      const std::string& filename, int num_configs, const std::string& metadata, int NtCorr,
      bool use_new_format = std::is_same<IsNewFormatAllowed, OneCanUseTheNewFormat>::value)
    {
      close();

      Storage s;
      s.filename = filename;
      if (!use_new_format)
      {
	s.db_filehash =
	  std::make_shared<FileHash>(open_db_writing_filehash(filename, num_configs, metadata));
      }
      else if constexpr (!std::is_same<IsNewFormatAllowed, OneCanUseTheNewFormat>::value)
      {
	throw std::runtime_error("Disabled the use of the new format");
      }
      stos.push_back(s);
      this->metadata = metadata;
      this->num_configs = num_configs;
      this->NtCorr = NtCorr;
    }

    /**
     * Close the database
     */

    void close()
    {
      stos.clear();
    }

    /**
     * Return all keys
     */

    std::vector<Key> keys() const
    {
      std::vector<Key> keys;
      for (const auto& s : stos)
      {
	if (s.db_simple)
	{
	  const auto from = keys.size();
	  keys.resize(keys.size() + s.db_simple->entries_pos.size());
	  for (const auto& it : s.db_simple->entries_pos)
	    keys[from + it.second] = get_key_from_db_string(it.first, s.version);
	}
	else if (s.db_filehash)
	{
	  for_each_key_version(s.version, [&]<typename SdbKey>() {
	    std::vector<Datalib::Storage::SerialDBKey<SdbKey>> ks;
	    s.db_filehash->keys(ks);
	    keys.reserve(keys.size() + ks.size());
	    for (const auto& k : ks)
	    {
	      keys.push_back(set_default_phasing(k.key(), s.default_phasing));
	    }
	  });
	}
      }
      return keys;
    }

    /**
     * Return all key-value pairs
     */

    std::pair<std::vector<Key>, std::vector<std::vector<std::vector<std::complex<double>>>>>
    keys_and_data() const
    {
      std::vector<Key> keys;
      std::vector<std::vector<std::vector<std::complex<double>>>> vals;
      for (const auto& s : stos)
      {
	if (s.db_simple)
	{
	  const auto from = keys.size();
	  const auto entries = s.db_simple->entries_pos.size();
	  keys.resize(keys.size() + entries);
	  for (const auto& it : s.db_simple->entries_pos)
	    keys[from + it.second] = get_key_from_db_string(it.first, s.version);
	  vals.resize(keys.size());
	  for (std::size_t i = 0; i < entries; ++i)
	    s.db_simple->read_index(i, vals[from + i]);
	}
	else if (s.db_filehash)
	{
	  for_each_key_version(s.version, [&]<typename SdbKey>() {
	    std::vector<std::vector<Datalib::Storage::SerialDBData<std::vector<std::complex<double>>>>> d;
	    std::vector<Datalib::Storage::SerialDBKey<Key>> bin_keys;
	    s.db_filehash->keysAndData(bin_keys, d);
	    keys.reserve(keys.size() + bin_keys.size());
	    for (const auto& bin_key : bin_keys)
	    {
	      keys.push_back(set_default_phasing(bin_key.key(), s.default_phasing));
	    }
	    vals.reserve(vals.size() + d.size());
	    for (const auto& val : d)
	      vals.push_back(unpackLegacy(val));
	  });
	}
      }
      return {keys, vals};
    }

    /**
     * Return if a key is in the collection
     * @param key given key
     * @return true if the key is there, false otherwise
     */

    bool exist(const Key& key) const
    {
      for (const auto& s : stos)
      {
	if (s.db_simple)
	{
	  std::string str_key;
	  try
	  {
	    str_key = get_db_string_from_key(key, s.version);
	  } catch (...)
	  {
	    continue;
	  }
	  if (s.db_simple->entries_pos.count(str_key) > 0)
	    return true;
	}
	else if (s.db_filehash)
	{
	  if (has_phasing(key, s.default_phasing))
	  {
	    bool does_exists = false;
	    for_each_key_version(s.version, [&]<typename SdbKey>() {
	      if (s.db_filehash->exist(Datalib::Storage::SerialDBKey<SdbKey>(key)))
		does_exists = true;
	    });
	    if (does_exists)
	      return true;
	  }
	}
      }
      return false;
    }

    /**
     * Get data for a given key
     * @param key given key
     * @param data (out) value associated to the key
     * @return 0 on success, otherwise the key not found
     */

    int get(const Key& key, std::vector<std::vector<std::complex<double>>>& val) const
    {
      for (const auto& s : stos)
      {
	if (s.db_simple)
	{
	  std::string str_key;
	  try
	  {
	    str_key = get_db_string_from_key(key, s.version);
	  } catch (...)
	  {
	    continue;
	  }
	  if (s.db_simple->read(str_key, val))
	    return 0;
	}
	else if (s.db_filehash)
	{
	  if (!has_phasing(key, s.default_phasing))
	    continue;
	  bool does_exists = false;
	  for_each_key_version(s.version, [&]<typename SdbKey>() {
	    std::vector<Datalib::Storage::SerialDBData<std::vector<std::complex<double>>>> d;
	    if (s.db_filehash->get(Datalib::Storage::SerialDBKey<SdbKey>(key), d) == 0)
	    {
	      val = unpackLegacy(d);
	      does_exists = true;
	    }
	  });
	  if (does_exists)
	    return 0;
	}
      }
      return -1;
    }

    /**
     * Get all key-value pairs
     * @param key user supplied key
     * @param data after the call data will be populated
     * @return 0 on success, otherwise the key not found
     */

    std::pair<std::vector<std::string>, std::vector<std::vector<std::vector<std::complex<double>>>>>
    string_keys_and_data() const
    {
      std::vector<std::string> keys;
      std::vector<std::vector<std::vector<std::complex<double>>>> vals;
      for (const auto& s : stos)
      {
	if (s.db_simple)
	{
	  const auto from = keys.size();
	  const auto entries = s.db_simple->entries_pos.size();
	  keys.resize(keys.size() + entries);
	  for (const auto& it : s.db_simple->entries_pos)
	    keys[from + it.second] = get_string_from_db_string(it.first, s.version);
	  vals.resize(keys.size());
	  for (std::size_t i = 0; i < entries; ++i)
	    s.db_simple->read_index(i, vals[from + i]);
	}
	else if (s.db_filehash)
	{
	  for_each_key_version(s.version, [&]<typename SdbKey>() {
	    std::vector<std::vector<Datalib::Storage::SerialDBData<std::vector<std::complex<double>>>>> d;
	    std::vector<Datalib::Storage::SerialDBKey<SdbKey>> bin_keys;
	    s.db_filehash->keysAndData(bin_keys, d);
	    keys.reserve(keys.size() + bin_keys.size());
	    for (const auto& bin_key : bin_keys)
	    {
	      keys.push_back(
		get_db_string_from_key(set_default_phasing(bin_key.key(), s.default_phasing)));
	    }
	    vals.reserve(keys.size());
	    for (const auto& val : d)
	      vals.push_back(unpackLegacy(val));
	  });
	}
      }
      return {keys, vals};
    }

    /**
     * Insert a list of keys
     * @param keys given keys
     */

    void insert_keys(const std::vector<Key>& keys)
    {
      if (stos.size() > 1)
	throw std::runtime_error("insert_keys: invalid operation");

      if (stos.at(0).db_filehash)
	return;

      // Get the universal representation of the keys
      std::vector<std::string> keys_str;
      keys_str.reserve(keys.size());
      for (const auto& key : keys)
	keys_str.push_back(get_db_string_from_key(key));

      insert_string_keys_and_dictionary(keys_str, get_dictionary_from_keys(keys));
    }

    /**
     * Insert a list of keys
     * @param keys given keys in string representation
     */

    void insert_string_keys(const std::vector<std::string>& keys)
    {
      if (stos.size() > 1)
	throw std::runtime_error("insert_string_keys: invalid operation");

      if (stos.at(0).db_filehash)
	return;

      insert_string_keys_and_dictionary(keys, get_dictionary_from_string_keys(keys));
    }

    /**
     * Insert data for a given key
     * @param key given key
     * @param val list of rows to be associated to the key
     * @param first_row index of the first row to write
     * @return 0 on success, otherwise error
     */

    int insert(const Key& key, const std::vector<std::vector<std::complex<double>>>& val,
	       unsigned int first_row = 0)
    {
      if (stos.size() != 1)
	throw std::runtime_error("insert: invalid operation");

      if (stos.at(0).db_simple)
      {
	stos.at(0).db_simple->write(get_db_string_from_key(key), val, first_row);
	return 0;
      }

      if (!stos.at(0).db_filehash)
	return -1;
      if (first_row != 0)
	return -1;
      return stos.at(0).db_filehash->insert(Datalib::Storage::SerialDBKey<Key>(key),
					    packLegacy(val));
    }

    /**
     * Insert data for a given key
     * @param key string representation of the key
     * @param val list of rows to be associated to the key
     * @param first_row index of the first row to write
     * @return 0 on success, otherwise error
     */

    int insert_from_string_key(const std::string& key,
                       const std::vector<std::vector<std::complex<double>>>& val,
                       unsigned int first_row = 0)
    {
      if (stos.size() != 1)
	throw std::runtime_error("insert_from_string_key: invalid operation");

      if (stos.at(0).db_simple)
      {
        stos.at(0).db_simple->write(key, val, first_row);
        return 0;
      }
      else if (stos.at(0).db_filehash)
      {
        if (first_row != 0)
          return -1;
	return stos.at(0).db_filehash->insert(Datalib::Storage::SerialDBKey<Key>(get_key_from_db_string(key)),
					      packLegacy(val));
      }
      return -1;
    }

  private:

    /**
     * Insert a list of keys
     * @param keys user supplied key
     * @return 0 on success, otherwise the key not found
     */

    void insert_string_keys_and_dictionary(const std::vector<std::string>& keys,
					   const std::vector<std::string>& dictionary)
    {
      if (stos.at(0).db_filehash)
	return;

      stos.at(0).db_simple = std::make_shared<Datalib::Storage::KeyValueDB>(stos.at(0).filename, metadata, dictionary,
							  keys, num_configs, NtCorr);
    }

    static std::pair<FileHash, std::string> open_db_reading_filehash(const std::string& filename)
    {
      return Datalib::Storage::openFileDBForReading<Key, std::vector<std::complex<double>>>(filename);
    }

    static FileHash open_db_writing_filehash(const std::string& filename, int num_configs,
                                             const std::string& metadata)
    {
      return Datalib::Storage::openFileDBForWriting<Key, std::vector<std::complex<double>>>(
          filename, num_configs, metadata);
    }

    static std::string get_db_string_from_key(const Key& key, int version = -1)
    {
      if constexpr (std::is_same<IsNewFormatAllowed, OneCanUseTheNewFormat>::value)
      {
	if constexpr (std::is_same<Key, Hadron::KeyHadronSUNClassNPartNPtCorr_t>::value)
	{
	  if (version == 0)
	    return SB::getXML(Hadron::KeyHadronSUNClassNPartNPtCorr_tGen<0>(key), "key");
	}
	return SB::getXML(key, "key");
      }
      else
      {
	throw std::runtime_error("Unsupported operation: disabled the use of the new format");
      }
    }

    static Key get_key_from_db_string(const std::string& s, int version = -1)
    {
      if constexpr (std::is_same<IsNewFormatAllowed, OneCanUseTheNewFormat>::value)
      {
	if constexpr (std::is_same<Key, Hadron::KeyHadronSUNClassNPartNPtCorr_t>::value)
	{
	  if (version == 0)
	    return SB::getOptionFast<Hadron::KeyHadronSUNClassNPartNPtCorr_tGen<0>>(s, "key");
	}
	return SB::getOptionFast<Key>(s, "key");
      }
      else
      {
	throw std::runtime_error("Unsupported operation: disabled the use of the new format");
      }
    }

    static std::string get_string_from_db_string(const std::string& str_key, int version)
    {
      if constexpr (std::is_same<IsNewFormatAllowed, OneCanUseTheNewFormat>::value)
      {
	if constexpr (std::is_same<Key, Hadron::KeyHadronSUNClassNPartNPtCorr_t>::value)
	{
	  if (version == 0) {
	    return SB::getXML(get_key_from_db_string(str_key, version), "key");
	  }
	}
      }
      return str_key;
    }

    static std::vector<std::string> get_dictionary_from_keys(const std::vector<Key>& keys)
    {
      if constexpr (std::is_same<IsNewFormatAllowed, OneCanUseTheNewFormat>::value)
      {
	// Prepare the dictionary and avoid repeating entries
	std::unordered_set<std::string> dictionary_set;
	dictionary_set.reserve(keys.size() * 3);
	std::vector<std::string> dictionary;
	dictionary.reserve(keys.size() * 3);

	// Insert the given keys
	for (const auto& key : keys)
	{
	  const auto& dict_entries = Datalib::Storage::CorrKeyDetail::detail::get_xml_fragments(key, "key");
	  for (const auto& dict_entry : dict_entries)
	  {
	    const auto& it_inserted = dictionary_set.insert(dict_entry);
	    if (it_inserted.second)
	      dictionary.push_back(std::move(dict_entry));
	  }
	}

	return dictionary;
      }
      else
      {
	return {};
      }
    }

    std::vector<std::string>
    get_dictionary_from_string_keys(const std::vector<std::string>& keys) const
    {
      if constexpr (std::is_same<IsNewFormatAllowed, OneCanUseTheNewFormat>::value)
      {
        // Prepare the dictionary and avoid repeating entries
        std::unordered_set<std::string> dictionary_set;
        dictionary_set.reserve(keys.size() * 3);
        std::vector<std::string> dictionary;
        dictionary.reserve(keys.size() * 3);

        // Insert the given keys
        for (const auto& key : keys)
        {
	  const auto& dict_entries = Datalib::Storage::CorrKeyDetail::detail::get_xml_fragments(get_key_from_db_string(key), "key");
	  for (const auto& dict_entry : dict_entries)
          {
            const auto& it_inserted = dictionary_set.insert(dict_entry);
            if (it_inserted.second)
              dictionary.push_back(std::move(dict_entry));
          }
        }

        return dictionary;
      }
      else
      {
        return {};
      }
    }

    /// Execute the lambda with the version of the key given as input
    /// version: version of the key
    /// fun: function of the form []<Key>() { ... }

    template <typename F>
    static void for_each_key_version(int version, F fun)
    {
      if constexpr (std::is_same<Key, Hadron::KeyHadronSUNClassNPartNPtCorr_t>::value)
      {
	if ((std::size_t)version == Hadron::KeyHadronSUNVersion::Devel)
	{
	  using K =
	    typename Hadron::KeyHadronSUNClassNPartNPtCorr_tGen<Hadron::KeyHadronSUNVersion::Devel>;
	  fun.template operator()<K>();
	}
	else if ((std::size_t)version == Hadron::KeyHadronSUNVersion::DevelPdf)
	{
	  using K = typename Hadron::KeyHadronSUNClassNPartNPtCorr_tGen<
	    Hadron::KeyHadronSUNVersion::DevelPdf>;
	  fun.template operator()<K>();
	}
	else
	{
	  throw std::runtime_error("Unsupported version number");
	}
      }
      else
      {
	fun.template operator()<Key>();
      }
    }

    /// Return the key with a given phasing
    /// key: key to set phasing
    /// default_phasing: phasing to set

    template <typename SdbKey>
    static Key set_default_phasing(const SdbKey& key, const XMLArray::Array<int>& default_phasing)
    {
      if constexpr (std::is_same<Key, Hadron::KeyHadronSUNClassNPartNPtCorr_t>::value)
      {
	Key new_key = key;
	new_key.set_phasing(default_phasing);
	return new_key;
      }
      else
      {
	return key;
      }
    }

    /// Return whether the key can have the given phasing
    /// key: key the probe the phasing
    /// default_phasing: phasing to check

    static bool has_phasing(const Key& key, const XMLArray::Array<int>& default_phasing)
    {
      if constexpr (std::is_same<Key, Hadron::KeyHadronSUNClassNPartNPtCorr_t>::value)
      {
	Key single_phasing_key = key;
	single_phasing_key.set_phasing(default_phasing);
	return key == single_phasing_key;
      }
      else
      {
	return true;
      }
    }
  };
} // namespace Datalib::Storage

#endif
