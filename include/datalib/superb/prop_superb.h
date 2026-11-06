// -*- C++ -*-
/*! \file
 * \brief Propagator elemental superbblas database
 */

#ifndef __redstar_datalib_prop_elem_superbblas_h__
#define __redstar_datalib_prop_elem_superbblas_h__

#include "datalib/superb/types.h"
#include "ConfDataStoreDBSimple.h"
#include "datalib/superb/prop_elem_type.h"
#include "datalib/storage/filedb.h"
#include "datalib/superb/superb_aux.h"
#include <xml_array.h>
#include <xml_array4d.h>

namespace Hadron
{
  using namespace XMLArray;

  //! Old Key for prop operator
  struct OldKeyPropElementalOperator_t
  {
    int                t_slice;       /*!< Propagator time slice */
    int                t_source;      /*!< Source time slice */
    int                spin_l;        /*!< Sink spin index */
    int                spin_r;        /*!< Source spin index */
    std::string        mass_label;    /*!< A mass label */
  };

  inline void read(BinaryReader& bin, OldKeyPropElementalOperator_t& param)
  {
    read(bin, param.t_slice);
    read(bin, param.t_source);
    read(bin, param.spin_r);
    read(bin, param.spin_l);
    read(bin, param.mass_label, 32);
  }

  inline void write(BinaryWriter& bin, const OldKeyPropElementalOperator_t& param)
  {
    write(bin, param.t_slice);
    write(bin, param.t_source);
    write(bin, param.spin_r);
    write(bin, param.spin_l);
    write(bin, param.mass_label);
  }
}

namespace Datalib::TensorStorage
{
  //! ConfDataStoreMultipleDB compatible class supporting the two prop storage formats
  //! but providing a Prop4 interface

  class StorageProp4
  {
  public:
    using T = std::complex<double>; // storage number type

    bool use_cache; ///< whether to use cache
    int num_vecs;   ///< distillation basis rank
    bool is_dummy;  ///< whether can produce fake data

#ifdef USE_SUPERBBLAS
    static constexpr std::size_t N = std::size_t{6}; ///< expected storage size
    enum GetOrStore { Get, Store, Check };	     ///< Used by get_or_store_key_from_cache

    struct Storage {
      unsigned int format_version;		// 0: single phasing, 1: multiple phasing
      superbblas::Storage_handle stoh;		// storage handle
      std::string order;			// storage order
      std::map<char, int> dim;			// dimensions
      std::string mass;				// mass label
      std::vector<std::array<int, 3>> phasings; // phasings
    };

    std::vector<Storage> sb_stores;
    superbblas::Context cpu_ctx;
    std::shared_ptr<int> cache_identity; // this object identity
#endif

    struct OldStorage {
      FILEDB::ConfDataStoreDBSimple db;
      XMLArray::Array<int> default_phasing;
    };
    std::vector<OldStorage> db_stores;

    StorageProp4(bool use_cache = true)
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

    ~StorageProp4()
    {
      close();
    }

    /**
     * Open a list of files with genprops
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
      {
	if (it.format_version == 0)
	  superbblas::close_storage<N, T>(it.stoh);
	else
	  superbblas::close_storage<N + 2, T>(it.stoh);
      }
      sb_stores.clear();
#endif
      db_stores.clear();
      return 0;
    }

    /**
     * Return all available Prop4 keys regardless of the underneath file formats
     * @param keys: user supplied an empty vector which is populated
     * by keys after this call.
     */
    void keys(std::vector<Hadron::KeyProp4ElementalOperator_t>& keys) const
    {
#ifdef USE_SUPERBBLAS
      for (const auto& store : sb_stores)
	keys_sb(store, keys);
#endif

      for (const auto& store : db_stores)
	keys_db(store, keys);
    }

    /**
     * Get data for a given key in Prop4 format
     * @param key user supplied key
     * @param data after the call data will be populated
     * @return 0 on success, otherwise the key not found
     */

    int get(const Hadron::KeyProp4ElementalOperator_t& key,
	    Hadron::ValProp4ElementalOperator_t& val) const
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
     * Store or retrieve a Prop4 key-value pair from cache
     * @param key user supplied key
     * @param val elemental to read/write
     * @param get_or_store one of Get, Store, Check
     * @param spin_l (optional) left spin to retrieve, otherwise, all of them
     * @param spin_r (optional) right spin to retrieve, otherwise, all of them
     * @return true on success, otherwise the key not found
     */

    bool get_or_store_key_from_cache(const Hadron::KeyProp4ElementalOperator_t& key,
				     Hadron::ValProp4ElementalOperator_t& val,
				     GetOrStore get_or_store, int spin_l = -1,
				     int spin_r = -1) const
    {
      if (!use_cache)
	return false;

      detail::Tracker t_(get_or_store == Get
                             ? "props get key-val from cache"
                             : "props store key-val from cache");

      using namespace detail;
      using mass_time_src_sink_phasings_t =
	std::tuple<void*, std::string, int, int, std::vector<int>, std::vector<int>>;
      using value_t = std::tuple<int, superbblas::detail::vector<T, superbblas::detail::Cpu>>;

      auto cpu = cpu_ctx.toCpu(0);
      struct cache_tag {
      };
      auto cache =
	superbblas::detail::getCache<mass_time_src_sink_phasings_t, value_t,
				     superbblas::detail::TupleHash<mass_time_src_sink_phasings_t>,
				     cache_tag>(cpu);
      auto to_superb_vector = [&](const std::vector<int>& v) {
	superbblas::detail::vector<int, superbblas::detail::Cpu> r(v.size(), cpu);
	std::copy_n(v.data(), v.size(), r.data());
	return r;
      };
      mass_time_src_sink_phasings_t cache_key{cache_identity.get(), key.mass_label,
					      key.t_source,	    key.t_slice,
					      key.phasing_source,   key.phasing_sink};
      if (get_or_store == Get)
      {
	auto it = cache.find(cache_key);
	if (it == cache.end()) return false;
	int n = std::get<0>(it->second.value);
	auto v = std::get<1>(it->second.value);
	if (spin_l == -1 && spin_r == -1)
	{
	  val.op.resize(n, n, Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count);
	  T* val_ptr = (T*)val.op;
	  superbblas::detail::copy_n<std::size_t, T, T>(T{1}, v.data(), v.ctx(), v.size(), val_ptr,
							v.ctx(), superbblas::detail::EWOp::Copy{});
	}
	else if (spin_l >= 0 && spin_r >= 0)
	{
	  val.op.resize(n, n, 1, 1);
	  T* val_ptr = (T*)val.op;
	  for (int i = 0; i < n; ++i)
	    for (int j = 0; j < n; ++j)
	      val.op(i, j, 0, 0) = v[(n * i + j) * Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count + spin_l * Datalib::TensorStorage::spin_count + spin_r];
	} else {
	  throw std::runtime_error("invalid value of spin_l and spin_r");
	}
	return true;
      }
      else if (get_or_store == Store)
      {
	int n = val.op.size3();
	superbblas::detail::vector<T, superbblas::detail::Cpu> v(n * n * Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count, cpu);
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

    static Storage open_sb(const std::string& filename)
    {
      detail::Tracker t_("props open db");

      std::vector<char> metadatav;
      std::vector<superbblas::IndexType> dimv;
      superbblas::values_datatype values_dtype;
      superbblas::read_storage_header(filename.c_str(), superbblas::FastToSlow, values_dtype,
				      metadatav, dimv);

      // Check that storage tensor dimension and value type match template arguments
      if (dimv.size() != N && dimv.size() != N + 2)
	throw std::runtime_error("Unexpected storage tensor dimension for perambulators");
      if (superbblas::detail::get_values_datatype<T>() != values_dtype)
	throw std::runtime_error("Unexpected storage type for perambulators");
      unsigned int format_version = (dimv.size() == N ? 0 : 1);

      //
      // Read options
      //
      std::shared_ptr<SB::Options> ops =
	SB::getOptionsFromXML(std::string(metadatav.begin(), metadatav.end()));

      std::string dtype = SB::getOption<std::string>(*ops, "DBMetaData/id");
      if (dtype != std::string("propElemOp"))
      {
	throw std::runtime_error("unsupported prop type in file: " + filename);
      }

      // Read order; letter meaning:
      // n/N: source/sink eigenvector index
      // s/q: source/sink spin index
      // p/P: time slice source/sink
      std::string order = SB::getOption<std::string>(*ops, "DBMetaData/tensorOrder");

      // Read mass
      std::string mass = SB::getOption<std::string>(*ops, "DBMetaData/mass_label");

      // Read phasing
      std::vector<std::array<int, 3>> phasings;
      if (format_version == 0)
      {
	phasings.push_back(SB::getOption<std::array<int, 3>>(
	  *ops, "DBMetaData/Params/Contractions/phase", std::array<int, 3>{{}}));
      }
      else
      {
	phasings = SB::getOption<std::vector<std::array<int, 3>>>(*ops, "DBMetaData/phasings");
      }

      // Open storage
      superbblas::Storage_handle stoh;
      if (format_version == 0)
	superbblas::open_storage<N, T>(filename.c_str(), false /* don't allow writing */, &stoh);
      else
	superbblas::open_storage<N + 2, T>(filename.c_str(), false /* don't allow writing */,
					   &stoh);

      // Create dictionary with the dimensions
      std::map<char, int> kvdim = detail::to_kvdim(order, dimv);

      return Storage{format_version, stoh, order, kvdim, mass, phasings};
    }

    void keys_sb(const Storage& store, std::vector<Hadron::KeyProp4ElementalOperator_t>& keys) const
    {
      if (store.format_version == 0)
      {
	std::vector<superbblas::PartitionItem<N>> blocks;
	superbblas::get_blocks<N, N, T>(
	  store.stoh, store.order.c_str(), store.order.c_str(), superbblas::Coor<N>{{}},
	  detail::kvcoors<N>(store.order, store.dim), blocks, superbblas::FastToSlow);
	Hadron::KeyProp4ElementalOperator_t key;
	key.mass_label = store.mass;
	key.phasing_source = detail::toXMLArray(store.phasings.at(0));
	key.phasing_sink = Datalib::TensorStorage::detail::negate(detail::toXMLArray(store.phasings.at(0)));
	int T = store.dim.at('P');
	for (const auto& b : blocks)
	{
	  const auto kvfrom = detail::to_kvdim(store.order, b[0]);
	  const auto kvsize = detail::to_kvdim(store.order, b[1]);
	  for (unsigned int p = 0; p < kvsize.at('p'); ++p)
	  {
	    for (unsigned int P = 0; P < kvsize.at('P'); ++P)
	    {
	      if (kvfrom.at('s') != 0)
		continue;
	      key.t_source = (kvfrom.at('p') + p) % T;
	      key.t_slice = (kvfrom.at('P') + P) % T;
	      keys.push_back(key);
	    }
	  }
	}
      }
      else
      {
	std::vector<superbblas::PartitionItem<N + 2>> blocks;
	superbblas::get_blocks<N + 2, N + 2, T>(
	  store.stoh, store.order.c_str(), store.order.c_str(), superbblas::Coor<N + 2>{{}},
	  detail::kvcoors<N + 2>(store.order, store.dim), blocks, superbblas::FastToSlow);
	Hadron::KeyProp4ElementalOperator_t key;
	key.mass_label = store.mass;
	int T = store.dim.at('P');
	int PH = store.dim.at('H');
	for (const auto& b : blocks)
	{
	  const auto kvfrom = detail::to_kvdim(store.order, b[0]);
	  const auto kvsize = detail::to_kvdim(store.order, b[1]);
	  for (unsigned int p = 0; p < kvsize.at('p'); ++p)
	  {
	    for (unsigned int P = 0; P < kvsize.at('P'); ++P)
	    {
	      for (unsigned int h = 0; h < kvsize.at('h'); ++h)
	      {
		for (unsigned int H = 0; H < kvsize.at('H'); ++H)
		{
		  if (kvfrom.at('s') != 0)
		    continue;
		  key.t_source = (kvfrom.at('p') + p) % T;
		  key.t_slice = (kvfrom.at('P') + P) % T;
		  key.phasing_source =
		    detail::toXMLArray(store.phasings.at((kvfrom.at('h') + h) % PH));
		  key.phasing_sink = Datalib::TensorStorage::detail::negate(
		    detail::toXMLArray(store.phasings.at((kvfrom.at('H') + H) % PH)));
		  keys.push_back(key);
		}
	      }
	    }
	  }
	}
      }
    }

    bool get_sb(const Storage& store, const Hadron::KeyProp4ElementalOperator_t& key,
		Hadron::ValProp4ElementalOperator_t& val) const
    {
      detail::Tracker t_("props get key-val (*Prop4ElementalOperator)");

      using namespace detail;

      int n = store.dim.at('n');
      if (n == 0)
	return false;

      if (key.mass_label != store.mass)
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
	if (store.format_version == 0 && phasing_source_idx != 0)
	  return false;
	it_phasing = std::find(store.phasings.begin(), store.phasings.end(),
			       Datalib::TensorStorage::detail::negate(toCoor(key.phasing_sink)));
	if (it_phasing == store.phasings.end())
	  return false;
	phasing_sink_idx = it_phasing - store.phasings.begin();
	if (store.format_version == 0 && phasing_sink_idx != 1 &&
	    store.phasings.at(0) != store.phasings.at(1))
	  return false;
      }

      // NOTE: val is Array4D with dims: colorvec_snk, colorvec_src, spin_snk, spin_src (slowest index to fastest index)
      val.op.resize(n, n, Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count);
      const char* val_order = "sqnN";
      std::array<int, 4> val_from{{}};
      std::array<int, 4> val_size{{Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count, n, n}};
      std::array<std::array<int, 4>, 2> val_partition{{val_from, val_size}};
      T* val_ptr = (T*)val.op;
      val_ptr[0] = NaN<T>::get();

      if (store.format_version == 0)
      {
	auto store_from = kvcoors<N>(store.order, {{'p', key.t_source}, {'P', key.t_slice}});
	auto store_size = kvcoors<N>(store.order, {{'n', n}, {'N', n}, {'s', Datalib::TensorStorage::spin_count}, {'q', Datalib::TensorStorage::spin_count}}, 1);
#  if SUPERBBLAS_VERSION < 2
	superbblas::load<N, 4, T, T>(T{1}, store.stoh, store.order.c_str(), store_from, store_size,
				     &val_partition, 1, val_order, val_from, &val_ptr, &cpu_ctx,
				     superbblas::FastToSlow, superbblas::Copy);
#  else
	superbblas::load<N, 4, T, T>(T{1}, store.stoh, store.order.c_str(), store_from, store_size,
				     &val_partition, 1, val_order, val_from, val_size, &val_ptr,
				     &cpu_ctx, superbblas::FastToSlow, superbblas::Copy);
#  endif
      }
      else
      {
	auto store_from = kvcoors<N + 2>(store.order, {{'p', key.t_source},
						       {'P', key.t_slice},
						       {'h', phasing_source_idx},
						       {'H', phasing_sink_idx}});
	auto store_size =
	  kvcoors<N + 2>(store.order, {{'n', n}, {'N', n}, {'s', Datalib::TensorStorage::spin_count}, {'q', Datalib::TensorStorage::spin_count}}, 1);
#  if SUPERBBLAS_VERSION < 2
	superbblas::load<N + 2, 4, T, T>(
	  T{1}, store.stoh, store.order.c_str(), store_from, store_size, &val_partition, 1,
	  val_order, val_from, &val_ptr, &cpu_ctx, superbblas::FastToSlow, superbblas::Copy);
#  else
	superbblas::load<N + 2, 4, T, T>(T{1}, store.stoh, store.order.c_str(), store_from,
					 store_size, &val_partition, 1, val_order, val_from,
					 val_size, &val_ptr, &cpu_ctx, superbblas::FastToSlow,
					 superbblas::Copy);
#  endif
      }

      // If the keys was not found, val_ptr[0] will be nan
      if (NaN<T>::is(val_ptr[0]))
	return false;

      return true;
    }
#endif

    static OldStorage open_db(const std::string& filename_with_suffix)
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
      if (dtype != std::string("propElemOp"))
      {
	throw std::runtime_error("unsupported prop type in file: " + filename);
      }

      XMLArray::Array<int> this_default_phasing;
      if (default_phasing.size() > 0)
      {
	this_default_phasing = default_phasing;
      }
      else
      {
	this_default_phasing = detail::toXMLArray(SB::getOption<std::array<int, 3>>(
	  *ops, "DBMetaData/Params/Contractions/phase", std::array<int, 3>{{}}));
      }

      return {db, this_default_phasing};
    }

    static void keys_db(const OldStorage& sto,
			std::vector<Hadron::KeyProp4ElementalOperator_t>& keys)
    {
      std::vector<SerialDBKey<Hadron::OldKeyPropElementalOperator_t>> old_keys;
      sto.db.keys<SerialDBKey<Hadron::OldKeyPropElementalOperator_t>,
		  SerialDBKey<Hadron::ValPropElementalOperator_t>>(old_keys);
      for (const auto& old_key : old_keys)
      {
	if (old_key.key().spin_l == 0 && old_key.key().spin_r == 0)
	{
	  Hadron::KeyProp4ElementalOperator_t key;
	  key.t_source = old_key.key().t_source;
	  key.t_slice = old_key.key().t_slice;
	  key.mass_label = old_key.key().mass_label;
	  key.phasing_source = sto.default_phasing;
	  key.phasing_sink = -sto.default_phasing;
	  keys.push_back(key);
	}
      }
    }

    static bool get_db(const OldStorage& sto, const Hadron::KeyProp4ElementalOperator_t& key,
		       Hadron::ValProp4ElementalOperator_t& val)
    {

      // Old databases don't support phasing
      if (key.phasing_source != sto.default_phasing || key.phasing_sink != -sto.default_phasing)
	return false;

      // Prepare key
      SerialDBKey<Hadron::OldKeyPropElementalOperator_t> old_key;
      old_key.key().t_slice = key.t_slice;
      old_key.key().t_source = key.t_source;
      old_key.key().mass_label = key.mass_label;

      // Array3D with dims colorvec_snk, spin_snk, spin_src (slowest index to fastest index)
      SerialDBData<Hadron::ValPropElementalOperator_t> val0;

      // NOTE: val is Array4D with dims: colorvec_snk, colorvec_src, spin_snk, spin_src (slowest index to fastest index)
      int n = 1;
      for (int sl = 0; sl < Datalib::TensorStorage::spin_count; ++sl)
      {
	for (int sr = 0; sr < Datalib::TensorStorage::spin_count; ++sr)
	{
	  old_key.key().spin_l = sl;
	  old_key.key().spin_r = sr;
	  if (sto.db.get(old_key, val0) != 0)
	    return false;

	  if (sl == 0 && sr == 0)
	  {
	    n = val0.data().mat.nrows();
	    val.op.resize(n, n, Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count);
	  }

	  for (int colorvec_snk = 0; colorvec_snk < n; ++colorvec_snk)
	    for (int colorvec_src = 0; colorvec_src < n; ++colorvec_src)
	      val.op(colorvec_snk, colorvec_src, sl, sr) =
		val0.data().mat(colorvec_snk, colorvec_src);
	}
      }

      return true;
    }

    void get_default(Hadron::ValProp4ElementalOperator_t& val,
		     const Hadron::KeyProp4ElementalOperator_t& key) const
    {
      if (num_vecs < 0)
	throw std::runtime_error("number of vectors not set!");
      val.op.resize(num_vecs, num_vecs, Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count);

      // Generate a \gamma_5-Hermitian matrix by multiplying \gamma_5 to a Hermitian matrix
      int t_source = key.t_source;
      int t_sink = key.t_slice;
      int h_source = SB::getHash(Datalib::TensorStorage::detail::toCoor(key.phasing_source)) % 8ul;
      int h_sink =
	SB::getHash(Datalib::TensorStorage::detail::negate(Datalib::TensorStorage::detail::toCoor(key.phasing_sink))) % 8ul;
      const auto get_val = [&](int s0, int n0, int t0, int h0, int s1, int n1, int t1, int h1) {
	auto h = s0 +						      //
		 n0 * Datalib::TensorStorage::spin_count +					      //
		 (t0 % 16) * Datalib::TensorStorage::spin_count * num_vecs +			      //
		 h0 * Datalib::TensorStorage::spin_count * num_vecs * 16 +			      //
		 s1 * Datalib::TensorStorage::spin_count * num_vecs * 16 * 8 +			      //
		 n1 * Datalib::TensorStorage::spin_count * num_vecs * 16 * 8 * Datalib::TensorStorage::spin_count +		      //
		 (t1 % 16) * Datalib::TensorStorage::spin_count * num_vecs * 16 * 8 * Datalib::TensorStorage::spin_count * num_vecs + //
		 h1 * Datalib::TensorStorage::spin_count * num_vecs * 16 * 8 * Datalib::TensorStorage::spin_count * num_vecs * 16;
	return T{(double)(h % 128), (double)(h % 57)};
      };

#ifdef _OPENMP
#  pragma omp parallel for schedule(static)
#endif
      for (int i = 0; i < num_vecs; ++i)
	for (int j = 0; j < num_vecs; ++j)
	  for (int k = 0; k < Datalib::TensorStorage::spin_count; ++k)
	    for (int l = 0; l < Datalib::TensorStorage::spin_count; ++l)
	      val.op(i, j, k, l) =
		(get_val(l, j, t_source, h_source, k, i, t_sink, h_sink) +
		 std::conj(get_val(k, i, t_sink, h_sink, l, j, t_source, h_source))) *
		(k / 2 == 0 ? 1.0 : -1.0);
    }
  };
}

#endif
