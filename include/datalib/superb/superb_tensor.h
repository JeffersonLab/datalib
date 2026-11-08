/*! \file
 * \brief Tensor operations using superbblas
 */

#ifndef __redstar_datalib_superb_tensor_h
#define __redstar_datalib_superb_tensor_h

#include "datalib/superb/types.h"
#include <algorithm>
#include <chrono>
#include <complex>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "datalib/superb/superb_aux.h"
#include "datalib/codec/superb_options.h"
#include <datalib/model/xml_array2d.h>
#include <datalib/model/xml_array3d.h>
#include <datalib/model/xml_array4d.h>

#if defined(SUPERBBLAS_HAS_GPU_SUPPORT) && defined(SUPERBBLAS_NOT_USE_GPU)
#  error "superbblas won't use the gpu version despite being compiled with gpu support"
#endif

namespace SBN
{
  using IndexType = int;
  using Coor = std::vector<IndexType>;
  using Order = std::string;
  using remap = std::map<char, char>;
  using SCoor = std::map<char, IndexType>;
  using value_type = std::complex<double>;
  using CustomDistribuition = std::vector<std::array<Coor, 2>>;

  namespace detail
  {
    /// Return the debug level

    inline int& get_debug_level()
    {
      static int c = [] {
	const char* l = std::getenv("SUPERBNOVA_DEBUG");
	if (l)
	  return std::atoi(l);
	return 0;
      }();
      return c;
    }
  }

  /// Return the number of seconds from some start

  inline double w_time()
  {
    return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch())
      .count();
  }

#ifdef SUPERBBLAS_USE_MPI
  /// Alias for MPI_Comm
  using MpiComm = MPI_Comm;
#else
  struct Empty {
  };

  /// Dummy type for MPI_Comm
  using MpiComm = Empty;
#endif

  /// Return the local (self) communicator

  inline MpiComm get_local_mpi_comm()
  {
#ifdef SUPERBBLAS_USE_MPI
    return MPI_COMM_SELF;
#else
    return {};
#endif
  }

  /// Return the global (world) communicator

  inline MpiComm get_global_mpi_comm()
  {
#ifdef SUPERBBLAS_USE_MPI
    return MPI_COMM_WORLD;
#else
    return {};
#endif
  }

  inline IndexType get_rank(MpiComm comm = get_global_mpi_comm());

  /// Return the default gpu device

  inline int& get_default_gpu_device()
  {
    static int device =
#if defined(USE_SUPERBBLAS)
      [] {
	const int num_gpus = superbblas::getGpuDevicesCount();
	return num_gpus == 0 ? 0 : get_rank() % num_gpus;
      }()
#else
      0
#endif
      ;
    return device;
  }

  inline Coor operator+(const Coor& a, const Coor& b)
  {
    if (a.size() != b.size())
      throw std::runtime_error("operator+: input vectors don't match");
    Coor r(a.size());
    for (std::size_t i = 0; i < a.size(); ++i)
      r[i] = a[i] + b[i];
    return r;
  }

  inline Coor operator-(const Coor& a, const Coor& b)
  {
    if (a.size() != b.size())
      throw std::runtime_error("operator+: input vectors don't match");
    Coor r(a.size());
    for (std::size_t i = 0; i < a.size(); ++i)
      r[i] = a[i] - b[i];
    return r;
  }

  /// Return whether a(i) < b(i) for all i

  inline bool is_less_than(const Coor& a, const Coor& b)
  {
    if (a.size() != b.size())
      throw std::runtime_error("operator+: input vectors don't match");
    for (std::size_t i = 0; i < a.size(); ++i)
      if (a[i] >= b[i])
	return false;
    return true;
  }

  /// Return whether a(i) <= b(i) for all i

  inline bool is_less_or_equal_than(const Coor& a, const Coor& b)
  {
    if (a.size() != b.size())
      throw std::runtime_error("operator+: input vectors don't match");
    for (std::size_t i = 0; i < a.size(); ++i)
      if (a[i] > b[i])
	return false;
    return true;
  }

  /// Return the number of elements in a tensor with the given dimensions
  /// \param dim: tensor dimensions

  template <std::size_t N>
  inline std::size_t volume(const std::array<IndexType, N>& dim)
  {
    if (dim.size() == 0)
      return 0;
    std::size_t r = 1;
    for (IndexType it : dim)
      r *= it;
    return r;
  }

  /// Return the number of elements in a tensor with the given dimensions
  /// \param dim: tensor dimensions
  /// \param empty_vol: returned volume for empty tensor

  inline std::size_t volume(const Coor& dim, std::size_t empty_vol = 0)
  {
    if (dim.size() == 0)
      return empty_vol;
    std::size_t r = 1;
    for (int it : dim)
      r *= it;
    return r;
  }

  /// Return the number of elements in a tensor with the given dimensions
  /// \param dim_begin: begin iterator for tensor dimensions
  /// \param dim_end: end iterator for tensor dimensions
  /// \param empty_vol: returned volume for empty tensor

  template <typename iterator>
  inline std::size_t volume(iterator dim_begin, iterator dim_end, std::size_t empty_vol = 0)
  {
    if (std::distance(dim_begin, dim_end) == 0)
      return empty_vol;
    std::size_t r = 1;
    for (iterator it = dim_begin; it != dim_end; it++)
      r *= (std::size_t)*it;
    return r;
  }

  /// Return the number of elements in a tensor with the given dimensions
  /// \param dim: tensor dimensions

  inline std::size_t volume(const SCoor& dim)
  {
    if (dim.size() == 0)
      return 0;
    std::size_t r = 1;
    for (const auto& it : dim)
      r *= it.second;
    return r;
  }

  /// Return the number of elements in a tensor with the given dimensions
  /// \param order: dimensions to consider
  /// \param dim: tensor dimensions
  /// \param empty_vol: return volume in case of no dimensions

  inline std::size_t volume(const Order& order, const SCoor& dim, std::size_t empty_vol = 0)
  {
    if (order.size() == 0)
      return empty_vol;
    std::size_t r = 1;
    for (const auto& label : order)
      r *= dim.at(label);
    return r;
  }

  /// Return the concatenation of orders
  /// \param a: first order to appear
  /// \param b: next order to appear
  /// \param c: last order to appear

  inline Order concat(const Order& a, const Order& b, const Order& c)
  {
    static_assert(std::is_same<Order, std::string>::value, "not supported");
    return a + b + c;
  }

  /// Return the concatenation of coors
  /// \param a: first coors to appear
  /// \param b: next coors to appear

  inline Coor concat(const Coor& a, const Coor& b)
  {
    Coor r;
    r.reserve(a.size() + b.size());
    r.insert(r.end(), a.begin(), a.end());
    r.insert(r.end(), b.begin(), b.end());
    return r;
  }

  /// Return the canonized coordinate in a periodic dimension
  /// \param coor: input coordinate
  /// \param dim: dimension
  /// NOTE: deal with negative number in an explicit way, because the remainder of
  ///       of a negative number is platform dependent

  inline IndexType norm_coor(IndexType coor, IndexType dim)
  {
    return (dim == 0 ? 0 : (coor + dim * (coor < 0 ? -coor / dim + 1 : 0)) % dim);
  }

  /// Return the canonized coordinate in a periodic dimension
  /// \param coor: input coordinate
  /// \param dim: dimension

  inline Coor norm_coor(const Coor& coor, const Coor& dim)
  {
    if (coor.size() != dim.size())
      throw std::runtime_error("norm_coor: input vectors don't match");
    Coor r(coor.size());
    for (std::size_t i = 0; i < coor.size(); ++i)
      r[i] = norm_coor(coor[i], dim[i]);
    return r;
  }

  /// Return the canonized range in a periodic dimension
  /// \param from: input from
  /// \param size: input size
  /// \param dim: dimension

  inline std::array<Coor, 2> norm_from_size(const Coor& from, const Coor& size, const Coor& dim)
  {
    if (from.size() != dim.size() || size.size() != dim.size())
      throw std::runtime_error("norm_from_size: input vectors don't match");
    auto new_from = norm_coor(from, dim);
    Coor new_size(dim.size());
    for (std::size_t i = 0; i < size.size(); ++i)
      new_size[i] = std::min(size[i], dim[i]);
    return {new_from, new_size};
  }

  /// Get the strides, the number of elements to jump for the next element with one unit more
  /// in each coordinate.
  /// \param dim: dimension of the tensor

  template <typename Coor>
  inline Coor get_strides(const Coor& dim)
  {
    if (dim.size() == 0)
      return {};
    Coor p(dim.size());
    // p(i) = prod(dim(1:i))
    p[0] = 1;
    for (std::size_t i = 1; i < dim.size(); ++i)
      p[i] = p[i - 1] * dim[i - 1];
    return p;
  }

  /// Return the coordinates associated to an index
  /// \param index: input index
  /// \param dim: tensor dimensions

  inline Coor indexToCoor(IndexType index, const Coor& dim)
  {
    if (volume(dim) == 0)
      return Coor(dim.size());
    const auto& strides = get_strides(dim);
    Coor r(dim.size());
    for (std::size_t j = 0; j < dim.size(); j++)
      r[j] = norm_coor(index / strides[j], dim[j]);
    return r;
  }

  /// Return the index associated to a coordinate
  /// \param coor: input coordinate
  /// \param dim: tensor dimensions

  inline IndexType coorToIndex(const Coor& coor, const Coor& dim)
  {
    if (coor.size() != dim.size())
      throw std::runtime_error("coorToIndex: invalid input");
    if (volume(dim) == 0)
      return 0;
    const auto& strides = get_strides(dim);
    IndexType r = 0;
    for (std::size_t j = 0; j < dim.size(); j++)
      r += norm_coor(coor[j], dim[j]) * strides[j];
    return r;
  }

  /// Return the permutation of coordinates from one ordering to another
  /// \param a: origin ordering
  /// \param b: destination ordering

  template <typename Order>
  inline Coor get_perm(const Order& a, const Order& b)
  {
    Coor perm(b.size(), -1);
    for (std::size_t i = 0; i < b.size(); ++i)
    {
      for (std::size_t j = 0; j < a.size(); ++j)
      {
	if (b[i] == a[j])
	{
	  perm[i] = j;
	  break;
	}
      }
    }
    return perm;
  }

  /// Apply a permutation produced by `get_perm`
  /// \param a: origin coordinate
  /// \param perm: permutation

  template <typename T>
  inline std::vector<T> do_perm(const std::vector<T>& a, const Coor& perm)
  {
    std::vector<T> r;
    r.reserve(perm.size());
    for (IndexType pi : perm)
      r.push_back(a.at(pi));
    return r;
  }

  /// Apply a permutation produced by `get_perm`
  /// \param a: origin coordinate
  /// \param perm: permutation
  /// \param missing: value for output coordinates not present in the origin ordering

  template <typename T>
  inline std::vector<T> do_perm(const std::vector<T>& a, const Coor& perm, T missing)
  {
    std::vector<T> r;
    r.reserve(perm.size());
    for (IndexType pi : perm)
      r.push_back(pi < 0 ? missing : a.at(pi));
    return r;
  }

  /// Reverse a permutation
  /// \param perm: permutation
  /// \param origin_size: size of the origin part of the given permutation

  inline Coor reverse_perm(const Coor& perm, std::size_t origin_size)
  {
    Coor r(origin_size, -1);
    for (std::size_t i = 0; i < perm.size(); ++i)
      if (perm.at(i) >= 0)
	r.at(perm.at(i)) = i;
    return r;
  }

  namespace detail
  {
    /// Return the intersection of two 1D ranges for a NOT toroidal lattice
    /// \param from0: first coordinate of the first range
    /// \param size0: size of the first range
    /// \param from1: first coordinate of the second range
    /// \param size1: size of the second range
    /// \param fromr: first coordinate of the resulting range
    /// \param sizer: size of the resulting range

    inline void intersection_coor(IndexType from0, IndexType size0, IndexType from1,
				  IndexType size1, IndexType dim, IndexType& fromr,
				  IndexType& sizer)
    {
      fromr = from0 + std::min(std::max(from1 - from0, IndexType{0}), size0);
      sizer = from0 + std::min(std::max(from1 + size1 - from0, IndexType{0}), size0) - fromr;
      fromr = (fromr + dim) % dim;
      if (sizer == dim)
	fromr = from0;
    }
  }

  /// Return whether the given ranges do not overlap
  /// \param from0: first range from
  /// \param size0: first range size
  /// \param from1: second range from
  /// \param size1: second range size
  /// \param dim: size of the periodic dimensions

  inline bool is_intersection_empty(const Coor& from0, const Coor& size0, const Coor& from1,
				    const Coor& size1, const Coor& dim)
  {
    // Check input
    if (from0.size() != size0.size() || from0.size() != from1.size() ||
	from0.size() != size1.size() || from0.size() != dim.size())
      throw std::runtime_error("is_intersection_empty: invalid input");
    for (std::size_t i = 0; i < dim.size(); ++i)
      if (size0.at(i) > dim.at(i) || size1.at(i) > dim.at(i))
	throw std::runtime_error("is_intersection_empty: invalid input");

    // Trivial case
    if (dim.size() == 0 || volume(size0) == 0 || volume(size1) == 0)
      return true;

    // Check whether the point `from0' is in range (from1, from1+size1) or vice versa
    bool empty = false;
    for (std::size_t i = 0; i < dim.size(); ++i)
    {
      // Check whether from0[i] is in (from1[i], size1[i]) and from1[i] is in (from0[i], size0[i])
      if (norm_coor(from0.at(i) - from1.at(i), dim.at(i)) >= size1.at(i) &&
	  norm_coor(from1.at(i) - from0.at(i), dim.at(i)) >= size0.at(i))
      {
	empty = true;
	break;
      }
    }
    return empty;
  }

  /// Return a coordinate contained int the given ranges or empty if they don't overlap
  /// \param from0: first range from
  /// \param size0: first range size
  /// \param from1: second range from
  /// \param size1: second range size
  /// \param dim: size of the periodic dimensions

  inline Coor get_common_coor(const Coor& from0, const Coor& size0, const Coor& from1,
			      const Coor& size1, const Coor& dim)
  {
    // Check input
    if (from0.size() != size0.size() || from0.size() != from1.size() ||
	from0.size() != size1.size() || from0.size() != dim.size())
      throw std::runtime_error("is_intersection_empty: invalid input");
    for (std::size_t i = 0; i < dim.size(); ++i)
      if (size0.at(i) > dim.at(i) || size1.at(i) > dim.at(i))
	throw std::runtime_error("is_intersection_empty: invalid input");

    // Trivial case
    if (dim.size() == 0 || volume(size0) == 0 || volume(size1) == 0)
      return {};

    // Check whether the point `from0' is in range (from1, from1+size1) or vice versa
    bool empty0 = false; ///< whether from0 is not in (from1, size1)
    bool empty1 = false; ///< whether from1 is not in (from0, size0)
    Coor coor(dim.size());
    for (std::size_t i = 0; i < dim.size(); ++i)
    {
      // Check whether from0[i] is in (from1[i], size1[i])
      if (norm_coor(from0.at(i) - from1.at(i), dim.at(i)) < size1.at(i))
	coor.at(i) = from0.at(i);
      // Check whether from1[i] is in (from0[i], size0[i])
      else if (norm_coor(from1.at(i) - from0.at(i), dim.at(i)) < size0.at(i))
	coor.at(i) = from1.at(i);
      else
	return {};
    }
    return coor;
  }

  /// Return whether the given ranges do not overlap
  /// \param from0: first range from
  /// \param size0: first range size
  /// \param from1: second range from
  /// \param size1: second range size
  /// \param from2: third range from
  /// \param size2: third range size
  /// \param dim: size of the periodic dimensions

  inline bool is_intersection_empty(const Coor& from0, const Coor& size0, const Coor& from1,
				    const Coor& size1, const Coor& from2, const Coor& size2,
				    const Coor& dim)
  {
    // Check input
    if (from0.size() != size0.size() || from0.size() != from1.size() ||
	from0.size() != size1.size() || from0.size() != from2.size() ||
	from0.size() != size2.size() || from0.size() != dim.size())
      throw std::runtime_error("is_intersection_empty: invalid input");
    for (std::size_t i = 0; i < dim.size(); ++i)
      if (size0.at(i) > dim.at(i) || size1.at(i) > dim.at(i) || size2.at(i) > dim.at(i))
	throw std::runtime_error("is_intersection_empty: invalid input");

    // Check trivial case
    if (dim.size() == 0 || volume(size0) == 0 || volume(size1) == 0 || volume(size2) == 0)
      return true;

    // Check whether the point `from_i' is in the other ranges (from_j,from_j+size_j)
    const std::array<const Coor, 3> from{from0, from1, from2};
    const std::array<const Coor, 3> size{size0, size1, size2};

    for (std::size_t i = 0; i < dim.size(); ++i)
    {
      bool is_empty = true;
      // Check whether from[f][i] is in (from[t][i], size[t][i])
      for (std::size_t f = 0; is_empty && f < 3; ++f)
      {
	bool is_empty_f = false;
	for (std::size_t t = 0; !is_empty_f && t < 3; ++t)
	  if (f != t)
	    if (norm_coor(from.at(f).at(i) - from.at(t).at(i), dim.at(i)) >= size.at(t).at(i))
	      is_empty_f = true;
	if (!is_empty_f)
	  is_empty = false;
      }
      if (is_empty)
	return true;
    }
    return false;
  }

  /// Return the common range between two given ranges
  /// \param from0: first range from
  /// \param size0: first range size
  /// \param from1: second range from
  /// \param size1: second range size

  inline std::array<Coor, 2> get_simple_intersection(const Coor& from0, const Coor& size0,
						     const Coor& from1, const Coor& size1,
						     const Coor& dim)
  {
    if (from0.size() != size0.size() || from0.size() != from1.size() ||
	from0.size() != size1.size() || from0.size() != dim.size())
      throw std::runtime_error("is_intersection_empty: invalid input");
    std::array<Coor, 2> r{Coor(dim.size()), Coor(dim.size())};
    if (dim.size() == 0)
      return r;
    if (volume(size0) == 0 || volume(size1) == 0)
      return {Coor(dim.size()), Coor(dim.size())};
    for (std::size_t i = 0; i < dim.size(); ++i)
    {
      const auto di = dim.at(i);
      const auto f0 = norm_coor(from0.at(i), di);
      const auto s0 = size0.at(i);
      const auto f1 = norm_coor(from1.at(i), di);
      const auto s1 = size1.at(i);
      int fr = 0, sr = 0;
      detail::intersection_coor(f0, s0, f1, s1, di, fr, sr);
      bool is_set = false;
      if (sr > 0)
      {
	r.at(0).at(i) = fr;
	r.at(1).at(i) = sr;
	is_set = true;
      }
      if (f0 + s0 > di && f1 + s1 <= di)
      {
	detail::intersection_coor(f0, s0, f1 + di, s1, di, fr, sr);
	if (sr > 0)
	{
	  if (is_set)
	    throw std::runtime_error("get_simple_intersection: the intersection isn't simple");
	  r.at(0).at(i) = fr;
	  r.at(1).at(i) = sr;
	  is_set = true;
	}
      }
      if (f0 + s0 <= di && f1 + s1 > di)
      {
	detail::intersection_coor(f0 + di, s0, f1, s1, di, fr, sr);
	if (sr > 0)
	{
	  if (is_set)
	    throw std::runtime_error("get_simple_intersection: the intersection isn't simple");
	  r.at(0).at(i) = fr;
	  r.at(1).at(i) = sr;
	  is_set = true;
	}
      }
      if (!is_set)
      {
	return {Coor(dim.size()), Coor(dim.size())};
      }
    }
    return r;
  }

  /// Return whether two orders have the same labels
  /// \param a: one order
  /// \param b: another order

  inline bool have_same_labels(const Order& a, const Order& b)
  {
    return std::set<char>(a.begin(), a.end()) == std::set<char>(b.begin(), b.end());
  }

  inline SCoor get_scoor(const Order& order, const Coor& coor);

  /// Return whether the new order can be a subset of the current range
  /// \param order: range labels
  /// \param size: range size
  /// \param new_order: new order to test

  inline bool is_same_range(const Order& order, const Coor& size, const Order& new_order)
  {
    // If the new order has a new label, then it is not a subrange
    const auto& s = get_scoor(order, size);
    for (char c : new_order)
      if (s.count(c) == 0)
	return false;

    // If the new order is missing some dimensions that are aren't singlet, then it is not a subrange
    const std::set<char> visited(new_order.begin(), new_order.end());
    for (char c : order)
      if (visited.count(c) == 0 && s.at(c) > 1)
	return false;

    return true;
  }

  /// Return whether the second argument labels are on the first
  /// \param a: one order
  /// \param b: another order

  inline bool contain_labels(const Order& a, const Order& b)
  {
    const auto& sa = std::set<char>(a.begin(), a.end());
    for (char c : b)
      if (sa.count(c) == 0)
	return false;
    return true;
  }

  /// Return the common labels

  inline Order common_labels(const Order& a, const Order& b)
  {
    const auto& sa = std::set<char>(a.begin(), a.end());
    const auto& sb = std::set<char>(b.begin(), b.end());
    Order r;
    r.reserve(std::min(sa.size(), sb.size()));
    for (char c : sa)
      if (sb.count(c) > 0)
	r.push_back(c);
    return r;
  }

  /// Return the inverse of a given map
  /// \param m: renaming map

  inline remap inverse(const remap& m)
  {
    remap r;
    for (const auto& it : m)
    {
      if (r.count(it.second) > 0)
	throw std::runtime_error("inverse: invalid input, it can't be inverted");
      r[it.second] = it.first;
    }

    return r;
  }

  /// Return the given order after renaming the labels
  /// \param m: renaming map
  /// \param order: the input order

  inline Order relabel(const remap& m, const Order& order)
  {
    Order r;
    r.reserve(order.size());
    for (char label : order)
      r.push_back(m.count(label) == 1 ? m.at(label) : label);

    return r;
  }

  /// Return a reverse copy of an order
  /// \param order: order to reverse

  inline Order reverse(const Order& order)
  {
    Order new_order = order;
    std::reverse(new_order.begin(), new_order.end());
    return new_order;
  }

  /// Return a list of pairs of elements from each list
  /// \param t: first elements of the pair
  /// \param q: second elements of the pair

  template <typename Tv, typename Qv>
  std::vector<std::pair<typename Tv::value_type, typename Qv::value_type>> zip(const Tv& t,
									       const Qv& q)
  {
    if (t.size() != q.size())
      throw std::runtime_error("zip: input vector lengths don't match");
    std::vector<std::pair<typename Tv::value_type, typename Qv::value_type>> r;
    r.reserve(t.size());
    for (std::size_t i = 0; i < t.size(); ++i)
      r.push_back({t[i], q[i]});
    return r;
  }

  /// Return a range given the dimension sizes and labels
  /// \param order: dimension labels
  /// \param coor: dimension sizes

  inline SCoor get_scoor(const Order& order, const Coor& coor)
  {
    if (order.size() != coor.size())
      throw std::runtime_error("get_scoor: dimensions don't match");

    const auto& z = zip(order, coor);
    return SCoor(z.begin(), z.end());
  }

  /// Return a range by completing missing values with another range
  /// \param in: default range values
  /// \param extra: values for missing labels in `in`

  inline SCoor complete_scoor(const SCoor& in, const SCoor& extra)
  {
    SCoor out = in;
    for (const auto& it : extra)
      if (out.count(it.first) == 0)
	out[it.first] = it.second;
    return out;
  }

  /// Return the coordinates for the given dimensions
  /// \param in: input range
  /// \param labels: dimensions to return

  inline SCoor slice_scoor(const SCoor& in, const Order& labels)
  {
    SCoor r;
    for (const auto it : in)
      if (std::find(labels.begin(), labels.end(), it.first) != labels.end())
	r[it.first] = it.second;
    return r;
  }

  /// Return the coordinates for the given dimensions
  /// \param in: input range
  /// \param labels: dimensions to return

  inline std::array<SCoor, 2> slice_ranges(const std::array<SCoor, 2>& in, const Order& labels)
  {
    return {slice_scoor(in.at(0), labels), slice_scoor(in.at(1), labels)};
  }

  /// Return a coordinate given a range and an order
  /// \param order: dimension labels
  /// \param scoor: range

  inline Coor get_coor(const Order& order, const SCoor& scoor)
  {
    Coor r;
    r.reserve(order.size());
    for (char label : order)
      r.push_back(scoor.at(label));

    return r;
  }

  /// Return a coordinate given a range and an order
  /// \param order: dimension labels
  /// \param scoor: range

  inline Coor get_coor(const Order& order, const SCoor& scoor, IndexType missing)
  {
    Coor r;
    r.reserve(order.size());
    for (char label : order)
      r.push_back(scoor.count(label) == 0 ? missing : scoor.at(label));

    return r;
  }

  /// Return if the given ranges are compatible
  /// \param sc0: one range
  /// \param sc1: another range

  inline bool is_compatible_for_copying(const SCoor& sc0, const SCoor& sc1)
  {
    if (volume(sc0) != volume(sc1))
      return false;
    for (const auto& it : sc0)
    {
      if ((sc1.count(it.first) == 0 && it.second > 1) ||
	  (sc1.count(it.first) == 1 && sc1.at(it.first) != it.second))
	return false;
    }
    for (const auto& it : sc1)
    {
      if (sc0.count(it.first) == 0 && it.second > 1)
	return false;
    }
    return true;
  }

  /// Return if the given ranges are compatible for being contracted
  /// \param sc0: one range
  /// \param sc1: another range
  /// \param final_order: dimensions of the resulting tensor

  inline bool is_compatible_for_contracting(const SCoor& sc0, const SCoor& sc1,
					    const Order& final_order)
  {
    for (const auto& it : sc0)
    {
      if (sc1.count(it.first) == 1 && sc1.at(it.first) != it.second)
	return false;
    }

    return true;
  }

  /// Return if an order is valid
  /// \param order: input order

  inline bool is_valid(const Order& order)
  {
    return std::set<char>(order.begin(), order.end()).size() == order.size();
  }

  /// Return the value but conjugated if `do_it`
  /// \param do_it: conditional
  /// \param i: value

  inline value_type cond_conj(bool do_it, const value_type& i)
  {
    return do_it ? std::conj(i) : i;
  }

  /// Return a new label
  /// \param order: already used labels

  inline char get_new_label(const Order& order)
  {
    for (char i = 1; i < std::numeric_limits<char>::max(); ++i)
      if (std::find(order.begin(), order.end(), i) == order.end())
	return i;
    throw std::runtime_error("get_new_label: out of new labels");
  }

  /// Return the number processes

  inline IndexType get_nprocs(MpiComm comm = get_global_mpi_comm())
  {
#ifdef SUPERBBLAS_USE_MPI
    int nprocs = 0;
    superbblas::detail::MPI_check(MPI_Comm_size(comm, &nprocs));
    return nprocs;
#else
    return 1;
#endif
  }

  /// Return the number processes

  inline IndexType get_rank(MpiComm comm)
  {
#ifdef SUPERBBLAS_USE_MPI
    int rank = 0;
    superbblas::detail::MPI_check(MPI_Comm_rank(comm, &rank));
    return rank;
#else
    return 0;
#endif
  }

  namespace detail
  {
    inline bool& check_comms()
    {
      static bool c = SBN::detail::get_debug_level() > 0;
      return c;
    }

    inline void check_consistency(const std::size_t& value, MpiComm comm);
  }

  /// Broadcast the content from process zero to all processes
  /// \param t: object to broadcast

  template <typename T>
  inline T broadcast(const T& t, MpiComm comm = get_global_mpi_comm())
  {
#ifdef SUPERBBLAS_USE_MPI
    if constexpr (std::is_integral<T>::value)
    {
      if (detail::check_comms())
	detail::check_consistency(SB::getHash(std::string("broadcast-integral")), comm);
      T r = t;
      superbblas::detail::MPI_check(MPI_Bcast((void*)&r, sizeof(T), MPI_CHAR, 0, comm));
      return r;
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
      if (detail::check_comms())
	detail::check_consistency(SB::getHash(std::string("broadcast-string")), comm);
      const auto n = broadcast(t.size(), comm);
      std::string r = get_rank(comm) == 0 ? t : std::string(n, 0);
      superbblas::detail::MPI_check(MPI_Bcast((void*)r.data(), r.size(), MPI_CHAR, 0, comm));
      return r;
    }
    else
    {
      return SB::getOptionFast<T>(
	broadcast(get_rank(comm) == 0 ? SB::getXML(t, "v") : std::string{}, comm), "v");
    }
#else
    return t;
#endif // SUPERBBLAS_USE_MPI
  }

  /// Gather values from all processes
  /// \param v: value to broadcast
  /// \return vector with the values from all processes

  template <typename T>
  inline std::vector<T> gather(const T& v, MpiComm comm = get_global_mpi_comm())
  {
#ifdef SUPERBBLAS_USE_MPI
    if constexpr (std::is_trivially_copyable<T>::value)
    {
      if (detail::check_comms())
	detail::check_consistency(SB::getHash(std::string("gather-trivial")), comm);
      std::vector<T> r(get_nprocs(comm));
      superbblas::detail::MPI_check(
	MPI_Allgather((void*)&v, sizeof(T), MPI_CHAR, (void*)r.data(), sizeof(T), MPI_CHAR, comm));
      return r;
    }
    else if constexpr (std::is_same<T, std::string>::value)
    {
      if (detail::check_comms())
	detail::check_consistency(SB::getHash(std::string("gather-string")), comm);
      const auto ni = gather((int)v.size(), comm);
      const auto n = std::accumulate(ni.begin(), ni.end(), int{0});
      std::vector<int> disps(ni.size());
      for (std::size_t i = 1; i < ni.size(); ++i)
	disps.at(i) = disps.at(i - 1) + ni.at(i - 1);
      std::vector<char> output_buffer(n);
      superbblas::detail::MPI_check(MPI_Allgatherv((void*)v.data(), v.size(), MPI_CHAR,
						   (void*)output_buffer.data(), ni.data(),
						   disps.data(), MPI_CHAR, comm));
      std::vector<std::string> r;
      r.reserve(ni.size());
      for (std::size_t i = 0; i < ni.size(); ++i)
	r.push_back(std::string(output_buffer.begin() + disps.at(i),
				output_buffer.begin() + disps.at(i) + ni.at(i)));
      return r;
    }
    else
    {
      std::vector<T> r;
      r.reserve(get_nprocs(comm));
      for (const auto& s : gather(SB::getXML(v, "v"), comm))
	r.push_back(SB::getOptionFast<T>(s, "v"));
      return r;
    }
#else
    return {v};
#endif // SUPERBBLAS_USE_MPI
  }

  /// Wait until all processes execute the function

  inline void barrier(MpiComm comm = get_global_mpi_comm())
  {
#ifdef SUPERBBLAS_USE_MPI
    if (detail::check_comms())
      detail::check_consistency(SB::getHash(std::string("barrier")), comm);
    superbblas::detail::MPI_check(MPI_Barrier(comm));
#endif // SUPERBBLAS_USE_MPI
  }

  namespace detail
  {
    /// Return whether the given MPI communicators have the same MPI_COMM_WORLD ranks and in the same order
    /// \param comm0: one communicator
    /// \param comm1: another communicator

    inline bool is_same_comm(MpiComm comm0, MpiComm comm1)
    {
#ifdef SUPERBBLAS_USE_MPI
      if (comm0 == comm1)
	return true;
      int result;
      superbblas::detail::MPI_check(MPI_Comm_compare(comm0, comm1, &result));
      return (result == MPI_IDENT || result == MPI_CONGRUENT);
#else
      return true;
#endif
    }

    /// Return the MPI_COMM_WORLD ranks associated to the given communicator
    /// \param comm: MPI communicator

    inline std::vector<int> get_ranks(MpiComm comm)
    {
#ifdef SUPERBBLAS_USE_MPI
      /// Check for invalid communicators
      if (comm == MPI_COMM_NULL)
	throw std::runtime_error("invalid communicator");

      /// Trivial case: default communicator
      if (is_same_comm(MPI_COMM_WORLD, comm))
      {
	std::vector<int> ranks;
	const auto nprocs = get_nprocs();
	ranks.reserve(nprocs);
	for (int i = 0; i < nprocs; ++i)
	  ranks.push_back(i);
	return ranks;
      }

	/// Trivial case: self communicator
      if (is_same_comm(MPI_COMM_SELF, comm))
      {
	return std::vector<int>{get_rank()};
      }

      /// General case
      return gather(get_rank(), comm);
#else
      return {0};
#endif
    }
  }

  namespace detail
  {
    /// Check that all processes report the same value
    /// \param value: value to report
    /// \param comm: MPI communicator

    inline void check_consistency(const std::size_t& value, MpiComm comm)
    {
      // Avoid activating the checking for the following operations
      bool c = false;
      std::swap(check_comms(), c);

      static std::map<std::vector<int>, unsigned int> from_ranks_to_i;
      const auto& ranks = SBN::detail::get_ranks(comm);
      if (from_ranks_to_i.count(ranks) == 0)
	from_ranks_to_i.insert({ranks, 0u});
      const auto i = from_ranks_to_i[ranks]++;
      const auto value_with_i = SB::getHash(std::make_tuple(value, ranks, i));
      if (broadcast(value_with_i, comm) != value_with_i)
	throw std::runtime_error("consistency broken!");
      barrier(comm);

      // Restore the checking
      std::swap(check_comms(), c);
    }
  }

  struct Comm {
    /// MPI_COMM_WORLD ranks associate to each rank in `comm`
    std::vector<int> ranks;

    /// MPI communicator
    MpiComm comm;

    Comm()
#  ifdef SUPERBBLAS_USE_MPI
      : comm(MPI_COMM_NULL)
#  endif
    {
    }

    Comm(const std::vector<int>& ranks, MpiComm comm) : ranks(ranks), comm(comm)
    {
    }
  };

  /// Return a SBN::Comm from an MPI communicator
  /// \param comm: MPI communicator
  /// NOTE: it may involve a global communication on the given communicator

  inline Comm get_comm_from_mpi_comm(MpiComm comm)
  {
    return {detail::get_ranks(comm), comm};
  }

  /// Return the local (self) communicator

  inline Comm get_local_comm()
  {
    return Comm{{get_rank()}, get_local_mpi_comm()};
  }

  /// Return the global (world) communicator

  inline Comm get_global_comm()
  {
    return Comm{SBN::detail::get_ranks(get_global_mpi_comm()), get_global_mpi_comm()};
  }

  constexpr auto complex_nan = value_type(std::numeric_limits<double>::signaling_NaN(),
					  std::numeric_limits<double>::signaling_NaN());

  namespace Options
  {
    namespace Distribution
    {
      static const char* Local = "__local__";
      static const char* OnMaster = "__on_master__";
      static const char* Replicated = "";
      static const char* Default = "__default__";
    }
    enum struct Alloc {
      Host,   ///< allocate the tensor on cpu
      Device, ///< allocate the tensor on device if possible
      Default ///< default policy
    };
    enum struct IsEg {
      False,  ///< non example tensor
      True,   ///< example tensor
      Default ///< default policy
    };
    enum struct TightView {
      True,		   ///< the first coordinate is at the origin and size == dim and the view
			   ///  is not conjugated and the scale is 1.0
      TrueAndRedistribute, ///< as `True` but redistribute the tensor among the processes if a new copy is needed
      Default		   ///< default policy
    };
    enum struct AllowCopy {
      False, ///< do not allow copy
      True,  ///< allow copy
    };
    enum struct ComplainIfIsntTightView {
      True, ///< complain if the view is not tight
      False ///< do not complain
    };
    enum struct ReturnUniqueIdForEmptyRanges {
      False, ///< do not return different ids for empty ranges
      True   ///< do return different ids for empty ranges
    };
    enum struct Ordering {
      Natural,	    ///< the first dimension moves the fastest
      Lexicographic ///< the first dimension moves the slowest
    };
  }

  /// Get the sparse labels
  /// \param order: input order
  /// \param n_sp: number of dimensions that are sparse

  inline std::string get_sparse_labels(const Order& order, int n_sp)
  {
    if (n_sp > order.size())
      throw std::runtime_error("wtf");
    return std::string(order.end() - n_sp, order.end());
  }

  /// Return whether the distribution is one of `Local', `OnMaster' or `Default'
  /// \param distribution: input distribution

  inline bool is_predefined_distribution(const std::string& distribution)
  {
    return distribution.size() >= 2 &&
	   std::string(distribution.begin(), distribution.begin() + 2) == std::string("__");
  }

  /// Return the maximum number of nonzeros to store
  /// \param dim: tensor dimensions
  /// \param n_sp: the last `n_sp` dimensions are sparse
  /// \param max_nnz_per_row: maximum nonzeros for each nonsparse coordinate

  inline std::size_t volume(const Coor& dim, int n_sp, int max_nnz_per_row)
  {
    if (n_sp == 0)
      return volume(dim);
    if (volume(dim) == 0)
      return 0;
    return volume(Coor(dim.begin(), dim.end() - n_sp), 1) * max_nnz_per_row;
  }

  //
  // Reference implementation for debugging and checking correctness
  //

  namespace reference_implementation
  {
    struct Tensor {
      using Ptr = std::shared_ptr<std::vector<value_type>>;
      using IPtr = std::shared_ptr<std::vector<IndexType>>;
      Coor from;   ///< first coordinate in the view
      Coor size;   ///< number of elements on each direction in the view
      Coor dim;	   ///< tensor dimensions
      Order order; ///< labels of the tensor
      Ptr ptr;	   ///< values of the tensor in (natural ordering) if n_sp==0;
	///  otherwise the nonzero values in CSR format
      value_type scale;	   ///< implicit scaling on the tensor elements
      bool is_conjugated;  ///< whether the tensor elements are implicitly conjugated
      int n_sp;		   ///< the last n_sp dimensions are sparse
      int max_nnz_per_row; ///< max nonzero values per dense coordinate
      IPtr cols;	   ///< indices of the sparse elements per dense element

      Tensor() : scale(0.), is_conjugated(false), n_sp(0), max_nnz_per_row(0) {};

      Tensor(const Coor& from, const Coor& size, const Coor& dim, const Order& order,
	     const Ptr& ptr, value_type scale, bool is_conjugated, int n_sp, int max_nnz_per_row,
	     const IPtr& cols)
	: from(from),
	  size(size),
	  dim(dim),
	  order(order),
	  ptr(ptr),
	  scale(scale),
	  is_conjugated(is_conjugated),
	  n_sp(n_sp),
	  max_nnz_per_row(max_nnz_per_row),
	  cols(cols)
      {
      }
    };

    /// Return a pointer to the cpu data
    /// \param t: input tensor

    inline value_type* data(const Tensor& t)
    {
      if (t.ptr)
	return t.ptr->data();
      return nullptr;
    }

    /// Return a pointer to the cpu data
    /// \param t: input tensor

    inline IndexType* nnz_indices(const Tensor& t)
    {
      if (t.cols)
	return t.cols->data();
      return nullptr;
    }

    /// Return whether the tensor has a nonzero support on the local process
    /// \param t: input tensor

    inline bool has_local_support(const Tensor& t)
    {
      return volume(t.size) > 0;
    }

    /// Return a local tensor with the content of the given tensor with support on the current process
    /// \param t: input tensor

    inline Tensor get_local_tensor(const Tensor& t)
    {
      return t;
    }

    /// Return another tensor with the content of the given tensor with support on the given communicator
    /// \param t: input tensor
    /// \param comm: communicator

    inline Tensor get_tensor_supported_on(const Tensor& t, const Comm& comm)
    {
      if (comm.ranks != std::vector<int>{0})
	throw std::runtime_error("get_tensor_supported_on: unsupported communicator");
      return t;
    }

    /// Return the MPI communicator associated to a tensor
    /// \param t: input tensor

    inline MpiComm get_mpi_comm(const Tensor& t)
    {
      return {};
    }

    /// Return the communicator associated to a tensor
    /// \param t: input tensor

    inline Comm get_comm(const Tensor& t)
    {
      return {{0}, {}};
    }

    /// Return an unique id for each partition of the given tensor
    /// \param t: input tensor
    /// \param return_unique_id_for_empty_ranges: (optional) whether to return different ids for empty ranges (default, false)

    inline std::vector<int> get_unique_range_ids(
      const Tensor& t,
      Options::ReturnUniqueIdForEmptyRanges = Options::ReturnUniqueIdForEmptyRanges::False)
    {
      return std::vector<int>{0};
    }

    /// Return a new tensor
    /// \param dim: dimensions of the new tensor
    /// \param order: labels of the new tensor
    /// \param distribution: policy to distribute the content of the tensor among the processes:
    ///          - Options::Distribution::Local: with support on the current process
    ///          - Options::Distribution::OnMaster: with support on process 0
    ///          - Options::Distribution::Replicated: with full support on all processes
    ///          - string with the labels to distribute among the processes
    ///        NOTE: operations with tensors supported on several processes may invoke MPI functions
    /// \param xpu: either
    ///          - Options::Alloc:Host: the tensor content is on the CPU
    ///          - Options::Alloc:Device: the tensor content may be on a GPU device.
    /// \param n_sp: the last `n_sp` dimensions are sparse
    /// \param max_nnz_per_row: maximum number of nonzero values for non-sparse coordinate
    /// \param eg: either
    ///          - Options::IsEg::False: allocate a tensor
    ///          - Options::IsEg::True: don't allocate a tensor and just create a template
    /// \param custom_distribution: (optional) range supported for each process
    /// \param comm: (optional) communicator

    inline Tensor create_tensor(const Coor& dim, const Order& order,
				const std::string& distribution = Options::Distribution::Local,
				Options::Alloc xpu = Options::Alloc::Host, int n_sp = 0,
				int max_nnz_per_row = 0, Options::IsEg eg = Options::IsEg::False,
				const CustomDistribuition& custom_distribution = {}, Comm comm = {})
    {
      (void)xpu;
      (void)eg;
      if (custom_distribution.size() > 1)
	throw std::runtime_error("create_tensor: basic implementation does not support "
				 "`custom_distribution' with more than one item");
      for (const auto& fs : custom_distribution)
	if (fs.at(0).size() != dim.size() || fs.at(1).size() != dim.size() ||
	    !is_less_or_equal_than(fs.at(1), dim))
	  throw std::runtime_error("create_tensor: invalid input");
      if (custom_distribution.size() > 0 && (custom_distribution.at(0).at(0) != Coor(dim.size()) ||
					     custom_distribution.at(0).at(1) != dim))
	throw std::runtime_error("create_tensor: basic implementation does not support "
				 "`custom_distribution' with anything than the full range");
      if (!is_valid(order) || dim.size() != order.size() || n_sp > order.size())
	throw std::runtime_error("create_tensor: invalid inputs");
      if (comm.ranks.size() > 0 && comm.ranks != std::vector<int>{0})
	throw std::runtime_error("create_tensor: invalid inputs");
      return Tensor{
	Coor(dim.size()),
	dim,
	dim,
	order,
	std::make_shared<std::vector<value_type>>(volume(dim, n_sp, max_nnz_per_row), complex_nan),
	1.0,
	false,
	n_sp,
	n_sp == 0 ? 0 : max_nnz_per_row,
	std::make_shared<std::vector<IndexType>>(
	  n_sp == 0 ? 0ul : volume(dim, n_sp, max_nnz_per_row), -1)};
    }

    inline void copyTo(const Tensor& tf, const Tensor& td);

    /// Return a new tensor like the given one
    /// \param distribution: policy to distribute the content of the tensor among the processes:
    ///          - Options::Distribution::OnMaster: with support on process 0
    ///          - Options::Distribution::Replicated: with full support on all processes
    ///          - Options::Distribution::Default: as the given tensor
    ///          - string with the labels to distribute among the processes
    ///        NOTE: operations with tensors supported on several processes may invoke MPI functions
    /// \param xpu: either
    ///          - Options::Alloc:Host: the tensor content is on the CPU
    ///          - Options::Alloc:Device: the tensor content may be on a GPU device.
    ///          - Options::Alloc:Default: as the given tensor
    /// \param n_sp: the last `n_sp` dimensions are sparse
    /// \param max_nnz_per_row: maximum number of nonzero values for non-sparse coordinate

    inline Tensor like_this(const Tensor& t, const Order& new_order = {}, const Coor& new_size = {},
			    const std::string& distribution = Options::Distribution::Default,
			    Options::Alloc xpu = Options::Alloc::Default, int n_sp = -1,
			    int max_nnz_per_row = -1, Options::IsEg eg = Options::IsEg::Default)
    {
      (void)xpu;
      (void)eg;

      // Check input
      if (new_order.size() > 0 && new_size.size() != new_order.size())
	throw std::runtime_error("like_this: invalid `new_size`");
      if (new_order.size() == 0 && new_size.size() > 0 && t.dim.size() != new_size.size())
	throw std::runtime_error("like_this: invalid `new_size`");
      if (distribution == Options::Distribution::Local)
	throw std::runtime_error("like_this: invalid `distribution`");

      // Get the new order
      const Order& this_new_order = new_order.size() == 0 ? t.order : new_order;
      if (!is_valid(this_new_order))
	throw std::runtime_error("like_this: invalid `new_order`");
      const Coor& this_new_size = new_size.size() == 0 ? t.size : new_size;

      // Get the new sparse parameters
      const int new_n_sp = n_sp < 0 ? t.n_sp : n_sp;
      const int new_max_nnz_per_row =
	new_n_sp == 0 ? 0 : (max_nnz_per_row < 0 ? t.max_nnz_per_row : max_nnz_per_row);
      if (new_n_sp > new_order.size())
	throw std::runtime_error(
	  "like_this: invalid `n_sp` or unsupported the distribution of sparse dimensions");

      // Create new tensor
      return Tensor{
	Coor(this_new_size.size()),
	this_new_size,
	this_new_size,
	this_new_order,
	std::make_shared<std::vector<value_type>>(
	  volume(this_new_size, new_n_sp, new_max_nnz_per_row), complex_nan),
	1.0,
	false /* not conjugated */,
	new_n_sp,
	new_max_nnz_per_row,
	std::make_shared<std::vector<IndexType>>(
	  new_n_sp == 0 ? 0ul : volume(this_new_size, new_n_sp, new_max_nnz_per_row), -1)};
    }

    /// Return a new tensor
    /// \param distribution: policy to distribute the content of the tensor among the processes:
    ///          - Options::Distribution::OnMaster: with support on process 0
    ///          - Options::Distribution::Replicated: with full support on all processes
    ///          - Options::Distribution::Default: as the given tensor
    ///          - string with the labels to distribute among the processes
    ///        NOTE: operations with tensors supported on several processes may invoke MPI functions
    /// \param xpu: either
    ///          - Options::Alloc:Host: the tensor content is on the CPU
    ///          - Options::Alloc:Device: the tensor content may be on a GPU device.
    ///          - Options::Alloc:Default: as the given tensor

    inline Tensor make_sure(const Tensor& t, const Order& new_order,
			    const std::string& distribution = Options::Distribution::Default,
			    Options::Alloc xpu = Options::Alloc::Default,
			    Options::TightView view = Options::TightView::Default, int n_sp = -1,
			    int max_nnz_per_row = -1, Options::IsEg eg = Options::IsEg::Default)
    {
      // Check input
      if (new_order.size() > 0 && !is_same_range(t.order, t.size, new_order))
	throw std::runtime_error("make_sure: invalid `new_order`");

      bool do_copy = false; ///< whether to create a new tensor and copy the content

      // Check the ordering
      if (!do_copy && new_order.size() > 0 && t.order != new_order)
      {
	do_copy = true;
      }

      // Check thew view
      if (!do_copy && view != Options::TightView::Default)
      {
	do_copy =
	  (t.from == Coor(t.dim.size()) && t.size == t.dim && !t.is_conjugated && t.scale == 1.0)
	    ? false
	    : true;
      }

      // Check for the sparse dimensions
      if (!do_copy && n_sp >= 0 && t.n_sp != n_sp)
	do_copy = true;
      if (!do_copy && max_nnz_per_row >= 0 && t.max_nnz_per_row != max_nnz_per_row)
	do_copy = true;

      // Check the example gratia
      if (!do_copy && eg != Options::IsEg::Default)
      {
	do_copy = bool(t.ptr) == (eg == Options::IsEg::False) ? false : true;
      }

      // Return the given tensor if we don't need to copy it
      if (!do_copy)
	return t;

      // Create new tensor
      const auto& n = like_this(t, new_order, get_coor(new_order, get_scoor(t.order, t.size)),
				distribution, xpu, n_sp, max_nnz_per_row, eg);

      // Copy the content to the new tensor
      if (n.ptr)
	copyTo(t, n);

      // Return the new tensor
      return n;
    }

    /// Return whether the given tensor communicator has more than one rank
    /// \param t: input tensor

    inline bool has_multiple_ranks(const Tensor& t)
    {
      return false;
    }

    /// Return the labels along the tensor is distributed
    /// \param t: input tensor

    inline Order get_distribution_labels(const Tensor& t)
    {
      return {};
    }

    /// Return the local range of a tensor
    /// \param t: input tensor
    /// \param complainIfItsntTightView: complain if the given tensor doesn't have a tight view

    inline std::array<Coor, 2>
    get_local_range(const Tensor& t, Options::ComplainIfIsntTightView complainIfItsntTightView =
				       Options::ComplainIfIsntTightView::True)
    {
      if (complainIfItsntTightView == Options::ComplainIfIsntTightView::True &&
	  (t.from != Coor(t.dim.size()) || t.size != t.dim))
	throw std::runtime_error("get_local_range: unsupported on non tight views");
      return {t.from, t.size};
    }

    /// Return the local range of a tensor
    /// \param t: input tensor
    /// \param complainIfItsntTightView: complain if the given tensor doesn't have a tight view

    inline std::array<SCoor, 2>
    get_local_srange(const Tensor& t, Options::ComplainIfIsntTightView complainIfItsntTightView =
					Options::ComplainIfIsntTightView::True)
    {
      if (complainIfItsntTightView == Options::ComplainIfIsntTightView::True &&
	  (t.from != Coor(t.dim.size()) || t.size != t.dim))
	throw std::runtime_error("get_local_srange: unsupported on non tight views");
      return {get_scoor(t.order, t.from), get_scoor(t.order, t.size)};
    }

    /// Return the distribution of the tensor among the processes
    /// \param t: input tensor

    inline CustomDistribuition get_distribution(const Tensor& t)
    {
      return {{Coor(t.dim.size()), t.dim}};
    }

    /// Return whether the tensor is allocated on host or on device
    /// \param t: input tensor

    inline Options::Alloc get_allocation_type(const Tensor& t)
    {
      return Options::Alloc::Host;
    }

    /// Return the range for the dense and sparse dimensions
    /// \param t: input tensor
    /// \return {dense_from, dense_size, dense_dim, sp_from, sp_size, sp_dim}: where
    ///   - dense_from, dense_size and dense_dim: dense range
    ///   - sp_from, sp_size and sp_dim: dense range

    inline std::array<Coor, 6> get_dense_sparse_ranges(const Tensor& t)
    {
      return {Coor(t.from.begin(), t.from.end() - t.n_sp), //
	      Coor(t.size.begin(), t.size.end() - t.n_sp), //
	      Coor(t.dim.begin(), t.dim.end() - t.n_sp),   //
	      Coor(t.from.end() - t.n_sp, t.from.end()),   //
	      Coor(t.size.end() - t.n_sp, t.size.end()),   //
	      Coor(t.dim.end() - t.n_sp, t.dim.end())};
    }

    /// Set the values of the tensor to zero
    /// \param t: tensor to set zero

    inline void set_zero(const Tensor& t)
    {
      if (std::abs(t.scale) == 0)
	return;
      auto& v = *t.ptr.get();
      if (t.n_sp == 0)
      {
	for (std::size_t i = 0; i < volume(t.size); ++i)
	{
	  v.at(coorToIndex(indexToCoor(i, t.size) + t.from, t.dim)) = 0.0;
	}
      }
      else
      {
	auto& cols = *t.cols.get();
	const auto& [dense_from, dense_size, dense_dim, sp_from, sp_size, sp_dim] =
	  get_dense_sparse_ranges(t);
	if (volume(sp_size) == 0)
	  return;
	for (std::size_t i = 0; i < volume(dense_size); ++i)
	{
	  const auto row = coorToIndex(indexToCoor(i, dense_size) + dense_from, dense_dim);
	  for (int j = 0; j < t.max_nnz_per_row; ++j)
	  {
	    auto& col = cols.at(row * t.max_nnz_per_row + j);
	    if (col < 0 ||
		!is_less_than(norm_coor(indexToCoor(col, sp_dim) - sp_from, sp_dim), sp_size))
	    {
	      continue;
	    }
	    col = -1;
	    v.at(row * t.max_nnz_per_row + j) = 0;
	  }
	}
      }
    }

    /// Set an element on a tensor
    /// \param t: tensor to modify
    /// \param coor: coordinate of the tensor to modify
    /// \param val: new value of the tensor element

    inline void set(const Tensor& t, const Coor& coor, const value_type& val)
    {
      if (volume(t.dim) == 0)
	throw std::runtime_error("set: invalid operation on an empty tensor");
      if (coor.size() != t.dim.size())
	throw std::runtime_error("set: invalid input");
      if (std::abs(t.scale) == 0)
	return;
      const auto& coor0 = norm_coor(coor, t.dim);
      const auto explicit_val = cond_conj(t.is_conjugated, val) / t.scale;
      auto& v = *t.ptr.get();
      if (t.n_sp == 0)
      {
	v.at(coorToIndex(coor0 + t.from, t.dim)) = explicit_val;
      }
      else
      {
	auto& cols = *t.cols.get();
	const auto& [dense_from, dense_size, dense_dim, sp_from, sp_size, sp_dim] =
	  get_dense_sparse_ranges(t);
	if (volume(sp_size) == 0)
	  return;
	const auto& dense_coor = Coor(coor0.begin(), coor0.end() - t.n_sp);
	const auto& sp_coor = Coor(coor0.end() - t.n_sp, coor0.end());
	const auto row = coorToIndex(dense_coor + dense_from, dense_dim);
	const auto sp_col = coorToIndex(sp_coor + sp_from, sp_dim);

	// Set the value in an nonzero value
	for (int j = 0; j < t.max_nnz_per_row; ++j)
	{
	  const auto col = cols.at(row * t.max_nnz_per_row + j);
	  if (col == sp_col)
	  {
	    v.at(row * t.max_nnz_per_row + j) = explicit_val;
	    return;
	  }
	}

	// Otherwise, insert a new nonzero value
	for (int j = 0; j < t.max_nnz_per_row; ++j)
	{
	  auto& col = cols.at(row * t.max_nnz_per_row + j);
	  if (col >= 0)
	    continue;
	  col = sp_col;
	  v.at(row * t.max_nnz_per_row + j) = explicit_val;
	  return;
	}

	throw std::runtime_error("set: no more space for inserting a new nonzero value");
      }
    }

    /// Get an element on a tensor
    /// \param t: input tensor
    /// \param coor: coordinate of the tensor to get

    inline value_type get(const Tensor& t, const Coor& coor)
    {
      if (volume(t.dim) == 0)
	throw std::runtime_error("get: invalid operation on an empty tensor");
      if (coor.size() != t.dim.size())
	throw std::runtime_error("get: invalid input");
      if (std::abs(t.scale) == 0)
	return 0;
      const auto& coor0 = norm_coor(coor, t.dim);
      auto& v = *t.ptr.get();
      value_type explicit_val;
      if (t.n_sp == 0)
      {
	explicit_val = v.at(coorToIndex(coor0 + t.from, t.dim));
      }
      else
      {
	auto& cols = *t.cols.get();
	const auto& [dense_from, dense_size, dense_dim, sp_from, sp_size, sp_dim] =
	  get_dense_sparse_ranges(t);
	const auto& dense_coor = Coor(coor0.begin(), coor0.end() - t.n_sp);
	const auto& sp_coor = Coor(coor0.end() - t.n_sp, coor0.end());
	const auto row = coorToIndex(dense_coor + dense_from, dense_dim);
	const auto sp_col = coorToIndex(sp_coor + sp_from, sp_dim);

	// Get the value in an nonzero value
	explicit_val = 0;
	for (int j = 0; j < t.max_nnz_per_row; ++j)
	{
	  const auto col = cols.at(row * t.max_nnz_per_row + j);
	  if (col == sp_col)
	  {
	    explicit_val = v.at(row * t.max_nnz_per_row + j);
	    break;
	  }
	}
      }

      return cond_conj(t.is_conjugated, explicit_val * t.scale);
    }

    /// Execute the given function passing the coordinate and the value for the local nonzero elements
    /// \param t: input tensor
    /// \param fun: function to execute on each value; signature: fun(coor, value)

    template <typename Fun>
    void for_each_coor_and_nonzero_value(const Tensor& t, const Fun& fun)
    {
      if (std::abs(t.scale) == 0)
	return;

      if (!t.ptr)
	throw std::runtime_error("for_each_coor_and_nonzero_value: unsupported on example tensors");

      auto& v = *t.ptr.get();
      if (t.n_sp == 0)
      {
	for (std::size_t i = 0; i < volume(t.size); ++i)
	{
	  const auto& coor = indexToCoor(i, t.size);
	  const auto& explicit_val = v.at(coorToIndex(coor + t.from, t.dim));
	  const auto& val = cond_conj(t.is_conjugated, explicit_val * t.scale);
	  if (std::norm(val) == 0)
	    continue;
	  fun(coor, val);
	}
      }
      else
      {
	auto& cols = *t.cols.get();
	const auto& [dense_from, dense_size, dense_dim, sp_from, sp_size, sp_dim] =
	  get_dense_sparse_ranges(t);
	if (volume(sp_size) == 0)
	  return;
	for (std::size_t i = 0; i < volume(dense_size); ++i)
	{
	  const auto& dense_coor = indexToCoor(i, dense_size);
	  const auto row = coorToIndex(dense_coor + dense_from, dense_dim);
	  for (int j = 0; j < t.max_nnz_per_row; ++j)
	  {
	    const auto col = cols.at(row * t.max_nnz_per_row + j);
	    if (col < 0)
	      continue;
	    const auto& sp_coor = norm_coor(indexToCoor(col, sp_dim) - sp_from, sp_dim);
	    if (!is_less_than(sp_coor, sp_size))
	      continue;
	    const auto& explicit_val = v.at(row * t.max_nnz_per_row + j);
	    const auto& val = cond_conj(t.is_conjugated, explicit_val * t.scale);
	    if (std::norm(val) == 0)
	      continue;
	    fun(concat(dense_coor, sp_coor), val);
	  }
	}
      }
    }

    inline Tensor clone(const Tensor& t);
    inline Tensor scale(const Tensor& t, const value_type& val);

    /// Copy the content of a tensor into another tensor
    /// \param tf: origin tensor
    /// \param td: destination tensor

    inline void copyTo(const Tensor& tf, const Tensor& td)
    {
      if (!is_compatible_for_copying(get_scoor(tf.order, tf.size), get_scoor(td.order, td.size)))
	throw std::runtime_error("copyTo: tensor dimensions don't match");
      if (std::abs(td.scale) == 0 || volume(tf.size) == 0)
	return;
      if (tf.ptr.get() == td.ptr.get())
      {
	copyTo(clone(tf), td);
	return;
      }
      set_zero(td);
      const auto& perm = get_perm(tf.order, td.order);
      for_each_coor_and_nonzero_value(
	tf, [&](const Coor& coor, const value_type& val) { set(td, do_perm(coor, perm, 0), val); });
    }

    /// Add the content of a tensor into another tensor
    /// \param tf: origin tensor
    /// \param td: destination tensor

    inline void addTo(const Tensor& tf, const Tensor& td)
    {
      if (!is_compatible_for_copying(get_scoor(tf.order, tf.size), get_scoor(td.order, tf.size)))
	throw std::runtime_error("addTo: tensor dimensions don't match");
      if (std::abs(tf.scale) == 0 || std::abs(td.scale) == 0 || volume(tf.size) == 0)
	return;
      if (tf.ptr.get() == td.ptr.get())
      {
	addTo(clone(tf), td);
	return;
      }
      const auto& perm = get_perm(tf.order, td.order);
      for_each_coor_and_nonzero_value(tf, [&](const Coor& coor, const value_type& val) {
	const auto& td_coor = do_perm(coor, perm, 0);
	set(td, td_coor, get(td, td_coor) + val);
      });
    }

    /// Return a tensor with the same content but new layout
    /// \param t: input tensor
    /// \param new_order: new order

    inline Tensor reorder(const Tensor& t, const Order& new_order)
    {
      if (!have_same_labels(t.order, new_order))
	throw std::runtime_error("reorder: incompatible new labels");

      return make_sure(t, new_order);
    }

    /// Contract the given tensors
    /// \param a: tensor to contract
    /// \param b: tensor to contract
    /// \param final_order: order of the resulting tensor

    inline Tensor contract(const Tensor& a, const Tensor& b, const Order& final_order)
    {
      if (!is_valid(final_order))
	throw std::runtime_error("contract: invalid `final_order`");
      if (!is_compatible_for_contracting(get_scoor(a.order, a.size), get_scoor(b.order, b.size),
					 final_order))
	throw std::runtime_error("contract: tensor dimensions don't match");

      // Quick case
      if (a.size.size() == 0)
	return scale(b, cond_conj(a.is_conjugated, a.scale));
      if (b.size.size() == 0)
	return scale(a, cond_conj(b.is_conjugated, b.scale));

      // Get the dimensions of the final tensor
      SCoor sa = get_scoor(a.order, a.size);
      SCoor sb = get_scoor(b.order, b.size);
      SCoor sr = get_scoor(final_order, Coor(final_order.size(), 1));
      for (const auto& it : sa)
	if (sr.count(it.first) == 1)
	  sr.at(it.first) = it.second;
      for (const auto& it : sb)
	if (sr.count(it.first) == 1)
	  sr.at(it.first) = it.second;

      // Set the tensors in canonical order for a multiplication:
      // (A, B, T) x (B, C, T) -> (A, C, T)
      Order oA, oB, oC, oT;
      for (char label : a.order)
      {
	if (sb.count(label) == 1 && sr.count(label) == 1)
	  oT.push_back(label);
	else if (sb.count(label) == 0 && sr.count(label) == 1)
	  oA.push_back(label);
	else if (sb.count(label) == 1 && sr.count(label) == 0)
	  oB.push_back(label);
      }
      for (char label : b.order)
      {
	if (sa.count(label) == 0 && sr.count(label) == 1)
	  oC.push_back(label);
      }
      Order new_order_a = concat(oA, oB, oT);
      Order new_order_b = concat(oB, oC, oT);
      Order new_order_r = concat(oA, oC, oT);
      // NOTE: besides reordering the input tensors, clone them to avoid dealing with implicit
      // attributes (scale, conj)
      const auto& ca = make_sure(a, new_order_a, Options::Distribution::Default,
				 Options::Alloc::Host, Options::TightView::True, 0);
      const auto& cb = make_sure(b, new_order_b, Options::Distribution::Default,
				 Options::Alloc::Host, Options::TightView::True, 0);

      // Create final tensor
      Tensor r = create_tensor(get_coor(new_order_r, sr), new_order_r);
      set_zero(r);

      // Do the multiplication
      std::size_t volA = volume(oA, sa, 1);
      std::size_t volB = volume(oB, sa, 1);
      std::size_t volC = volume(oC, sb, 1);
      std::size_t volT = volume(oT, sa, 1);
      const auto& va = *ca.ptr.get();
      const auto& vb = *cb.ptr.get();
      auto& vc = *r.ptr.get();
      for (std::size_t t = 0; t < volT; ++t)
	for (std::size_t i = 0; i < volA; ++i)
	  for (std::size_t j = 0; j < volC; ++j)
	    for (std::size_t k = 0; k < volB; ++k)
	      vc[i + j * volA + t * volA * volC] +=
		va[i + k * volA + t * volA * volB] * vb[k + j * volB + t * volB * volC];

      return r;
    }

    /// Return the Frobenius norm of the tensor
    /// \param t: input tensor

    inline double frob(const Tensor& t)
    {
      if (std::abs(t.scale) == 0)
	return 0;
      double r = 0;
      for_each_coor_and_nonzero_value(
	t, [&](const Coor&, const value_type& val) { r += std::norm(val); });
      return std::sqrt(r);
    }

    /// Return a copy of the tensor
    /// \param t: input tensor

    inline Tensor clone(const Tensor& t)
    {
      Tensor n = like_this(t);
      copyTo(t, n);
      return n;
    }

    /// Return a tensor with values implicitly scaled
    /// \param t: input tensor
    /// \param val: weight of the scale

    inline Tensor scale(const Tensor& t, const value_type& val)
    {
      Tensor n = t;
      n.scale *= cond_conj(t.is_conjugated, val);
      return n;
    }

    /// Return a view of a vector
    /// \param v: object
    /// \param order: labels for the dimensions
    /// \param size: size for the dimensions

    inline Tensor toTensor(const std::vector<value_type>& v, const Order& order, const SCoor& size)
    {
      if (order.size() == 0)
	throw std::runtime_error("toTensor: order with invalid number of dimensions");
      if (volume(order, size) != v.size())
	throw std::runtime_error("toTensor: dimensions don't match vector length");
      Coor dim = get_coor(order, size);
      auto p = (const std::complex<double>*)v.data();
      auto ptr = std::make_shared<std::vector<std::complex<double>>>(p, p + volume(dim));
      return Tensor{Coor(dim.size()), dim, dim, order, ptr, 1.0, false, 0, 0, {}};
    }

    /// Return a view of the Array2d object
    /// \param op: object
    /// \param order: labels for the dimensions

    inline Tensor toTensor(const XMLArray::Array2d<std::complex<double>>& op, const Order& order)
    {
      if (order.size() != 2)
	throw std::runtime_error("toTensor: invalid number of dimensions");
      Coor dim = {op.size1(), op.size2()};
      auto p = (const std::complex<double>*)op;
      auto ptr = std::make_shared<std::vector<std::complex<double>>>(p, p + volume(dim));
      return Tensor{Coor(dim.size()), dim, dim, reverse(order), ptr, 1.0, false, 0, 0, {}};
    }

    /// Return a view of the Array3d object
    /// \param op: object
    /// \param order: labels for the dimensions

    inline Tensor toTensor(const XMLArray::Array3d<value_type>& op, const Order& order)
    {
      if (order.size() != 3)
	throw std::runtime_error("toTensor: invalid number of dimensions");
      Coor dim = {op.size1(), op.size2(), op.size3()};
      auto p = (const value_type*)op;
      auto ptr = std::make_shared<std::vector<value_type>>(p, p + volume(dim));
      return Tensor{Coor(dim.size()), dim, dim, reverse(order), ptr, 1.0, false, 0, 0, {}};
    }

    /// Return a view of the Array4d object
    /// \param op: object
    /// \param order: labels for the dimensions

    inline Tensor toTensor(const XMLArray::Array4d<value_type>& op, const Order& order)
    {
      if (order.size() != 4)
	throw std::runtime_error("toTensor: invalid number of dimensions");
      Coor dim = {op.size1(), op.size2(), op.size3(), op.size4()};
      auto p = (const value_type*)op;
      auto ptr = std::make_shared<std::vector<value_type>>(p, p + volume(dim));
      return Tensor{Coor(dim.size()), dim, dim, reverse(order), ptr, 1.0, false, 0, 0, {}};
    }

    /// Finalize pending operation and destroy handlers

    inline void finalize()
    {
      // do nothing
    }
  }

#if defined(USE_SUPERBBLAS) && !defined(SUPERBNOVA_DEBUG)
  //
  // Superbblas implementation for performance
  //

  namespace superbblas_implementation
  {
    struct Tensor;
  }

  inline superbblas_implementation::Tensor slice(const superbblas_implementation::Tensor& t,
						 const Coor& from, const Coor& size);

  namespace superbblas_implementation
  {
    namespace detail
    {
      /// Return whether to check against the reference implementation

      inline bool& check_against_reference_implementation()
      {
	static bool c = SBN::detail::get_debug_level() > 1;
	return c;
      }

      /// Destroy function
      using DestroyFun = std::function<void()>;

      /// Return a list of destroy callbacks to execute just before finishing chroma

      inline std::vector<DestroyFun>& getDestroyList()
      {
	static std::vector<DestroyFun> list;
	return list;
      }

      // Get the cpu context
      inline const std::shared_ptr<superbblas::Context>& getCpuContext()
      {
	static std::shared_ptr<superbblas::Context> ctx = [&] {
	  getDestroyList().push_back([&] { ctx.reset(); });
	  return std::make_shared<superbblas::Context>(superbblas::createCpuContext());
	}();
	return ctx;
      }

      // Return a context on either the host or the device
      inline const std::shared_ptr<superbblas::Context>& getGpuContext()
      {
#  ifdef SUPERBBLAS_USE_GPU
	// Creating GPU context can be expensive; so do it once
	static std::shared_ptr<superbblas::Context> ctx = [&] {
	  getDestroyList().push_back([&] { ctx.reset(); });
	  return std::make_shared<superbblas::Context>(
	    superbblas::createGpuContext(get_default_gpu_device()));
	}();
	return ctx;
#  else
	return getCpuContext();
#  endif
      }

      /// Return whether a context is on the cpu

      inline bool is_context_cpu(const superbblas::Context& ctx)
      {
#  ifdef SUPERBBLAS_USE_GPU
	return ctx.plat == superbblas::CPU;
#  else
	return true;
#  endif
      }

      /// Data allocation
      template <typename T>
      struct Allocation {
	/// Allocation
	T* const ptr;

	/// Context of the allocation
	const std::shared_ptr<superbblas::Context> ctx;

	/// Unfinished operations on the allocation
	std::vector<superbblas::Request> pending_operations;

	/// Deallocate the pointer on destruction (aka being managed)
	const bool destroy_ptr;

	/// Allocate n elements of type T with the given context
	/// \param n: number of elements to allocate
	/// \param ctx: context of the allocation

	Allocation(std::size_t n, const std::shared_ptr<superbblas::Context>& ctx)
	  : ptr(superbblas::allocate<T>(n, *ctx)), ctx(ctx), destroy_ptr(true)
	{
	}

	/// User given pointer that will not be deallocated automatically
	/// \param ptr: pointer to an allocation
	/// \param ctx: context of the allocation

	Allocation(T* ptr, const std::shared_ptr<superbblas::Context>& ctx)
	  : ptr(ptr), ctx(ctx), destroy_ptr(false)
	{
	}

	/// Destructor

	~Allocation()
	{
	  finish_pending_operations();
	  if (destroy_ptr)
	    superbblas::deallocate(ptr, *ctx);
	}

	/// Return the pointer

	T* data()
	{
	  finish_pending_operations();
	  return ptr;
	}

	/// Append a pending operation
	/// \param req: pending operation to finish later
	/// NOTE: finish the operation for pointers not managed; those come from Chroma objects
	///       and Chroma users may get unexpected results if they access the Chroma object
	///       while the proxy Tensor object created with `asTensorView` is still alive.

	void append_pending_operation(const superbblas::Request& req)
	{
	  if (destroy_ptr)
	  {
	    if (req)
	      pending_operations.push_back(req);
	  }
	  else
	  {
	    superbblas::wait(req);
	  }
	}

	/// Finish all pending operations
	void finish_pending_operations()
	{
	  for (const auto& i : pending_operations)
	    superbblas::wait(i);
	  pending_operations.clear();
	}
      };

      template <typename T, std::size_t N>
      struct tensorNd {

	tensorNd(const std::array<T, N>& dims = {{}}) : v(N * 2 + volume(dims))
	{
	  std::copy_n(dims.begin(), N, v.begin());
	  set_strides();
	}

	std::size_t size(std::size_t dimension) const
	{
	  if (dimension < N)
	    return std::size_t(v.at(dimension));
	  throw std::runtime_error("vector2d::size(): invalid input");
	}

	const T& at(const std::array<T, N>& coors) const
	{
	  return v.at(N * 2 + get_index(coors));
	}

	T& at(const std::array<T, N>& coors)
	{
	  return v.at(N * 2 + get_index(coors));
	}

	typename std::vector<T>::const_iterator begin() const
	{
	  return v.begin() + N * 2;
	}

	typename std::vector<T>::iterator begin()
	{
	  return v.begin() + N * 2;
	}

	typename std::vector<T>::const_iterator begin(const std::array<T, N>& coor) const
	{
	  return v.begin() + N * 2 + get_index(coor);
	}

	typename std::vector<T>::iterator begin(const std::array<T, N>& coor)
	{
	  return v.begin() + N * 2 + get_index(coor);
	}

	bool operator==(const tensorNd&) const = default;

      private:
	void set_strides()
	{
	  if (N == 0)
	    return;
	  v.at(N) = 1;
	  for (std::size_t i = 1; i < N; ++i)
	    v.at(N + i) = v.at(N + i - 1) * v.at(i - 1);
	}

	std::size_t get_index(const std::array<T, N>& coor) const
	{
	  if (N == 0)
	    return 0;
	  const auto& dim = v.begin();
	  const auto& strides = v.begin() + N;
	  T r = 0;
	  for (std::size_t j = 0; j < N; j++)
	    r += coor[j] * strides[j];
	  if (r + N * 2 > v.size())
	    throw std::out_of_range("coordinate out of range");
	  return r;
	}

      public:
	/// Container
	std::vector<T> v;
      };

      /// Return the dense and the sparse coordinates
      /// \param coor: coordinate to split

      template <typename V>
      inline std::array<V, 2> split_dense_sparse(const V& v, int n_sp)
      {
	return {V(v.begin(), v.end() - n_sp), V(v.end() - n_sp, v.end())};
      }

      struct Distribution {
	using Kind = enum { Distributed, OnMaster };
	using Fs = tensorNd<IndexType, 3>;
	using PreferredDistDims = std::vector<unsigned char>;
	Kind kind;
	Coor dim;
	PreferredDistDims preferred_dist_dims;
	Fs fs;			/// (coor, from/size, proc)
	std::vector<int> ranks; ///< world comm ranks
	MpiComm comm;		///< MPI communicator

	Distribution(Kind kind, const Coor& dim,
		     const std::vector<unsigned char>& preferred_dist_dims,
		     const tensorNd<IndexType, 3>& fs, const std::vector<int>& ranks, MpiComm comm)
	  : kind(kind),
	    dim(dim),
	    preferred_dist_dims(preferred_dist_dims),
	    fs(fs),
	    ranks(ranks),
	    comm(comm)
	{
	}

	Distribution(Kind kind, const Coor& dim,
		     const std::vector<unsigned char>& preferred_dist_dims = {},
		     const std::vector<int>& ranks = {get_rank()},
		     MpiComm comm = get_local_mpi_comm())
	  : kind(kind),
	    dim(dim),
	    preferred_dist_dims(preferred_dist_dims),
	    fs({IndexType(dim.size()), 2, (IndexType)ranks.size()}),
	    ranks(ranks),
	    comm(comm)
	{
	  // Filter out dimensions with a single element to be distributed
	  PreferredDistDims this_preferred_dist_dims;
	  this_preferred_dist_dims.reserve(preferred_dist_dims.size());
	  for (const auto i : preferred_dist_dims)
	    if (dim.at(i) > 1)
	      this_preferred_dist_dims.push_back(i);

	  IndexType nprocs = get_nprocs(comm);
	  if (ranks.size() != nprocs)
	    throw std::runtime_error("wtf");

	  if (nprocs == 1 || kind == OnMaster)
	  {
	    std::copy(dim.begin(), dim.end(), fs.begin({{0, 1, 0}}));
	  }
	  else if (this_preferred_dist_dims.size() == 0)
	  {
	    // Replicated
	    for (IndexType nproc = 0; nproc < nprocs; ++nproc)
	      std::copy(dim.begin(), dim.end(), fs.begin({{0, 1, nproc}}));
	  }
	  else if (kind == Distributed)
	  {
	    // Simple approach, divide by the first proposed dimension to parallelize
	    for (IndexType nproc = 0; nproc < nprocs; ++nproc)
	      std::copy(dim.begin(), dim.end(), fs.begin({{0, 1, nproc}}));
	    const int p = this_preferred_dist_dims.at(0);
	    if (p < 0 || p >= (int)dim.size())
	      throw std::runtime_error("wtf");
	    for (IndexType nproc = 0; nproc < nprocs; ++nproc)
	    {
	      fs.at({{p, 0, nproc}}) =
		dim.at(p) / nprocs * nproc + std::min(dim.at(p) % nprocs, nproc);
	      fs.at({{p, 1, nproc}}) = dim.at(p) / nprocs + (dim.at(p) % nprocs > nproc ? 1 : 0);
	    }
	  }
	}
      };

      /// Return a hash associated to the content of a distribution
      /// \param dist: input distribution

      inline std::size_t get_fingerprint(const Distribution& dist)
      {
	return SB::getHash(
	  std::make_tuple(dist.kind, dist.dim, dist.preferred_dist_dims, dist.fs.v, dist.ranks));
      }

      /// Return the indices of the labels appearing in an order
      /// \param order: input order
      /// \param dist_dims: labels to look for at `order`
      /// NOTE: if a label is present more than once at `dist_dims` only the first time is reported

      inline Distribution::PreferredDistDims get_preferred_dist_dims(const Order& order,
								     const std::string& dist_dims)
      {
	Distribution::PreferredDistDims preferred_dist_dims;
	preferred_dist_dims.reserve(dist_dims.size());
	std::vector<char> selected(order.size());
	for (char c : dist_dims)
	{
	  unsigned int index = std::find(order.begin(), order.end(), c) - order.begin();
	  if (index < order.size() && selected.at(index) == 0)
	  {
	    preferred_dist_dims.push_back(index);
	    selected.at(index) = 1;
	  }
	}
	return preferred_dist_dims;
      }

      /// Return whether the distribution can qualified as on master
      /// \param distribution: given distribution

      inline bool is_distribution_on_master(const Distribution& distribution)
      {
	if (distribution.kind != Distribution::OnMaster)
	  return false;
	for (IndexType i = 1; i < distribution.fs.size(2); ++i)
	{
	  if (volume(Coor(distribution.fs.begin({0, 1, i}), distribution.fs.begin({0, 0, i + 1}))) >
	      0)
	  {
	    return false;
	  }
	}
	return Coor(distribution.fs.begin({0, 0, 0}), distribution.fs.begin({0, 1, 0})) ==
		 Coor(distribution.fs.size(0)) &&
	       Coor(distribution.fs.begin({0, 1, 0}), distribution.fs.begin({0, 0, 1})) ==
		 distribution.dim;
      }

      /// Get the first coordinate of the local support of a distribution
      /// \param distribution: given distribution

      inline Coor get_local_from(const Distribution& distribution)
      {
	const int rank = get_rank(distribution.comm);
	return Coor(distribution.fs.begin({0, 0, rank}), distribution.fs.begin({0, 1, rank}));
      }

      /// Get the dimensions of the local support of a distribution
      /// \param distribution: given distribution

      inline Coor get_local_size(const Distribution& distribution)
      {
	const int rank = get_rank(distribution.comm);
	return Coor(distribution.fs.begin({0, 1, rank}), distribution.fs.begin({0, 0, rank + 1}));
      }

      /// Restrict the distribution to a subset of the ranks
      /// \param distribution: given distribution
      /// \param new_comm: new communicator

      inline Distribution get_distribution_supported_on(const Distribution& distribution,
							const Comm& new_comm)
      {
	// Get the position of the new ranks
	std::vector<int> idx;
	idx.reserve(new_comm.ranks.size());
	for (const auto rank : new_comm.ranks)
	{
	  auto it = std::find(distribution.ranks.begin(), distribution.ranks.end(), rank);
	  if (it == distribution.ranks.end())
	    throw std::runtime_error("get_distribution_supported_on: the new communicator is not a "
				     "subset of the given tensor communicator");
	  idx.push_back(it - distribution.ranks.begin());
	}
	Distribution r(distribution.kind, distribution.dim, distribution.preferred_dist_dims,
		       new_comm.ranks, new_comm.comm);
	for (IndexType i = 0; i < new_comm.ranks.size(); ++i)
	{
	  std::copy(distribution.fs.begin({0, 0, idx.at(i)}),
		    distribution.fs.begin({0, 0, idx.at(i) + 1}), r.fs.begin({0, 0, i}));
	}
	return r;
      }

      /// Return a coordinate from std::vector to std::array
      /// \param v: input coordinate

      template <std::size_t N>
      std::array<int, N> to_coor(const std::vector<int>& v)
      {
	if (v.size() != N)
	  throw std::runtime_error("to_coor: invalid input");
	std::array<int, N> r;
	std::copy_n(v.begin(), N, r.begin());
	return r;
      }

      /// Return a coordinate from std::vector to std::array
      /// \param v: input coordinate
      /// \param missing: extend the remaining coordinates with the given value

      template <std::size_t N, typename iterator>
      std::array<int, N> extend_to_coor(iterator begin, iterator end, int missing)
      {
	std::size_t size = std::distance(begin, end);
	if (size > N)
	  throw std::runtime_error("to_coor: invalid input");
	std::array<int, N> r;
	std::copy(begin, end, r.begin());
	for (std::size_t i = size; i < N; ++i)
	  r.at(i) = missing;
	return r;
      }

      /// Return a coordinate from std::vector to std::array
      /// \param v: input coordinate
      /// \param missing: extend the remaining coordinates with the given value

      template <std::size_t N, typename iterator>
      std::array<int, N> extend_to_coor(IndexType c, iterator begin, iterator end, int missing)
      {
	std::size_t size = std::distance(begin, end);
	if (1 + size > N)
	  throw std::runtime_error("extend_to_coor: invalid input");
	std::array<int, N> r;
	r.at(0) = c;
	std::copy(begin, end, r.begin() + 1);
	for (std::size_t i = 1 + size; i < N; ++i)
	  r.at(i) = missing;
	return r;
      }

      /// Return a coordinate from std::vector to std::array
      /// \param v: input coordinate
      /// \param missing: extend the remaining coordinates with the given value

      template <std::size_t N, typename iterator>
      std::array<int, N> extend_to_coor(IndexType c0, IndexType c1, iterator begin, iterator end,
					int missing)
      {
	std::size_t size = std::distance(begin, end);
	if (2 + size > N)
	  throw std::runtime_error("extend_to_coor: invalid input");
	std::array<int, N> r;
	r.at(0) = c0;
	r.at(1) = c1;
	std::copy(begin, end, r.begin() + 2);
	for (std::size_t i = 2 + size; i < N; ++i)
	  r.at(i) = missing;
	return r;
      }

      /// Return a coordinate from std::vector to std::array
      /// \param v: input coordinate
      /// \param missing: extend the remaining coordinates with the given value

      template <std::size_t N, typename iterator>
      std::array<int, N> extend_to_coor(IndexType c0, IndexType c1, IndexType c2, iterator begin,
					iterator end, int missing)
      {
	std::size_t size = std::distance(begin, end);
	if (3 + size > N)
	  throw std::runtime_error("extend_to_coor: invalid input");
	std::array<int, N> r;
	r.at(0) = c0;
	r.at(1) = c1;
	r.at(2) = c2;
	std::copy(begin, end, r.begin() + 3);
	for (std::size_t i = 3 + size; i < N; ++i)
	  r.at(i) = missing;
	return r;
      }

      /// Return a coordinate from std::vector to std::array
      /// \param v: input coordinate
      /// \param missing: extend the remaining coordinates with the given value

      template <std::size_t N>
      std::array<int, N> extend_to_coor(const Coor& coor, int missing)
      {
	std::size_t size = coor.size();
	if (size > N)
	  throw std::runtime_error("extend_to_coor: invalid input");
	std::array<int, N> r;
	std::copy(coor.begin(), coor.end(), r.begin());
	for (std::size_t i = size; i < N; ++i)
	  r.at(i) = missing;
	return r;
      }

      /// Return a coordinate from std::vector to std::array
      /// \param v: input coordinate
      /// \param missing: extend the remaining coordinates with the given value

      template <std::size_t N>
      std::array<int, N> extend_to_coor(IndexType c, const Coor& coor, int missing)
      {
	std::size_t size = coor.size();
	if (size + 1 > N)
	  throw std::runtime_error("extend_to_coor: invalid input");
	std::array<int, N> r;
	r.at(0) = c;
	std::copy(coor.begin(), coor.end(), r.begin() + 1);
	for (std::size_t i = 1 + size; i < N; ++i)
	  r.at(i) = missing;
	return r;
      }

      /// Return a coordinate from std::vector to std::array
      /// \param v: input coordinate
      /// \param missing: extend the remaining coordinates with the given value

      template <std::size_t N>
      std::array<int, N> extend_to_coor(IndexType c0, IndexType c1, const Coor& coor, int missing)
      {
	std::size_t size = coor.size();
	if (size + 2 > N)
	  throw std::runtime_error("extend_to_coor: invalid input");
	std::array<int, N> r;
	r.at(0) = c0;
	r.at(1) = c1;
	std::copy(coor.begin(), coor.end(), r.begin() + 2);
	for (std::size_t i = 2 + size; i < N; ++i)
	  r.at(i) = missing;
	return r;
      }

      /// Return a coordinate from std::vector to std::array
      /// \param v: input coordinate
      /// \param missing: extend the remaining coordinates with the given value

      template <std::size_t N>
      std::array<int, N> extend_to_coor(IndexType c0, IndexType c1, IndexType c2, const Coor& coor,
					int missing)
      {
	std::size_t size = coor.size();
	if (size + 3 > N)
	  throw std::runtime_error("extend_to_coor: invalid input");
	std::array<int, N> r;
	r.at(0) = c0;
	r.at(1) = c1;
	r.at(2) = c2;
	std::copy(coor.begin(), coor.end(), r.begin() + 3);
	for (std::size_t i = 3 + size; i < N; ++i)
	  r.at(i) = missing;
	return r;
      }

      /// Return whether all processes have the same support on the last coordinates
      /// \param ranges: input ranges
      /// \param n_sp: the last `n_sp` coordinates to check

      inline bool are_sparse_components_replicated(const Distribution::Fs& ranges, int n_sp)
      {
	if (n_sp == 0)
	  return true;
	const auto ndim_dense = ranges.size(0) - n_sp;
	IndexType first_nonempty_component = -1;
	for (IndexType i = 0; i < IndexType(ranges.size(2)); ++i)
	{
	  if (volume(ranges.begin({0, 0, i}), ranges.begin({0, 1, i})) == 0)
	    continue;
	  if (first_nonempty_component < 0)
	  {
	    first_nonempty_component = i;
	    continue;
	  }
	  for (IndexType j = ndim_dense; j < ranges.size(0); ++j)
	  {
	    if (*ranges.begin({j, 0, first_nonempty_component}) != *ranges.begin({j, 0, i}) ||
		*ranges.begin({j, 1, first_nonempty_component}) != *ranges.begin({j, 1, i}))
	    {
	      return false;
	    }
	  }
	}
	return true;
      }

      /// Return the distribution of the tensor among the processes
      /// \param ranges: input distribution

      inline CustomDistribuition get_distribution(const Distribution::Fs& ranges)
      {
	CustomDistribuition r;
	r.reserve(ranges.size(2));
	for (int proc = 0; proc < ranges.size(2); ++proc)
	{
	  r.push_back({Coor(ranges.begin({0, 0, proc}), ranges.begin({0, 1, proc})),
		       Coor(ranges.begin({0, 1, proc}), ranges.begin({0, 0, proc + 1}))});
	}
	return r;
      }

      /// Return an unique id for each partition
      /// \param ranges: input distribution
      /// \param dim: tensor dimensions
      /// \param n_sp: (optional) number of sparse dimensions
      /// \param return_unique_id_for_empty_ranges: (optional) whether to return different ids for empty ranges (default, false)

      inline std::vector<int>
      get_unique_range_ids(const Distribution::Fs& ranges, const Coor& dim, int n_sp = 0,
			   Options::ReturnUniqueIdForEmptyRanges return_unique_id_for_empty_ranges =
			     Options::ReturnUniqueIdForEmptyRanges::False)
      {
	if (n_sp > 0 && are_sparse_components_replicated(ranges, n_sp))
	  return std::vector<int>(ranges.size(2), 0);
	std::vector<int> r(ranges.size(2));
	const auto& d = get_distribution(ranges);
	int n = 0;
	for (std::size_t i = 0; i < d.size(); ++i)
	{
	  if (volume(d.at(i).at(1)) > 0)
	  {
	    bool is_intersection = false;
	    for (std::size_t j = 0; j < i; ++j)
	    {
	      if (d.at(i) == d.at(j))
	      {
		r.at(i) = r.at(j);
		is_intersection = true;
		break;
	      }
	      if (!is_intersection_empty(d.at(i).at(0), d.at(i).at(1), d.at(j).at(0), d.at(j).at(1),
					 dim))
	      {
		throw std::runtime_error(
		  "get_unique_range_ids: unsupported case with overlap ranges");
	      }
	    }
	    if (!is_intersection)
	      r.at(i) = n++;
	  }
	  else if (return_unique_id_for_empty_ranges !=
		   Options::ReturnUniqueIdForEmptyRanges::False)
	  {
	    r.at(i) = n++;
	  }
	}

	return r;
      }

      /// Return an unique id for each partition
      /// \param from0: first coordinate to copy from origin
      /// \param size0: number of elements to copy from origin
      /// \param ranges0: origin distribution
      /// \param dim0: origin tensor dimensions
      /// \param n_sp0: origin number of sparse dimensions
      /// \param max_nnz_per_row0: origin number of sparse dimensions
      /// \param perm: coordinates permutation from origin to destination
      /// \param from1: first coordinate to copy from destination
      /// \param ranges1: destination distribution
      /// \param dim1: destination tensor dimensions
      /// \param n_sp1: destination number of sparse dimensions
      /// \param max_nnz_per_row1: destination number of sparse dimensions
 
      inline std::tuple<std::vector<int>, std::vector<int>, int> get_unique_range_ids_for_copy(
        const Coor& from0, const Coor& size0, const Distribution::Fs& ranges0, const Coor& dim0,
        int n_sp0, int max_nnz_per_row0, const Coor& perm, const Coor& from1,
        const Distribution::Fs& ranges1, const Coor& dim1, int n_sp1, int max_nnz_per_row1)
      {
	// Check input arguments
	if (from0.size() != size0.size() || from0.size() != ranges0.size(0) ||
	    from0.size() != dim0.size() || n_sp0 > from0.size() || perm.size() != from1.size() ||
	    from1.size() != ranges1.size(0) || from1.size() != dim1.size() ||
	    n_sp1 > from1.size() || ranges0.size(2) != ranges1.size(2))
	  throw std::runtime_error("get_unique_range_ids_for_copy: invalid input");

	// Check for unsupported cases: any transformation of the sparse components
	if (n_sp0 != n_sp1)
	  throw std::runtime_error("get_unique_range_ids_for_copy: unsupported case, different "
				   "number of sparse dimensions");
	for (unsigned int i = 0; i < n_sp0; ++i)
	  if (perm.at(perm.size() - n_sp1 + i) != dim0.size() - n_sp0 + i)
	    throw std::runtime_error("get_unique_range_ids_for_copy: unsupported case, permutation "
				     "of the sparse components");
	for (unsigned int i = 0; i < n_sp0; ++i)
	  if (from0.at(dim0.size() - n_sp0 + i) != from1.at(dim1.size() - n_sp1 + i))
	    throw std::runtime_error(
	      "get_unique_range_ids_for_copy: unsupported case, different dimensions");

	// Deal with trivial case
	if (n_sp0 == 0)
	  return {{}, {}, 0};

	// Check for unsupported cases the maximum number of nonzeros should be congruence
	if (max_nnz_per_row0 % max_nnz_per_row1 != 0 && max_nnz_per_row1 % max_nnz_per_row0 != 0)
	  throw std::runtime_error(
	    "get_unique_range_ids_for_copy: unsupported case, uncongruence max_nnz_per_row");

	// Normalize case for max_nnz_per_row0 <= max_nnz_per_row1
	if (max_nnz_per_row0 > max_nnz_per_row1)
	{
	  const auto& [new_ids1, new_ids0, new_max_nnz_per_row] = get_unique_range_ids_for_copy(
	    from1, do_perm(size0, perm, 1), ranges1, dim1, n_sp1, max_nnz_per_row1,
	    reverse_perm(perm, dim0.size()), from0, ranges0, dim0, n_sp0, max_nnz_per_row0);
	  return {new_ids0, new_ids1, new_max_nnz_per_row};
	}

	// Get the ids for the destination
	const auto& ids1 = get_unique_range_ids(ranges1, dim1);
	std::vector<int> new_ids1(ranges0.size(2));
	const auto num_blocks = max_nnz_per_row1 / max_nnz_per_row0;
	for (int i = 0; i < ranges0.size(2); ++i)
	  new_ids1.at(i) = ids1.at(i) * num_blocks;

	// Get the ids for the origin
	const auto& ids0 = get_unique_range_ids(ranges0, dim0);
	std::vector<int> new_ids0(ranges0.size(2));
	std::vector<int> from_id_to_new_id(ranges0.size(2), -1);
	std::vector<int> num_hits(ranges0.size(2));
	const auto& d0 = get_distribution(ranges0);
	const auto& d1 = get_distribution(ranges1);
	const auto& size1 = do_perm(size0, perm, 1);
	for (int i = 0; i < ranges0.size(2); ++i)
	{
	  if (from_id_to_new_id.at(ids0.at(i)) >= 0)
	  {
	    new_ids0.at(i) = from_id_to_new_id.at(ids0.at(i));
	  }
	  else
	  {
	    const auto& [from0i, size0i] =
	      norm_from_size(do_perm(norm_coor(d0.at(i).at(0) - from0, dim0), perm, 0) + from1,
			     do_perm(d0.at(i).at(1), perm, 1), dim1);
	    auto& new_id_for_i = from_id_to_new_id.at(ids0.at(i));
	    new_id_for_i = -1;
	    for (int j = 0; j < ranges0.size(2); ++j)
	    {
	      if (!is_intersection_empty(from0i, size0i, from1, size1, d1.at(j).at(0),
					 d1.at(j).at(1), dim1))
	      {
		if (new_id_for_i < 0)
		{
		  new_ids0.at(i) = new_id_for_i =
		    ids1.at(j) * num_blocks + num_hits.at(ids1.at(j))++;
		}
		else if (ids1.at(j) != new_id_for_i / num_blocks)
		{
		  throw std::runtime_error(
		    "get_unique_range_ids_for_copy: unsupported case, unsupported partition");
		}
	      }
	    }
	  }
	}

	const auto new_max_nnz_per_row = std::min(max_nnz_per_row0, max_nnz_per_row1);
	return {new_ids0, new_ids1, new_max_nnz_per_row};
      }

      /// Return the dense ranges after extending the dimensions and the proper labels
      /// \param ranges: input ranges to extend
      /// \param dim: input tensor dimensions
      /// \param complex_label: label for an explicit complex dimension (give 0 for implicit)
      /// \param order: input order
      /// \param n_sp: the last `n_sp` dimensions of the input coordinate are sparse
      /// \param max_nnz_per_row: maximum nonzeros for each dense coordinate
      /// \param used_labels: labels to avoid when assigning labels to the new dimensions
      /// \return {new_ranges, new_order}

      template <std::size_t N>
      std::tuple<std::vector<superbblas::PartitionItem<N>>, std::string>
      extend_range_and_order_dense(const Distribution::Fs& ranges, const Coor& dim,
				   char complex_label, char sp_label, const std::string& order,
				   int n_sp = 0, int max_nnz_per_row = -1,
				   const std::vector<int>& ids = {}, int nnz_block = -1,
				   const std::string& used_labels = {})
      {
	if (ranges.size(0) + (n_sp > 0 ? 1 : 0) + (complex_label ? 1 : 0) > N)
	  throw std::runtime_error("extend_range_and_order_dense: invalid input");
	if (ranges.size(0) != order.size() || n_sp >= order.size())
	  throw std::runtime_error("extend_range_and_order_dense: invalid input");
	if (n_sp > 0 && max_nnz_per_row < 0)
	  throw std::runtime_error("extend_range_and_order_dense: invalid input");
	if (n_sp > 0 && sp_label == 0)
	  throw std::runtime_error("extend_range_and_order_dense: invalid input");

	// Extend the order; grab the last sparse label for sparse tensors as part of the dense tensor
	std::string new_order(order.begin(), order.end() - n_sp);
	if (n_sp > 0)
	{
	  new_order.insert(0, 1, order.back());
	  new_order.insert(0, 1, sp_label);
	}
	if (complex_label)
	  new_order.insert(0, 1, complex_label);
	std::string all_used_labels = (complex_label ? std::string{complex_label} : std::string{}) +
				      (n_sp > 0 ? std::string{sp_label} : std::string{}) + order +
				      used_labels;
	new_order.reserve(N);
	all_used_labels.reserve(used_labels.size() + N);
	for (std::size_t i = new_order.size(); i < N; ++i)
	{
	  char new_label = get_new_label(all_used_labels);
	  new_order.push_back(new_label);
	  all_used_labels.push_back(new_label);
	}

	// Extend the ranges
	std::vector<superbblas::PartitionItem<N>> new_ranges(ranges.size(2));
	const int ndim_dense = (int)order.size() - n_sp;
	if (n_sp == 0)
	{
	  for (IndexType i = 0; i < IndexType(ranges.size(2)); ++i)
	  {
	    if (!complex_label)
	    {
	      new_ranges.at(i) = {
		extend_to_coor<N>(ranges.begin({0, 0, i}), ranges.begin({ndim_dense, 0, i}), 0),
		extend_to_coor<N>(ranges.begin({0, 1, i}), ranges.begin({ndim_dense, 1, i}), 1)};
	    }
	    else
	    {
	      new_ranges.at(i) = {
		extend_to_coor<N>(0, ranges.begin({0, 0, i}), ranges.begin({ndim_dense, 0, i}), 0),
		extend_to_coor<N>(2, ranges.begin({0, 1, i}), ranges.begin({ndim_dense, 1, i}), 1)};
	    }
	  }
	}
	else
	{
	  const auto& ids0 = ids.size() > 0 ? ids : get_unique_range_ids(ranges, dim, n_sp);
	  if (nnz_block < 0)
	    nnz_block = max_nnz_per_row;
	  for (IndexType i = 0; i < IndexType(ranges.size(2)); ++i)
	  {
	    const auto is_empty =
	      (volume(Coor(ranges.begin({0, 1, i}), ranges.begin({0, 0, i + 1}))) == 0);
	    if (!complex_label)
	    {
	      new_ranges.at(i) = {
		extend_to_coor<N>(0, ids0.at(i) * max_nnz_per_row / nnz_block,
				  ranges.begin({0, 0, i}), ranges.begin({ndim_dense, 0, i}), 0),
		extend_to_coor<N>(is_empty ? 0 : nnz_block,
				  is_empty ? 0 : max_nnz_per_row / nnz_block,
				  ranges.begin({0, 1, i}), ranges.begin({ndim_dense, 1, i}), 1)};
	    }
	    else
	    {
	      new_ranges.at(i) = {
		extend_to_coor<N>(0, 0, ids0.at(i) * max_nnz_per_row / nnz_block,
				  ranges.begin({0, 0, i}), ranges.begin({ndim_dense, 0, i}), 0),
		extend_to_coor<N>(is_empty ? 0 : 2, is_empty ? 0 : nnz_block,
				  is_empty ? 0 : max_nnz_per_row / nnz_block,
				  ranges.begin({0, 1, i}), ranges.begin({ndim_dense, 1, i}), 1)};
	    }
	  }
	}

	return {new_ranges, new_order};
      }

      /// Return the dense and sparse BSR ranges after extending the dimensions and the proper labels
      /// \param ranges: input ranges to extend
      /// \param dim: input tensor dimensions
      /// \param order: input order
      /// \param n_sp: the last `n_sp` dimensions of the input coordinate are sparse
      /// \param used_labels: labels to avoid when assigning labels to the new dimensions
      /// \return {new_ranges_dense, new_order_dense, new_ranges_sp, new_order_sp}

      template <std::size_t N>
      std::tuple<std::vector<superbblas::PartitionItem<N>>,
		 std::vector<superbblas::PartitionItem<N>>, std::string, std::string>
      extend_range_and_order_bsr(const Distribution::Fs& ranges, const Coor& dim,
				 const std::string& order, int n_sp,
				 const std::string& used_labels = {})
      {
	if (ranges.size(0) > N)
	  throw std::runtime_error("extend_range_and_order_bsr: invalid input");
	if (ranges.size(0) != order.size() || dim.size() != order.size())
	  throw std::runtime_error("extend_range_and_order_bsr: invalid input");
	if (n_sp == 0 || n_sp >= ranges.size(0))
	  throw std::runtime_error("extend_range_and_order_bsr: invalid input");

	// Extend the order
	auto [new_order_dense, new_order_sp] = split_dense_sparse(order, n_sp);
	auto all_used_labels = order + used_labels;
	new_order_dense.reserve(N);
	new_order_sp.reserve(N);
	all_used_labels.reserve(used_labels.size() + 2 * N);
	for (std::size_t i = new_order_dense.size(); i < N; ++i)
	{
	  char new_label = get_new_label(all_used_labels);
	  new_order_dense.push_back(new_label);
	  all_used_labels.push_back(new_label);
	}
	for (std::size_t i = new_order_sp.size(); i < N; ++i)
	{
	  char new_label = get_new_label(all_used_labels);
	  new_order_sp.push_back(new_label);
	  all_used_labels.push_back(new_label);
	}

	// Extend the ranges
	const auto& [dim_dense, dim_sp] = split_dense_sparse(dim, n_sp);
	std::vector<superbblas::PartitionItem<N>> new_ranges_dense(ranges.size(2)),
	  new_ranges_sp(ranges.size(2));
	const int ndim_dense = (int)order.size() - n_sp;
	const int ndim = (int)order.size();
	for (IndexType i = 0; i < IndexType(ranges.size(2)); ++i)
	{
	  new_ranges_dense.at(i) = {
	    extend_to_coor<N>(ranges.begin({0, 0, i}), ranges.begin({ndim_dense, 0, i}), 0),
	    extend_to_coor<N>(ranges.begin({0, 1, i}), ranges.begin({ndim_dense, 1, i}), 1)};
	  new_ranges_sp.at(i) = {
	    extend_to_coor<N>(ranges.begin({ndim_dense, 0, i}), ranges.begin({ndim, 0, i}), 0),
	    extend_to_coor<N>(ranges.begin({ndim_dense, 1, i}), ranges.begin({ndim, 1, i}), 1)};
	}

	return {new_ranges_dense, new_ranges_sp, new_order_dense, new_order_sp};
      }
    }

    struct Tensor {
      using Ptr = std::shared_ptr<detail::Allocation<value_type>>;
      using Distro = std::shared_ptr<detail::Distribution>;
      using IPtr = std::shared_ptr<detail::Allocation<IndexType>>;
      Coor from;	   ///< first coordinate in the view
      Coor size;	   ///< number of elements on each direction in the view
      Coor dim;		   ///< tensor dimensions
      Order order;	   ///< labels of the tensor
      Ptr ptr;		   ///< values of the tensor in (natural ordering) if n_sp==0;
			   ///  otherwise the nonzero values in CSR format
      Distro distribution; ///< ranges support for each process
      value_type scale;	   ///< implicit scaling on the tensor elements
      bool is_conjugated;  ///< whether the tensor elements are implicitly conjugated
      int n_sp;		   ///< the last n_sp dimensions are sparse
      int max_nnz_per_row; ///< max nonzero values per dense coordinate
      IPtr cols;	   ///< indices of the sparse elements per dense element

      Tensor() : scale(0.), is_conjugated(false), n_sp(0), max_nnz_per_row(0) {};

      Tensor(const Coor& from, const Coor& size, const Coor& dim, const Order& order,
	     const Ptr& ptr, const Distro& distribution, value_type scale, bool is_conjugated,
	     int n_sp, int max_nnz_per_row, const IPtr& cols)
	: from(from),
	  size(size),
	  dim(dim),
	  order(order),
	  ptr(ptr),
	  distribution(distribution),
	  scale(scale),
	  is_conjugated(is_conjugated),
	  n_sp(n_sp),
	  max_nnz_per_row(max_nnz_per_row),
	  cols(cols)
      {
      }
    };

    namespace detail
    {
      /// Return a hash associated to the layout of a tensor
      /// \param t: input tensor

      inline std::size_t get_fingerprint(const Tensor& t)
      {
	return SB::getHash(std::make_tuple(
	  t.from, t.size, t.dim, t.order, bool(t.ptr), get_fingerprint(*t.distribution), t.scale,
	  t.is_conjugated, t.n_sp, t.max_nnz_per_row, bool(t.cols)));
      }
    }

    /// Return whether the given tensor communicator has more than one rank
    /// \param t: input tensor

    inline bool has_multiple_ranks(const Tensor& t)
    {
      return t.distribution->ranks.size() > 1;
    }

    /// Return the MPI communicator associated to a tensor
    /// \param t: input tensor

    inline MpiComm get_mpi_comm(const Tensor& t)
    {
      return t.distribution->comm;
    }

    /// Return the communicator associated to a tensor
    /// \param t: input tensor

    inline Comm get_comm(const Tensor& t)
    {
      return {t.distribution->ranks, t.distribution->comm};
    }

    /// Return a pointer to the cpu/gpu data
    /// \param t: input tensor

    inline value_type* data(const Tensor& t)
    {
      if (SBN::detail::get_debug_level() > 0 && has_multiple_ranks(t))
	SBN::detail::check_consistency(
	  SB::getHash(std::make_tuple(std::string("data"), detail::get_fingerprint(t))),
	  get_mpi_comm(t));

      if (volume(t.size) == 0)
	return nullptr;
      return t.ptr->data();
    }

    /// Return a pointer to the cpu data
    /// \param t: input tensor

    inline IndexType* nnz_indices(const Tensor& t)
    {
      if (SBN::detail::get_debug_level() > 0 && has_multiple_ranks(t))
	SBN::detail::check_consistency(
	  SB::getHash(std::make_tuple(std::string("nnz_indices"), detail::get_fingerprint(t))),
	  get_mpi_comm(t));

      if (t.cols)
	return t.cols->data();
      return nullptr;
    }

    /// Return whether the tensor has a nonzero support on the local process
    /// \param t: input tensor

    inline bool has_local_support(const Tensor& t)
    {
      const auto& local_from = get_local_from(*t.distribution);
      const auto& local_size = get_local_size(*t.distribution);
      return !is_intersection_empty(local_from, local_size, t.from, t.size, t.dim);
    }

    /// Return another tensor with the content of the given tensor with support on the given communicator
    /// \param t: input tensor
    /// \param comm: communicator

    inline Tensor get_tensor_supported_on(const Tensor& t, const Comm& comm)
    {
      const auto& p = std::make_shared<detail::Distribution>(
	detail::get_distribution_supported_on(*t.distribution, comm));
      return Tensor{t.from,  t.size,	      t.dim,  t.order,		 t.ptr, p,
		    t.scale, t.is_conjugated, t.n_sp, t.max_nnz_per_row, t.cols};
    }

    /// Return a local tensor with the content of the given tensor with support on the current process
    /// \param t: input tensor

    inline Tensor get_local_tensor(const Tensor& t)
    {
      return get_tensor_supported_on(t, get_local_comm());
    }

    /// Return an unique id for each partition of the given tensor
    /// \param t: input tensor
    /// \param return_unique_id_for_empty_ranges: (optional) whether to return different ids for empty ranges (default, false)

    inline std::vector<int>
    get_unique_range_ids(const Tensor& t,
			 Options::ReturnUniqueIdForEmptyRanges return_unique_id_for_empty_ranges =
			   Options::ReturnUniqueIdForEmptyRanges::False)
    {
      return get_unique_range_ids(t.distribution->fs, t.distribution->dim, 0,
				  return_unique_id_for_empty_ranges);
    }

    namespace detail
    {
      inline superbblas::Context& get_context(const Tensor& t)
      {
	return *(t.ptr->ctx);
      }

      /// Return the dense coordinates
      /// \param coor: input coordinate
      /// \param n_sp: the last `n_sp` dimensions of the input coordinate are sparse
      /// \param max_nnz_per_row: maximum nonzeros for each dense coordinate

      inline Coor get_dense_coor(const Coor& coor, int n_sp, int max_nnz_per_row)
      {
	if (n_sp <= 0)
	  return coor;
	return concat(Coor(coor.begin(), coor.end() - n_sp), Coor{max_nnz_per_row});
      }
    }

    /// Return a new tensor
    /// \param dim: dimensions of the new tensor
    /// \param order: labels of the new tensor
    /// \param distribution: policy to distribute the content of the tensor among the processes:
    ///          - Options::Distribution::Local: with support on the current process
    ///          - Options::Distribution::OnMaster: with support on process 0
    ///          - Options::Distribution::Replicated: with full support on all processes
    ///          - string with the labels to distribute among the processes
    ///        NOTE: operations with tensors supported on several processes may invoke MPI functions
    /// \param xpu: either
    ///          - Options::Alloc:Host: the tensor content is on the CPU
    ///          - Options::Alloc:Device: the tensor content may be on a GPU device.
    /// \param n_sp: the last `n_sp` dimensions are sparse
    /// \param max_nnz_per_row: maximum number of nonzero values for non-sparse coordinate
    /// \param eg: either
    ///          - Options::IsEg::False: allocate a tensor
    ///          - Options::IsEg::True: don't allocate a tensor and just create a template
    /// \param custom_distribution: (optional) range supported for each process
    /// \param comm: (optional) communicator

    inline Tensor create_tensor(const Coor& dim, const Order& order,
				const std::string& distribution = Options::Distribution::OnMaster,
				Options::Alloc xpu = Options::Alloc::Host, int n_sp = 0,
				int max_nnz_per_row = 0, Options::IsEg eg = Options::IsEg::False,
				const CustomDistribuition& custom_distribution = {},
				Comm comm = {})
    {
      // Do some checks
      if (!is_valid(order) || dim.size() != order.size() || n_sp > order.size())
	throw std::runtime_error("create_tensor: invalid inputs");

      // Get distribution and the indices of the dimensions to distribute
      detail::Distribution::Kind kind;
      detail::Distribution::PreferredDistDims preferred_dist_dims;
      if (distribution == Options::Distribution::OnMaster ||
	  distribution == Options::Distribution::Default)
      {
	kind = detail::Distribution::OnMaster;
      }
      else
      {
	kind = detail::Distribution::Distributed;
	preferred_dist_dims = detail::get_preferred_dist_dims(order, distribution);
      }

      if (comm.ranks.size() == 0)
      {
	comm =
	  (distribution == Options::Distribution::Local ? get_local_comm() : get_global_comm());
      }
      if (custom_distribution.size() > 0 && custom_distribution.size() != comm.ranks.size())
	throw std::runtime_error("create_tensor: invalid inputs");
      for (const auto& fs : custom_distribution)
	if (fs.at(0).size() != dim.size() || fs.at(1).size() != dim.size() ||
	    !is_less_or_equal_than(fs.at(1), dim))
	  throw std::runtime_error("create_tensor: invalid input");

      // Generate a new distribution and allocation
      auto p = std::make_shared<detail::Distribution>(kind, dim, preferred_dist_dims, comm.ranks,
						      comm.comm);
      if (custom_distribution.size() > 0)
      {
	for (int i = 0; i < custom_distribution.size(); ++i)
	  for (int j = 0; j < 2; ++j)
	    std::copy(custom_distribution.at(i).at(j).begin(),
		      custom_distribution.at(i).at(j).end(), p->fs.begin({0, j, i}));
      }
      const auto& local_vol = volume(get_local_size(*p), n_sp, max_nnz_per_row);
      auto alloc = eg == Options::IsEg::True
		     ? Tensor::Ptr()
		     : std::make_shared<detail::Allocation<value_type>>(
			 local_vol, xpu != Options::Alloc::Device ? detail::getCpuContext()
								  : detail::getGpuContext());

      auto cols_alloc =
	n_sp == 0 || eg == Options::IsEg::True
	  ? Tensor::IPtr()
	  : std::make_shared<detail::Allocation<IndexType>>(local_vol, detail::getCpuContext());

      const auto& t = Tensor{Coor(dim.size()),
			     dim,
			     dim,
			     order,
			     alloc,
			     p,
			     1.0,
			     false /* not conjugated */,
			     n_sp,
			     n_sp == 0 ? 0 : max_nnz_per_row,
			     cols_alloc};

      if (SBN::detail::get_debug_level() > 0 && has_multiple_ranks(t))
	SBN::detail::check_consistency(
	  SB::getHash(std::make_tuple(std::string("create_tensor"), detail::get_fingerprint(t))),
	  get_mpi_comm(t));

      return t;
    }

    inline void copyTo(const Tensor& tf, const Tensor& td);

    namespace detail
    {
      inline unsigned int get_num_unique_ids(const Distribution& d, int n_sp = -1);

      /// Return the labels along the tensor is distributed by inspecting the actual distribution
      /// \param t: input tensor

      inline Order get_true_distribution_labels(const Tensor& t)
      {
	Order r;
	r.reserve(t.dim.size());
	for (IndexType i = 0; i < t.distribution->fs.size(0); ++i)
	{
	  bool is_i_dist = false;
	  for (IndexType p = 0; p < t.distribution->fs.size(2); ++p)
	  {
	    if (t.distribution->fs.at({i, 1, p}) != t.dim.at(i))
	    {
	      is_i_dist = true;
	      break;
	    }
	  }
	  if (is_i_dist)
	    r.push_back(t.order.at(i));
	}
	return r;
      }
    }

    inline Order get_distribution_labels(const Tensor& t);

    /// Return a new tensor like the given one
    /// \param distribution: policy to distribute the content of the tensor among the processes:
    ///          - Options::Distribution::OnMaster: with support on process 0
    ///          - Options::Distribution::Replicated: with full support on all processes
    ///          - Options::Distribution::Default: as the given tensor
    ///          - string with the labels to distribute among the processes
    ///        NOTE: operations with tensors supported on several processes may invoke MPI functions
    /// \param xpu: either
    ///          - Options::Alloc:Host: the tensor content is on the CPU
    ///          - Options::Alloc:Device: the tensor content may be on a GPU device.
    ///          - Options::Alloc:Default: as the given tensor
    /// \param n_sp: the last `n_sp` dimensions are sparse
    /// \param max_nnz_per_row: maximum number of nonzero values for non-sparse coordinate

    inline Tensor like_this(const Tensor& t, const Order& new_order = {}, const Coor& new_size = {},
			    const std::string& distribution = Options::Distribution::Default,
			    Options::Alloc xpu = Options::Alloc::Default, int n_sp = -1,
			    int max_nnz_per_row = -1, Options::IsEg eg = Options::IsEg::Default)
    {
      // Check input
      if (new_order.size() > 0 && new_size.size() != new_order.size())
	throw std::runtime_error("like_this: invalid `new_size`");
      if (new_order.size() == 0 && new_size.size() > 0 && t.dim.size() != new_size.size())
	throw std::runtime_error("like_this: invalid `new_size`");
      if (distribution == Options::Distribution::Local)
	throw std::runtime_error("like_this: invalid `distribution`");

      // Get the new order
      const Order& this_new_order = new_order.size() == 0 ? t.order : new_order;
      if (!is_valid(this_new_order))
	throw std::runtime_error("like_this: invalid `new_order`");
      const Coor& this_new_size = new_size.size() == 0 ? t.size : new_size;

      // Get the new sparse parameters
      const int new_n_sp = n_sp < 0 ? t.n_sp : n_sp;
      if (new_n_sp > this_new_order.size())
	throw std::runtime_error("like_this: invalid `n_sp`");

      // Get the new distribution
      Tensor::Distro p;
      std::string new_distribution;
      if (distribution == Options::Distribution::Default)
      {
	detail::Distribution::Kind kind;
	detail::Distribution::PreferredDistDims preferred_dist_dims;
	if (detail::is_distribution_on_master(*t.distribution))
	{
	  kind = detail::Distribution::OnMaster;
	  new_distribution = Options::Distribution::OnMaster;
	  p = std::make_shared<detail::Distribution>(kind, this_new_size, preferred_dist_dims,
						     t.distribution->ranks, t.distribution->comm);
	}
	else if (t.distribution->ranks.size() == 1)
	{
	  new_distribution = common_labels(get_distribution_labels(t), this_new_order);
	  kind = detail::Distribution::Distributed;
	  preferred_dist_dims = detail::get_preferred_dist_dims(this_new_order, new_distribution);
	  p = std::make_shared<detail::Distribution>(kind, this_new_size, preferred_dist_dims,
						     t.distribution->ranks, t.distribution->comm);
	}
	else
	{
	  new_distribution = common_labels(detail::get_true_distribution_labels(t), this_new_order);
	  kind = detail::Distribution::Distributed;
	  preferred_dist_dims = detail::get_preferred_dist_dims(this_new_order, new_distribution);
	  if (new_distribution.size() > 0 &&
	      get_coor(new_distribution, get_scoor(t.order, t.size)) ==
		get_coor(new_distribution, get_scoor(this_new_order, this_new_size)))
	  {
	    p = std::make_shared<detail::Distribution>(detail::Distribution(
	      kind, this_new_size, {}, t.distribution->ranks, t.distribution->comm));
	    const auto& old_preferred_dist_dims =
	      detail::get_preferred_dist_dims(t.order, new_distribution);
	    for (int i = 0; i < p->fs.size(2); ++i)
	    {
	      if (is_intersection_empty(
		    Coor(t.distribution->fs.begin({0, 0, i}), t.distribution->fs.begin({0, 1, i})),
		    Coor(t.distribution->fs.begin({0, 1, i}),
			 t.distribution->fs.begin({0, 0, i + 1})),
		    t.from, t.size, t.dim))
	      {
		for (int j = 0; j < p->fs.size(0); ++j)
		  p->fs.at({j, 0, i}) = p->fs.at({j, 1, i}) = 0;
	      }
	      else
	      {
		for (int j = 0; j < preferred_dist_dims.size(); ++j)
		{
		  const auto old_j = old_preferred_dist_dims.at(j);
		  const auto& fs_restricted = get_simple_intersection(
		    {t.distribution->fs.at({old_j, 0, i}) - t.from.at(old_j)},
		    {t.distribution->fs.at({old_j, 1, i})}, {0}, {t.size.at(old_j)},
		    {t.dim.at(old_j)});
		  p->fs.at({preferred_dist_dims.at(j), 0, i}) = fs_restricted.at(0).at(0);
		  p->fs.at({preferred_dist_dims.at(j), 1, i}) = fs_restricted.at(1).at(0);
		}
	      }
	    }
	    p->preferred_dist_dims = preferred_dist_dims;
	  }
	  else
	  {
	    p = std::make_shared<detail::Distribution>(kind, this_new_size, preferred_dist_dims,
						       t.distribution->ranks, t.distribution->comm);
	  }
	}
      }
      else
      {
	new_distribution = distribution;
	detail::Distribution::Kind kind;
	detail::Distribution::PreferredDistDims preferred_dist_dims;
	if (distribution == Options::Distribution::OnMaster)
	{
	  kind = detail::Distribution::OnMaster;
	}
	else
	{
	  kind = detail::Distribution::Distributed;
	  preferred_dist_dims = detail::get_preferred_dist_dims(this_new_order, distribution);
	}
	p = std::make_shared<detail::Distribution>(kind, this_new_size, preferred_dist_dims,
						   t.distribution->ranks, t.distribution->comm);
      }

      // Check max nonzeros consistency
      int new_max_nnz_per_row = -1;
      if (t.n_sp > 0 && new_n_sp > 0 && max_nnz_per_row < 0)
      {
	new_max_nnz_per_row =
	  t.max_nnz_per_row *
	  (distribution == Options::Distribution::OnMaster ? (int)p->ranks.size() : 1);
      }
      else
      {
	new_max_nnz_per_row =
	  new_n_sp == 0 ? 0 : (max_nnz_per_row < 0 ? t.max_nnz_per_row : max_nnz_per_row);
      }

      // Get the new allocation
      Tensor::Ptr alloc;
      Tensor::IPtr cols_alloc;
      if (eg == Options::IsEg::False || (eg == Options::IsEg::Default && t.ptr))
      {
	Options::Alloc this_xpu;
#  ifdef SUPERBBLAS_USE_GPU
	if (xpu != Options::Alloc::Default)
	{
	  this_xpu = xpu;
	}
	else if (t.ptr)
	{
	  this_xpu =
	    detail::is_context_cpu(*t.ptr->ctx) ? Options::Alloc::Host : Options::Alloc::Device;
	}
	else
#  endif
	{
	  this_xpu = Options::Alloc::Host;
	}

	const auto& local_vol = volume(get_local_size(*p), new_n_sp, new_max_nnz_per_row);

	alloc = std::make_shared<detail::Allocation<value_type>>(
	  local_vol,
	  this_xpu != Options::Alloc::Device ? detail::getCpuContext() : detail::getGpuContext());

	if (new_n_sp > 0)
	{
	  cols_alloc =
	    std::make_shared<detail::Allocation<IndexType>>(local_vol, detail::getCpuContext());
	}
      }

      // Create new tensor
      const auto& r = Tensor{Coor(this_new_size.size()),
			     this_new_size,
			     this_new_size,
			     this_new_order,
			     alloc,
			     p,
			     1.0,
			     false /* not conjugated */,
			     new_n_sp,
			     new_max_nnz_per_row,
			     cols_alloc};

      if (SBN::detail::get_debug_level() > 0 && has_multiple_ranks(r))
	SBN::detail::check_consistency(
	  SB::getHash(std::make_tuple(std::string("like_this"), detail::get_fingerprint(r))),
	  get_mpi_comm(r));

      return r;
    }

    /// Return a new tensor
    /// \param distribution: policy to distribute the content of the tensor among the processes:
    ///          - Options::Distribution::OnMaster: with support on process 0
    ///          - Options::Distribution::Replicated: with full support on all processes
    ///          - Options::Distribution::Default: as the given tensor
    ///          - string with the labels to distribute among the processes
    ///        NOTE: operations with tensors supported on several processes may invoke MPI functions
    /// \param xpu: either
    ///          - Options::Alloc:Host: the tensor content is on the CPU
    ///          - Options::Alloc:Device: the tensor content may be on a GPU device.
    ///          - Options::Alloc:Default: as the given tensor

    inline Tensor make_sure(const Tensor& t, const Order& new_order,
			    std::string distribution = Options::Distribution::Default,
			    Options::Alloc xpu = Options::Alloc::Default,
			    Options::TightView view = Options::TightView::Default, int n_sp = -1,
			    int max_nnz_per_row = -1, Options::IsEg eg = Options::IsEg::Default)
    {

      // Check input
      if (new_order.size() > 0 && !is_same_range(t.order, t.size, new_order))
	throw std::runtime_error("make_sure: invalid `new_order`");
      if (distribution == Options::Distribution::Local)
	throw std::runtime_error("like_this: invalid `distribution`");

      bool do_copy = false; ///< whether to create a new tensor and copy the content

      // Check the ordering
      if (!do_copy && new_order.size() > 0 && t.order != new_order)
      {
	do_copy = true;
      }

      // Check the distribution
      if (!do_copy && distribution != Options::Distribution::Default)
      {
	detail::Distribution::PreferredDistDims preferred_dist_dims;
	if (distribution == Options::Distribution::OnMaster)
	{
	  do_copy = detail::is_distribution_on_master(*t.distribution) ? false : true;
	}
	else
	{
	  do_copy = t.distribution->preferred_dist_dims ==
			detail::get_preferred_dist_dims(t.order, distribution)
		      ? false
		      : true;
	}
      }

#  ifdef SUPERBBLAS_USE_GPU
      // Check allocation
      if (!do_copy && xpu != Options::Alloc::Default)
      {
	do_copy = (!t.ptr || detail::is_context_cpu(*t.ptr->ctx) == (xpu != Options::Alloc::Device))
		    ? false
		    : true;
      }
#  endif

      // Check the sparsity
      if (!do_copy && ((n_sp >= 0 && n_sp != t.n_sp) ||
		       (n_sp > 0 && max_nnz_per_row >= 0 && max_nnz_per_row != t.max_nnz_per_row)))
	do_copy = true;

      // Check the view
      if (!do_copy && view != Options::TightView::Default)
      {
	do_copy =
	  (t.from == Coor(t.dim.size()) && t.size == t.dim && !t.is_conjugated && t.scale == 1.0)
	    ? false
	    : true;
      }

      // Check the example gratia
      if (!do_copy && eg != Options::IsEg::Default)
      {
	do_copy = bool(t.ptr) == (eg == Options::IsEg::False) ? false : true;
      }

      // Return the given tensor if we don't need to copy it
      if (!do_copy)
      {
	if (SBN::detail::get_debug_level() > 0 && has_multiple_ranks(t))
	  SBN::detail::check_consistency(
	    SB::getHash(
	      std::make_tuple(std::string("make_sure-return"), detail::get_fingerprint(t))),
	    get_mpi_comm(t));

	return t;
      }

      // Redistribute the tensor if view is Options::TightView::TrueAndRedistribute
      if (view == Options::TightView::TrueAndRedistribute &&
	  distribution == Options::Distribution::Default)
      {
	distribution = get_distribution_labels(t);
      }

      // Create new tensor
      const auto& n = like_this(t, new_order, get_coor(new_order, get_scoor(t.order, t.size)),
				distribution, xpu, n_sp, max_nnz_per_row, eg);

      // Copy the content to the new tensor
      if (n.ptr)
	copyTo(t, n);

      // Return the new tensor
      return n;
    }

    /// Return the labels along the tensor is distributed
    /// \param t: input tensor

    inline Order get_distribution_labels(const Tensor& t)
    {
      Order r;
      r.reserve(t.distribution->preferred_dist_dims.size());
      for (const auto& it : t.distribution->preferred_dist_dims)
	r.push_back(t.order.at(it));
      return r;
    }

    /// Return the local range of a tensor
    /// \param t: input tensor
    /// \param complainIfItsntTightView: complain if the given tensor doesn't have a tight view

    inline std::array<Coor, 2>
    get_local_range(const Tensor& t, Options::ComplainIfIsntTightView complainIfItsntTightView =
				       Options::ComplainIfIsntTightView::True)
    {
      if (complainIfItsntTightView == Options::ComplainIfIsntTightView::True &&
	  (t.from != Coor(t.dim.size()) || t.size != t.dim))
	throw std::runtime_error("get_local_range: unsupported on non tight views");
      return {get_local_from(*t.distribution), get_local_size(*t.distribution)};
    }

    /// Return the local range of a tensor
    /// \param t: input tensor
    /// \param complainIfItsntTightView: complain if the given tensor doesn't have a tight view

    inline std::array<SCoor, 2>
    get_local_srange(const Tensor& t, Options::ComplainIfIsntTightView complainIfItsntTightView =
					Options::ComplainIfIsntTightView::True)
    {
      if (complainIfItsntTightView == Options::ComplainIfIsntTightView::True &&
	  (t.from != Coor(t.dim.size()) || t.size != t.dim))
	throw std::runtime_error("get_local_srange: unsupported on non tight views");
      return {get_scoor(t.order, get_local_from(*t.distribution)),
	      get_scoor(t.order, get_local_size(*t.distribution))};
    }

    /// Return the distribution of the tensor among the processes
    /// \param t: input tensor

    inline CustomDistribuition get_distribution(const Tensor& t)
    {
      return detail::get_distribution(t.distribution->fs);
    }

    /// Return whether the tensor is allocated on host or on device
    /// \param t: input tensor

    inline Options::Alloc get_allocation_type(const Tensor& t)
    {
      if (!t.ptr)
	return Options::Alloc::Host;
      return detail::is_context_cpu(*t.ptr->ctx) ? Options::Alloc::Host : Options::Alloc::Device;
    }

    namespace detail
    {
      /// Check that the nonzero values column indices have values within the range
      /// \param t: input tensor

      inline void check_nnz_col_values(const Tensor& t)
      {
	if (t.n_sp == 0 || t.max_nnz_per_row == 0)
	  return;

	const auto& [from_dense, _0] = split_dense_sparse(t.from, t.n_sp);
	const auto& [size_dense, _1] = split_dense_sparse(t.size, t.n_sp);
	const auto& [dim_dense, dim_sp] = split_dense_sparse(t.dim, t.n_sp);
	const auto& [local_size_dense, local_size_sp] =
	  split_dense_sparse(get_local_size(*t.distribution), t.n_sp);
	const auto& [local_from_dense, local_from_sp] =
	  split_dense_sparse(get_local_from(*t.distribution), t.n_sp);
	const auto num_rows = volume(local_size_sp) == 0 ? 0 : volume(local_size_dense);
	const auto vol = num_rows * t.max_nnz_per_row;
	const auto* cols = nnz_indices(t);
	for (IndexType i = 0; i < vol; ++i)
	{
	  if (is_less_than(
		norm_coor(indexToCoor(i, local_size_dense) + local_from_dense - from_dense,
			  dim_dense),
		size_dense) &&
	      cols[i] >= 0)
	  {
	    if (!is_less_than(norm_coor(indexToCoor(cols[i], dim_sp) - local_from_sp, dim_sp),
			      local_size_sp))
	      throw std::runtime_error("check_nnz_col_values: invalid local column coordinates");
	  }
	}
      }

      enum struct CheckNnzIndices { False, True };

      /// Return a copy of the tensor on host and either on master or local for the reference implementation tensor
      /// \param t: input tensor
      /// \param check_nnz_indices: (optional) whether to check the nonzero values

      inline reference_implementation::Tensor
      to_ref_tensor(const Tensor& t, CheckNnzIndices check_nnz_indices = CheckNnzIndices::True)
      {
	// Check that the nonzeros are valid
	if (check_nnz_indices != CheckNnzIndices::False)
	  check_nnz_col_values(t);

	// Avoid activating the checking for the following operations
	bool c = false;
	std::swap(check_against_reference_implementation(), c);

	const auto t0 = get_local_tensor(make_sure(
	  t, {},
	  !has_multiple_ranks(t) ? Options::Distribution::Default : Options::Distribution::OnMaster,
	  Options::Alloc::Host));

	// Stablish back the value of the checking
	std::swap(check_against_reference_implementation(), c);

	if (get_rank(get_mpi_comm(t)) == 0)
	{
	  const auto& local_from = detail::get_local_from(*t0.distribution);
	  const auto& local_size = detail::get_local_size(*t0.distribution);
	  const auto* p = data(t0);
	  const auto* cols = nnz_indices(t0);
	  const auto vol = volume(local_size, t0.n_sp, t0.max_nnz_per_row);
	  reference_implementation::Tensor n;
	  if (local_size == t0.dim && (t0.n_sp == 0 || local_from == Coor(t0.dim.size())))
	  {
	    n = reference_implementation::Tensor{
	      norm_coor(t0.from - local_from, t0.dim),
	      t0.size,
	      t0.dim,
	      t0.order,
	      std::make_shared<std::vector<value_type>>(p, p + vol),
	      t0.scale,
	      t0.is_conjugated,
	      t0.n_sp,
	      t0.max_nnz_per_row,
	      t0.n_sp == 0 ? reference_implementation::Tensor::IPtr{}
			   : std::make_shared<std::vector<IndexType>>(cols, cols + vol)};
	  }
	  else
	  {
	    const auto& n0 = reference_implementation::Tensor{
	      Coor(t0.dim.size()),
	      local_size,
	      local_size,
	      t0.order,
	      std::make_shared<std::vector<value_type>>(p, p + vol),
	      t0.scale,
	      t0.is_conjugated,
	      t0.n_sp,
	      t0.max_nnz_per_row,
	      t0.n_sp == 0 ? reference_implementation::Tensor::IPtr{}
			   : std::make_shared<std::vector<IndexType>>(cols, cols + vol)};
	    if (t0.n_sp > 0 && t0.max_nnz_per_row > 0)
	    {
	      const auto& [dim_dense, dim_sp] = split_dense_sparse(t0.dim, t0.n_sp);
	      const auto& [local_from_dense, local_from_sp] = split_dense_sparse(local_from, t0.n_sp);
	      const auto& [local_size_dense, local_size_sp] = split_dense_sparse(local_size, t0.n_sp);
	      for (std::size_t i = 0; i < vol; ++i)
	      {
		if (n0.cols->at(i) < 0)
		{
		  n0.ptr->at(i) = value_type{0};
		}
		else
		{
		  const auto& sp_coor = indexToCoor(n0.cols->at(i), dim_sp);
		  const auto& rel_sp_coor = norm_coor(sp_coor - local_from_sp, dim_sp);
		  if (is_less_than(rel_sp_coor, local_size_sp))
		  {
		    n0.cols->at(i) = coorToIndex(rel_sp_coor, local_size_sp);
		  }
		  else
		  {
		    n0.cols->at(i) = -1;
		    n0.ptr->at(i) = value_type{0};
		  }
		}
	      }
	    }
	    n = reference_implementation::create_tensor(t0.dim, t0.order, {}, Options::Alloc::Host,
							t0.n_sp, t0.max_nnz_per_row);
	    const auto quick_slice = [](const reference_implementation::Tensor& t, const Coor& from,
					const Coor& size) {
	      auto n = t;
	      n.from = norm_coor(t.from + from, t.dim);
	      n.size = size;
	      return n;
	    };

	    reference_implementation::copyTo(n0, quick_slice(n, local_from, local_size));
	    n = quick_slice(n, t0.from, t0.size);
	  }

	  // Make it dense
	  return reference_implementation::make_sure(
	    n, {}, Options::Distribution::Default, Options::Alloc::Default,
	    Options::TightView::Default, 0 /* no sparse dimensions */);
	}
	else
	{
	  return reference_implementation::Tensor();
	}
      }
    }

    namespace detail
    {
      constexpr std::size_t N = 12; ///< maximum tensor dimension (used by superbblas)

      /// Return the dense and sparse parts of the tensor range
      /// \param t: input tensor
      /// \return {from_dense, size_dense, dim_dense, from_sp, size_sp, dim_sp}

      inline std::array<Coor, 6> split_dense_sparse_range(const Tensor& t)
      {
	const auto& [from_dense, from_sp] = split_dense_sparse(t.from, t.n_sp);
	const auto& [size_dense, size_sp] = split_dense_sparse(t.size, t.n_sp);
	const auto& [dim_dense, dim_sp] = split_dense_sparse(t.dim, t.n_sp);
	return {from_dense, size_dense, dim_dense, from_sp, size_sp, dim_sp};
      }

      /// Return whether slicing not the full range of the sparse components
      /// \param t: input tensor

      inline bool slicing_sparse_components(const Tensor& t)
      {
	// Trivial case: dense or empty tensors
	if (t.n_sp == 0 || volume(t.size) == 0)
	  return false;

	// General case: the sparse range doesn't start at the origin or doesn't extend
	// the whole dimension
	const auto& [from_dense, size_dense, dim_dense, from_sp, size_sp, dim_sp] =
	  split_dense_sparse_range(t);
	return (from_sp != Coor(t.n_sp) || size_sp != dim_sp);
      }

      /// Return the different number of labels
      /// \param ids: label for each process

      inline unsigned int get_num_unique_ids(const std::vector<int>& ids)
      {
	int max_id = 0;
	for (const auto& it : ids)
	  max_id = std::max(it, max_id);
	return (unsigned int)max_id + 1u;
      }

      /// Return the different number of sparse domains
      /// \param d: tensor distribution
      /// \param n_sp: the last `n_sp` components are sparse

      inline unsigned int get_num_unique_ids(const Distribution& d, int n_sp)
      {
	return get_num_unique_ids(get_unique_range_ids(d.fs, d.dim, n_sp));
      }

      /// Return the dense components of the tensor range extended up to `N` components
      /// \param decomplexify: whether represent the complex numbers as a dimension
      /// \param t: input tensor
      /// \param new_max_nnz_per_row: number of active components on the sparse dimension
      /// \param common_max_nnz_per_row: common number of active components on the sparse dimension
      /// \return {from, size, dim}

      template <std::size_t N>
      inline std::array<std::array<int, N>, 3>
      get_dense_range(bool decomplexify, const Tensor& t, int new_max_nnz_per_row = -1,
		      const std::vector<int>& ids = {}, int common_max_nnz_per_row = -1)
      {
	if (slicing_sparse_components(t))
	  throw std::runtime_error("wtf");

	const auto& [from_dense, size_dense, dim_dense, from_sp, size_sp, dim_sp] =
	  split_dense_sparse_range(t);
	if (t.n_sp == 0)
	{
	  if (!decomplexify)
	  {
	    return {extend_to_coor<N>(from_dense, 0), extend_to_coor<N>(size_dense, 1),
		    extend_to_coor<N>(dim_dense, 1)};
	  }
	  else
	  {
	    return {extend_to_coor<N>(0, from_dense, 0), extend_to_coor<N>(2, size_dense, 1),
		    extend_to_coor<N>(2, dim_dense, 1)};
	  }
	}
	else
	{
	  const auto num_ids =
	    ids.size() > 0 ? get_num_unique_ids(ids) : get_num_unique_ids(*t.distribution, t.n_sp);
	  if (new_max_nnz_per_row >= 0 && ids.size() > 0 &&
	      new_max_nnz_per_row > num_ids * t.max_nnz_per_row)
	    throw std::runtime_error("get_dense_range: invalid input");
	  if (common_max_nnz_per_row >= 0 && t.max_nnz_per_row % common_max_nnz_per_row != 0)
	    throw std::runtime_error("get_dense_range: unsupported case");
	  new_max_nnz_per_row =
	    (new_max_nnz_per_row >= 0 ? new_max_nnz_per_row : t.max_nnz_per_row);
	  const int nnz_block =
	    (common_max_nnz_per_row >= 0 ? common_max_nnz_per_row : new_max_nnz_per_row);
	  if (!decomplexify)
	  {
	    return {
	      extend_to_coor<N>(0, 0, from_dense, 0),
	      extend_to_coor<N>(nnz_block, num_ids * new_max_nnz_per_row / nnz_block, size_dense,
				1),
	      extend_to_coor<N>(nnz_block, num_ids * t.max_nnz_per_row / nnz_block, dim_dense, 1)};
	  }
	  else
	  {
	    return {extend_to_coor<N>(0, 0, 0, from_dense, 0),
		    extend_to_coor<N>(2, nnz_block, num_ids * new_max_nnz_per_row / nnz_block,
				      size_dense, 1),
		    extend_to_coor<N>(2, nnz_block, num_ids * t.max_nnz_per_row / nnz_block,
				      dim_dense, 1)};
	  }
	}
      }
    }

    /// Set the values of the tensor to zero
    /// \param t: tensor to set zero

    inline void set_zero(const Tensor& t)
    {
      if (SBN::detail::get_debug_level() > 0 && has_multiple_ranks(t))
	SBN::detail::check_consistency(
	  SB::getHash(std::make_tuple(std::string("set_zero"), detail::get_fingerprint(t))),
	  get_mpi_comm(t));

      using namespace detail;
      if (volume(t.size) == 0)
	return;
      if (!t.ptr)
	throw std::runtime_error("set_zero: invalid action on an example tensor");
      if (detail::slicing_sparse_components(t))
	throw std::runtime_error("set_zero: unsupported operation when slicing sparse components");
      const char sp_label = (t.n_sp == 0 ? 0 : get_new_label(t.order));
      auto* ptr = data(t);
      const auto& ctx = get_context(t);
      const auto& [from_dense, size_dense, dim_dense] = get_dense_range<N>(false, t);
      const auto& [fs, order] = extend_range_and_order_dense<N>(
	t.distribution->fs, t.distribution->dim, 0, sp_label, t.order, t.n_sp, t.max_nnz_per_row);
      superbblas::copy<N, N, value_type, value_type>(
	value_type{0}, fs.data(), 1, order.c_str(), from_dense, size_dense, dim_dense,
	(const value_type**)&ptr, nullptr, &get_context(t), fs.data(), 1, order.c_str(), from_dense,
	dim_dense, (value_type**)&ptr, nullptr, &ctx,
#  ifdef SUPERBBLAS_USE_MPI
	t.distribution->comm,
#  endif
	superbblas::FastToSlow, superbblas::Copy);

      if (t.n_sp > 0)
      {
	// Make an array with all -1 of the same size the local `cols`
	std::vector<IndexType> ones(
	  volume(get_local_size(*t.distribution), t.n_sp, t.max_nnz_per_row), -1);
	auto* ones_ptr = ones.data();
	IndexType* cols = nnz_indices(t);
	superbblas::copy<N, N, IndexType, IndexType>(
	  IndexType{1}, fs.data(), 1, order.c_str(), from_dense, size_dense, dim_dense,
	  (const IndexType**)&ones_ptr, nullptr, &*getCpuContext(), fs.data(), 1, order.c_str(),
	  from_dense, dim_dense, (IndexType**)&cols, nullptr, &*getCpuContext(),
#  ifdef SUPERBBLAS_USE_MPI
	  t.distribution->comm,
#  endif
	  superbblas::FastToSlow, superbblas::Copy);
      }

      // Check the result against the reference implementation
      if (check_against_reference_implementation())
      {
	const auto& t_ref = detail::to_ref_tensor(t);
	if (get_rank(get_mpi_comm(t)) == 0 && reference_implementation::frob(t_ref) > 0)
	  throw std::runtime_error("set_zero: invalid result");
      }
    }

    /// Set an element on a tensor
    /// \param t: tensor to modify
    /// \param coor: coordinate of the tensor to modify
    /// \param val: new value of the tensor element

    inline void set(const Tensor& t, const Coor& coor, const value_type& val)
    {
      if (SBN::detail::get_debug_level() > 0 && has_multiple_ranks(t))
	SBN::detail::check_consistency(
	  SB::getHash(std::make_tuple(std::string("set"), detail::get_fingerprint(t), coor, val)),
	  get_mpi_comm(t));

      // Quick shortcut
      if (std::abs(t.scale) == 0)
	return;

      // Check input arguments
      if (!t.ptr)
	throw std::runtime_error("set: invalid operation on an example tensor");
      if (!detail::is_context_cpu(*t.ptr->ctx))
	throw std::runtime_error("set: invalid operation on a tensor on device");
      if (volume(t.size) == 0)
	throw std::runtime_error("set: invalid operation on an empty tensor");

      // Do the thing
      const auto& local_from =
	norm_coor(coor + t.from + t.dim - get_local_from(*t.distribution), t.dim);
      const auto& local_size = get_local_size(*t.distribution);
      auto* p = data(t);
      IndexType* cols = nnz_indices(t);
      if (is_less_than(local_from, local_size))
      {
	const auto explicit_val = cond_conj(t.is_conjugated, val) / t.scale;
	if (t.n_sp == 0)
	{
	  p[coorToIndex(local_from, local_size)] = explicit_val;
	}
	else
	{
	  const auto& global_from = norm_coor(coor + t.from, t.dim);
	  const auto& [dense_coor, a0_] = detail::split_dense_sparse(local_from, t.n_sp);
	  const auto& [a1_, sp_coor] = detail::split_dense_sparse(global_from, t.n_sp);
	  const auto& [dense_size, local_sp_size] =
	    detail::split_dense_sparse(get_local_size(*t.distribution), t.n_sp);
	  if (volume(local_sp_size) == 0)
	    return;
	  const auto& [a3_, sp_size] = detail::split_dense_sparse(t.dim, t.n_sp);
	  const auto row = coorToIndex(dense_coor, dense_size);
	  const auto sp_col = coorToIndex(sp_coor, sp_size);

	  // Set the value in an nonzero value
	  bool is_set = false;
	  const auto val_is_zero = std::norm(explicit_val) == 0;
	  for (int j = 0; !is_set && j < t.max_nnz_per_row; ++j)
	  {
	    auto& col = cols[row * t.max_nnz_per_row + j];
	    if (col == sp_col)
	    {
	      p[row * t.max_nnz_per_row + j] = explicit_val;
	      is_set = true;
	      if (val_is_zero)
		col = -1;
	    }
	  }

	  // Otherwise, insert a new nonzero value
	  if (val_is_zero)
	    is_set = true;
	  for (int j = 0; !is_set && j < t.max_nnz_per_row; ++j)
	  {
	    auto& col = cols[row * t.max_nnz_per_row + j];
	    if (col >= 0)
	      continue;
	    col = sp_col;
	    p[row * t.max_nnz_per_row + j] = explicit_val;
	    is_set = true;
	  }

	  if (!is_set)
	    throw std::runtime_error("set: no more space for inserting a new nonzero value");
	}
      }

      // Check the result against the reference implementation
      if (detail::check_against_reference_implementation())
      {
	const auto& t_ref = detail::to_ref_tensor(t);
	if (get_rank(get_mpi_comm(t)) == 0 && reference_implementation::get(t_ref, coor) != val)
	  throw std::runtime_error("set: invalid result");
      }
    }

    /// Get an element from a tensor
    /// \param t: input tensor
    /// \param coor: coordinate of the tensor to get

    inline value_type get(const Tensor& t, const Coor& coor)
    {
      if (SBN::detail::get_debug_level() > 0 && has_multiple_ranks(t))
	SBN::detail::check_consistency(
	  SB::getHash(std::make_tuple(std::string("get"), detail::get_fingerprint(t), coor)),
	  get_mpi_comm(t));

      // Quick shortcut
      if (std::abs(t.scale) == 0)
	return value_type{0};

      // Check input arguments
      if (!t.ptr)
	throw std::runtime_error("get: invalid operation on an example tensor");
      if (!detail::is_context_cpu(*t.ptr->ctx))
	throw std::runtime_error("get: invalid operation on a tensor on device");
      if (volume(t.size) == 0)
	throw std::runtime_error("get: invalid operation on an empty tensor");

      // Do the thing
      const auto& local_from =
	norm_coor(coor + t.from + t.dim - get_local_from(*t.distribution), t.dim);
      const auto& local_size = get_local_size(*t.distribution);
      value_type explicit_val{0};
      bool is_get = false;
      if (is_less_than(local_from, local_size))
      {
	auto* p = data(t);
	value_type implicit_val{0};
	if (t.n_sp == 0)
	{
	  implicit_val = p[coorToIndex(local_from, local_size)];
	}
	else
	{
	  IndexType* cols = nnz_indices(t);
	  const auto& global_from = norm_coor(coor + t.from, t.dim);
	  const auto& [dense_coor, a0_] = detail::split_dense_sparse(local_from, t.n_sp);
	  const auto& [a1_, sp_coor] = detail::split_dense_sparse(global_from, t.n_sp);
	  const auto& [dense_size, a2_] =
	    detail::split_dense_sparse(get_local_size(*t.distribution), t.n_sp);
	  const auto& [a3_, sp_size] = detail::split_dense_sparse(t.dim, t.n_sp);
	  const auto row = coorToIndex(dense_coor, dense_size);
	  const auto sp_col = coorToIndex(sp_coor, sp_size);

	  // Set the value in an nonzero value
	  for (int j = 0; j < t.max_nnz_per_row; ++j)
	  {
	    auto& col = cols[row * t.max_nnz_per_row + j];
	    if (col == sp_col)
	    {
	      implicit_val = p[row * t.max_nnz_per_row + j];
	      break;
	    }
	  }
	}
	explicit_val = cond_conj(t.is_conjugated, implicit_val) * t.scale;
	is_get = true;
      }

      // Gather all results and pick the one from the smallest rank
      if (has_multiple_ranks(t))
      {
	for (const auto& it :
	     gather(std::array<value_type, 2>{is_get ? value_type{1} : value_type{0}, explicit_val},
		    get_mpi_comm(t)))
	{
	  if (it.at(0) != value_type{0})
	  {
	    explicit_val = it.at(1);
	    break;
	  }
	}
      }

      // Check the result against the reference implementation
      if (detail::check_against_reference_implementation())
      {
	const auto& t_ref = detail::to_ref_tensor(t, detail::CheckNnzIndices::False);
	if (get_rank(get_mpi_comm(t)) == 0 &&
	    reference_implementation::get(t_ref, coor) != explicit_val)
	  throw std::runtime_error("get: invalid result");
      }

      return explicit_val;
    }

    /// Return the range for the dense and sparse dimensions
    /// \param t: input tensor
    /// \return {dense_from, dense_size, dense_dim, sp_from, sp_size, sp_dim}: where
    ///   - dense_from, dense_size and dense_dim: dense range
    ///   - sp_from, sp_size and sp_dim: dense range

    inline std::array<Coor, 6> get_dense_sparse_ranges(const Tensor& t)
    {
      return {Coor(t.from.begin(), t.from.end() - t.n_sp), //
	      Coor(t.size.begin(), t.size.end() - t.n_sp), //
	      Coor(t.dim.begin(), t.dim.end() - t.n_sp),   //
	      Coor(t.from.end() - t.n_sp, t.from.end()),   //
	      Coor(t.size.end() - t.n_sp, t.size.end()),   //
	      Coor(t.dim.end() - t.n_sp, t.dim.end())};
    }

    /// Execute the given function passing the coordinate and the value for the local nonzero elements
    /// \param t: input tensor
    /// \param fun: function to execute on each value; signature: fun(coor, value)

    template <typename Fun>
    void for_each_coor_and_nonzero_value(const Tensor& t, const Fun& fun)
    {
      if (std::abs(t.scale) == 0)
	return;

      if (!t.ptr)
	throw std::runtime_error("for_each_coor_and_nonzero_value: unsupported on example tensors");

      if (t.n_sp == 0)
      {
	const auto& t0_local =
	  make_sure(get_local_tensor(t), {}, Options::Distribution::Default, Options::Alloc::Host);
	const auto& [from, size] = get_local_range(t, Options::ComplainIfIsntTightView::False);
	value_type* ptr = data(t0_local);
	IndexType vol = volume(size);
	for (IndexType i = 0; i < vol; ++i)
	{
	  const auto& coor = norm_coor(indexToCoor(i, size) + from, t.dim);
	  if (is_less_than(norm_coor(coor - t.from, t.dim), t.size))
	    fun(coor, ptr[i]);
	}
      }
      else
      {
	const auto& t0 =
	  get_local_tensor(make_sure(t, {}, Options::Distribution::Default, Options::Alloc::Host));
	const auto& [from, size] = get_local_range(t);
	const value_type* v = data(t0);
	IndexType vol = volume(size, t0.n_sp, t0.max_nnz_per_row);
	const IndexType* cols = nnz_indices(t0);
	const auto& [dense_from, dense_size, dense_dim, sp_from, sp_size, sp_dim] =
	  get_dense_sparse_ranges(t0);
	for (std::size_t i = 0; i < vol; ++i)
	{
	  const auto& dense_coor = norm_coor(indexToCoor(i, dense_size) + dense_from, dense_dim);
	  for (int j = 0; j < t0.max_nnz_per_row; ++j)
	  {
	    const auto col = cols[i * t0.max_nnz_per_row + j];
	    if (col < 0)
	      continue;
	    const auto& sp_coor = norm_coor(indexToCoor(col, sp_dim) - sp_from, sp_dim);
	    if (!is_less_than(sp_coor, sp_size))
	      continue;
	    const auto& explicit_val = v[i * t0.max_nnz_per_row + j];
	    const auto& val = cond_conj(t0.is_conjugated, explicit_val * t0.scale);
	    if (std::norm(val) == 0)
	      continue;
	    const auto& coor = concat(dense_coor, sp_coor);
	    if (is_less_than(norm_coor(coor - t.from, t.dim), t.size))
	      fun(coor, val);
	  }
	}
      }
    }

    inline Tensor clone(const Tensor& t);

    namespace detail
    {
      enum class CopyOrAdd { Copy, Add };

      inline void copyOrAddTo(const Tensor& tf, const Tensor& td, const CopyOrAdd copy_or_add);

      /// Copy or add the content of a tensor into another tensor
      /// \param tf: origin tensor
      /// \param td: destination tensor
      /// \param copy_or_add: either CopyOrAdd::Copy or CopyOrAdd::Add, flag to indicate to copy or to add

      inline void copyOrAddTo_slow(const Tensor& tf, const Tensor& td, const CopyOrAdd copy_or_add)
      {
	const auto& distribution = !has_multiple_ranks(tf) ? Options::Distribution::Default
							   : Options::Distribution::OnMaster;
	const auto& tf0 = make_sure(tf, {}, distribution, Options::Alloc::Host);
	const auto& td0 = copy_or_add == CopyOrAdd::Copy
			    ? like_this(td, {}, {}, distribution, Options::Alloc::Host)
			    : make_sure(td, {}, distribution, Options::Alloc::Host);
	const auto& tf0_local = get_local_tensor(tf0);
	const auto& td0_local = get_local_tensor(td0);
	const auto& perm = get_perm(tf0_local.order, td0_local.order);
	if (get_rank(get_mpi_comm(tf)) == 0)
	{
	  if (copy_or_add == CopyOrAdd::Copy)
	  {
	    set_zero(td0_local);
	    for_each_coor_and_nonzero_value(tf0_local,
					    [&](const Coor& coor, const value_type& val) {
					      set(td0_local, do_perm(coor, perm, 0), val);
					    });
	  }
	  else
	  {
	    for_each_coor_and_nonzero_value(
	      tf0_local, [&](const Coor& coor, const value_type& val) {
		const auto& td_coor = do_perm(coor, perm, 0);
		set(td0_local, td_coor, get(td0_local, td_coor) + val);
	      });
	  }
	}
	if (td0.ptr != td.ptr)
	  copyOrAddTo(td0, td, CopyOrAdd::Copy);
      }

      inline bool is_supported_sparse_distribution_for_fast_copy(const Tensor& tf, const Tensor& td)
      {
	// Check for having the same distribution
	if (tf.distribution->fs == td.distribution->fs)
	  return true;

	// Check for the destination having only one domain for the sparse components
	return get_num_unique_ids(*td.distribution, td.n_sp) <= 1;
      }

      /// Return the first coordinate for the real components and the imaginary components from a
      /// given complex explicit first coordinate
      /// \param from: complex explicit coordinate
      /// \return {from_real, from_imag}, where `from_real` is the real coordinate and `from_imag`
      ///         is the imaginary component

      template <typename Coor>
      inline std::array<Coor, 3> get_from_real_and_imag_and_size(const Coor& from, const Coor& size)
      {
	auto from_real = from;
	auto from_imag = from;
	from_real.at(0) = 0;
	from_imag.at(0) = 1;
	auto size_real = size;
	size_real.at(0) = 1;
	return {from_real, from_imag, size_real};
      }

      /// Copy or add the content of a tensor into another tensor
      /// \param tf: origin tensor
      /// \param td: destination tensor
      /// \param copy_or_add: either CopyOrAdd::Copy or CopyOrAdd::Add, flag to indicate to copy or to add

      inline void copyOrAddTo(const Tensor& tf, const Tensor& td, const CopyOrAdd copy_or_add)
      {
	if (SBN::detail::get_debug_level() > 0 && has_multiple_ranks(tf) && has_multiple_ranks(td))
	  SBN::detail::check_consistency(
	    SB::getHash(std::make_tuple(std::string("copyOrAddTo"), get_fingerprint(tf),
					get_fingerprint(td), copy_or_add)),
	    get_mpi_comm(tf));

	// Check consistency of the operation
	if (!is_compatible_for_copying(get_scoor(tf.order, tf.size), get_scoor(td.order, td.size)))
	  throw std::runtime_error("copyOrAddTo: tensor dimensions don't match");
	if (!tf.ptr || !td.ptr)
	  throw std::runtime_error("copyOrAddTo: invalid action on an example tensor");
	if (tf.distribution->ranks != td.distribution->ranks)
	  throw std::runtime_error("copyOrAddTo: the origin and the destination tensor don't "
				   "have compatible communicators");
	if (std::abs(td.scale) == 0)
	  throw std::runtime_error(
	    "copyOrAddTo: invalid operation for a destination tensor with scale zero");

	// Quick shortcut
	if (volume(tf.size) == 0)
	  return;
	if (std::abs(tf.scale) == 0 && copy_or_add == CopyOrAdd::Add)
	  return;
	if (std::abs(tf.scale) == 0 && copy_or_add == CopyOrAdd::Copy)
	{
	  set_zero(td);
	  return;
	}

	// Deal for the case that tf and td have the same allocation
	if (tf.ptr == td.ptr)
	{
	  if (!is_intersection_empty(tf.from, tf.size, td.from, td.size, tf.dim))
	  {
	    copyOrAddTo(clone(tf), td, copy_or_add);
	    return;
	  }
	}

	// Redo the operation with the reference implementation
	reference_implementation::Tensor td_ref;
	if (detail::check_against_reference_implementation())
	{
	  const auto& tf_ref = detail::to_ref_tensor(tf);
	  td_ref = detail::to_ref_tensor(
	    td, copy_or_add == CopyOrAdd::Copy ? CheckNnzIndices::False : CheckNnzIndices::True);
	  if (get_rank(get_mpi_comm(tf)) == 0 && get_rank(get_mpi_comm(td)) == 0)
	  {
	    if (copy_or_add == CopyOrAdd::Copy)
	      reference_implementation::copyTo(tf_ref, td_ref);
	    else
	      reference_implementation::addTo(tf_ref, td_ref);
	  }
	}

	// Check for unsupported case for fast copy
	const auto& [_0, order_sp_f] = split_dense_sparse(tf.order, tf.n_sp);
	const auto& [_1, order_sp_d] = split_dense_sparse(td.order, td.n_sp);
	if (
	  // a) the origin is sparse and the destination dense or vice versa
	  (tf.n_sp == 0) != (td.n_sp == 0) ||
	  // b) add sparse tensors
	  ((tf.n_sp > 0 || td.n_sp > 0) && copy_or_add == CopyOrAdd::Add) ||
	  // c) slicing sparse components
	  (detail::slicing_sparse_components(tf) || detail::slicing_sparse_components(td)) ||
	  // d) permutation of the sparse components
	  (order_sp_f != order_sp_d) ||
	  // e) unsupported distribution of the sparse components
	  ((tf.n_sp > 0 || td.n_sp > 0) && !is_supported_sparse_distribution_for_fast_copy(tf, td)))
	{
	  copyOrAddTo_slow(tf, td, copy_or_add);
	}
	else
	{
	  // Check unsupported case: there are more nonzeros on the origin than on the distination
	  if (tf.n_sp > 0)
	  {
	    if (tf.max_nnz_per_row > td.max_nnz_per_row || order_sp_f != order_sp_d)
	    {
	      throw std::runtime_error("copyOrAddTo: not implemented");
	    }
	  }

	  // Zero out the destination if the maximum number of nonzeros does not coincide
	  if (tf.n_sp > 0 && tf.max_nnz_per_row < td.max_nnz_per_row)
	    set_zero(td);

	  auto* ptr0 = data(tf);
	  auto* ptr1 = data(td);

	  const char sp_label =
	    (tf.n_sp == 0 && td.n_sp == 0 ? 0 : get_new_label(tf.order + td.order));
	  const auto& [ids0, ids1, nnz_blocks] =
	    get_unique_range_ids_for_copy(tf.from, tf.size, tf.distribution->fs, tf.dim, tf.n_sp,
					  tf.max_nnz_per_row, get_perm(tf.order, td.order), td.from,
					  td.distribution->fs, td.dim, td.n_sp, td.max_nnz_per_row);
	  const auto& [from_dense0, size_dense0, dim_dense0] =
	    get_dense_range<N>(false, tf, -1, ids0, nnz_blocks);
	  const auto& [from_dense1, size_dense1, dim_dense1] =
	    get_dense_range<N>(false, td, get_num_unique_ids(ids0) * nnz_blocks, ids1, nnz_blocks);
	  const auto& [fs0, order0] = extend_range_and_order_dense<N>(
	    tf.distribution->fs, tf.distribution->dim, 0, sp_label, tf.order, tf.n_sp,
	    tf.max_nnz_per_row, ids0, nnz_blocks);
	  const auto& [fs1, order1] = extend_range_and_order_dense<N>(
	    td.distribution->fs, td.distribution->dim, 0, sp_label, td.order, td.n_sp,
	    td.max_nnz_per_row, ids1, nnz_blocks, order0);
	  if (tf.is_conjugated == td.is_conjugated)
	  {
	    superbblas::copy<N, N, value_type, value_type>(
	      tf.scale / td.scale, //
	      fs0.data(), 1, order0.c_str(), from_dense0, size_dense0, dim_dense0,
	      (const value_type**)&ptr0, nullptr, &get_context(tf), //
	      fs1.data(), 1, order1.c_str(), from_dense1, dim_dense1, (value_type**)&ptr1, nullptr,
	      &get_context(td),
#  ifdef SUPERBBLAS_USE_MPI
	      get_mpi_comm(tf),
#  endif
	      superbblas::FastToSlow,
	      copy_or_add == CopyOrAdd::Copy ? superbblas::Copy : superbblas::Add);
	  }
	  else
	  {
	    using real_type = typename value_type::value_type;
	    const char c = get_new_label(tf.order + td.order + std::string{sp_label});
	    const auto& scale = tf.scale / std::conj(td.scale);
	    const auto& [from_dense0, size_dense_all0, dim_dense0] =
	      get_dense_range<N>(true, tf, -1, ids0, nnz_blocks);
	    const auto& [from_dense_real0, from_dense_imag0, size_dense0] =
	      get_from_real_and_imag_and_size(from_dense0, size_dense_all0);
	    const auto& [from_dense1, size_dense_all1, dim_dense1] =
	      get_dense_range<N>(true, td, tf.max_nnz_per_row, ids1, nnz_blocks);
	    const auto& [from_dense_real1, from_dense_imag1, size_dense1] =
	      get_from_real_and_imag_and_size(from_dense1, size_dense_all1);
	    const auto& [fs0, order0] = extend_range_and_order_dense<N>(
	      tf.distribution->fs, tf.distribution->dim, c, sp_label, tf.order, tf.n_sp,
	      tf.max_nnz_per_row, ids0, nnz_blocks);
	    const auto& [fs1, order1] = extend_range_and_order_dense<N>(
	      td.distribution->fs, td.distribution->dim, c, sp_label, td.order, td.n_sp,
	      td.max_nnz_per_row, ids1, nnz_blocks, order0);

	    // real(tf)*real(scale) -> real(td)
	    superbblas::copy<N, N, real_type, real_type>(
	      std::real(scale), //
	      fs0.data(), 1, order0.c_str(), from_dense_real0, size_dense0, dim_dense0,
	      (const real_type**)&ptr0, nullptr, &get_context(tf), //
	      fs1.data(), 1, order1.c_str(), from_dense_real1, dim_dense1, (real_type**)&ptr1,
	      nullptr, &get_context(td),
#  ifdef SUPERBBLAS_USE_MPI
	      get_mpi_comm(tf),
#  endif
	      superbblas::FastToSlow,
	      copy_or_add == CopyOrAdd::Copy ? superbblas::Copy : superbblas::Add);

	    // -imag(conj(tf))*imag(conj(scale)) add to real(td)
	    superbblas::copy<N, N, real_type, real_type>(
	      -std::imag(scale), //
	      fs0.data(), 1, order0.c_str(), from_dense_imag0, size_dense0, dim_dense0,
	      (const real_type**)&ptr0, nullptr, &get_context(tf), //
	      fs1.data(), 1, order1.c_str(), from_dense_real1, dim_dense1, (real_type**)&ptr1,
	      nullptr, &get_context(td),
#  ifdef SUPERBBLAS_USE_MPI
	      get_mpi_comm(tf),
#  endif
	      superbblas::FastToSlow, superbblas::Add);

	    // real(tf)*imag(conj(scale)) -> imag(td)
	    superbblas::copy<N, N, real_type, real_type>(
	      -std::imag(scale), //
	      fs0.data(), 1, order0.c_str(), from_dense_real0, size_dense0, dim_dense0,
	      (const real_type**)&ptr0, nullptr, &get_context(tf), //
	      fs1.data(), 1, order1.c_str(), from_dense_imag1, dim_dense1, (real_type**)&ptr1,
	      nullptr, &get_context(td),
#  ifdef SUPERBBLAS_USE_MPI
	      get_mpi_comm(tf),
#  endif
	      superbblas::FastToSlow,
	      copy_or_add == CopyOrAdd::Copy ? superbblas::Copy : superbblas::Add);

	    // imag(conj(tf))*real(scale) add to imag(td)
	    superbblas::copy<N, N, real_type, real_type>(
	      -std::real(scale), //
	      fs0.data(), 1, order0.c_str(), from_dense_imag0, size_dense0, dim_dense0,
	      (const real_type**)&ptr0, nullptr, &get_context(tf), //
	      fs1.data(), 1, order1.c_str(), from_dense_imag1, dim_dense1, (real_type**)&ptr1,
	      nullptr, &get_context(td),
#  ifdef SUPERBBLAS_USE_MPI
	      get_mpi_comm(tf),
#  endif
	      superbblas::FastToSlow, superbblas::Add);
	  }

	  if (tf.n_sp > 0)
	  {
	    IndexType* cols0 = nnz_indices(tf);
	    IndexType* cols1 = nnz_indices(td);
	    const auto& [tf_dim_dense, tf_dim_sp] = split_dense_sparse(tf.dim, tf.n_sp);
	    const auto& [td_dim_dense, td_dim_sp] = split_dense_sparse(td.dim, td.n_sp);
	    if (tf_dim_sp == td_dim_sp)
	    {
	      superbblas::copy<N, N, IndexType, IndexType>(
		IndexType{1}, //
		fs0.data(), 1, order0.c_str(), from_dense0, size_dense0, dim_dense0,
		(const IndexType**)&cols0, nullptr, &*getCpuContext(), //
		fs1.data(), 1, order1.c_str(), from_dense1, dim_dense1, (IndexType**)&cols1,
		nullptr, &*getCpuContext(),
#  ifdef SUPERBBLAS_USE_MPI
		get_mpi_comm(tf),
#  endif
		superbblas::FastToSlow, superbblas::Copy);
	    }
	    else
	    {
	      throw std::runtime_error("copyOrAddTo: unsupported case");
	    }
	  }
	}

	// Check the result against the reference implementation
	if (detail::check_against_reference_implementation())
	{
	  const auto& td_ref_r = detail::to_ref_tensor(td);
	  if (get_rank(get_mpi_comm(tf)) == 0 && get_rank(get_mpi_comm(td)) == 0)
	  {
	    reference_implementation::addTo(reference_implementation::scale(td_ref, -1), td_ref_r);
	    if (reference_implementation::frob(td_ref_r) >
		reference_implementation::frob(td_ref) * 1e-5)
	      throw std::runtime_error("copyOrAddTo: invalid result");
	  }
	}
      }
    }

    /// Copy the content of a tensor into another tensor
    /// \param tf: origin tensor
    /// \param td: destination tensor

    inline void copyTo(const Tensor& tf, const Tensor& td)
    {
      detail::copyOrAddTo(tf, td, detail::CopyOrAdd::Copy);
    }

    /// Add the content of a tensor into another tensor
    /// \param tf: origin tensor
    /// \param td: destination tensor

    inline void addTo(const Tensor& tf, const Tensor& td)
    {
      detail::copyOrAddTo(tf, td, detail::CopyOrAdd::Add);
    }

    /// Return a tensor with the same content but new layout
    /// \param t: input tensor
    /// \param new_order: new order

    inline Tensor reorder(const Tensor& t, const Order& new_order)
    {
      if (!have_same_labels(t.order, new_order))
	throw std::runtime_error("reorder: incompatible new labels");

      return make_sure(t, new_order);
    }

    namespace detail
    {
      inline std::tuple<superbblas::BSR_handle*, Order, Order>
      get_bsr(const Tensor& t, const std::string& used_labels = {})
      {
	if (SBN::detail::get_debug_level() > 0)
	  check_nnz_col_values(t);
	if (t.from != Coor(t.dim.size()) || t.size != t.dim)
	  throw std::runtime_error("get_bsr: unsupported on non tight views");
	const auto& [fs_dense, fs_sp, order_dense, order_sp] =
	  extend_range_and_order_bsr<N>(t.distribution->fs, t.dim, t.order, t.n_sp, used_labels);
	const auto& [dim_dense, dim_sp] = split_dense_sparse(t.dim, t.n_sp);
	const auto& [local_size_dense, local_size_sp] =
	  split_dense_sparse(get_local_size(*t.distribution), t.n_sp);
	const auto& [local_from_dense, local_from_sp] =
	  split_dense_sparse(get_local_from(*t.distribution), t.n_sp);
	const auto& ctx = get_context(t);
	const auto& ctx_cpu = *getCpuContext();
	const auto is_empty = volume(get_local_size(*t.distribution)) == 0;
	if (!is_empty && volume(local_size_sp) < t.max_nnz_per_row)
	  throw std::runtime_error("get_bsr: unsupported case, max_nnz_per_row is too large");
	const auto num_rows = is_empty ? 0 : volume(local_size_dense);
	std::vector<IndexType> ii(num_rows, t.max_nnz_per_row);
	std::vector<superbblas::Coor<N>> jj(num_rows * t.max_nnz_per_row);
	const auto* cols = nnz_indices(t);
	for (IndexType i = 0; i < jj.size(); ++i)
	{
	  jj.at(i) =
	    cols[i] < 0
	      ? superbblas::Coor<N>{{}}
	      : extend_to_coor<N>(
		  norm_coor(indexToCoor(cols[i], dim_sp) - local_from_sp, local_size_sp), 0);
	}
	const auto* v_ptr = data(t);
	IndexType* ii_ptr = ii.data();
	superbblas::Coor<N>* jj_ptr = jj.data();
	superbblas::BSR_handle* bsrh = nullptr;
	superbblas::create_bsr<N, N, value_type>(
	  fs_dense.data(), extend_to_coor<N>(dim_dense, 1), fs_sp.data(),
	  extend_to_coor<N>(dim_sp, 1), 1, extend_to_coor<N>({}, 1), extend_to_coor<N>({}, 1), true,
	  &ii_ptr, &ctx_cpu, &jj_ptr, &ctx_cpu, &v_ptr, &ctx,
#  ifdef SUPERBBLAS_USE_MPI
	  get_mpi_comm(t),
#  endif
	  superbblas::FastToSlow, &bsrh);

	return {bsrh, order_dense, order_sp};
      }
    }

    /// Contract the given tensors
    /// \param a: tensor to contract
    /// \param b: tensor to contract
    /// \param final_order: order of the resulting tensor

    inline Tensor contract(const Tensor& a, const Tensor& b, const Order& final_order)
    {
      if (SBN::detail::get_debug_level() > 0 && has_multiple_ranks(a) && has_multiple_ranks(b))
	SBN::detail::check_consistency(
	  SB::getHash(std::make_tuple(std::string("contract"), detail::get_fingerprint(a),
				      detail::get_fingerprint(b), final_order)),
	  get_mpi_comm(a));

      if (!is_valid(final_order))
	throw std::runtime_error("contract: invalid `final_order`");
      if (!is_compatible_for_contracting(get_scoor(a.order, a.size), get_scoor(b.order, b.size),
					 final_order))
	throw std::runtime_error("contract: tensor dimensions don't match");
      if (a.n_sp > 0 && b.n_sp > 0)
	throw std::runtime_error("contract: unsupported case, product of two sparse matrices");
      bool is_sparse = (a.n_sp > 0 || b.n_sp > 0);

      // Unsupported implicit conjugacy when input sparse matrix
      if (is_sparse && (a.is_conjugated || b.is_conjugated))
	throw std::runtime_error("contract: unsupported case");

      if (a.distribution->ranks != b.distribution->ranks)
	throw std::runtime_error("contract: the input tensors don't have compatible communicators");

      // superbblas follows the distribution of the first tensor, so put the biggest tensor first
      if (!is_sparse && volume(a.size) < volume(b.size))
      {
	return contract(b, a, final_order);
      }

      // Reorder the operands such that `b` is dense
      if (is_sparse && b.n_sp > 0)
      {
	return contract(b, a, final_order);
      }

      // Check that a has a tight view
      if (is_sparse && (a.from != Coor(a.dim.size()) || a.size != a.dim))
	return contract(make_sure(a, {}, Options::Distribution::Default, Options::Alloc::Default,
				  Options::TightView::True),
			b, final_order);

      // Get the dimensions of the final tensor
      SCoor sa = get_scoor(a.order, a.size);
      SCoor sb = get_scoor(b.order, b.size);
      SCoor sr = get_scoor(final_order, Coor(final_order.size(), 1));
      for (const auto& it : sa)
	if (sr.count(it.first) == 1)
	  sr.at(it.first) = it.second;
      for (const auto& it : sb)
	if (sr.count(it.first) == 1)
	  sr.at(it.first) = it.second;

      // Get the ordering for the resulting tensor
      std::vector<char> new_order_str(final_order.size() + 1);
      superbblas::suggested_orders_for_contraction(
	a.order.c_str(), a.order.size(), a.is_conjugated, b.order.c_str(), b.order.size(),
	b.is_conjugated, final_order.c_str(), final_order.size(), superbblas::FastToSlow, nullptr,
	nullptr, new_order_str.data());
      Order new_order(new_order_str.begin(), new_order_str.begin() + final_order.size());
      if (!have_same_labels(final_order, new_order))
	throw std::runtime_error("wtf");

      // Creating the resulting tensor
      // Set as the model the dense, largest volume input tensor
      Tensor model = (is_sparse ? b : a);
      // Set the tensor in the same place as the biggest tensor
      const auto xpu =
	detail::is_context_cpu(*model.ptr->ctx) ? Options::Alloc::Host : Options::Alloc::Device;
      auto new_distribution = std::string(Options::Distribution::Default);
      {
	const auto& a_dist = get_distribution_labels(a);
	const auto& b_dist = get_distribution_labels(b);
	const auto& model_dist = get_distribution_labels(model);
	std::string new_dist;
	for (char c : model_dist + (model_dist == a_dist ? b_dist : a_dist))
	{
	  if (std::find(new_order.begin(), new_order.end(), c) != new_order.end())
	    new_dist.push_back(c);
	}
	if (new_dist.size() > 0)
	  new_distribution = new_dist;
      }
      const auto& n = like_this(model, new_order, get_coor(new_order, sr), new_distribution, xpu);
      if (n.is_conjugated)
	throw std::runtime_error("wtf");

      using namespace detail;
      const auto& [fs1, order1] =
	extend_range_and_order_dense<N>(b.distribution->fs, b.distribution->dim, 0, 0, b.order,
					b.n_sp, b.max_nnz_per_row, {}, -1, a.order + n.order);
      const auto& [fs2, order2] =
	extend_range_and_order_dense<N>(n.distribution->fs, n.distribution->dim, 0, 0, n.order,
					n.n_sp, n.max_nnz_per_row, {}, -1, a.order + order1);
      const auto comm = get_mpi_comm(a);
      auto* ptr0 = data(a);
      auto* ptr1 = data(b);
      auto* ptr2 = data(n);
      if (!is_sparse)
      {
	const auto& [fs0, order0] =
	  extend_range_and_order_dense<N>(a.distribution->fs, a.distribution->dim, 0, 0, a.order,
					  a.n_sp, a.max_nnz_per_row, {}, -1, order1 + order2);
	superbblas::contraction<N, N, N>(
	  // implicit product of the tensors
	  cond_conj(a.is_conjugated, a.scale) * cond_conj(b.is_conjugated, b.scale),
	  // information of tensor `a`
	  fs0.data(), extend_to_coor<N>(a.from, 0), extend_to_coor<N>(a.size, 1),
	  extend_to_coor<N>(a.dim, 1), 1, order0.c_str(), a.is_conjugated,
	  (const value_type**)&ptr0, &get_context(a),
	  // information of tensor `b`
	  fs1.data(), extend_to_coor<N>(b.from, 0), extend_to_coor<N>(b.size, 1),
	  extend_to_coor<N>(b.dim, 1), 1, order1.c_str(), b.is_conjugated,
	  (const value_type**)&ptr1, &get_context(b),
	  // information of tensor `n`
	  value_type{0}, fs2.data(), extend_to_coor<N>(n.from, 0), extend_to_coor<N>(n.size, 1),
	  extend_to_coor<N>(n.dim, 1), 1, order2.c_str(), (value_type**)&ptr2, &get_context(n),
#  ifdef SUPERBBLAS_USE_MPI
	  comm,
#  endif
	  superbblas::FastToSlow);
      }
      else
      {
	const auto [bsrh, order0_dense, order0_sp] = detail::get_bsr(a, order1 + order2);
	superbblas::bsr_krylov<N, N, N, N>(
	  // implicit product of the tensors
	  cond_conj(a.is_conjugated, a.scale) * cond_conj(b.is_conjugated, b.scale) / n.scale,
	  // information of the input sparse tensor `a`
	  bsrh, order0_dense.c_str(), order0_sp.c_str(),
	  // information of tensor `b`
	  fs1.data(), 1, order1.c_str(), extend_to_coor<N>(b.from, 0), extend_to_coor<N>(b.size, 1),
	  extend_to_coor<N>(b.dim, 1), (const value_type**)&ptr1,
	  // information of tensor `n`
	  value_type{0}, fs2.data(), order2.c_str(), extend_to_coor<N>(n.from, 0),
	  extend_to_coor<N>(n.size, 1), extend_to_coor<N>(n.dim, 1), (char)0 /* no power */,
	  (value_type**)&ptr2, &get_context(n),
#  ifdef SUPERBBLAS_USE_MPI
	  comm,
#  endif
	  superbblas::FastToSlow);
	superbblas::destroy_bsr(bsrh);
      }

      // Check the result against the reference implementation
      if (detail::check_against_reference_implementation())
      {
	const auto& a_ref = detail::to_ref_tensor(a);
	const auto& b_ref = detail::to_ref_tensor(b);
	const auto& n_ref = detail::to_ref_tensor(n);
	if (get_rank(comm) == 0)
	{
	  const auto& n_ref_r = reference_implementation::contract(a_ref, b_ref, final_order);
	  reference_implementation::addTo(reference_implementation::scale(n_ref_r, -1), n_ref);
	  if (reference_implementation::frob(n_ref) >
	      reference_implementation::frob(n_ref_r) * 1e-5)
	  {
	    throw std::runtime_error("contract: invalid result");
	  }
	}
      }

      return n;
    }

    /// Return a copy of the tensor
    /// \param t: input tensor

    inline Tensor clone(const Tensor& t)
    {
      const auto& n = like_this(t);
      copyTo(t, n);
      return n;
    }

    /// Return a tensor with values implicitly scaled
    /// \param t: input tensor
    /// \param val: weight of the scale

    inline Tensor scale(const Tensor& t, const value_type& val)
    {
      if (SBN::detail::get_debug_level() > 0 && has_multiple_ranks(t))
	SBN::detail::check_consistency(
	  SB::getHash(std::make_tuple(std::string("scale"), detail::get_fingerprint(t), val)),
	  get_mpi_comm(t));

      Tensor n = t;
      n.scale *= cond_conj(t.is_conjugated, val);
      return n;
    }

    /// Return a view of a vector
    /// \param v: object
    /// \param order: labels for the dimensions
    /// \param size: size for the dimensions

    inline Tensor toTensor(const std::vector<value_type>& v, const Order& order, const SCoor& size)
    {
      if (order.size() == 0)
	throw std::runtime_error("toTensor: order with invalid number of dimensions");
      if (volume(order, size) != v.size())
	throw std::runtime_error("toTensor: dimensions don't match vector length");
      Coor dim = get_coor(order, size);
      auto v_ptr = const_cast<std::complex<double>*>(v.data());
      auto alloc = std::make_shared<detail::Allocation<value_type>>(v_ptr, detail::getCpuContext());
      auto p = std::make_shared<detail::Distribution>(detail::Distribution::Distributed, dim);
      return Tensor{Coor(dim.size()), dim, dim, order, alloc, p, 1.0, false, 0, 0, {}};
    }

    /// Return a view of the Array2d object
    /// \param op: object
    /// \param order: labels for the dimensions

    inline Tensor toTensor(const XMLArray::Array2d<value_type>& op, const Order& order)
    {
      if (order.size() != 2)
	throw std::runtime_error("toTensor: order with invalid number of dimensions");
      Coor dim = {op.size1(), op.size2()};
      auto op_ptr = const_cast<std::complex<double>*>((const std::complex<double>*)op);
      auto alloc =
	std::make_shared<detail::Allocation<value_type>>(op_ptr, detail::getCpuContext());
      auto p = std::make_shared<detail::Distribution>(detail::Distribution::Distributed, dim);
      return Tensor{Coor(dim.size()), dim, dim, reverse(order), alloc, p, 1.0, false, 0, 0, {}};
    }

    /// Return a view of the Array3d object
    /// \param op: object
    /// \param order: labels for the dimensions

    inline Tensor toTensor(const XMLArray::Array3d<value_type>& op, const Order& order)
    {
      if (order.size() != 3)
	throw std::runtime_error("toTensor: order with invalid number of dimensions");
      Coor dim = {op.size1(), op.size2(), op.size3()};
      auto op_ptr = const_cast<std::complex<double>*>((const std::complex<double>*)op);
      auto alloc =
	std::make_shared<detail::Allocation<value_type>>(op_ptr, detail::getCpuContext());
      auto p = std::make_shared<detail::Distribution>(detail::Distribution::Distributed, dim);
      return Tensor{Coor(dim.size()), dim, dim, reverse(order), alloc, p, 1.0, false, 0, 0, {}};
    }

    /// Return a view of the Array4d object
    /// \param op: object
    /// \param order: labels for the dimensions

    inline Tensor toTensor(const XMLArray::Array4d<value_type>& op, const Order& order)
    {
      if (order.size() != 4)
	throw std::runtime_error("toTensor: order with invalid number of dimensions");
      Coor dim = {op.size1(), op.size2(), op.size3(), op.size4()};
      auto op_ptr = const_cast<std::complex<double>*>((const std::complex<double>*)op);
      auto alloc =
	std::make_shared<detail::Allocation<value_type>>(op_ptr, detail::getCpuContext());
      auto p = std::make_shared<detail::Distribution>(detail::Distribution::Distributed, dim);
      return Tensor{Coor(dim.size()), dim, dim, reverse(order), alloc, p, 1.0, false, 0, 0, {}};
    }

    /// Finalize pending operation and destroy handlers

    inline void finalize()
    {
      // Report timings and cache usage on first node
      if (get_rank() == 0)
      {
	superbblas::reportTimings(std::cout);
	superbblas::reportCacheUsage(std::cout);
      }

      // Clear superbblas cache
      superbblas::clearCaches();

      // Call the destructor functions and erase the list
      for (const auto& fun : detail::getDestroyList())
	fun();
      detail::getDestroyList().resize(0);
    }
  }

  using namespace superbblas_implementation;
  using superbblas_implementation::contract;
#else
  /// FIXME: for now, use the reference implementation
  using namespace reference_implementation;
  using reference_implementation::contract;
#endif // USE_SUPERBBLAS

  /// Return the first active coordinate
  /// \param t: tensor

  inline SCoor get_kv_from(const Tensor& t)
  {
    return get_scoor(t.order, t.from);
  }

  /// Return the size of each dimension
  /// \param t: tensor

  inline SCoor get_kv_size(const Tensor& t)
  {
    return get_scoor(t.order, t.size);
  }

  /// Release the content of the given tensor
  /// \param t: tensor

  inline void release(Tensor& t)
  {
    t = Tensor();
  }

  /// Release the content of the given tensor returning the original tensor
  /// \param t: tensor

  inline Tensor move(Tensor& t)
  {
    auto r = t;
    t = Tensor();
    return r;
  }

  /// Return a tensor with values implicitly conjugated
  /// \param t: input tensor

  inline Tensor conj(const Tensor& t)
  {
    Tensor n = t;
    n.is_conjugated = !n.is_conjugated;
    return n;
  }

  /// Return a view of tensor with the given range
  /// \param t: input tensor
  /// \param from: first element of the view
  /// \param size: the extend of the view

  inline Tensor slice(const Tensor& t, const Coor& from, const Coor& size)
  {
    if (!is_less_or_equal_than(size, t.size))
      throw std::runtime_error("slice: invalid new size");

    Tensor n = t;
    n.from = norm_coor(t.from + from, t.dim);
    n.size = size;
    return n;
  }

  /// Return a view of tensor with the given range
  /// \param t: input tensor
  /// \param from: first element of the view
  /// \param size: the extend of the view

  inline Tensor slice_kv(const Tensor& t, const SCoor& from, const SCoor& size)
  {
    return slice(t, get_coor(t.order, from, 0),
		 get_coor(t.order, complete_scoor(size, get_scoor(t.order, t.size))));
  }

  /// Return a view of tensor with the given range
  /// \param t: input tensor
  /// \param ranges: the extend of the view

  inline Tensor slice_kv(const Tensor& t, const std::array<SCoor, 2>& ranges)
  {
    return slice_kv(t, ranges.at(0), ranges.at(1));
  }

  /// Return a tensor with the dimensions labels renamed
  /// \param t: input tensor
  /// \param m: a map to the new labels

  inline Tensor relabel(const Tensor& t, const remap& m)
  {
    const auto& new_order = relabel(m, t.order);
    if (!is_valid(new_order))
      throw std::runtime_error("relabel: invalid new ordering");

    Tensor n = t;
    n.order = new_order;
    return n;
  }

  /// Contract the given tensors
  /// \param a: tensor to contract
  /// \param b: tensor to contract
  /// \param c: tensor to contract
  /// \param final_order: order of the resulting tensor

  inline Tensor contract(Tensor a, Tensor b, Tensor c, const Order& final_order)
  {
    if (!is_valid(final_order))
      throw std::runtime_error("contract: invalid `final_order`");
    const auto& sc_a = get_scoor(a.order, a.size);
    const auto& sc_b = get_scoor(b.order, b.size);
    const auto& sc_c = get_scoor(c.order, c.size);
    if (!is_compatible_for_contracting(sc_a, sc_b, final_order) ||
	!is_compatible_for_contracting(sc_a, sc_c, final_order) ||
	!is_compatible_for_contracting(sc_b, sc_c, final_order))
      throw std::runtime_error("contract: tensor dimensions don't match");

    // Get order for contracting a and b
    const auto& order_ab = common_labels(a.order + b.order, c.order + final_order);

    // Get order for contracting a and c
    const auto& order_ac = common_labels(a.order + c.order, b.order + final_order);

    // Get order for contracting b and c
    const auto& order_bc = common_labels(b.order + c.order, a.order + final_order);

    // Do the contraction
    if (a.n_sp == 0 && b.n_sp == 0 && c.n_sp == 0)
    {
      // Get the volume for the auxiliary tensors ab and bc
      SCoor sc_abc = sc_a;
      for (const auto& sc : std::vector<SCoor>{sc_b, sc_c})
	for (const auto& it : sc)
	  sc_abc[it.first] = it.second;
      const auto vol_ab = volume(order_ab, sc_abc);
      const auto vol_ac = volume(order_ac, sc_abc);
      const auto vol_bc = volume(order_bc, sc_abc);

      // If the input tensors are dense, contract them in a way that minimizes the
      // storage of the intermediate tensor
      if (vol_ab <= vol_ac && vol_ab <= vol_bc)
	return contract(contract(move(a), move(b), order_ab), move(c), final_order);
      else if (vol_bc <= vol_ab && vol_bc <= vol_ac)
	return contract(contract(move(b), move(c), order_bc), move(a), final_order);
      else
	return contract(contract(move(a), move(c), order_ac), move(b), final_order);
    }
    else
    {
      // If some of the tensors are sparse, avoid contracting two sparse tensors
      if ((a.n_sp == 0) != (b.n_sp == 0))
	return contract(contract(move(a), move(b), order_ab), move(c), final_order);
      else if ((a.n_sp == 0) != (c.n_sp == 0))
	return contract(contract(move(a), move(c), order_ac), move(b), final_order);
      else
	return contract(contract(move(b), move(c), order_bc), move(a), final_order);
    }
  }

  /// Execute the given function passing the coordinate and the value for the local elements
  /// \param t: input tensor
  /// \param fun: function to execute on each value; signature: fun(coor, value)

  template <typename Fun>
  void for_each_coor_value(const Tensor& t, const Fun& fun)
  {
    if (!t.ptr)
      throw std::runtime_error("for_each_coor_value: unsupported on example tensors");

    const auto& t_local = get_local_tensor(t);
    const auto& t0_local = make_sure(t_local, {}, Options::Distribution::Default,
				     Options::Alloc::Host, Options::TightView::Default, 0);

    const auto& [from, size] = get_local_range(t0_local, Options::ComplainIfIsntTightView::False);
    value_type* ptr = data(t0_local);
    IndexType vol = volume(size);
    for (IndexType i = 0; i < vol; ++i)
    {
      const auto& absolute_coor = norm_coor(indexToCoor(i, size) + from, t0_local.dim);
      const auto& relative_coor = norm_coor(absolute_coor - t0_local.from, t0_local.dim);
      if (is_less_than(relative_coor, t0_local.size))
	fun(relative_coor, ptr[i]);
    }
  }

  /// Execute the given function passing the coordinate and the value for the local elements
  /// \param t: first input tensor
  /// \param q: second input tensor
  /// \param fun: function to execute on each value; signature: fun(coor, t_value, q_value)

  template <typename Fun>
  void for_each_coor_value(const Tensor& t, const Tensor& q, const Fun& fun)
  {
    if (!t.ptr || !q.ptr)
      throw std::runtime_error("for_each_coor_value: unsupported on example tensors");
    if (get_scoor(t.order, t.size) != get_scoor(q.order, q.size))
      throw std::runtime_error("for_each_coor_value: both tensor should have the same size");

    // FIXME: avoid the copy
    const auto& t0 = make_sure(t, {}, Options::Distribution::Default, Options::Alloc::Host,
			       Options::TightView::True, 0);
    const auto& q0 = make_sure(q, t0.order, Options::Distribution::Default, Options::Alloc::Host,
			       Options::TightView::True, 0);
#if defined(USE_SUPERBBLAS) && !defined(SUPERBNOVA_DEBUG)
    if (t0.distribution->fs != q0.distribution->fs)
      throw std::runtime_error("for_each_coor_value: incompatible distributions");
#endif

    const auto& [from, size] = get_local_range(t0);
    const value_type* tptr = data(t0);
    const value_type* qptr = data(q0);
    IndexType vol = volume(size);
    for (IndexType i = 0; i < vol; ++i)
    {
      const auto& coor = indexToCoor(i, size);
      fun(norm_coor(coor + from, t0.dim), tptr[i], qptr[i]);
    }
  }

  /// Fill the tensor with the values given by a function
  /// \param t: input tensor
  /// \param fun: function to execute on coordinate; signature: fun(coor) -> value

  template <typename Fun>
  void fill_for_each_coor(const Tensor& t, const Fun& fun)
  {
    if (!t.ptr)
      throw std::runtime_error("fill_for_each_coor: unsupported on example tensors");
    const auto& t_local = get_local_tensor(t);
    const auto& t0_local = make_sure(t_local, {}, Options::Distribution::Default,
				     Options::Alloc::Host, Options::TightView::Default, 0);

    const auto& [from, size] = get_local_range(t0_local, Options::ComplainIfIsntTightView::False);
    value_type* ptr = data(t0_local);
    IndexType vol = volume(size);
    for (IndexType i = 0; i < vol; ++i)
    {
      const auto& absolute_coor = norm_coor(indexToCoor(i, size) + from, t0_local.dim);
      const auto& relative_coor = norm_coor(absolute_coor - t0_local.from, t0_local.dim);
      if (is_less_than(relative_coor, t0_local.size))
	ptr[i] = fun(relative_coor);
    }

    if (t0_local.ptr != t_local.ptr)
      copyTo(t0_local, t_local);
  }

  /// List of ranges to visit all elements of a tensor a given size
  /// \param size: tensor dimensions
  /// \param max_block_size: maximum number of elements in each direction in a range
  /// \param emit_when_empty: whether to return a single empty range element when the tensor is empty
  /// \param ordering: whether to return the blocks in natural or lexicographic order

  inline std::vector<std::array<Coor, 2>>
  list_blocks(const Coor& size, const Coor& max_block_size, bool emit_when_empty = false,
	      Options::Ordering ordering = Options::Ordering::Natural)
  {
    const int n = size.size();
    if (size.size() != max_block_size.size())
      throw std::runtime_error("list_blocks: invalid input");

    // Do trivial case
    if (volume(size) == 0 && emit_when_empty)
      return {std::array<Coor, 2>{Coor(n, 0), Coor(n, 0)}};

    // Reverse natural ordering if asked
    if (ordering != Options::Ordering::Natural)
    {
      auto rev_size = size;
      std::reverse(rev_size.begin(), rev_size.end());
      auto rev_max_block_size = max_block_size;
      std::reverse(rev_max_block_size.begin(), rev_max_block_size.end());
      auto list =
	list_blocks(rev_size, rev_max_block_size, emit_when_empty, Options::Ordering::Natural);
      for (auto& fs : list)
	for (auto& fsi : fs)
	  std::reverse(fsi.begin(), fsi.end());
      return list;
    }

    Coor dim(n);
    for (std::size_t i = 0; i < n; ++i)
      dim.at(i) = (size.at(i) + max_block_size.at(i) - 1) / max_block_size.at(i);

    std::size_t vol = volume(dim);
    std::vector<std::array<Coor, 2>> r;
    r.reserve(vol);
    for (IndexType chunk_index = 0; chunk_index < vol; ++chunk_index)
    {
      const auto& chunk_coor = indexToCoor(chunk_index, dim);
      Coor chunk_from(n), chunk_size(n);
      for (int i = 0; i < n; ++i)
	chunk_from.at(i) = size.at(i) / dim.at(i) * chunk_coor.at(i) +
			   std::min(chunk_coor.at(i), size.at(i) % dim.at(i));
      for (int i = 0; i < n; ++i)
	chunk_size.at(i) =
	  size.at(i) / dim.at(i) + (chunk_coor.at(i) < size.at(i) % dim.at(i) ? 1 : 0);
      if (volume(chunk_size) > 0)
	r.push_back({chunk_from, chunk_size});
    }
    return r;
  }

  /// Return a subtensor with support on the current process
  /// \param t: input tensor

  inline Tensor get_local_tensor_tight(const Tensor& t)
  {
    const auto t_local = get_local_tensor(t);
    const auto d = get_distribution(t_local);
    if (d.size() != 1)
      throw std::runtime_error("wtf");
    const auto& [from, size] = get_simple_intersection(t_local.from, t_local.size, d.at(0).at(0),
						       d.at(0).at(1), t_local.dim);
    return slice(t_local, from - t_local.from, size);
  }

  /// Return another tensor by applying a given function to subtensors of the given tensor
  /// \param t: input tensor
  /// \param block: maximum size of the subtensors
  /// \param ordering: whether to return the blocks in natural or lexicographic order
  /// \param fun: function to execute on each subtensor; signature: fun(tensor, index, num_subtensors,
  ///        current_from, next_from, next_size) -> tensor

  template <typename Fun>
  Tensor transform_for_each_block(const Tensor& t, const SCoor& block, Options::Ordering ordering,
				  const Fun& fun)
  {
    // Deal with trivial case: empty tensor
    if (volume(t.size) == 0)
      return t;

    const auto num_ranks = get_comm(t).ranks.size();
    Coor big_block(t.size.size());
    Coor procs(t.size.size());

    IndexType rem_ranks = num_ranks;
    for (std::size_t j = 0; j < t.size.size(); ++j) {
      const auto i = ordering == Options::Ordering::Natural ? j : t.size.size() - j - 1;
      const auto si = t.size.at(i);
      const auto label = t.order.at(i);
      const auto bi = block.count(label) == 0 ? si : block.at(label);
      const auto bbi = block.count(label) == 0 ? si : std::min(si, bi * rem_ranks);
      const auto di = std::min(rem_ranks, (bbi + bi - 1) / bi);
      rem_ranks /= di;
      big_block.at(i) = bbi;
      procs.at(i) = di;
      if ((bbi + di - 1) / di > bi)
	throw std::runtime_error("wtf");
    }

    if (volume(procs) > num_ranks)
      throw std::runtime_error("wtf");

    const auto big_ranges = list_blocks(t.size, big_block, false, ordering);
    const auto r = like_this(t);

    const auto& comm = SBN::get_comm(t);
    const auto rank = SBN::get_rank(comm.comm);
    SBN::Coor promised_from, promised_size;
    for (std::size_t big_ranges_index = 0; big_ranges_index < big_ranges.size(); ++big_ranges_index)
    {
      const auto& [from, size] = big_ranges.at(big_ranges_index);
      const auto& this_t = slice(t, from, size);
      Coor max_proc_size;
      for (std::size_t i = 0; i < this_t.size.size(); ++i)
	max_proc_size.push_back((size.at(i) + procs.at(i) - 1) / procs.at(i));
      auto proc_ranges = list_blocks(size, max_proc_size);
      if (proc_ranges.size() > num_ranks)
	throw std::runtime_error("wtf");
      while (proc_ranges.size() != num_ranks)
	proc_ranges.push_back({Coor(t.size.size()), Coor(t.size.size())});
      using namespace SBN::detail;
      const auto local_from = SBN::norm_coor(from + proc_ranges.at(rank).at(0), t.size);

      // Check consistency in the indicated next slice
      if (big_ranges_index > 0 &&
	  (local_from != promised_from || proc_ranges.at(rank).at(1) != promised_size))
	throw std::runtime_error("wtf");

      SBN::Coor next_local_from, next_local_size;
      if (big_ranges_index + 1 < big_ranges.size())
      {
	const auto& [from, size] = big_ranges.at(big_ranges_index + 1);
	Coor max_proc_size;
	for (std::size_t i = 0; i < size.size(); ++i)
	  max_proc_size.push_back((size.at(i) + procs.at(i) - 1) / procs.at(i));
	auto proc_ranges = list_blocks(size, max_proc_size);
	if (proc_ranges.size() > num_ranks)
	  throw std::runtime_error("wtf");
	while (proc_ranges.size() != num_ranks)
	  proc_ranges.push_back({Coor(t.size.size()), Coor(t.size.size())});
	promised_from = next_local_from = SBN::norm_coor(from + proc_ranges.at(rank).at(0), t.size);
	promised_size = next_local_size = proc_ranges.at(rank).at(1);
      }
      else
      {
	next_local_from = Coor(t.size.size());
	next_local_size = Coor(t.size.size());
      }
      const auto& dist_this_t =
	create_tensor(this_t.size, this_t.order, {}, get_allocation_type(this_t), this_t.n_sp,
		      this_t.max_nnz_per_row, Options::IsEg::False, proc_ranges, comm);
      const auto& dist_this_r =
	create_tensor(this_t.size, this_t.order, {}, get_allocation_type(this_t), this_t.n_sp,
		      this_t.max_nnz_per_row, Options::IsEg::False, proc_ranges, comm);
      copyTo(this_t, dist_this_t);
      copyTo(fun(get_local_tensor_tight(dist_this_t), big_ranges_index, big_ranges.size(),
		 local_from, next_local_from, next_local_size),
	     get_local_tensor_tight(dist_this_r));
      copyTo(dist_this_r, slice(r, from, size));
    }

    return r;
  }

  inline bool has_replicated_parts(const Tensor& t)
  {
    const auto& d = get_distribution(t);
    for (std::size_t i = 0; i < d.size(); ++i)
    {
      for (std::size_t j = 0; j < i; ++j)
      {
	if (!is_intersection_empty(d.at(i).at(0), d.at(i).at(1), d.at(j).at(0), d.at(j).at(1),
				   t.dim))
	{
	  return true;
	}
      }
    }
    return false;
  }

  /// Return a new tensor
  /// \param dim: dimensions of the new tensor
  /// \param order: labels of the new tensor
  /// \param distribution: policy to distribute the content of the tensor among the processes:
  ///          - Options::Distribution::OnMaster: with support on process 0
  ///          - Options::Distribution::Replicated: with full support on all processes
  ///          - string with the labels to distribute among the processes
  ///        NOTE: operations with tensors supported on several processes may invoke MPI functions
  /// \param xpu: either
  ///          - Options::Alloc:Host: the tensor content is on the CPU
  ///          - Options::Alloc:Device: the tensor content may be on a GPU device.
  /// \param n_sp: the last `n_sp` dimensions are sparse
  /// \param max_nnz_per_row: maximum number of nonzero values for non-sparse coordinate
  /// \param eg: either
  ///          - Options::IsEg::False: allocate a tensor
  ///          - Options::IsEg::True: don't allocate a tensor and just create a template
  /// \param guide: (optional) the distribution of the tensor will be used to map the local
  ///        components into the global
  /// \param comm: (optional) communicator

  inline Tensor create_tensor_with_local_components(
    const Coor& global_or_local_dim, const Order& order, const std::string& distribution,
    Options::Alloc xpu = Options::Alloc::Host, int n_sp = 0, int max_nnz_per_row = 0,
    Options::IsEg eg = Options::IsEg::False, const Tensor& guide = {}, Comm comm = {})
  {
    if (global_or_local_dim.size() != order.size())
      throw std::runtime_error("create_tensor_with_local_components: invalid input");
    if (is_predefined_distribution(distribution))
    {
      throw std::runtime_error("create_tensor_with_local_components: invalid input");
    }

    // Get communicator
    if (guide.dim.size() > 0)
    {
      if (comm.ranks.size() == 0)
	comm = get_comm(guide);
      else if (comm.ranks.size() > 0 && comm.ranks != get_comm(guide).ranks)
	throw std::runtime_error("create_tensor_with_local_components: invalid input `comm'");
    }
    else if (comm.ranks.size() == 0)
      comm = get_comm_from_mpi_comm(get_global_mpi_comm());
    const auto mpi_comm = comm.comm;

    std::string new_distribution = distribution;
    const auto& all_global_or_local_dim = gather(global_or_local_dim, mpi_comm);
    Coor dim = global_or_local_dim;
    std::vector<std::map<int, std::array<IndexType, 2>>> local_ranges(order.size());
    const auto trivial_ids = [=]() {
      std::vector<IndexType> v(get_nprocs(mpi_comm));
      for (std::size_t i = 0; i < v.size(); ++i)
	v.at(i) = i;
      return v;
    };
    const auto& u =
      guide.dim.size() == 0
	? trivial_ids()
	: SBN::get_unique_range_ids(guide, Options::ReturnUniqueIdForEmptyRanges::True);
    std::string local_order;
    std::vector<char> dim_i_is_local(order.size());
    for (std::size_t i = 0; i < order.size(); ++i)
    {
      bool dim_i_is_nonzero = false;
      for (int proc = 0; proc < get_nprocs(mpi_comm); ++proc)
      {
	const auto global_or_local_dim_at_master_at_i = all_global_or_local_dim.at(0).at(i);
	const auto global_or_local_dim_at_proc_at_i = all_global_or_local_dim.at(proc).at(i);
	if ((global_or_local_dim_at_proc_at_i > 0 || global_or_local_dim_at_master_at_i > 0) &&
	    global_or_local_dim_at_proc_at_i != global_or_local_dim_at_master_at_i)
	  throw std::runtime_error("create_tensor_with_local_components: inconsistent input");
	if (global_or_local_dim_at_proc_at_i != 0)
	{
	  if (dim_i_is_nonzero == false)
	  {
	    dim_i_is_nonzero = true;
	    dim_i_is_local.at(i) = global_or_local_dim_at_proc_at_i < 0 ? 1 : 0;
	  }
	  else if ((dim_i_is_local.at(i) != 0) != (global_or_local_dim_at_proc_at_i < 0))
	  {
	    throw std::runtime_error("create_tensor_with_local_components: inconsistent input");
	  }
	}
      }
      if (dim_i_is_local.at(i) != 0)
      {
	auto s = std::find(new_distribution.begin(), new_distribution.end(), order.at(i));
	if (s == new_distribution.end())
	  throw std::runtime_error("create_tensor_with_local_components: invalid input");
	new_distribution.erase(s);
	local_order.push_back(order.at(i));
	std::map<int, std::array<IndexType, 2>> m;
	IndexType size = 0;
	for (std::size_t j = 0; j < u.size(); ++j)
	{
	  const auto& local_size = -all_global_or_local_dim.at(j).at(i);
	  if (m.count(u.at(j)) == 0)
	  {
	    m[u.at(j)] = {size, local_size};
	    size += local_size;
	  }
	  else if (m.at(u.at(j)).at(1) != local_size)
	  {
	    throw std::runtime_error(
	      "create_tensor_with_local_components: inconsistent local sizes");
	  }
	}
	local_ranges.at(i) = m;
	dim.at(i) = size;
      }
    }

    auto d = get_distribution(create_tensor(dim, order, new_distribution, xpu, n_sp,
					    max_nnz_per_row, Options::IsEg::True, {}, comm));
    for (std::size_t i = 0; i < order.size(); ++i)
    {
      if (dim_i_is_local.at(i) != 0)
      {
	for (std::size_t j = 0; j < get_nprocs(mpi_comm); ++j)
	{
	  d.at(j).at(0).at(i) = local_ranges.at(i).at(u.at(j)).at(0);
	  d.at(j).at(1).at(i) = local_ranges.at(i).at(u.at(j)).at(1);
	}
      }
    }

    return create_tensor(dim, order, local_order + new_distribution, xpu, n_sp, max_nnz_per_row, eg,
			 d, comm);
  }

  /// Return a view or a copy of the given tensor with the dimensions reshaped
  /// \param t: input tensor
  /// \param new_order: new order
  /// \param new_dim: (optional) size of the new dimensions
  /// \param allow_copy: either Options::AllowCopy::True or Options::AllowCopy::False, whether
  ///        to allow return a copy of the tensor with the elements reshaped

  inline Tensor reshape(const Tensor& t, const Order& new_order, const Coor& new_dim = {},
			Options::AllowCopy allow_copy = Options::AllowCopy::True)
  {
    const auto& new_dim0 =
      new_dim.size() > 0 ? new_dim : SBN::get_coor(new_order, SBN::get_scoor(t.order, t.size), 1);
    if (new_order.size() != new_dim0.size())
      throw std::runtime_error(
	"reshape: invalid input, new_order and new_dim don't match in length");
    if (volume(new_dim0) != volume(t.size))
      throw std::runtime_error(
	"reshape: invalid input, the tensor size and the new_dim don't match in volume");

    // FIXME: avoid copying and support more cases
    if (allow_copy == Options::AllowCopy::False)
      throw std::runtime_error("reshape: unsupported case");
    const auto& r = like_this(t, new_order, new_dim0);
    copyTo(t, r);
    return r;
  }

  /// Return the result of concatenating the given tensors along a given dimension
  /// \param ts: tensor to concatenate
  /// \param label: dimensions to concatenate along
  /// \param model: (optional) return a tensor like this

  inline Tensor concat(const std::vector<Tensor>& ts, char label, const Tensor& model = {})
  {
    // Deal with trivial cases
    if (ts.size() == 0)
      return slice(model, Coor(model.size.size()), Coor(model.size.size()));
    if (ts.size() == 1)
      return ts.at(0);

    // Collect the dimensions of the final tensor
    SCoor sdim{{label, 0}};
    for (const auto& t : ts)
    {
      for (const auto& [l, s] : get_scoor(t.order, t.size))
      {
	if (l == label)
	  sdim.at(label) += s;
	else if (sdim.count(l) == 0)
	  sdim[l] = s;
	else if (sdim.at(l) != s)
	  throw std::runtime_error("concat: incompatible tensors to concat");
      }
    }

    // Create output tensor
    const Tensor& the_model = (model.size.size() == 0 ? ts.at(0) : model);
    const auto r = like_this(the_model, the_model.order, get_coor(the_model.order, sdim));

    // Copy the input tensors into the output tensor
    IndexType i = 0;
    for (const auto& t : ts)
    {
      const auto tdim = get_scoor(t.order, t.size);
      copyTo(t, slice_kv(r, {{label, i}}, {{label, tdim.at(label)}}));
      i += tdim.at(label);
    }

    return r;
  }

  /// Return a copy of the tensor without replicated parts
  /// \param t: input tensor

  inline Tensor without_replicated_parts(const Tensor& t)
  {
    if (!has_multiple_ranks(t) || !has_replicated_parts(t))
      return t;
    return make_sure(t, {}, Options::Distribution::OnMaster);
  }

  /// Return the Frobenius norm of the tensor
  /// \param t: input tensor

  inline double frob(const Tensor& t)
  {
    if (std::abs(t.scale) == 0 || volume(t.size) == 0)
      return 0;
    if (t.n_sp == 0)
    {
      const auto l = get_new_label(t.order);
      const auto& t0 = reshape(t, t.order + std::string{l}, concat(t.size, {1}));
      const auto& r = make_sure(contract(t0, SBN::conj(t0), std::string{l}), {},
				Options::Distribution::Replicated, Options::Alloc::Host);
      if (volume(r.size) != 1)
	throw std::runtime_error("wtf");
      double frob_norm = 0;
      for_each_coor_value(
	r, [&](const Coor&, const value_type& val) { frob_norm = std::sqrt(std::real(val)); });
      return frob_norm;
    }
    else
    {
      const auto& t0 = without_replicated_parts(t);
      double frob_norm2 = 0;
      for_each_coor_and_nonzero_value(
	t0, [&](const Coor&, const value_type& val) { frob_norm2 += std::norm(val); });
      double sum_frob_norm2 = 0;
      for (const auto& remote_frob_norm2 : gather(frob_norm2, get_mpi_comm(t0)))
	sum_frob_norm2 += remote_frob_norm2;
      return std::sqrt(sum_frob_norm2);
    }
  }

  /// Return a printing representation of labels
  /// \param order: input order

  inline std::string to_printing_order(const Order& order)
  {
    std::string r;
    for (char c : order)
    {
      if (std::isprint(static_cast<unsigned char>(c)))
      {
	r.push_back(c);
      }
      else
      {
	r = r + "<" + std::to_string((int)c) + ">";
      }
    }
    return r;
  }

  /// Print the tensor content in Matlab/Octave into std::cout
  /// \param t: tensor to print
  /// \param name: (optional) suffix of the Matlab variable name

  inline void print(const Tensor& t, const std::string& name = "")
  {
    const auto& t0 = make_sure(t, {}, Options::Distribution::OnMaster, Options::Alloc::Host,
			       Options::TightView::True, 0);
    const auto* v = data(t0);
    if (get_rank(get_mpi_comm(t0)) == 0)
    {
      std::cout << "%% labels: " << to_printing_order(t.order) << std::endl;
      std::cout << "t_" << name << "=reshape([";

      for (std::size_t i = 0; i < volume(t0.dim); ++i)
	std::cout << std::real(v[i]) << "+" << std::imag(v[i]) << "j ";

      std::cout << "], [";
      for (const auto i : t.size)
	std::cout << i << " ";
      std::cout << "]);" << std::endl;
    }
  }

  /// Store the tensor content into a file
  /// \param t: tensor to store
  /// \param filename: file name where to store the tensor

  inline void store(const Tensor& t, const std::string& filename)
  {
    const auto& t0 = make_sure(t, {}, Options::Distribution::OnMaster, Options::Alloc::Host,
			       Options::TightView::True);
    const auto* v = data(t0);
    const auto* vj = nnz_indices(t0);
    if (get_rank(get_mpi_comm(t0)) == 0)
    {
      auto f = std::fopen(filename.c_str(), "wb+");
      if (f == nullptr)
	throw std::runtime_error(std::string("error opening file ") + filename);

      const auto write_array = [=](const auto* v, std::size_t n) {
	if (std::fwrite(v, sizeof(v[0]), n, f) != n)
	  throw std::runtime_error(std::string("error writing on file ") + filename);
      };
      const auto write = [=](const auto& v) { write_array(&v, 1); };

      // Write number of dimensions
      write((int)t0.dim.size());

      // Write the labels
      if (t0.order.size() > 0)
	write_array(&t0.order[0], t0.order.size());

      // Write the size
      if (t0.size.size() > 0)
	write_array(&t0.size[0], t0.size.size());

      // Write the number of sparse dimensions
      write((int)t0.n_sp);

      // Write the maximum number of nonzero values
      write((int)t0.max_nnz_per_row);

      // Write the nonzero values
      write_array(v, volume(t0.size, t0.n_sp, t0.max_nnz_per_row));

      // Write the column values
      if (t0.n_sp > 0 && t0.max_nnz_per_row > 0)
	write_array(vj, volume(t0.size, t0.n_sp, t0.max_nnz_per_row));

      std::fclose(f);
    }
  }

  /// Load the tensor stored into a file
  /// \param filename: file name where to read the tensor

  inline Tensor load(const std::string& filename)
  {
    auto f = std::fopen(filename.c_str(), "rb");
    if (f == nullptr)
      throw std::runtime_error(std::string("error opening file ") + filename);

    const auto read_array = [=](auto* v, std::size_t n) {
      if (std::fread(v, sizeof(v[0]), n, f) != n)
	throw std::runtime_error(std::string("error reading on file ") + filename);
    };
    const auto read = [=](auto& v) { read_array(&v, 1); };

    // Read number of dimensions
    int n = 0;
    read(n);

    // Read the labels
    std::string order(n, 0);
    if (n > 0)
      read_array(&order[0], n);

    // Read the size
    Coor dim(n);
    if (n > 0)
      read_array(&dim[0], n);

    // Read the number of sparse dimensions
    int n_sp = 0;
    read(n_sp);

    // Read the maximum number of nonzero values
    int max_nnz_per_row = 0;
    read(max_nnz_per_row);

    // Create tensor
    auto t = create_tensor(dim, order, Options::Distribution::OnMaster, Options::Alloc::Host, n_sp,
			   max_nnz_per_row);
    auto* v = data(t);
    auto* vj = nnz_indices(t);

    if (get_rank() == 0)
    {
      // Read the nonzero values
      read_array(v, volume(dim, n_sp, max_nnz_per_row));

      // Read the column values
      if (n_sp > 0 && max_nnz_per_row > 0)
	read_array(vj, volume(dim, n_sp, max_nnz_per_row));
    }

    std::fclose(f);
    return t;
  }

  inline void record_or_compare(const Tensor& t, const std::string& filename)
  {
    static std::map<std::string, int> invocation;
    if (invocation.count(filename) == 0)
      invocation[filename] = 0;
    const std::string final_filename = filename + std::to_string(invocation.at(filename));
    invocation.at(filename)++;
    auto f = std::fopen(final_filename.c_str(), "rb");
    if (f != nullptr)
    {
      std::fclose(f);
      const auto& t0 = load(final_filename);
      auto d = clone(t0);
      addTo(scale(without_replicated_parts(t), -1), d);
      if (SBN::frob(d) > SBN::frob(t0) * 1e-5)
	throw std::runtime_error("record_or_compare: found a difference");
    }
    else
    {
      store(t, final_filename);
    }
  }

  inline void tests()
  {
    const auto check = [](bool c) {
      if (!c)
	throw std::runtime_error("wtf!");
    };
    const auto i = value_type{0.0, 1.0};

    for (auto s : std::vector<value_type>{1.0, i})
    {
      std::vector<value_type> v{s};
      const auto& t0 = toTensor(v, "i", {{'i', 1}});
      {
	const auto& t = clone(conj(scale(t0, i)));
	check(!t.is_conjugated);
	check(std::norm(SBN::get(t, {0}) - std::conj(s * i)) < 1e-7);
      }
      {
	std::vector<value_type> v{s};
	const auto& t = clone(scale(conj(t0), i));
	check(!t.is_conjugated);
	check(std::norm(SBN::get(t, {0}) - std::conj(s) * i) < 1e-7);
      }
    }
  }
}

#endif // __redstar_datalib_superb_tensor_h
