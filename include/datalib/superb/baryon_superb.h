// -*- C++ -*-
/*! \file
 * \brief Baryon elemental superbblas database
 */

#ifndef __redstar_datalib_baryon_superbblas_h__
#define __redstar_datalib_baryon_superbblas_h__

#include "datalib/superb/types.h"
#include "ConfDataStoreDBSimple.h"
#include "datalib/superb/colorvec_matelem_type.h"
#include "datalib/superb/baryon_elem_type.h"
#include "datalib/storage/filedb.h"
#include "datalib/superb/superb_aux.h"
#include <memory>
#include <map>
#include <xml_array.h>
#include <xml_array4d.h>

namespace Hadron
{
  using namespace XMLArray;

  //! Key for Baryon operator (old version)
  struct OldKeyBaryonElementalOperator_t {
    int t_slice;	     /*!< Baryon operator time slice */
    std::vector<int> left;   /*!< Displacement dirs of left colorvector */
    std::vector<int> middle; /*!< Displacement dirs of middle colorvector */
    std::vector<int> right;  /*!< Displacement dirs of right colorvector */
    Array<int> mom;	     /*!< D-1 momentum of this operator */
  };

  inline void read(BinaryReader& bin, OldKeyBaryonElementalOperator_t& param)
  {
    read(bin, param.t_slice);
    read(bin, param.left);
    read(bin, param.middle);
    read(bin, param.right);
    read(bin, param.mom);
  }

  inline void write(BinaryWriter& bin, const OldKeyBaryonElementalOperator_t& param)
  {
    write(bin, param.t_slice);
    write(bin, param.left);
    write(bin, param.middle);
    write(bin, param.right);
    write(bin, param.mom);
  }
}

namespace Datalib::TensorStorage
{
  //! ConfDataStoreMultipleDB compatible class supporting the two baryon storage formats

  class StorageBaryon
  {
  public:
    using T = std::complex<double>; // storage number type

    bool use_cache; ///< whether to use cache
    int num_vecs;   ///< distillation basis rank
    bool is_dummy;  ///< whether can produce fake data

#ifdef USE_SUPERBBLAS
    static constexpr std::size_t N = std::size_t{6}; ///< expected storage size
    enum GetOrStore { Get, Store };		     ///< Used by get_or_store_key_from_cache
    using Disps = std::vector<std::array<std::vector<int>, 3>>;

    struct Storage {
      unsigned int format_version;			  // 0: single phasing, 1: multiple phasing
      superbblas::Storage_handle stoh;			  // storage handle
      std::string order;				  // storage order
      std::map<char, int> dim;				  // dimensions
      Disps disps;					  // left, middle, right displacements
      std::vector<std::array<int, 3>> moms;		  // momenta
      std::vector<std::array<int, 3>> phasings;		  // phasings
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

    StorageBaryon(bool use_cache = true)
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

    ~StorageBaryon()
    {
      close();
    }

    /**
     * Open a list of files with baryons
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
	  superbblas::close_storage<N + 1, T>(it.stoh);
      }
      sb_stores.clear();
#endif
      db_stores.clear();
      return 0;
    }

    /**
     * Return all available baryon keys regardless of the underneath file formats
     * @param keys: user supplied an empty vector which is populated
     * by keys after this call.
     */
    void keys(std::vector<SerialDBKey<Hadron::KeyBaryonElementalOperator_t>>& keys) const
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

    int get(const SerialDBKey<Hadron::KeyBaryonElementalOperator_t>& key,
	    SerialDBData<Hadron::ValBaryonElementalOperator_t>& val) const
    {

#ifdef USE_SUPERBBLAS
      // Try to read from cache
      if (get_or_store_key_from_cache(key, val, Get))
	return 0;

      // Try to read from superbblas files
      for (const auto& store : sb_stores) {
	if (get_sb(store, key, val))
	{
	  get_or_store_key_from_cache(key, val, Store);
	  return 0;
	}
      }
#endif

      // Try to read from filehash databases
      for (const auto& store : db_stores)
      {
	if (get_db(store, key, val))
	{
#ifdef USE_SUPERBBLAS
	  get_or_store_key_from_cache(key, val, Store);
#endif
	  return 0;
	}
      }

      if (is_dummy)
      {
	get_default(val, key.key());
	return 0;
      }

      return -1;
    }

  private:
#ifdef USE_SUPERBBLAS
    static Storage open_sb(const std::string& filename_with_suffix)
    {
      detail::Tracker t_("baryons open db");

      // Check for the use of default phasing
      const auto& [filename, default_phasing] =
	detail::get_filename_and_phasing(filename_with_suffix);
      std::vector<char> metadatav;
      std::vector<superbblas::IndexType> dimv;
      superbblas::values_datatype values_dtype;
      superbblas::read_storage_header(filename.c_str(), superbblas::FastToSlow, values_dtype,
				      metadatav, dimv);

      // Check that storage tensor dimension and value type match template arguments
      if (dimv.size() != N && dimv.size() != N + 1)
	throw std::runtime_error(
	  "Unexpected storage tensor dimension for baryons");
      if (superbblas::detail::get_values_datatype<T>() != values_dtype)
	throw std::runtime_error("Unexpected storage type for baryons");
      unsigned int format_version = (dimv.size() == N ? 0 : 1);

      //
      // Read options
      //
      std::shared_ptr<SB::Options> ops =
	SB::getOptionsFromXML(std::string(metadatav.begin(), metadatav.end()));

      std::string dtype = SB::getOption<std::string>(*ops, "DBMetaData/id");
      if (dtype != std::string("baryonElemOpSuperb"))
      {
	throw std::runtime_error("unsupported baryon type in file: " + filename);
      }

      // Read order; letter meaning:
      /// i,j,k: eigenvector indices
      /// t: time slice
      /// d: the left, middle and right displacements
      /// m: momentum
      std::string order = SB::getOption<std::string>(*ops, "DBMetaData/tensorOrder");

      // Read derivatives
      std::vector<std::array<std::vector<int>, 3>> disps =
	SB::getOption<std::vector<std::array<std::vector<int>, 3>>>(
	  *ops, "DBMetaData/displacements_left_middle_right");

      // Read momenta
      std::vector<std::array<int, 3>> moms =
	SB::getOption<std::vector<std::array<int, 3>>>(*ops, "DBMetaData/moms");

      // Read phasing
      std::vector<std::array<int, 3>> phasings;
      if (format_version == 0)
      {
	if (default_phasing.size() == 0)
	{
	  phasings.push_back(SB::getOption<std::array<int, 3>>(*ops, "DBMetaData/phase"));
	}
	else
	{
	  phasings.push_back(Datalib::TensorStorage::detail::toCoor(default_phasing));
	}
      }
      else
      {
	const auto& xml_phasings = SB::getOption<std::vector<std::array<int, 3>>>(*ops, "DBMetaData/phasings");
 	if (default_phasing.size() == 0)
	{
	  phasings = xml_phasings;
	}
	else
	{
	  if (xml_phasings.size() > 1)
	    throw std::runtime_error(
	      "Unsupported use of the suffix `@phase=x_y_z' for files with several phases");
	  phasings.push_back(Datalib::TensorStorage::detail::toCoor(default_phasing));
	}
      }

      // Open storage
      superbblas::Storage_handle stoh;
      if (format_version == 0)
      {
	superbblas::open_storage<N, T>(filename.c_str(), false /* don't allow writing */, &stoh);
      }
      else
      {
	superbblas::open_storage<N + 1, T>(filename.c_str(), false /* don't allow writing */,
					   &stoh);
      }

      // Create dictionary with the dimensions
      std::map<char, int> kvdim = detail::to_kvdim(order, dimv);

      return Storage{format_version, stoh, order, kvdim, disps, moms, phasings};
    }

    void keys_sb(const Storage& store,
		 std::vector<SerialDBKey<Hadron::KeyBaryonElementalOperator_t>>& keys) const
    {
      if (store.format_version == 0)
      {
	std::vector<superbblas::PartitionItem<N>> blocks;
	superbblas::get_blocks<N, N, T>(
	  store.stoh, store.order.c_str(), store.order.c_str(), superbblas::Coor<N>{{}},
	  detail::kvcoors<N>(store.order, store.dim), blocks, superbblas::FastToSlow);
	SerialDBKey<Hadron::KeyBaryonElementalOperator_t> key;
	int T = store.dim.at('t');
	int M = store.dim.at('m');
	int D = store.dim.at('d');
	for (const auto& b : blocks)
	{
	  const auto kvfrom = detail::to_kvdim(store.order, b[0]);
	  const auto kvsize = detail::to_kvdim(store.order, b[1]);
	  for (unsigned int d = 0; d < kvsize.at('d'); ++d)
	  {
	    for (unsigned int m = 0; m < kvsize.at('m'); ++m)
	    {
	      for (unsigned int t = 0; t < kvsize.at('t'); ++t)
	      {
		key.key().left = store.disps.at((kvfrom.at('d') + d) % D)[0];
		key.key().middle = store.disps.at((kvfrom.at('d') + d) % D)[1];
		key.key().right = store.disps.at((kvfrom.at('d') + d) % D)[2];
		key.key().mom = detail::toXMLArray(store.moms.at((kvfrom.at('m') + m) % M));
		key.key().phasing = detail::toXMLArray(store.phasings.at(0));
		key.key().t_slice = (kvfrom.at('t') + t) % T;
		keys.push_back(key);
	      }
	    }
	  }
	}
      }
      else
      {
	std::vector<superbblas::PartitionItem<N + 1>> blocks;
	superbblas::get_blocks<N + 1, N + 1, T>(
	  store.stoh, store.order.c_str(), store.order.c_str(), superbblas::Coor<N + 1>{{}},
	  detail::kvcoors<N + 1>(store.order, store.dim), blocks, superbblas::FastToSlow);
	int PH = store.dim.at('h');
	int T = store.dim.at('t');
	int M = store.dim.at('m');
	int D = store.dim.at('d');
	SerialDBKey<Hadron::KeyBaryonElementalOperator_t> key;
	for (const auto& b : blocks)
	{
	  const auto kvfrom = detail::to_kvdim(store.order, b[0]);
	  const auto kvsize = detail::to_kvdim(store.order, b[1]);
	  for (unsigned int d = 0; d < kvsize.at('d'); ++d)
	  {
	    for (unsigned int m = 0; m < kvsize.at('m'); ++m)
	    {
	      for (unsigned int h = 0; h < kvsize.at('h'); ++h)
	      {
		for (unsigned int t = 0; t < kvsize.at('t'); ++t)
		{
		  key.key().left = store.disps.at((kvfrom.at('d') + d) % D)[0];
		  key.key().middle = store.disps.at((kvfrom.at('d') + d) % D)[1];
		  key.key().right = store.disps.at((kvfrom.at('d') + d) % D)[2];
		  key.key().mom = detail::toXMLArray(store.moms.at((kvfrom.at('m') + m) % M));
		  key.key().phasing =
		    detail::toXMLArray(store.phasings.at((kvfrom.at('h') + h) % PH));
		  key.key().t_slice = (kvfrom.at('t') + t) % T;
		  keys.push_back(key);
		}
	      }
	    }
	  }
	}
      }
    }

    /// Return the displacement matching in database and the indices permutation and the sign
    /// \param disp: requested displacement
    /// \return std::tuple{disp, perm, sign}:
    ///  - disp: iterator to an existing displacement in the database
    ///  - perm: permutation to apply to the returning 3d tensor
    ///  - sign: sign to apply to the returning 3d tensor

    static std::tuple<Disps::const_iterator, std::array<int, 3>, int>
    get_permutation(const Disps& disps, const std::array<std::vector<int>, 3>& disp)
    {
      // List all permutation of three elements; and the sign is the evenness of the number of single exchanges
      std::vector<std::pair<std::array<int, 3>, int>> perm_and_sign_list{
	{std::array<int, 3>{0, 1, 2}, 1},  //
	{std::array<int, 3>{0, 2, 1}, -1}, //
	{std::array<int, 3>{1, 0, 2}, -1}, //
	{std::array<int, 3>{1, 2, 0}, 1},  //
	{std::array<int, 3>{2, 0, 1}, 1},  //
	{std::array<int, 3>{2, 1, 0}, -1}  //
      };
      for (const auto& it : perm_and_sign_list)
      {
	const auto& perm = it.first;
	const auto& sign = it.second;
	std::array<int, 3> perm_rev;
	for (int i = 0; i < 3; ++i)
	  perm_rev[perm[i]] = i;
	auto it_disps =
	  std::find(disps.begin(), disps.end(),
		    std::array<std::vector<int>, 3>{disp[perm_rev[0]], disp[perm_rev[1]], disp[perm_rev[2]]});
	if (it_disps != disps.end())
	  return {it_disps, perm, sign};
      }
      return {disps.end(), {{}}, 0};
    }

    bool get_sb(const Storage& store, const SerialDBKey<Hadron::KeyBaryonElementalOperator_t>& key,
		SerialDBData<Hadron::ValBaryonElementalOperator_t>& val) const
    {
      detail::Tracker t_("baryons get key-val");

      using namespace detail;

      // Find key
      const auto& this_key = key.key();

      if (num_vecs >= 0 && num_vecs > store.dim.at('i'))
	return false;

      int n = (num_vecs < 0 ? store.dim.at('i') : num_vecs);
      if (n == 0)
	return false;

      auto [it_disps, perm, sign] =
	get_permutation(store.disps, std::array<std::vector<int>, 3>{this_key.left, this_key.middle,
								     this_key.right});
      if (it_disps == store.disps.end())
	return false;
      int disp_idx = it_disps - store.disps.begin();

      auto it_moms = std::find(store.moms.begin(), store.moms.end(), toCoor(this_key.mom));
      if (it_moms == store.moms.end())
	return false;
      int mom_idx = it_moms - store.moms.begin();

      // If no phasing restrictions is given but multiple phases are stored, just not use this file
      if (this_key.phasing.size() == 0 && store.phasings.size() != 1)
	return false;
      int phasing_idx = 0;
      if (this_key.phasing.size() > 0)
      {
	auto it_phasing =
	  std::find(store.phasings.begin(), store.phasings.end(), toCoor(this_key.phasing));
	if (it_phasing == store.phasings.end())
	  return false;
	phasing_idx = it_phasing - store.phasings.begin();
      }

      // NOTE: val is Array3D with dims ijk (slowest index to fastest index), so
      // k is right, j is middle, and i is left
      val.data().type_of_data = COLORVEC_MATELEM_TYPE_GENERIC;
      val.data().op.resize(n, n, n);
      std::array<char, 3> indices{'i', 'j', 'k'};
      std::string val_order_str{indices[perm[2]], indices[perm[1]], indices[perm[0]]}; // from fast to slow (right to left)
      std::array<int, 3> val_from{{}};
      std::array<int, 3> val_size{{n, n, n}};
      std::array<std::array<int, 3>, 2> val_partition{{val_from, val_size}};
      T* val_ptr = (T*)val.data().op;
      val_ptr[0] = NaN<T>::get();

      if (store.format_version == 0)
      {
	auto store_from =
	  kvcoors<N>(store.order, {{'d', disp_idx}, {'m', mom_idx}, {'t', this_key.t_slice}});
	auto store_size = kvcoors<N>(store.order, {{'i', n}, {'j', n}, {'k', n}}, 1);
#  if SUPERBBLAS_VERSION < 2
	superbblas::load<N, 3, T, T>((T)sign, store.stoh, store.order.c_str(), store_from,
				     store_size, &val_partition, 1, val_order_str.c_str(), val_from,
				     &val_ptr, &cpu_ctx, superbblas::FastToSlow, superbblas::Copy);
#  else
	superbblas::load<N, 3, T, T>((T)sign, store.stoh, store.order.c_str(), store_from,
				     store_size, &val_partition, 1, val_order_str.c_str(), val_from,
				     val_size, &val_ptr, &cpu_ctx, superbblas::FastToSlow,
				     superbblas::Copy);
#  endif
      }
      else
      {
	auto store_from = kvcoors<N + 1>(
	  store.order,
	  {{'d', disp_idx}, {'m', mom_idx}, {'h', phasing_idx}, {'t', this_key.t_slice}});
	auto store_size = kvcoors<N + 1>(store.order, {{'i', n}, {'j', n}, {'k', n}}, 1);
#  if SUPERBBLAS_VERSION < 2
	superbblas::load<N + 1, 3, T, T>((T)sign, store.stoh, store.order.c_str(), store_from,
					 store_size, &val_partition, 1, val_order_str.c_str(),
					 val_from, &val_ptr, &cpu_ctx, superbblas::FastToSlow,
					 superbblas::Copy);
#  else
	superbblas::load<N + 1, 3, T, T>((T)sign, store.stoh, store.order.c_str(), store_from,
					 store_size, &val_partition, 1, val_order_str.c_str(),
					 val_from, val_size, &val_ptr, &cpu_ctx,
					 superbblas::FastToSlow, superbblas::Copy);
#  endif
      }

      // If the keys was not found, val_ptr[0] will be nan
      if (NaN<T>::is(val_ptr[0]))
	return false;

      return true;
    }

    bool get_or_store_key_from_cache(const SerialDBKey<Hadron::KeyBaryonElementalOperator_t>& key,
				     SerialDBData<Hadron::ValBaryonElementalOperator_t>& val,
				     GetOrStore get_or_store) const
    {
      if (!use_cache)
	return false;

      detail::Tracker t_(get_or_store == Get
                             ? "baryons get key-val from cache"
                             : "baryons store key-val from cache");

      using namespace detail;
      using timeSlice_disp_mom_phasing_t =
	std::tuple<void*, int,
		   std::array<superbblas::detail::vector<int, superbblas::detail::Cpu>, 3>,
		   std::array<int, 3>, std::vector<int>>;
      using value_t = std::tuple<int, superbblas::detail::vector<T, superbblas::detail::Cpu>>;

      auto cpu = cpu_ctx.toCpu(0);
      struct cache_tag {
      };
      auto cache =
	superbblas::detail::getCache<timeSlice_disp_mom_phasing_t, value_t,
				     superbblas::detail::TupleHash<timeSlice_disp_mom_phasing_t>,
				     cache_tag>(cpu);
      auto to_superb_vector = [&](const std::vector<int>& v) {
	superbblas::detail::vector<int, superbblas::detail::Cpu> r(v.size(), cpu);
	std::copy_n(v.data(), v.size(), r.data());
	return r;
      };
      timeSlice_disp_mom_phasing_t cache_key{cache_identity.get(),
					     key.key().t_slice,
					     {to_superb_vector(key.key().left),
					      to_superb_vector(key.key().middle),
					      to_superb_vector(key.key().right)},
					     toCoor(key.key().mom),
					     key.key().phasing};
      if (get_or_store == Get)
      {
	auto it = cache.find(cache_key);
	if (it == cache.end()) return false;
	int n = std::get<0>(it->second.value);
	auto v = std::get<1>(it->second.value);
	val.data().type_of_data = COLORVEC_MATELEM_TYPE_GENERIC;
	val.data().op.resize(n, n, n);
	T* val_ptr = (T*)val.data().op;
	superbblas::detail::copy_n<std::size_t, T, T>(T{1}, v.data(), v.ctx(), v.size(), val_ptr,
						      v.ctx(), superbblas::detail::EWOp::Copy{});
	return true;
      }
      else
      {
	int n = val.data().op.size1();
	superbblas::detail::vector<T, superbblas::detail::Cpu> v(n * n * n, cpu);
	T* val_ptr = (T*)val.data().op;
	superbblas::detail::copy_n<std::size_t, T, T>(T{1}, val_ptr, v.ctx(), v.size(), v.data(),
						      v.ctx(), superbblas::detail::EWOp::Copy{});
	cache.insert(cache_key, value_t{n, v}, superbblas::detail::storageSize(v));
	return true;
      }
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
      if (dtype != std::string("baryonElemOp"))
      {
	throw std::runtime_error("unsupported baryon type in file: " + filename);
      }

      return {db, default_phasing.size() == 0 ? XMLArray::Array<int>(std::vector<int>{0, 0, 0}) : default_phasing};
    }

    static void keys_db(const OldStorage& sto,
			std::vector<SerialDBKey<Hadron::KeyBaryonElementalOperator_t>>& keys)
    {
      std::vector<SerialDBKey<Hadron::OldKeyBaryonElementalOperator_t>> old_keys;
      sto.db.keys<SerialDBKey<Hadron::OldKeyBaryonElementalOperator_t>,
	      SerialDBData<Hadron::ValBaryonElementalOperator_t>>(old_keys);
      for (const auto& old_key : old_keys)
      {
	Hadron::KeyBaryonElementalOperator_t key;
	key.t_slice = old_key.key().t_slice;
	key.left = old_key.key().left;
	key.middle = old_key.key().middle;
	key.right = old_key.key().right;
	key.mom = old_key.key().mom;
	key.phasing = sto.default_phasing;
	keys.push_back(SerialDBKey<Hadron::KeyBaryonElementalOperator_t>(key));
      }
    }

    static bool get_db(const OldStorage& sto,
		       const SerialDBKey<Hadron::KeyBaryonElementalOperator_t>& key,
		       SerialDBData<Hadron::ValBaryonElementalOperator_t>& val)
    {
      // Old databases don't support phasing
      if (key.key().phasing != sto.default_phasing)
	return false;
      Hadron::OldKeyBaryonElementalOperator_t old_key;
      old_key.t_slice = key.key().t_slice;
      old_key.left = key.key().left;
      old_key.middle = key.key().middle;
      old_key.right = key.key().right;
      old_key.mom = key.key().mom;
      return sto.db.get(SerialDBKey<Hadron::OldKeyBaryonElementalOperator_t>(old_key), val) == 0;
    }

    void get_default(SerialDBData<Hadron::ValBaryonElementalOperator_t>& val,
		     const Hadron::KeyBaryonElementalOperator_t& key) const
    {
      if (num_vecs < 0)
	throw std::runtime_error("number of vectors not set!");

      // Properties:
      // a) op_{d0,d1,d2}({i,j,k}) = e(p) * op_{p{d0,d1,d2}}(p{i,j,k})

      val.data().type_of_data = COLORVEC_MATELEM_TYPE_GENERIC;
      val.data().op.resize(num_vecs, num_vecs, num_vecs);
      std::map<std::vector<int>, int> derivs;
      for (const auto& it : std::vector<std::vector<int>>{key.left, key.middle, key.right})
	derivs[it] = 0;
      int num_derivs = 0;
      for (auto& it : derivs)
	it.second = num_derivs++;
      int h = SB::getHash(std::make_tuple(key.t_slice, Datalib::TensorStorage::detail::toCoor(key.mom),
					  Datalib::TensorStorage::detail::toCoor(key.phasing), derivs)) %
	      128ul;

      const int I = derivs.at(key.left);
      const int J = derivs.at(key.middle);
      const int K = derivs.at(key.right);
      const auto lex_cmp = [](int I, int i, int J, int j) {
	if (I < J)
	  return -1;
	else if (I > J)
	  return 1;
	else if (i < j)
	  return -1;
	else if (i > j)
	  return 1;
	else
	  return 0;
      };
      const auto get_value = [&](std::array<int, 3> IJK, std::array<int, 3> ijk, int h) {
	// Record the number of permutations for sorting (I,i), (J,j) and (K,k).
	// If two elements are the same, return 0
	// NOTE: we bubble sort: compare (0,1), then (1,2), then (0,1)
	double s = 1; // track sign due to swaps
	for (const auto& it : std::array<std::array<int, 2>, 3>{
	       std::array<int, 2>{0, 1}, std::array<int, 2>{1, 2}, std::array<int, 2>{0, 1}})
	{
	  int i = it.at(0), j = it.at(1);
	  int si = lex_cmp(IJK.at(i), ijk.at(i), IJK.at(j), ijk.at(j));
	  if (si == 0)
	    return T{0};
	  if (si > 0)
	  {
	    std::swap(IJK.at(i), IJK.at(j));
	    std::swap(ijk.at(i), ijk.at(j));
	    s *= -1;
	  }
	}
	const auto hash_val = SB::getHash(std::make_tuple(h, IJK, ijk));
	return s * T{(double)(hash_val % 128ul), (double)(hash_val % 99ul)};
      };

#ifdef _OPENMP
#  pragma omp parallel for schedule(static)
#endif
      for (int i = 0; i < num_vecs; ++i)
	for (int j = 0; j < num_vecs; ++j)
	  for (int k = 0; k < num_vecs; ++k)
	    val.data().op(i, j, k) = get_value({I, J, K}, {i, j, k}, h);
    }
  };
}

#endif
