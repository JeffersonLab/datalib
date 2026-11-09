#include "datalib/storage/record_store.h"
// -*- C++ -*-
/*! \file
 * \brief Meson elemental superbblas database
 */

#ifndef __redstar_datalib_meson_elem_superbblas_h__
#define __redstar_datalib_meson_elem_superbblas_h__

#include "datalib/superb/types.h"
#include "ConfDataStoreDBSimple.h"
#include "datalib/superb/colorvec_matelem_type.h"
#include "datalib/superb/meson_elem_type.h"
#include "datalib/storage/filedb.h"
#include "datalib/superb/superb_aux.h"
#include <memory>

namespace Hadron
{
  using namespace XMLArray;

  //! Key for Baryon operator (old version)
  struct OldKeyMesonElementalOperator_t {
    int              t_slice;             /*!< Meson operator time slice */ 
    std::vector<int> displacement;        /*!< Displacement dirs of right colorvector */
    Array<int>       mom;                 /*!< D-1 momentum of this operator */
  };

  inline void read(BinaryReader& bin, OldKeyMesonElementalOperator_t& param)
  {
    read(bin, param.t_slice);
    read(bin, param.displacement);
    read(bin, param.mom);
  }

  inline void write(BinaryWriter& bin, const OldKeyMesonElementalOperator_t& param)
  {
    write(bin, param.t_slice);
    write(bin, param.displacement);
    write(bin, param.mom);
  }
}


namespace Datalib::TensorStorage
{
  //! ConfDataStoreMultipleDB compatible class supporting the two meson storage formats

  class StorageMeson : public StorageRecords
  {
  public:
    using T = std::complex<double>; // storage number type

    bool use_cache; ///< whether to use cache
    int num_vecs;   ///< distillation basis rank
    bool is_dummy;  ///< whether can produce fake data

#ifdef USE_SUPERBBLAS
    static constexpr std::size_t N = std::size_t{7}; ///< expected storage size
    enum GetOrStore { Get, Store, Check };	     ///< Used by get_or_store_key_from_cache

    struct StorageSB {
      superbblas::Storage_handle stoh;		// storage handle
      std::string order;			// storage order
      std::map<char, int> dim;			// dimensions
      std::vector<std::vector<int>> disps;      // displacements
      std::vector<std::array<int, 3>> moms;     // momenta
      std::vector<std::array<int, 3>> phasings; // phasings
      bool use_derivP;                          // use derivP
    };

    std::vector<StorageSB> sb_stores;
    superbblas::Context cpu_ctx;
    std::shared_ptr<int> cache_identity; // this object identity
#endif

    struct StorageDB {
      FILEDB::ConfDataStoreDBSimple db;		// storage
      Array<int>  phasing_src;                  // phasing source
      Array<int>  phasing_snk;                  // phasing sink
    };

    std::vector<StorageDB> db_stores;

    StorageMeson(bool use_cache = true)
      : use_cache(use_cache),
	num_vecs(-1),
	is_dummy(false)
#ifdef USE_SUPERBBLAS
	,
	cpu_ctx(superbblas::createCpuContext()),
	cache_identity(std::make_shared<int>(0))
#endif
    {
    }

    ~StorageMeson()
    {
      close();
    }

    /**
     * Open a list of files with mesons
     * @param filenames: file paths 
     * @param num_vecs: suggested number of distillation vectors
     *
     * @return 0 on success, -1 on failure
     * 
     */

    int open(const std::vector<std::string>& filenames, int num_vecs = -1)
    {
      close();
      if (num_vecs >= 0)
	this->num_vecs = num_vecs;

      for (const auto& filename : filenames)
      {
	if (detail::is_filename_dummy(filename))
	{
	  is_dummy = true;
	  continue;
	}

#ifdef USE_SUPERBBLAS
	try
	{
	  sb_stores.push_back(open_sb(filename));
	} catch (...)
#endif
	{
	  db_stores.push_back(open_db(filename));
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
        superbblas::close_storage<N, T>(it.stoh);
      sb_stores.clear();
#endif
      db_stores.clear();
      return 0;
    }

    /**
     * Return all available meson keys regardless of the underneath file formats
     * @param keys: user supplied an empty vector which is populated
     * by keys after this call.
     */
    void keys(std::vector<Hadron::KeyMesonElementalOperator_t>& keys) const
    {
#ifdef USE_SUPERBBLAS
      for (const auto& store : sb_stores)
	keys_sb(store, keys);
#endif

      for (const auto& store : db_stores)
	keys_db(store, keys);
    }

    /**
     * Get data for a given key in meson format
     * @param key user supplied key
     * @param data after the call data will be populated
     * @return 0 on success, otherwise the key not found
     */

    int get(const Hadron::KeyMesonElementalOperator_t& key,
	    Hadron::ValMesonElementalOperator_t& val) const
    {

      bool found = false;

#ifdef USE_SUPERBBLAS
      // Try to read from cache
      if (get_or_store_key_from_cache(key, val, Get))
	return 0;

      for (const auto& store : sb_stores)
      {
	if (get_sb(store, key, val))
	{
	  found = true;
	  break;
	}
      }
#endif

      if (!found)
      {
	for (const auto& store : db_stores)
	{
	  if (get_db(store, key, val))
	  {
	    found = true;
	    break;
	  }
	}
      }

      if (!found && is_dummy)
      {
	get_default(val, key);
	return 0;
      }

      if (!found)
	return -1;

#ifdef USE_SUPERBBLAS
      // Store the value on cache
      get_or_store_key_from_cache(key, val, Store);
#endif

      return 0;
    }

  private:
#ifdef USE_SUPERBBLAS
    /**
     * Store or retrieve a meson key-value pair from cache
     * @param key user supplied key
     * @param val elemental to read/write
     * @param get_or_store one of Get, Store, Check
     * @return true on success, otherwise the key not found
     */

    bool
    get_or_store_key_from_cache(const Hadron::KeyMesonElementalOperator_t &key,
                                Hadron::ValMesonElementalOperator_t &val,
                                GetOrStore get_or_store) const {
      if (!use_cache)
	return false;

      detail::Tracker t_(get_or_store == Get
                             ? "meson get key-val from cache"
                             : "meson store key-val from cache");

      using namespace detail;
      using time_disp_mom_src_sink_phasings_t =
	std::tuple<void*, int, std::vector<int>, std::vector<int>, std::vector<int>,
		   std::vector<int>>;
      using value_t = std::tuple<int, superbblas::detail::vector<T, superbblas::detail::Cpu>>;

      auto cpu = cpu_ctx.toCpu(0);
      struct cache_tag {
      };
      auto cache = superbblas::detail::getCache<
	time_disp_mom_src_sink_phasings_t, value_t,
	superbblas::detail::TupleHash<time_disp_mom_src_sink_phasings_t>, cache_tag>(cpu);
      auto to_superb_vector = [&](const std::vector<int>& v) {
	superbblas::detail::vector<int, superbblas::detail::Cpu> r(v.size(), cpu);
	std::copy_n(v.data(), v.size(), r.data());
	return r;
      };
      time_disp_mom_src_sink_phasings_t cache_key{cache_identity.get(), key.t_slice,
						  key.displacement,	key.mom,
						  key.phasing_source,	key.phasing_sink};
      if (get_or_store == Get)
      {
	auto it = cache.find(cache_key);
	if (it == cache.end()) return false;
	int n = std::get<0>(it->second.value);
        auto v = std::get<1>(it->second.value);
	val.type_of_data = COLORVEC_MATELEM_TYPE_DERIV;
        val.op.resize(n, n);
        T *val_ptr = (T *)val.op;
        superbblas::detail::copy_n<std::size_t, T, T>(
            T{1}, v.data(), v.ctx(), v.size(), val_ptr, v.ctx(),
            superbblas::detail::EWOp::Copy{});
        return true;
      }
      else if (get_or_store == Store)
      {
        if (val.type_of_data != COLORVEC_MATELEM_TYPE_DERIV)
          throw std::runtime_error("Unsupported meson type");
        int n = val.op.size1();
	superbblas::detail::vector<T, superbblas::detail::Cpu> v(n * n, cpu);
	T* val_ptr = (T*)val.op;
	superbblas::detail::copy_n<std::size_t, T, T>(T{1}, val_ptr, v.ctx(), v.size(), v.data(),
						      v.ctx(), superbblas::detail::EWOp::Copy{});
	cache.insert(cache_key, value_t{n, v}, superbblas::detail::storageSize(v));
	return true;
      }
      else if (get_or_store == Check)
      {
	auto it = cache.find(cache_key);
	if (it == cache.end()) return false;
	return true;
      }
      else
      {
	throw std::runtime_error("invalid command");
      }
    }

    static StorageSB open_sb(const std::string& filename)
    {
      detail::Tracker t_("mesons open db");

      std::vector<char> metadatav;
      std::vector<superbblas::IndexType> dimv;
      superbblas::values_datatype values_dtype;
      superbblas::read_storage_header(filename.c_str(), superbblas::FastToSlow, values_dtype,
				      metadatav, dimv);

      // Check that storage tensor dimension and value type match template arguments
      if (dimv.size() != N)
	throw std::runtime_error("Unexpected storage tensor dimension for mesons");
      if (superbblas::detail::get_values_datatype<T>() != values_dtype)
	throw std::runtime_error("Unexpected storage type for mesons");

      //
      // Read options
      //
      std::shared_ptr<SB::Options> ops =
	SB::getOptionsFromXML(std::string(metadatav.begin(), metadatav.end()));

      std::string dtype = SB::getOption<std::string>(*ops, "DBMetaData/id");
      if (dtype != std::string("mesonElemOp"))
      {
	throw std::runtime_error("unsupported meson type in file: " + filename);
      }

      // Read order; letter meaning:
      /// i,j: eigenvector indices
      /// t: time slice
      /// d: displacements
      /// m: momentum
      /// h/H: phasing source/sink
      std::string order = SB::getOption<std::string>(*ops, "DBMetaData/tensorOrder");

      // Read derivatives
      std::vector<std::vector<int>> disps =
	SB::getOption<std::vector<std::vector<int>>>(*ops, "DBMetaData/displacements");

      // Read momenta
      std::vector<std::array<int, 3>> moms =
	SB::getOption<std::vector<std::array<int, 3>>>(*ops, "DBMetaData/moms");


      // Read phasing
      std::vector<std::array<int, 3>> phasings =
          SB::getOption<std::vector<std::array<int, 3>>>(*ops,
                                                         "DBMetaData/phasings");

      // Read use_derivP
      bool use_derivP = SB::getOption<bool>(*ops, "DBMetaData/Params/use_derivP");

      // Read version
      int version = SB::getOption<int>(*ops, "DBMetaData/Params/version");
      if (version < 5)
      {
	throw std::runtime_error("unsupported meson version in file: " + filename);
      }

      // Open storage
      superbblas::Storage_handle stoh;
      superbblas::open_storage<N, T>(filename.c_str(),
                                     false /* don't allow writing */, &stoh);

      // Create dictionary with the dimensions
      std::map<char, int> kvdim = detail::to_kvdim(order, dimv);

      return StorageSB{stoh, order, kvdim, disps, moms, phasings, use_derivP};
    }

    void keys_sb(const StorageSB &store,
                 std::vector<Hadron::KeyMesonElementalOperator_t> &keys) const {
      std::vector<superbblas::PartitionItem<N>> blocks;
      superbblas::get_blocks<N, N, T>(
          store.stoh, store.order.c_str(), store.order.c_str(),
          superbblas::Coor<N>{{}}, detail::kvcoors<N>(store.order, store.dim),
          blocks, superbblas::FastToSlow);
      Hadron::KeyMesonElementalOperator_t key;
      int T = store.dim.at('t');
      int M = store.dim.at('m');
      int D = store.dim.at('d');
      int PH = store.dim.at('h');
      for (const auto &b : blocks) {
        const auto kvfrom = detail::to_kvdim(store.order, b[0]);
        const auto kvsize = detail::to_kvdim(store.order, b[1]);
        for (unsigned int t = 0; t < kvsize.at('t'); ++t) {
          for (unsigned int d = 0; d < kvsize.at('d'); ++d) {
            for (unsigned int m = 0; m < kvsize.at('m'); ++m) {
              for (unsigned int h = 0; h < kvsize.at('h'); ++h) {
                for (unsigned int H = 0; H < kvsize.at('H'); ++H) {
                  key.t_slice = (kvfrom.at('t') + t) % T;
                  key.mom = detail::toXMLArray(
                      store.moms.at((kvfrom.at('m') + m) % M));
		  key.displacement = store.disps.at(kvfrom.at('d') + d);
		  key.phasing_source = detail::toXMLArray(
                      store.phasings.at((kvfrom.at('h') + h) % PH));
		  key.phasing_sink = detail::toXMLArray(
		    Datalib::TensorStorage::detail::negate(store.phasings.at((kvfrom.at('H') + H) % PH)));
		  keys.push_back(key);
                }
              }
            }
          }
        }
      }
    }

    bool get_sb(const StorageSB& store, const Hadron::KeyMesonElementalOperator_t& key,
		Hadron::ValMesonElementalOperator_t& val) const
    {
      detail::Tracker t_("mesons get key-val (*MesonElementalOperator)");

      using namespace detail;

      int n = store.dim.at('i');
      if (n == 0)
	return false;

      auto it_disps = std::find(store.disps.begin(), store.disps.end(), key.displacement);
      if (it_disps == store.disps.end())
	return false;
      int disp_idx = it_disps - store.disps.begin();

      auto it_moms = std::find(store.moms.begin(), store.moms.end(), toCoor(key.mom));
      if (it_moms == store.moms.end())
	return false;
      int mom_idx = it_moms - store.moms.begin();

      // Throw exception if some phasing is specified and the other is not
      if (key.phasing_source.size() != key.phasing_sink.size())
	throw std::runtime_error(
	  "Unsupported just either the source or the sink phasing being specified");

      // If no phasing restrictions is given but multiple phases are stored, just not use this file
      if (key.phasing_source.size() == 0 && store.phasings.size() != 1)
	return false;
      int phasing_source_idx = 0;
      int phasing_sink_idx = 0;
      if (key.phasing_source.size() > 0)
      {
	auto it_phasing =
	  std::find(store.phasings.begin(), store.phasings.end(), toCoor(key.phasing_source));
	if (it_phasing == store.phasings.end())
	  return false;
	phasing_source_idx = it_phasing - store.phasings.begin();
	it_phasing = std::find(store.phasings.begin(), store.phasings.end(),
			       Datalib::TensorStorage::detail::negate(toCoor(key.phasing_sink)));
	if (it_phasing == store.phasings.end())
	  return false;
	phasing_sink_idx = it_phasing - store.phasings.begin();
      }

      // NOTE: val is multi2d with dims: colorvec_src, colorvec_snk (slowest
      // index to fastest index). Check the order of this!!!!!!!
      val.op.resize(n, n);
      const char* val_order = "ij";
      std::array<int, 2> val_from{{}};
      std::array<int, 2> val_size{{n, n}};
      std::array<std::array<int, 2>, 2> val_partition{{val_from, val_size}};
      T* val_ptr = (T*)val.op;
      val_ptr[0] = NaN<T>::get();

      auto store_from = kvcoors<N>(store.order, {{'t', key.t_slice},
						 {'m', mom_idx},
						 {'d', disp_idx},
						 {'h', phasing_source_idx},
						 {'H', phasing_sink_idx}});
      auto store_size = kvcoors<N>(
          store.order, {{'i', n}, {'j', n}}, 1);
#if SUPERBBLAS_VERSION < 2
      superbblas::load<N, 2, T, T>(
          T{1}, store.stoh, store.order.c_str(), store_from, store_size,
          &val_partition, 1, val_order, val_from, &val_ptr, &cpu_ctx,
          superbblas::FastToSlow, superbblas::Copy);
#else
      superbblas::load<N, 2, T, T>(
          T{1}, store.stoh, store.order.c_str(), store_from, store_size,
          &val_partition, 1, val_order, val_from, val_size, &val_ptr, &cpu_ctx,
          superbblas::FastToSlow, superbblas::Copy);
#endif

      // If the keys was not found, val_ptr[0] will be nan
      if (NaN<T>::is(val_ptr[0]))
	return false;

      // Set derivP
      val.type_of_data =
	(store.use_derivP ? COLORVEC_MATELEM_TYPE_DERIV : COLORVEC_MATELEM_TYPE_GENERIC);

      return true;
    }
#endif

    static StorageDB open_db(const std::string& filename_with_suffix)
    {
      const auto& [filename, default_phasing] =
	detail::get_filename_and_phasing(filename_with_suffix);
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
      if (dtype != std::string("mesonElemOp"))
      {
	throw std::runtime_error("unsupported meson type in file: " + filename);
      }

      // Read version: <= 4, antiquark phase is applied before the dagger;
      // for version 5+, the antiquark phase is applied after conjugating the eigenvectors
      int version = SB::getOption<int>(*ops, "DBMetaData/Params/version");

      auto phasing_src = detail::toXMLArray(SB::getOption<std::array<int, 3>>(
	*ops, "DBMetaData/Params/quarkPhase", std::array<int, 3>{0, 0, 0}));
      auto phasing_snk = detail::toXMLArray(SB::getOption<std::array<int, 3>>(
	*ops, "DBMetaData/Params/aQuarkPhase", std::array<int, 3>{0, 0, 0}));
      if (version <= 4)
	phasing_snk = -phasing_snk;

      if (default_phasing.size() > 0)
      {
	phasing_src = default_phasing;
	phasing_snk = -default_phasing;
      }
      return StorageDB{db, phasing_src, phasing_snk};
    }

    static void
    keys_db(const StorageDB &db,
            std::vector<Hadron::KeyMesonElementalOperator_t> &keys) {
      std::vector<SerialDBKey<Hadron::OldKeyMesonElementalOperator_t>> old_keys;
      db.db.keys<SerialDBKey<Hadron::OldKeyMesonElementalOperator_t>,
                 SerialDBKey<Hadron::ValMesonElementalOperator_t>>(old_keys);
      Hadron::KeyMesonElementalOperator_t key;
      key.phasing_sink = db.phasing_snk;
      key.phasing_source = db.phasing_src;
      for (const auto& old_key : old_keys)
      {
        key.t_slice = old_key.key().t_slice;
        key.displacement = old_key.key().displacement;
        key.mom = old_key.key().mom;
        keys.push_back(key);
      }
    }

    static bool get_db(const StorageDB &db,
                       const Hadron::KeyMesonElementalOperator_t &key,
                       Hadron::ValMesonElementalOperator_t &val) {

      if (key.phasing_source != db.phasing_src || key.phasing_sink != db.phasing_snk)
	return false;

      // Prepare key
      SerialDBKey<Hadron::OldKeyMesonElementalOperator_t> old_key;
      old_key.key().t_slice = key.t_slice;
      old_key.key().displacement = key.displacement;
      old_key.key().mom = key.mom;

      SerialDBData<Hadron::ValMesonElementalOperator_t> val0;
      if (db.db.get(old_key, val0) != 0)
        return false;
      val = val0.data();

      return true;
    }

    void get_default(Hadron::ValMesonElementalOperator_t& val,
		     const Hadron::KeyMesonElementalOperator_t& key) const
    {
      if (num_vecs < 0)
	throw std::runtime_error("number of vectors not set!");
      val.type_of_data = COLORVEC_MATELEM_TYPE_DERIV;
      val.op.resize(num_vecs, num_vecs);

      // Generate a Hermitian matrix
      int h_source = SB::getHash(Datalib::TensorStorage::detail::toCoor(key.phasing_source)) % 8ul;
      int h_sink =
	SB::getHash(Datalib::TensorStorage::detail::negate(Datalib::TensorStorage::detail::toCoor(key.phasing_sink))) % 8ul;
      int h_med = SB::getHash(std::make_tuple(Datalib::TensorStorage::detail::toCoor(key.mom), key.displacement,
					      key.t_slice)) %
		  128ul;
      int h_neg_med =
	SB::getHash(std::make_tuple(
	  Datalib::TensorStorage::detail::negate(Datalib::TensorStorage::detail::toCoor(key.mom)),
	  Datalib::TensorStorage::detail::negate(Datalib::TensorStorage::detail::reverse(key.displacement)), key.t_slice)) %
	128ul;

      const auto get_val = [&](int n0, int h0, int n1, int h1, int h_med) {
	auto h = n0 +				//
		 h0 * num_vecs +		//
		 n1 * num_vecs * 8 +		//
		 h1 * num_vecs * 8 * num_vecs + //
		 h_med;
	return T{(double)(h % 128), (double)(h % 57)};
      };

#ifdef _OPENMP
#  pragma omp parallel for schedule(static)
#endif
      for (int i = 0; i < num_vecs; ++i)
	for (int j = 0; j < num_vecs; ++j)
	  val.op(i, j) = get_val(j, h_source, i, h_sink, h_med) +
			 std::conj(get_val(i, h_sink, j, h_source, h_neg_med));
    }
  };
}

#endif
