// -*- C++ -*-
/*! \file
 * \brief Unsmeared meson elemental superbblas database
 */

#ifndef __redstar_datalib_genprop4_superbblas_h__
#define __redstar_datalib_genprop4_superbblas_h__

#include "datalib/superb/types.h"
#include "ConfDataStoreDBSimple.h"
#include "datalib/superb/unsmeared_genprop4_elem.h"
#include "datalib/superb/unsmeared_meson_elem.h"
#include "datalib/storage/filedb.h"
#include "datalib/superb/superb_aux.h"
#include <array>
#include <memory>
#include <set>
#include <datalib/model/xml_array.h>
#include <datalib/model/xml_array4d.h>

namespace Hadron
{
  using namespace XMLArray;

  //! Old Generalized propagator operator
  struct OldKeyGenProp4ElementalOperator_t
  {
    int                t_sink;        /*!< Source time slice */
    int                t_slice;       /*!< Propagator time slice */
    int                t_source;      /*!< Source time slice */
    int                g;             /*!< DR spin matrices */
    std::vector<int>   displacement;  /*!< Displacement dirs of right colorstd::vector */
    Array<int>         mom;           /*!< D-1 momentum of this operator */
    std::string        mass;          /*!< A mass label */
  };
 
  inline void read(BinaryReader& bin, OldKeyGenProp4ElementalOperator_t& param)
  {
    read(bin, param.t_sink);
    read(bin, param.t_slice);
    read(bin, param.t_source);
    read(bin, param.g);
    read(bin, param.displacement);
    read(bin, param.mom);
    readDesc(bin, param.mass);
  }

  inline void write(BinaryWriter& bin, const OldKeyGenProp4ElementalOperator_t& param)
  {
    write(bin, param.t_sink);
    write(bin, param.t_slice);
    write(bin, param.t_source);
    write(bin, param.g);
    write(bin, param.displacement);
    write(bin, param.mom);
    writeDesc(bin, param.mass);
  }
}

namespace Datalib::TensorStorage
{
  //! ConfDataStoreMultipleDB compatible class supporting the three genprop storage formats
  //! but providing a Genprop4 interface

  class StorageGenprop4
  {
  public:
    using T = std::complex<double>; // storage number type
    using Coor4 = std::array<int, 4>;

    bool use_cache; ///< whether to use cache
    int num_vecs;   ///< distillation basis rank
    bool is_dummy;  ///< whether can produce fake data
    /// whether to return a zero tensor when t_slice isn't between t_source and t_sink
    bool zero_values_for_outside_t_slices;

#ifdef USE_SUPERBBLAS
    static constexpr std::size_t N = std::size_t{10}; ///< expected storage size
    enum GetOrStore { Get, Store };		      ///< Used by get_or_store_key_from_cache

    struct Storage {
      unsigned int format_version;		// 0: single phasing, 1: multiple phasing
      superbblas::Storage_handle stoh;		// storage handle
      std::string order;			// storage order
      std::map<char, int> dim;			// dimensions
      std::vector<std::vector<int>> disps;	// displacements
      std::vector<std::array<int, 3>> moms;	// momenta
      std::vector<std::array<int, 3>> phasings; // phasing
      std::vector<int> gammas;			// list of gammas
      std::string mass;				// mass label
      mutable std::set<int> requested_gammas;	// requested gammas
      mutable std::set<int> requested_disps;	// requested disps
    };

    std::vector<Storage> sb_stores;
    superbblas::Context cpu_ctx;
    std::shared_ptr<int> cache_identity; // this object identity
#endif

    struct OldStorage {
      FILEDB::ConfDataStoreDBSimple db;
      XMLArray::Array<int> default_phasing;
    };
    std::vector<OldStorage> db2_stores;
    std::vector<OldStorage> db4_stores;

    // NOTE: Nt is used by `get_db2`, `get_db4`, `keys_db2` and `keys_db4`
    int Nt;

    StorageGenprop4(bool use_cache = true)
      : use_cache(use_cache),
	num_vecs(-1),
	is_dummy(false),
	zero_values_for_outside_t_slices(false)
#ifdef USE_SUPERBBLAS
	,
	cpu_ctx(superbblas::createCpuContext()),
	cache_identity(std::make_shared<int>(0))
#endif
    {
    }

    ~StorageGenprop4()
    {
      close();
    }

    /**
     * Open a list of files with genprops
     * @param filenames: file paths 
     * @param num_vecs: suggested number of distillation vectors
     * @param zero_values_for_outside_t_slices: (optional) return a zero tensor when
     *       t_slice is not between t_source and t_sink, if the key isn't present.
     *
     * @return 0 on success, -1 on failure
     * 
     */

    int open(const std::vector<std::string>& filenames, int num_vecs = -1,
	     bool zero_values_for_outside_t_slices = false)
    {
      close();
      if (num_vecs >= 0)
	this->num_vecs = num_vecs;
      this->zero_values_for_outside_t_slices = zero_values_for_outside_t_slices;

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
	  bool is_db4 = false;
	  auto db = open_db(filename, is_db4, Nt);
	  if (!is_db4)
	    db2_stores.push_back(db);
	  else
	    db4_stores.push_back(db);
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
      db2_stores.clear();
      db4_stores.clear();
      return 0;
    }

    /**
     * Return all available Genprop4 keys regardless of the underneath file formats
     * @param keys: user supplied an empty vector which is populated
     * by keys after this call.
     */
    void keys(std::vector<SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t>>& keys) const
    {
#ifdef USE_SUPERBBLAS
      for (const auto& store : sb_stores)
	keys_sb(store, keys);
#endif

      for (const auto& store : db2_stores)
	keys_db2(store, keys, Nt);

      for (const auto& store : db4_stores)
	keys_db4(store, keys, Nt);
    }

    /**
     * Get data for a given key in Genprop4 format
     * @param key user supplied key
     * @param data after the call data will be populated
     * @return 0 on success, otherwise the key not found
     */

    int get(const SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t>& key,
	    SerialDBData<Hadron::ValGenProp4ElementalOperator_t>& val) const
    {

#ifdef USE_SUPERBBLAS
      // Try to read from cache
      if (get_or_store_key_from_cache(key, val, Get))
	return 0;

      // Try from superb files
      // NOTE: get_sb also deals with the cache
      for (const auto& store : sb_stores)
	if (get_sb(store, key, val))
	  return 0;
#endif

      bool found = false;

      // Try from original filehash format
      for (const auto& store : db2_stores)
      {
	if (get_db2(store, key, val, Nt))
	{
	  found = true;
	  break;
	}
      }

      // Try from genprop4 filehash format
      if (!found)
      {
	for (const auto& store : db4_stores)
	{
	  if (get_db4(store, key, val, Nt))
	  {
	    found = true;
	    break;
	  }
	}
      }

      if (!found && is_dummy)
      {
	get_default(val, key.key());
	return 0;
      }

      if (zero_values_for_outside_t_slices && get_default_for_tslice_outside_sink_source(key, val))
      {
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

    /**
     * Get data for a given key in Genprop4 format
     * @param key user supplied key
     * @param val_from first coordinates to retrieve
     * @param val_size size of the coordinates to retrieve
     * @param val_order a permutation of "sqnN" indicating the order, from fast to slow moving index
     * @param val_ptr pointer to the first element to modify
     * @return 0 on success, otherwise the key not found
     *
     * NOTE: labels description:
     *  s,q: spin source and sink
     *  n,N: distillation vector index source and sink
     */

    int get(const Hadron::KeyGenProp4ElementalOperator_t& key, const Coor4& val_from,
	    const Coor4& val_size, const std::string& val_order,
	    std::complex<double>* val_ptr) const
    {
#ifdef USE_SUPERBBLAS
      // Try from superb files
      for (const auto& store : sb_stores)
	if (get_sb(store, key, val_from, val_size, val_order, val_ptr))
	  return 0;

      if (is_dummy)
      {
	SerialDBData<Hadron::ValGenProp4ElementalOperator_t> val;
	get_default(val, key);
	constexpr std::size_t N = 4;
	using value_type = std::complex<double>;
	Coor4 from0{{}};
	Coor4 size0{Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count, num_vecs, num_vecs};
	superbblas::PartitionItem<N> fs0{from0, size0};
	superbblas::PartitionItem<N> fs1{val_from, val_size};
	auto* ptr0 = (const std::complex<double>*)val.data().op;
	auto* ptr1 = val_ptr;
	const std::string order0 = "sqnN";

	superbblas::copy<N, N, value_type, value_type>(
	  value_type{1}, //
	  &fs0, 1, order0.c_str(), Coor4{{}}, size0, size0, (const value_type**)&ptr0, nullptr,
	  &cpu_ctx, //
	  &fs1, 1, val_order.c_str(), Coor4{{}}, val_size, (value_type**)&ptr1, nullptr, &cpu_ctx,
	  superbblas::FastToSlow, superbblas::Copy);
	//std::size_t vol = 1;
	//for (int i : val_size)
	//  vol *= i;
	//get_default(val_ptr, vol);
	return 0;
      }
#endif

      return 1;
    }

  private:
#ifdef USE_SUPERBBLAS
    bool get_or_store_key_from_cache(const SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t>& key,
				     SerialDBData<Hadron::ValGenProp4ElementalOperator_t>& val,
				     GetOrStore get_or_store) const
    {
      if (!use_cache)
	return false;

      detail::Tracker t_(get_or_store == Get
                             ? "genprops get key-val from cache"
                             : "genprops store key-val from cache");

      using namespace detail;
      using mass_disp_mom_phasings_gamma_time_src_slc_sink_t =
	std::tuple<void*, std::string, superbblas::detail::vector<int, superbblas::detail::Cpu>,
		   std::array<int, 3>, std::vector<int>, std::vector<int>, int, int, int, int>;
      using value_t = std::tuple<int, superbblas::detail::vector<T, superbblas::detail::Cpu>>;

      auto cpu = cpu_ctx.toCpu(0);
      struct cache_tag {
      };
      auto cache = superbblas::detail::getCache<
	mass_disp_mom_phasings_gamma_time_src_slc_sink_t, value_t,
	superbblas::detail::TupleHash<mass_disp_mom_phasings_gamma_time_src_slc_sink_t>, cache_tag>(
	cpu);
      auto to_superb_vector = [&](const std::vector<int>& v) {
	superbblas::detail::vector<int, superbblas::detail::Cpu> r(v.size(), cpu);
	std::copy_n(v.data(), v.size(), r.data());
	return r;
      };
      mass_disp_mom_phasings_gamma_time_src_slc_sink_t cache_key{
	cache_identity.get(),  key.key().mass,		 to_superb_vector(key.key().displacement),
	toCoor(key.key().mom), key.key().phasing_source, key.key().phasing_sink,
	key.key().g,	       key.key().t_source,	 key.key().t_slice,
	key.key().t_sink};
      if (get_or_store == Get)
      {
	auto it = cache.find(cache_key);
	if (it == cache.end())
	  return false;
	int n = std::get<0>(it->second.value);
	auto v = std::get<1>(it->second.value);
	val.data().op.resize(n, n, Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count);
	T* val_ptr = (T*)val.data().op;
	superbblas::detail::copy_n<std::size_t, T, T>(T{1}, v.data(), v.ctx(), v.size(), val_ptr,
						      v.ctx(), superbblas::detail::EWOp::Copy{});
	return true;
      }
      else
      {
	int n = val.data().op.size3();
	superbblas::detail::vector<T, superbblas::detail::Cpu> v(n * n * Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count, cpu);
	T* val_ptr = (T*)val.data().op;
	superbblas::detail::copy_n<std::size_t, T, T>(T{1}, val_ptr, v.ctx(), v.size(), v.data(),
						      v.ctx(), superbblas::detail::EWOp::Copy{});
	cache.insert(cache_key, value_t{n, v}, superbblas::detail::storageSize(v));
	return true;
      }
    }

    static Storage open_sb(const std::string& filename)
    {
      detail::Tracker t_("genprops open db");

      std::vector<char> metadatav;
      std::vector<superbblas::IndexType> dimv;
      superbblas::values_datatype values_dtype;
      superbblas::read_storage_header(filename.c_str(), superbblas::FastToSlow, values_dtype,
				      metadatav, dimv);

      // Check that storage tensor dimension and value type match template arguments
      if (dimv.size() != N && dimv.size() != N + 2)
	throw std::runtime_error(
	  "Unexpected storage tensor dimension for genprops");
      if (superbblas::detail::get_values_datatype<T>() != values_dtype)
	throw std::runtime_error("Unexpected storage type for genprops");
      unsigned int format_version = (dimv.size() == N ? 0 : 1);

      //
      // Read options
      //
      std::shared_ptr<SB::Options> ops =
	SB::getOptionsFromXML(std::string(metadatav.begin(), metadatav.end()));

      std::string dtype = SB::getOption<std::string>(*ops, "DBMetaData/id");
      if (dtype != std::string("genprop5ElemOp"))
      {
	throw std::runtime_error("unsupported genprop type in file: " + filename);
      }

      // Read order; letter meaning:
      // n/N: source/sink eigenvector index
      // s/q: source/sink spin index
      // g: gamma
      // d: derivative
      // m: momentum
      // h/H: source/sink phasing (for venison 1)
      // p/P: time slice source/sink
      // t: time slice middle index
      std::string order = SB::getOption<std::string>(*ops, "DBMetaData/tensorOrder");

      // Read derivatives
      std::vector<std::vector<int>> disps =
	SB::getOption<std::vector<std::vector<int>>>(*ops, "DBMetaData/displacements");

      // Read momenta
      std::vector<std::array<int, 3>> moms =
	SB::getOption<std::vector<std::array<int, 3>>>(*ops, "DBMetaData/moms");

      // Read gamma
      std::vector<int> gammas = SB::getOption<std::vector<int>>(*ops, "DBMetaData/gammas");

      // Read mass
      std::string mass = SB::getOption<std::string>(*ops, "DBMetaData/mass_label");

      // Read phasing
      std::vector<std::array<int, 3>> phasings;
      if (format_version == 0)
      {
	phasings.push_back(SB::getOption<std::array<int, 3>>(*ops, "DBMetaData/quarkPhase"));
	phasings.push_back(Datalib::TensorStorage::detail::negate(
	  SB::getOption<std::array<int, 3>>(*ops, "DBMetaData/aQuarkPhase")));
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

      return Storage{format_version, stoh, order, kvdim, disps, moms, phasings, gammas, mass};
    }

    void keys_sb(const Storage& store,
		 std::vector<SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t>>& keys) const
    {
      if (store.format_version == 0)
      {
	std::vector<superbblas::PartitionItem<N>> blocks;
	superbblas::get_blocks<N, N, T>(
	  store.stoh, store.order.c_str(), store.order.c_str(), superbblas::Coor<N>{{}},
	  detail::kvcoors<N>(store.order, store.dim), blocks, superbblas::FastToSlow);
	SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t> key;
	key.key().mass = store.mass;
	int D = store.dim.at('d');
	int M = store.dim.at('m');
	int G = store.dim.at('g');
	int T = store.dim.at('t');
	for (const auto& b : blocks)
	{
	  const auto kvfrom = detail::to_kvdim(store.order, b[0]);
	  const auto kvsize = detail::to_kvdim(store.order, b[1]);
	  for (unsigned int d = 0; d < kvsize.at('d'); ++d)
	  {
	    for (unsigned int m = 0; m < kvsize.at('m'); ++m)
	    {
	      for (unsigned int g = 0; g < kvsize.at('g'); ++g)
	      {
		for (unsigned int p = 0; p < kvsize.at('p'); ++p)
		{
		  for (unsigned int t = 0; t < kvsize.at('t'); ++t)
		  {
		    for (unsigned int P = 0; P < kvsize.at('P'); ++P)
		    {
		      key.key().displacement = store.disps.at((kvfrom.at('d') + d) % D);
		      key.key().mom = detail::toXMLArray(store.moms.at((kvfrom.at('m') + m) % M));
		      key.key().phasing_source = detail::toXMLArray(store.phasings.at(0));
		      key.key().phasing_sink =
			Datalib::TensorStorage::detail::negate(detail::toXMLArray(store.phasings.at(1)));
		      key.key().g = store.gammas.at((kvfrom.at('g') + g) % G);
		      key.key().t_source = (kvfrom.at('p') + p) % T;
		      key.key().t_slice = (kvfrom.at('t') + t) % T;
		      key.key().t_sink = (kvfrom.at('P') + P) % T;
		      keys.push_back(key);
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
      else
      {
	std::vector<superbblas::PartitionItem<N+2>> blocks;
	superbblas::get_blocks<N+2, N+2, T>(
	  store.stoh, store.order.c_str(), store.order.c_str(), superbblas::Coor<N+2>{{}},
	  detail::kvcoors<N+2>(store.order, store.dim), blocks, superbblas::FastToSlow);
	SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t> key;
	key.key().mass = store.mass;
	int D = store.dim.at('d');
	int M = store.dim.at('m');
	int G = store.dim.at('g');
	int T = store.dim.at('t');
	int PH = store.dim.at('h');
	for (const auto& b : blocks)
	{
	  const auto kvfrom = detail::to_kvdim(store.order, b[0]);
	  const auto kvsize = detail::to_kvdim(store.order, b[1]);
	  for (unsigned int d = 0; d < kvsize.at('d'); ++d)
	  {
	    for (unsigned int m = 0; m < kvsize.at('m'); ++m)
	    {
	      for (unsigned int g = 0; g < kvsize.at('g'); ++g)
	      {
		for (unsigned int p = 0; p < kvsize.at('p'); ++p)
		{
		  for (unsigned int t = 0; t < kvsize.at('t'); ++t)
		  {
		    for (unsigned int P = 0; P < kvsize.at('P'); ++P)
		    {
		      for (unsigned int h = 0; h < kvsize.at('h'); ++h)
		      {
			for (unsigned int H = 0; H < kvsize.at('H'); ++H)
			{
			  key.key().displacement = store.disps.at((kvfrom.at('d') + d) % D);
			  key.key().mom =
			    detail::toXMLArray(store.moms.at((kvfrom.at('m') + m) % M));
			  key.key().phasing_source =
			    detail::toXMLArray(store.phasings.at((kvfrom.at('h') + h) % PH));
			  key.key().phasing_sink = Datalib::TensorStorage::detail::negate(
			    detail::toXMLArray(store.phasings.at((kvfrom.at('H') + H) % PH)));
			  key.key().g = store.gammas.at((kvfrom.at('g') + g) % G);
			  key.key().t_source = (kvfrom.at('p') + p) % T;
			  key.key().t_slice = (kvfrom.at('t') + t) % T;
			  key.key().t_sink = (kvfrom.at('P') + P) % T;
			  keys.push_back(key);
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }

    bool get_sb(const Storage& store, const Hadron::KeyGenProp4ElementalOperator_t& key,
		const Coor4& val_from, const Coor4& val_size, const std::string& val_order,
		std::complex<double>* val_ptr) const
    {
      detail::Tracker t_("genprops get key-val");

      // Check that `val_order` is a permutation of "nNsq"
      {
	auto sort_string = [](const std::string& s) {
	  auto r = s;
	  std::sort(r.begin(), r.end());
	  return r;
	};
	if (sort_string(val_order) != sort_string("nNsq"))
	  throw std::runtime_error("get_sb: invalid `val_order`");
      }

      using namespace detail;

      // Find key
      int n = store.dim.at('n');
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

      auto it_gammas = std::find(store.gammas.begin(), store.gammas.end(), key.g);
      if (it_gammas == store.gammas.end())
	return false;
      int gamma_idx = it_gammas - store.gammas.begin();

      if (key.mass != store.mass)
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
      std::array<std::array<int, 4>, 2> val_partition{{val_from, val_size}};
      auto cpu = cpu_ctx.toCpu(0);
      val_ptr[0] = NaN<T>::get();

      auto to_map = [&](const Coor4& coor) {
	std::map<char, int> r;
	for (std::size_t i = 0; i < 4; ++i)
	  r[val_order[i]] = coor[i];
	return r;
      };
      const auto& mfrom = to_map(val_from);
      const auto& msize = to_map(val_size);

      if (store.format_version == 0)
      {
	auto store_from = kvcoors<N>(store.order, {{'n', mfrom.at('n')},
						   {'N', mfrom.at('N')},
						   {'s', mfrom.at('s')},
						   {'q', mfrom.at('q')},
						   {'d', disp_idx},
						   {'m', mom_idx},
						   {'g', gamma_idx},
						   {'p', key.t_source},
						   {'t', key.t_slice},
						   {'P', key.t_sink}});
	auto store_size = kvcoors<N>(store.order,
				     {{'n', msize.at('n')},
				      {'N', msize.at('N')},
				      {'s', msize.at('s')},
				      {'q', msize.at('q')},
				      {'d', 1},
				      {'g', 1}},
				     1);
#  if SUPERBBLAS_VERSION < 2
	superbblas::load<N, 4, T, T>(T{1}, store.stoh, store.order.c_str(), store_from, store_size,
				     &val_partition, 1, val_order.c_str(), val_from, &val_ptr,
				     &cpu_ctx, superbblas::FastToSlow, superbblas::Copy);
#  else
	superbblas::load<N, 4, T, T>(T{1}, store.stoh, store.order.c_str(), store_from, store_size,
				     &val_partition, 1, val_order.c_str(), val_from, val_size,
				     &val_ptr, &cpu_ctx, superbblas::FastToSlow, superbblas::Copy);
#  endif
      }
      else
      {
	auto store_from = kvcoors<N + 2>(store.order, {{'n', mfrom.at('n')},
						       {'N', mfrom.at('N')},
						       {'s', mfrom.at('s')},
						       {'q', mfrom.at('q')},
						       {'d', disp_idx},
						       {'m', mom_idx},
						       {'h', phasing_source_idx},
						       {'H', phasing_sink_idx},
						       {'g', gamma_idx},
						       {'p', key.t_source},
						       {'t', key.t_slice},
						       {'P', key.t_sink}});
	auto store_size = kvcoors<N + 2>(store.order,
					 {{'n', msize.at('n')},
					  {'N', msize.at('N')},
					  {'s', msize.at('s')},
					  {'q', msize.at('q')},
					  {'d', 1},
					  {'g', 1}},
					 1);
#  if SUPERBBLAS_VERSION < 2
	superbblas::load<N + 2, 4, T, T>(T{1}, store.stoh, store.order.c_str(), store_from,
					 store_size, &val_partition, 1, val_order.c_str(), val_from,
					 &val_ptr, &cpu_ctx, superbblas::FastToSlow,
					 superbblas::Copy);
#  else
	superbblas::load<N + 2, 4, T, T>(T{1}, store.stoh, store.order.c_str(), store_from,
					 store_size, &val_partition, 1, val_order.c_str(), val_from,
					 val_size, &val_ptr, &cpu_ctx, superbblas::FastToSlow,
					 superbblas::Copy);
#  endif
      }

      // If the keys was not found, val_ptr[0] will be nan
      if (NaN<T>::is(val_ptr[0]))
	return false;

      return true;
    }

    bool get_sb(const Storage& store,
		const SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t>& key,
		SerialDBData<Hadron::ValGenProp4ElementalOperator_t>& val) const
    {
      int n = store.dim.at('n');
      if (n == 0)
	return false;

      // NOTE: val is Array4D with dims: colorvec_snk, colorvec_src, spin_snk, spin_src (slowest index to fastest index)
      val.data().op.resize(n, n, Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count);
      std::complex<double>* ptr = (std::complex<double>*)val.data().op;
      return get_sb(store, key.key(), Coor4{{}}, Coor4{Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count, n, n}, "sqnN", ptr);
    }
#endif

    static OldStorage open_db(const std::string& filename_with_suffix, bool& is_db4, int& Nt)
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
      Nt = SB::getOption<std::array<int, 4>>(*ops, "DBMetaData/lattSize")[3];
      if (dtype == std::string("unsmearedMesonElemOp"))
      {
	is_db4 = false;
      }
      else if (dtype == std::string("genprop4ElemOp"))
      {
	is_db4 = true;
      }
      else
      {
	throw std::runtime_error("unsupported genprop type in file: " + filename);
      }

      return {db, default_phasing.size() == 0 ? XMLArray::Array<int>(std::vector<int>{0, 0, 0}) : default_phasing};
    }

    static void keys_db2(const OldStorage& sto,
			 std::vector<SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t>>& keys,
			 int Nt)
    {
      std::vector<SerialDBKey<Hadron::KeyUnsmearedMesonElementalOperator_t>> keys0;
      sto.db.keys<SerialDBKey<Hadron::KeyUnsmearedMesonElementalOperator_t>,
		  SerialDBKey<Hadron::ValUnsmearedMesonElementalOperator_t>>(keys0);
      SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t> key;
      for (const auto& key0 : keys0)
      {
	if (key0.key().colorvec_src == 0)
	{
	  key.key().displacement = key0.key().displacement;
	  key.key().mom = key0.key().mom;
	  key.key().g = key0.key().gamma;
	  key.key().t_source = key0.key().t_source;
	  // NOTE: bug in genprop code may produce negative t_slice keys
	  key.key().t_slice = detail::normalize_coor(key0.key().t_slice, Nt);
	  key.key().t_sink = key0.key().t_sink;
	  key.key().mass = key0.key().mass;
	  key.key().phasing_source = sto.default_phasing;
	  key.key().phasing_sink = -sto.default_phasing;
	  keys.push_back(key);
	}
      }
    }

    static bool get_db2(const OldStorage& sto,
			const SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t>& key,
			SerialDBData<Hadron::ValGenProp4ElementalOperator_t>& val, int Nt)
    {

      // Old databases don't support phasing
      if (key.key().phasing_source != sto.default_phasing ||
	  key.key().phasing_sink != -sto.default_phasing)
	return false;

      // Prepare key
      SerialDBKey<Hadron::KeyUnsmearedMesonElementalOperator_t> key0;
      key0.key().t_sink = key.key().t_sink;
      key0.key().t_slice = key.key().t_slice;
      key0.key().t_source = key.key().t_source;
      key0.key().gamma = key.key().g;
      key0.key().displacement = key.key().displacement;
      key0.key().mom = key.key().mom;
      key0.key().derivP = 0;
      key0.key().mass = key.key().mass;

      // Array3D with dims colorvec_snk, spin_snk, spin_src (slowest index to fastest index)
      SerialDBData<Hadron::ValUnsmearedMesonElementalOperator_t> val0;

      // NOTE: val is Array4D with dims: colorvec_snk, colorvec_src, spin_snk, spin_src (slowest index to fastest index)

      int n = 1;
      for (int colorvec_src = 0; colorvec_src < n; ++colorvec_src)
      {
	key0.key().colorvec_src = colorvec_src;
	// NOTE: bug in genprop code may produce negative t_slice keys
	for (int attempt = 0; attempt < 2; attempt++)
	{
	  if (sto.db.get(key0, val0) != 0)
	  {
	    if (attempt == 0)
	      key0.key().t_slice -= Nt;
	    else
	      return false;
	  } else {
	    break;
	  }
	}

	if (colorvec_src == 0)
	{
	  n = val0.data().op.size3();
	  val.data().op.resize(n, n, Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count);
	}

	for (int colorvec_snk = 0; colorvec_snk < n; ++colorvec_snk)
	  for (int s_snk = 0; s_snk < Datalib::TensorStorage::spin_count; ++s_snk)
	    for (int s_src = 0; s_src < Datalib::TensorStorage::spin_count; ++s_src)
	      val.data().op(colorvec_snk, colorvec_src, s_snk, s_src) =
		val0.data().op(colorvec_snk, s_snk, s_src);
      }

      return true;
    }

    static void keys_db4(const OldStorage& sto,
			 std::vector<SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t>>& keys,
			 int Nt)
    {
      std::vector<SerialDBKey<Hadron::OldKeyGenProp4ElementalOperator_t>> old_keys;
      sto.db.keys<SerialDBKey<Hadron::OldKeyGenProp4ElementalOperator_t>,
		  SerialDBData<Hadron::ValGenProp4ElementalOperator_t>>(old_keys);
      keys.reserve(keys.size() + old_keys.size());
      for (const auto& old_key : old_keys)
      {
	Hadron::KeyGenProp4ElementalOperator_t key;
	key.t_sink = old_key.key().t_sink;
	// NOTE: bug in genprop code may produce negative t_slice keys
	key.t_slice = detail::normalize_coor(old_key.key().t_slice, Nt);
	key.t_source = old_key.key().t_source;
	key.g = old_key.key().g;
	key.displacement = old_key.key().displacement;
	key.mom = old_key.key().mom;
	key.mass = old_key.key().mass;
	key.phasing_source = sto.default_phasing;
	key.phasing_sink = -sto.default_phasing;
	keys.push_back(key);
      }
    }

    static bool get_db4(const OldStorage& sto,
			const SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t>& key,
			SerialDBData<Hadron::ValGenProp4ElementalOperator_t>& val, int Nt)
    {
      // Old databases don't support phasing
      if (key.key().phasing_source != sto.default_phasing ||
	  key.key().phasing_sink != -sto.default_phasing)
	return false;

      Hadron::OldKeyGenProp4ElementalOperator_t old_key;
      old_key.t_sink = key.key().t_sink;
      old_key.t_slice = key.key().t_slice;
      old_key.t_source = key.key().t_source;
      old_key.g = key.key().g;
      old_key.displacement = key.key().displacement;
      old_key.mom = key.key().mom;
      old_key.mass = key.key().mass;

      if (sto.db.get(SerialDBKey<Hadron::OldKeyGenProp4ElementalOperator_t>(old_key), val) == 0)
	return true;
      // NOTE: bug in genprop code may produce negative t_slice keys
      old_key.t_slice -= Nt;
      return sto.db.get(SerialDBKey<Hadron::OldKeyGenProp4ElementalOperator_t>(old_key), val) == 0;
    }

    bool get_default_for_tslice_outside_sink_source(
      const SerialDBKey<Hadron::KeyGenProp4ElementalOperator_t>& key,
      SerialDBData<Hadron::ValGenProp4ElementalOperator_t>& val) const
    {
#ifdef USE_SUPERBBLAS
      // Get the number of vectors and the time lattice size from the files
      if (sb_stores.size() == 0)
	return false;
      int n = (num_vecs >= 0 ? num_vecs : sb_stores.at(0).dim.at('n'));
      const int Nt = sb_stores.at(0).dim.at('t');
      if (n == 0 || Nt == 0)
	return false;

      int t_source = key.key().t_source;
      int t_slice = key.key().t_slice;
      int t_sink = key.key().t_sink;
      auto dist = [&](int a, int b) { return (b >= a ? b - a : b + Nt - a); };

      // Canonize (t_sink, t_source) such that from t_source to t_sink is shorter than
      // from t_sink to t_source
      if (dist(t_source, t_sink) < dist(t_sink, t_source))
	std::swap(t_source, t_sink);

      // If the distance between the t_source and t_slice is smaller than between
      // t_source and t_sink, then t_slice is between t_source and t_sink, so
      // return false
      if (dist(t_slice, t_source) <= dist(t_sink, t_source))
	return false;

      val.data().op.resize(n, n, Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count);
      T* v = (T*)val.data().op;
#  ifdef _OPENMP
#    pragma omp parallel for schedule(static)
#  endif
      for (std::size_t i = 0; i < (std::size_t)n * n * Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count; ++i)
	v[i] = T{0};
      return true;
#else
      (void)key;
      (void)val;
      return false;
#endif
    }

    void get_default(T* v, std::size_t vol) const
    {
#ifdef _OPENMP
#  pragma omp parallel for schedule(static)
#endif
      for (std::size_t i = 0; i < vol; ++i)
	v[i] = T{0};
    }

    void get_default(SerialDBData<Hadron::ValGenProp4ElementalOperator_t>& val,
		     const Hadron::KeyGenProp4ElementalOperator_t& key) const
    {
      if (num_vecs < 0)
	throw std::runtime_error("number of vectors not set!");
      val.data().op.resize(num_vecs, num_vecs, Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count);

      // Generate a \gamma_5-Hermitian matrix by multiplying \gamma_5 to a Hermitian matrix
      int t_source = key.t_source;
      int t_sink = key.t_sink;
      int h_source = SB::getHash(Datalib::TensorStorage::detail::toCoor(key.phasing_source)) % 8ul;
      int h_sink =
	SB::getHash(Datalib::TensorStorage::detail::negate(Datalib::TensorStorage::detail::toCoor(key.phasing_sink))) % 8ul;
      int h_med = SB::getHash(std::make_tuple(Datalib::TensorStorage::detail::toCoor(key.mom), key.displacement,
					      key.g, key.t_slice)) %
		  128ul;
      int h_neg_med =
	SB::getHash(std::make_tuple(
	  Datalib::TensorStorage::detail::negate(Datalib::TensorStorage::detail::toCoor(key.mom)),
	  Datalib::TensorStorage::detail::negate(Datalib::TensorStorage::detail::reverse(key.displacement)), key.g, key.t_slice)) %
	128ul;

      const auto get_val = [&](int s0, int n0, int t0, int h0, int s1, int n1, int t1, int h1,
			       int h_med) {
	auto h = s0 +						      //
		 n0 * Datalib::TensorStorage::spin_count +					      //
		 (t0 % 16) * Datalib::TensorStorage::spin_count * num_vecs +			      //
		 h0 * Datalib::TensorStorage::spin_count * num_vecs * 16 +			      //
		 s1 * Datalib::TensorStorage::spin_count * num_vecs * 16 * 8 +			      //
		 n1 * Datalib::TensorStorage::spin_count * num_vecs * 16 * 8 * Datalib::TensorStorage::spin_count +		      //
		 (t1 % 16) * Datalib::TensorStorage::spin_count * num_vecs * 16 * 8 * Datalib::TensorStorage::spin_count * num_vecs + //
		 h1 * Datalib::TensorStorage::spin_count * num_vecs * 16 * 8 * Datalib::TensorStorage::spin_count * num_vecs * 16 +   //
		 h_med;
	return T{(double)(h % 128), (double)(h % 57)};
      };

#ifdef _OPENMP
#  pragma omp parallel for schedule(static)
#endif
      for (int i = 0; i < num_vecs; ++i)
	for (int j = 0; j < num_vecs; ++j)
	  for (int k = 0; k < Datalib::TensorStorage::spin_count; ++k)
	    for (int l = 0; l < Datalib::TensorStorage::spin_count; ++l)
	      val.data().op(i, j, k, l) =
		(get_val(l, j, t_source, h_source, k, i, t_sink, h_sink, h_med) +
		 std::conj(get_val(k, i, t_sink, h_sink, l, j, t_source, h_source, h_neg_med))) *
		(k / 2 == 0 ? 1.0 : -1.0);
    }
  };
}

#endif
