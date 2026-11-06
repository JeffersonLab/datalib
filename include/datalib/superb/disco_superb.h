// -*- C++ -*-
/*! \file
 * \brief Disconnected loops superbblas database
 */

#ifndef __disco_superbblas_h__
#define __disco_superbblas_h__

#include "datalib/superb/types.h"
#include "ConfDataStoreDBSimple.h"
#include "datalib/superb/unsmeared_twoquark_disco.h"
#include "datalib/storage/filedb.h"
#include "datalib/superb/superb_aux.h"
#include <mutex>
#include <set>
#include <fcntl.h>
#include <unistd.h>

namespace Datalib::TensorStorage
{
  //! ConfDataStoreMultipleDB compatible class supporting legacy FILEDB, single-mass rank-4 and multi-mass rank-5 storage

  class StorageDisco
  {
  public:
    using T = std::complex<double>; // storage number type

#ifdef USE_SUPERBBLAS
    static constexpr std::size_t N = std::size_t{4}; ///< expected storage size

    struct Storage {
      superbblas::Storage_handle stoh;			  // storage handle
      std::string order;				  // storage order
      std::string mass_label;				  // mass label
      std::map<char, int> dim;				  // dimensions
      std::vector<std::vector<int>> disps;		  // displacements
      std::vector<std::array<int, 3>> moms;		  // momenta
      std::vector<std::string> mass_labels; // rank-5 mass dictionary
    };

    std::vector<Storage> sb_stores;
    superbblas::Context cpu_ctx;
#endif

    std::vector<FILEDB::ConfDataStoreDBSimple> db_stores;

    std::vector<std::string> meta;

    StorageDisco()
#ifdef USE_SUPERBBLAS
      : cpu_ctx(superbblas::createCpuContext())
#endif
    {
    }

    ~StorageDisco()
    {
      close();
    }

    /**
     * Open a list of files with disco loops
     * @param filenames: file paths 
     *
     * @return 0 on success, -1 on failure
     * 
     */

    int open(const std::vector<std::string>& filenames)
    {
      close();

      for (const auto& filename : filenames)
      {
#ifdef USE_SUPERBBLAS
	try
	{
	  const auto& db_and_meta = open_sb(filename);
	  sb_stores.push_back(db_and_meta.first);
	  meta.push_back(db_and_meta.second);
	} catch (...)
#endif
	{
	  const auto& db_and_meta = open_db(filename);
	  db_stores.push_back(db_and_meta.first);
	  meta.push_back(db_and_meta.second);
	}
      }

      return 0;
    }

    /**
     * Close the database
     *
     * @return 0 on success, -1 on failure
     * 
     */

    int close()
    {
#ifdef USE_SUPERBBLAS
      for (auto& it : sb_stores)
      {
	if (it.order.size() == 5) superbblas::close_storage<5, T>(it.stoh);
        else superbblas::close_storage<4, T>(it.stoh);
      }
      sb_stores.clear();
#endif
      db_stores.clear();
      meta.clear();
      return 0;
    }

    /**
     * Return all available disco keys regardless of the underneath file formats
     * @param keys: user supplied an empty vector which is populated
     * by keys after this call.
     */
    void keys(std::vector<Hadron::KeyTwoQuarkDiscoOperator_t>& keys) const
    {
#ifdef USE_SUPERBBLAS
      for (const auto& store : sb_stores)
	keys_sb(store, keys);
#endif

      for (const auto& store : db_stores)
	keys_db(store, keys);
    }

    /**
     * Get data for a given key
     * @param key user supplied key
     * @param data after the call data will be populated
     * @return 0 on success, otherwise the key not found
     */

    int get(const Hadron::KeyTwoQuarkDiscoOperator_t& key,
	    Hadron::ValTwoQuarkDiscoOperator_t& val) const
    {

#ifdef USE_SUPERBBLAS
      // Try to read from superbblas files
      for (const auto& store : sb_stores) {
	if (get_sb(store, key, val))
	{
	  return 0;
	}
      }
#endif

      // Try to read from filehash databases
      for (const auto& store : db_stores)
      {
	if (get_db(store, key, val))
	{
	  return 0;
	}
      }

      return -1;
    }

    /**
     * Create a file with disco loops
     * @param filename path and name of the file
     * @param latt_size four-vector lattice dimensions
     * @param decay_dir decay direction (usually 3)
     * @param probing_distance reduce the error for displacements at that distance
     * @param noise_vectors number of noise vectors
     * @param mass_label mass label
     * @param disps list of displacements
     * @param moms list of momenta
     * @return 0 on success, otherwise the key not found
     */

    int create(const std::string& filename, std::vector<int>& latt_size, int decay_dir,
	       int probing_distance, int probing_power, int noise_vectors,
	       const std::string& mass_label, const std::vector<std::vector<int>>& disps,
	       const std::vector<std::array<int, 3>>& moms)
    {
      close();

#ifdef USE_SUPERBBLAS
      try
      {
	const auto& db_and_meta = create_sb(filename, latt_size, decay_dir, probing_distance,
					    probing_power, noise_vectors, mass_label, disps, moms);
	sb_stores.push_back(db_and_meta.first);
	meta.push_back(db_and_meta.second);
	return 0;
      } catch (...)
      {
	return -1;
      }
#else
      throw std::runtime_error("Please compile with superb support to write disco loops");
#endif
    }

    /**
     * Insert data for a given key
     * @param key user supplied key
     * @param data after the call data will be populated
     * @return 0 on success, otherwise the key not found
     */

    int insert(const Hadron::KeyTwoQuarkDiscoOperator_t& key,
	       const Hadron::ValTwoQuarkDiscoOperator_t& val)
    {

#ifdef USE_SUPERBBLAS
      if (sb_stores.size() != 1)
	return -1;

      return (insert_sb(sb_stores.at(0), key, val) ? 0 : -1);
#else
      throw std::runtime_error("Please compile with superb support to write disco loops");
#endif
    }

    int create_mass_collection(const std::string& filename, const std::vector<int>& lattice,
        const std::vector<std::string>& masses, const std::vector<std::vector<int>>& disps,
        const std::vector<std::array<int,3>>& moms, const std::vector<std::string>& provenance = {}) {
#ifdef USE_SUPERBBLAS
      close();
      if (lattice.size()!=4 || lattice[3]<=0 || masses.empty() || disps.empty() || moms.empty())
        throw std::runtime_error("Invalid DISCO mass collection dimensions");
      auto unique = masses; std::sort(unique.begin(),unique.end());
      if (unique.front().empty() || std::adjacent_find(unique.begin(),unique.end())!=unique.end())
        throw std::runtime_error("Invalid DISCO mass dictionary");
      const std::string order="itdmf";
      const std::array<int,5> dim{16,lattice[3],int(disps.size()),int(moms.size()),int(masses.size())};
      XMLBufferWriter xml; push(xml,"DBMetaData");
      write(xml,"id",std::string("DiscoBlocks"));write(xml,"tensorOrder",order);
      write(xml,"mass_labels",masses);write(xml,"lattSize",lattice);write(xml,"decay_dir",3);
      write(xml,"displacements",disps);
      std::vector<std::vector<int>> mm;for(const auto&m:moms)mm.push_back({m[0],m[1],m[2]});
      write(xml,"moms",mm);
      // Preserve complete source documents without embedding XML declarations.
      std::vector<std::string> encoded;
      const char* hex="0123456789abcdef";
      for (const auto& source : provenance) {
        std::string text; text.reserve(source.size()*2);
        for (unsigned char c : source) { text.push_back(hex[c>>4]); text.push_back(hex[c&15]); }
        encoded.push_back(text);
      }
      write(xml,"source_metadata_hex",encoded);pop(xml);
      const auto metadata=xml.str();superbblas::Storage_handle handle;
      superbblas::create_storage<5,T>(dim,superbblas::FastToSlow,filename.c_str(),metadata.c_str(),metadata.size(),superbblas::checksum_type::BlockChecksum,&handle);
      sb_stores.push_back(Storage{handle,order,"",detail::to_kvdim(order,std::vector<int>(dim.begin(),dim.end())),disps,moms,masses});
      meta.push_back(metadata);return 0;
#else
      throw std::runtime_error("Compile with SUPERB support to write a DISCO collection");
#endif
    }

    // Direct producer output: only append to a collection created in this process.
    // Fresh-process reuse is rejected, so a new run cannot silently mix configurations.
    static void append_mass_time(const std::string& filename, const std::vector<int>& lattice,
        const std::vector<std::string>& masses,
        const std::vector<std::pair<Hadron::KeyTwoQuarkDiscoOperator_t, Hadron::ValTwoQuarkDiscoOperator_t>>& records,
        const std::string& contract, const std::string& mass_contract,
        const std::string& provenance) {
#ifdef USE_SUPERBBLAS
      using Pattern = std::set<std::pair<std::vector<int>,std::array<int,3>>>;
      struct Session {
        std::vector<int> lattice;
        std::vector<std::string> masses;
        Pattern pattern;
        std::string contract, metadata;
        std::map<std::string,std::string> mass_contracts;
        std::set<std::pair<std::string,int>> written;
      };
      static std::mutex mutex;
      static std::map<std::string,Session> sessions;
      std::lock_guard<std::mutex> lock(mutex);
      if (lattice.size()!=4 || lattice[3]<=0 || records.empty() || contract.empty() || mass_contract.empty())
        throw std::runtime_error("Invalid direct DISCO collection parameters");
      const auto& first=records.front().first;
      if (first.t_slice<0 || first.t_slice>=lattice[3] || std::find(masses.begin(),masses.end(),first.mass_label)==masses.end())
        throw std::runtime_error("Invalid direct DISCO mass/time");
      Pattern pattern;
      std::set<std::vector<int>> disps;
      std::set<std::array<int,3>> moms;
      for (const auto& item : records) {
        const auto& k=item.first;const auto& v=item.second;
        if (k.mass_label!=first.mass_label || k.t_slice!=first.t_slice || k.mom.size()!=3 || v.op.size()!=16)
          throw std::runtime_error("Direct DISCO packet must contain one complete mass/time");
        for (const auto& z:v.op) if (!std::isfinite(z.real()) || !std::isfinite(z.imag()))
          throw std::runtime_error("Nonfinite direct DISCO gamma vector");
        const std::array<int,3> mom{k.mom[0],k.mom[1],k.mom[2]};
        if (!pattern.insert({k.disp,mom}).second) throw std::runtime_error("Duplicate direct DISCO key");
        disps.insert(k.disp);moms.insert(mom);
      }
      auto session=sessions.find(filename);
      const auto point=std::make_pair(first.mass_label,first.t_slice);
      if (session!=sessions.end()) {
        const auto& known=session->second;
        if (known.lattice!=lattice || known.masses!=masses || known.pattern!=pattern || known.contract!=contract)
          throw std::runtime_error("Incompatible direct DISCO collection metadata");
        if (known.written.count(point)) throw std::runtime_error("Direct DISCO mass/time already written");
        auto action=known.mass_contracts.find(first.mass_label);
        if (action!=known.mass_contracts.end() && action->second!=mass_contract)
          throw std::runtime_error("Direct DISCO action changed for a mass label");
      }
      StorageDisco output;
      if (session==sessions.end()) {
        // Reserve atomically before create_storage opens the new, owned file.
        int fd=::open(filename.c_str(),O_WRONLY|O_CREAT|O_EXCL,0664);
        if (fd<0) throw std::runtime_error("Direct DISCO output exists or cannot be created: "+filename);
        ::close(fd);
        output.create_mass_collection(filename,lattice,masses,{disps.begin(),disps.end()},
                                      {moms.begin(),moms.end()},{contract,provenance});
      } else {
        output.open_mass_collection_for_update(filename);
        if (output.meta.at(0)!=session->second.metadata)
          throw std::runtime_error("Direct DISCO file metadata changed outside this run");
        superbblas::check_storage<5,T>(output.sb_stores.at(0).stoh);
      }
      for (const auto& item:records) {
        Hadron::ValTwoQuarkDiscoOperator_t previous;
        if (output.get(item.first,previous)==0) throw std::runtime_error("Refusing to overwrite direct DISCO data");
        if (output.insert(item.first,item.second)!=0) throw std::runtime_error("Cannot write direct DISCO block");
      }
      const auto metadata=output.meta.at(0);
      output.close(); // Flush the block checksums before the next measurement.
      if (session==sessions.end())
        session=sessions.emplace(filename,Session{lattice,masses,pattern,contract,metadata,{},{}}).first;
      session->second.mass_contracts.emplace(first.mass_label,mass_contract);
      session->second.written.insert(point);
#else
      throw std::runtime_error("Direct DISCO output requires SUPERB support");
#endif
    }

    int open_mass_collection_for_update(const std::string& filename) {
#ifdef USE_SUPERBBLAS
      close();
      std::vector<char> metadata;std::vector<superbblas::IndexType> dimensions;
      superbblas::values_datatype dtype;
      superbblas::read_storage_header(filename.c_str(),superbblas::FastToSlow,dtype,metadata,dimensions);
      if (dimensions.size()!=5) throw std::runtime_error("Direct DISCO append requires a mass-axis collection");
      const auto opened=open_sb(filename,true);sb_stores.push_back(opened.first);meta.push_back(opened.second);
      return 0;
#else
      throw std::runtime_error("Direct DISCO append requires SUPERB support");
#endif
    }

  private:
#ifdef USE_SUPERBBLAS
    static std::pair<Storage, std::string> open_sb(const std::string& filename, bool allow_writing = false)
    {
      detail::Tracker t_("disco open db");

      std::vector<char> metadatav;
      std::vector<superbblas::IndexType> dimv;
      superbblas::values_datatype values_dtype;
      superbblas::read_storage_header(filename.c_str(), superbblas::FastToSlow, values_dtype,
				      metadatav, dimv);

      // Check that storage tensor dimension and value type match template arguments
      if (dimv.size() != 4 && dimv.size() != 5)
	throw std::runtime_error(
	  "Unexpected storage tensor dimension for disco loops");
      if (superbblas::detail::get_values_datatype<T>() != values_dtype)
	throw std::runtime_error("Unexpected storage type for disco loops");

      //
      // Read options
      //
      const std::string meta(metadatav.begin(), metadatav.end());
      std::shared_ptr<SB::Options> ops = SB::getOptionsFromXML(meta);

      std::string dtype = SB::getOption<std::string>(*ops, "DBMetaData/id");
      if (dtype != std::string("DiscoBlocks"))
      {
	throw std::runtime_error("unsupported disco type in file: " + filename);
      }

      // Read order; letter meaning:
      /// i: Datalib::TensorStorage::spin_count*Datalib::TensorStorage::spin_count element
      /// t: time slice
      /// d: displacement
      /// m: momentum
      std::string order = SB::getOption<std::string>(*ops, "DBMetaData/tensorOrder");

      // Read mass label
      std::string mass_label;
      std::vector<std::string> mass_labels;
      if (dimv.size() == 4) mass_label = SB::getOption<std::string>(*ops, "DBMetaData/mass_label");
      else mass_labels = SB::getOption<std::vector<std::string>>(*ops, "DBMetaData/mass_labels");

      // Read derivatives
      std::vector<std::vector<int>> disps =
	SB::getOption<std::vector<std::vector<int>>>(*ops, "DBMetaData/displacements");

      // Read momenta
      std::vector<std::array<int, 3>> moms =
	SB::getOption<std::vector<std::array<int, 3>>>(*ops, "DBMetaData/moms");

      // Open storage
      superbblas::Storage_handle stoh;
      if (dimv.size() == 5) superbblas::open_storage<5, T>(filename.c_str(), allow_writing, &stoh);
      else superbblas::open_storage<4, T>(filename.c_str(), allow_writing, &stoh);

      // Create dictionary with the dimensions
      std::map<char, int> kvdim = detail::to_kvdim(order, dimv);

      return {Storage{stoh, order, mass_label, kvdim, disps, moms, mass_labels}, meta};
    }

    template <std::size_t Rank>
    void keys_sb_rank(const Storage& store, std::vector<Hadron::KeyTwoQuarkDiscoOperator_t>& keys) const
    {
      std::vector<superbblas::PartitionItem<Rank>> blocks;
      superbblas::get_blocks<Rank, Rank, T>(
	store.stoh, store.order.c_str(), store.order.c_str(), std::array<int, Rank>{{}},
	detail::kvcoors<Rank>(store.order, store.dim), blocks, superbblas::FastToSlow);
      Hadron::KeyTwoQuarkDiscoOperator_t key;
      key.mass_label = store.mass_label;
      int T = store.dim.at('t');
      int M = store.dim.at('m');
      for (const auto& b : blocks)
      {
	const auto kvfrom = detail::to_kvdim(store.order, b[0]);
	const auto kvsize = detail::to_kvdim(store.order, b[1]);
        for (int f = 0; f < (Rank == 5 ? kvsize.at('f') : 1); ++f) {
          key.mass_label = Rank == 5 ? store.mass_labels.at(kvfrom.at('f') + f) : store.mass_label;
	for (unsigned int d = 0; d < kvsize.at('d'); ++d)
	{
	  for (unsigned int m = 0; m < kvsize.at('m'); ++m)
	  {
	    for (unsigned int t = 0; t < kvsize.at('t'); ++t)
	    {
	      key.disp = store.disps.at(kvfrom.at('d') + d);
	      key.mom = detail::toXMLArray(store.moms.at((kvfrom.at('m') + m) % M));
	      key.t_slice = (kvfrom.at('t') + t) % T;
	      keys.push_back(key);
	    }
	  }
	}
        }
      }
    }
    void keys_sb(const Storage& store, std::vector<Hadron::KeyTwoQuarkDiscoOperator_t>& keys) const {
      if (store.order.size() == 5) keys_sb_rank<5>(store, keys);
      else keys_sb_rank<4>(store, keys);
    }


    template <std::size_t Rank>
    bool get_sb_rank(const Storage& store, const Hadron::KeyTwoQuarkDiscoOperator_t& key,
		Hadron::ValTwoQuarkDiscoOperator_t& val) const
    {
      detail::Tracker t_("disco get key-val");

      using namespace detail;

      int n = store.dim.at('i');
      if (n != Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count || key.t_slice < 0 || key.t_slice >= store.dim.at('t'))
	return false;

      int mass_idx = 0;
      if constexpr (Rank == 4) {
        if (store.mass_label != key.mass_label) return false;
      } else {
        const auto mass = std::find(store.mass_labels.begin(), store.mass_labels.end(), key.mass_label);
        if (mass == store.mass_labels.end()) return false;
        mass_idx = mass - store.mass_labels.begin();
      }

      auto it_disps = std::find(store.disps.begin(), store.disps.end(), std::vector<int>(key.disp));
      if (it_disps == store.disps.end())
	return false;
      int disp_idx = it_disps - store.disps.begin();

      auto it_moms = std::find(store.moms.begin(), store.moms.end(), toCoor(key.mom));
      if (it_moms == store.moms.end())
	return false;
      int mom_idx = it_moms - store.moms.begin();

      const char* val_order = "i";
      std::array<int, 1> val_from{{}};
      std::array<int, 1> val_size{{Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count}};
      std::array<std::array<int, 1>, 2> val_partition{{val_from, val_size}};
      val.op.assign(Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count, NaN<T>::get());
      T* val_ptr = val.op.data();

      auto store_from =
	kvcoors<Rank>(store.order, {{'d', disp_idx}, {'m', mom_idx}, {'t', key.t_slice}});
      if constexpr (Rank == 5) store_from.at(store.order.find('f')) = mass_idx;
      auto store_size = kvcoors<Rank>(store.order, {{'i', Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count}}, 1);
#  if SUPERBBLAS_VERSION < 2
      superbblas::load<Rank, 1, T, T>(T{1}, store.stoh, store.order.c_str(), store_from, store_size,
				   &val_partition, 1, val_order, val_from, &val_ptr, &cpu_ctx,
				   superbblas::FastToSlow, superbblas::Copy);
#  else
      superbblas::load<Rank, 1, T, T>(T{1}, store.stoh, store.order.c_str(), store_from, store_size,
				   &val_partition, 1, val_order, val_from, val_size, &val_ptr,
				   &cpu_ctx, superbblas::FastToSlow, superbblas::Copy);
#  endif

      // Missing/partial blocks must not masquerade as complete gamma vectors.
      for (const auto& z : val.op)
        if (!std::isfinite(z.real()) || !std::isfinite(z.imag())) return false;

      return true;
    }
    bool get_sb(const Storage& store, const Hadron::KeyTwoQuarkDiscoOperator_t& key,
                 Hadron::ValTwoQuarkDiscoOperator_t& val) const {
      return store.order.size() == 5 ? get_sb_rank<5>(store,key,val) : get_sb_rank<4>(store,key,val);
    }


    std::pair<Storage, std::string> create_sb(const std::string& filename,
					      std::vector<int>& latt_size, int decay_dir,
					      int probing_distance, int probing_power,
					      int noise_vectors, const std::string& mass_label,
					      const std::vector<std::vector<int>>& disps,
					      const std::vector<std::array<int, 3>>& moms) const
    {
      const std::string order = "itdm";
      const std::array<int, 4> dim{Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count, latt_size.at(3), (int)disps.size(), (int)moms.size()};

      // Create new meta
      std::string new_meta;
      {
	XMLBufferWriter file_xml;

	push(file_xml, "DBMetaData");
	{
	  write(file_xml, "id", "DiscoBlocks");
	  write(file_xml, "tensorOrder", order);
	  write(file_xml, "mass_label", mass_label);
	  write(file_xml, "lattSize", latt_size);
	  write(file_xml, "decay_dir", decay_dir);
	  push(file_xml, "Params");
	  {
	    write(file_xml, "mass_label", mass_label);
	    write(file_xml, "probing_distance", probing_distance);
	    write(file_xml, "probing_power", probing_power);
	    write(file_xml, "noise_vectors", noise_vectors);
	  }
	  pop(file_xml);
	  write(file_xml, "displacements", disps);
	  std::vector<std::vector<int>> moms0(moms.size());
	  std::transform(moms.begin(), moms.end(), moms0.begin(),
			 [](const auto& it) { return std::vector<int>(it.begin(), it.end()); });
	  write(file_xml, "moms", moms0);
	}
	pop(file_xml);

	new_meta = file_xml.str();
      }

      // Open storage
      superbblas::Storage_handle stoh;
      superbblas::create_storage<N, T>(dim, superbblas::FastToSlow, filename.c_str(),
				       new_meta.c_str(), new_meta.size(),
				       superbblas::checksum_type::BlockChecksum, &stoh);

      // Create dictionary with the dimensions
      std::map<char, int> kvdim = detail::to_kvdim(order, std::vector<int>(dim.begin(), dim.end()));

      return {Storage{stoh, order, mass_label, kvdim, disps, moms}, new_meta};
    }

    template <std::size_t Rank>
    bool insert_sb_rank(const Storage& store, const Hadron::KeyTwoQuarkDiscoOperator_t& key,
		   const Hadron::ValTwoQuarkDiscoOperator_t& val) const
    {
      using namespace detail;

      int n = store.dim.at('i');
      if (n == 0)
	return true;

      int mass_idx = 0;
      if constexpr (Rank == 4) {
        if (store.mass_label != key.mass_label) return false;
      } else {
        const auto mass = std::find(store.mass_labels.begin(), store.mass_labels.end(), key.mass_label);
        if (mass == store.mass_labels.end()) return false;
        mass_idx = mass - store.mass_labels.begin();
      }

      auto it_disps = std::find(store.disps.begin(), store.disps.end(), std::vector<int>(key.disp));
      if (it_disps == store.disps.end())
	return false;
      int disp_idx = it_disps - store.disps.begin();

      auto it_moms = std::find(store.moms.begin(), store.moms.end(), toCoor(key.mom));
      if (it_moms == store.moms.end())
	return false;
      int mom_idx = it_moms - store.moms.begin();

      const T* val_ptr = val.op.data();
      auto store_from =
	kvcoors<Rank>(store.order, {{'d', disp_idx}, {'m', mom_idx}, {'t', key.t_slice}});
      if constexpr (Rank == 5) store_from.at(store.order.find('f')) = mass_idx;
      auto store_size = kvcoors<Rank>(store.order, {{'i', Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count}}, 1);
      auto store_dim = kvcoors<Rank>(store.order, store.dim);
      const std::array<std::array<int, Rank>, 2> val_par{store_from, store_size};
      superbblas::append_blocks<Rank, Rank, T>(
	&val_par, 1, store.order.c_str(), std::array<int, Rank>{{}}, store_dim, store_dim,
	store.order.c_str(), std::array<int, Rank>{{}}, store.stoh, superbblas::FastToSlow);
      const std::array<std::array<int, 1>, 2> val_par_j{0, Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count};
      superbblas::save<1, Rank, T, T>(T{1}, &val_par_j, 1, "i", std::array<int, 1>{0},
				   std::array<int, 1>{Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count}, std::array<int, 1>{Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count},
				   (const T**)&val_ptr, &cpu_ctx, store.order.c_str(), store_from,
				   store.stoh, superbblas::FastToSlow);

      return true;
    }
    bool insert_sb(const Storage& store, const Hadron::KeyTwoQuarkDiscoOperator_t& key,
                 const Hadron::ValTwoQuarkDiscoOperator_t& val) const {
      return store.order.size() == 5 ? insert_sb_rank<5>(store,key,val) : insert_sb_rank<4>(store,key,val);
    }

#endif

    static std::pair<FILEDB::ConfDataStoreDBSimple, std::string>
    open_db(const std::string& filename)
    {
      FILEDB::ConfDataStoreDBSimple db;
      db.setCacheSizeMB(10 * 1024);
      if (db.open(filename, O_RDONLY, 0400) != 0)
	throw std::runtime_error("error opening database file: " + filename);

      // Get the metadata
      std::string metadata;
      db.getUserdata(metadata);

      // Get the type
      std::shared_ptr<SB::Options> ops = SB::getOptionsFromXML(metadata);
      std::string dtype = SB::getOption<std::string>(*ops, "DBMetaData/id");
      if (dtype != std::string("DiscoBlocks"))
      {
	throw std::runtime_error("unsupported disco type in file: " + filename);
      }

      return {db, metadata};
    }

    static void keys_db(const FILEDB::ConfDataStoreDBSimple& db,
			std::vector<Hadron::KeyTwoQuarkDiscoOperator_t>& keys)
    {
      std::vector<SerialDBKey<Hadron::KeyTwoQuarkDiscoOperator_t>> keys0;
      db.keys<SerialDBKey<Hadron::KeyTwoQuarkDiscoOperator_t>,
	      SerialDBData<Hadron::ValTwoQuarkDiscoOperator_t>>(keys0);
      keys.reserve(keys.size() + keys0.size());
      for (const auto& key : keys0)
	keys.push_back(key.key());
    }

    static bool get_db(const FILEDB::ConfDataStoreDBSimple& db,
		       const Hadron::KeyTwoQuarkDiscoOperator_t& key,
		       Hadron::ValTwoQuarkDiscoOperator_t& val)
    {
      SerialDBData<Hadron::ValTwoQuarkDiscoOperator_t> val0;
      if (db.get(SerialDBKey<Hadron::KeyTwoQuarkDiscoOperator_t>(key), val0) != 0)
	return false;
      val = val0.data();
      return true;
    }
  };
}

#endif
