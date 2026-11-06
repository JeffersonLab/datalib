/*! \file
 * \brief Compute N-pt correlators with a simple graph contraction algorithm
 * using superbblas for contracting tensors
 */

#ifndef __redstar_datalib_superb_contraction_h
#define __redstar_datalib_superb_contraction_h

/******
Example of a graph for a nucleon 2 point function:

<GraphList>
  <elem>
    <ConnGraph>
        <-- absolutely enumeration of the vertex slots (eg, Bn:n:n:, Mn:n:, An:n:)
             and the connections between slots (Qn:n)
               ,-----, vertex with three slots  -->
        <graph>B1:2:3:B4:5:6:Q1:4:Q2:6:Q3:5:</graph>
        <---    ^ ^ ^ slots of the vertex  -->
        <-- visual representation of this example:

            | 1 | ------------- | 4 |
            |   |               |   |
            | 2 | ------\/----- | 5 |
            |   |       /\      |   |
            | 3 | -----/  \---- | 6 |

            first               second
            vertex              vertex
         -->

        <-- indicate the propagator direction (from antiquark to quark) and flavor -->
        <AdjMap>
          <elem>
            <Key>1</Key> <-- prop starting from base-1 vertex (B, M, A) index ... -->
            <Val>
              <elem>
                <Key>1</Key> <-- ... and base-1 slot index ... ->
                <Val>
                  <vertex_num>2</vertex_num> <-- ... to base-1 vertex index ... -->
                  <slot_num>1</slot_num>     <-- ... and slot base-1 slot index ... -->
                  <flavor>l</flavor>         <-- ... with this flavor -->
                  <conj>true</conj>          <-- ... and reversed direction (true for quark-to-antiquark) -->
                </Val>
              </elem>
              <elem>
                <Key>2</Key>
                <Val>
                  <vertex_num>2</vertex_num>
                  <slot_num>3</slot_num>
                  <flavor>l</flavor>
                  <conj>true</conj>
                </Val>
              </elem>
              <elem>
                <Key>3</Key>
                <Val>
                  <vertex_num>2</vertex_num>
                  <slot_num>2</slot_num>
                  <flavor>l</flavor>
                  <conj>true</conj>
                </Val>
              </elem>
            </Val>
          </elem>
          <elem>
            <Key>2</Key>
            <Val>
              <elem>
                <Key>1</Key>
                <Val>
                  <vertex_num>1</vertex_num>
                  <slot_num>1</slot_num>
                  <flavor>l</flavor>
                  <conj>false</conj>
                </Val>
              </elem>
              <elem>
                <Key>2</Key>
                <Val>
                  <vertex_num>1</vertex_num>
                  <slot_num>3</slot_num>
                  <flavor>l</flavor>
                  <conj>false</conj>
                </Val>
              </elem>
              <elem>
                <Key>3</Key>
                <Val>
                  <vertex_num>1</vertex_num>
                  <slot_num>2</slot_num>
                  <flavor>l</flavor>
                  <conj>false</conj>
                </Val>
              </elem>
            </Val>
          </elem>
        </AdjMap>
        <Vertices>
          <elem>
            <Key>1</Key> <-- base-1 vertex index -->
            <Val>
              <First> <-- HadronDispVertex_t content -->
                <name>NucleonMG1g1MxD0J0S_J1o2_H1o2C4nm0E</name>
                <type>B</type>
                <piece_num>1</piece_num>
                <Cconj>false</Cconj>
                <twoI_z>1</twoI_z>
                <row>2</row>
                <mom>1 0 6</mom>
                <phasings>
                  <elem>0 0 2</elem>
                </phasings>
                <creation_op>false</creation_op>
                <smearedP>true</smearedP>
              </First>
              <Second>1</Second> <-- time slice index -->
            </Val>
          </elem>
          <elem>
            <Key>2</Key>
            <Val>
              <First>
                <name>NucleonMG1g1MxD0J0S_J1o2_H1o2C4nnmE</name>
                <type>B</type>
                <piece_num>1</piece_num>
                <Cconj>false</Cconj>
                <twoI_z>1</twoI_z>
                <row>2</row>
                <mom>2 2 5</mom>
                <phasings>
                  <elem>0 0 2</elem>
                </phasings>
                <creation_op>true</creation_op>
                <smearedP>true</smearedP>
              </First>
              <Second>2</Second>
            </Val>
          </elem>
        </Vertices>
      </ConnGraph>
  </elem>
</GraphList>

Lets show the graph representation one more time for fun
for a nucleon 3 point function:

<graph>B1:2:3:A4:5:B6:7:8:Q1:6:Q2:8:Q3:4:Q5:7:</graph

                     /-----------\
            | 1 | --/             \---- | 6 |
            |   |     /-- | 4 |         |   |
            | 2 | --\/    |   |     /-- | 7 |
            |   |   /\    | 5 | ---/    |   |
            | 3 | -/  \         /------ | 8 |
                       \-------/
            first         second        third 
            vertex        vertex        vertex

Strategy for arranging the contractions:
- Minimize the memory footprint
- Contract two tensors at a time
- Only hold in memory the current result and the next tensor to contract
- Minimize the number of contractions

Example:

graph=             B1:2:3 B4:5:6 Q1:4 Q2:5 Q3:6
i=0, txvertex[i]=  2      0
i=1, txvertex[i]=  3      0
i=2, txvertex[i]=  4      0
i=3, txvertex[i]=  5      1
i=4, txvertex[i]=  6      1
i=5, txvertex[i]=  7      1

where i is the correlation function index, and txvertex[i] is a list
of unique keys identifying a pair of time slice and vertex for each
vertex with time information.

The algorithm that finds a contraction plan will star by choosing the
leftmost vertex with largest number of slots. And then contract subsequent
vertices such that the resulting tensor has the minimum number of slots.
Example:

graph=                  B1:2:3:i B4:5:6:i Q1:4:i Q2:5:i Q3:6:i
vertex_contract_order=  3        0        1      2      4
trace of active vertices:
 iteration=0:                    B4:5:6:i Q1:4:i               -> B1:5:6:i
 iteration=1:                    B1:5:6:i        Q2:5:i        -> B1:2:6:i
 iteration=2:           B1:2:3:i B1:2:6:i                      -> B3:6:i
 iteration=3:                    B3:6:i                 Q3:6:i -> corr_fun:i

NOTE: THE ABOVE SEQUENCE WILL BE EXECUTED FOR ALL CORRELATION
      FUNCTIONS i=0..5 SIMULTANEOUSLY!!!

That is a little bit of a waste because the B4:5:6 for corr i=0..2 will
be constructed three times and contracted with the corresponding Q1:4
and Q2:5 three times also.

graph=                   B1:2:3 B4:5:6 Q1:4 Q2:5 Q3:6
j,i=0,0, txvertex[j,i]=  2      0
j,i=1,0, txvertex[j,i]=  3      0
j,i=2,0, txvertex[j,i]=  4      0
j,i=0,1, txvertex[j,i]=  5      1
j,i=1,1, txvertex[j,i]=  6      1
j,i=2,1, txvertex[j,i]=  7      1

graph=                  B1:2:3:i:j B4:5:6:i Q1:4:i:j Q2:5:i:j Q3:6:i:j
vertex_contract_order=  3          0        1        2        4
trace of active vertices:
 iteration=0:                      B4:5:6:i Q1:4:i:j                   -> B1:5:6:i:j
 iteration=1:                      B1:5:6:i:j        Q2:5:i:j          -> B1:2:6:i:j
 iteration=2:           B1:2:3:i:j B1:2:6:i:j                          -> B3:6:i:j
 iteration=3:                      B3:6:i:j                   Q3:6:i:j -> corr_fun:i:j

NOTE: B4:5:6:i AND ITS CONTRACTIONS WITH Q1:4:i:j IT'S DONE ONLY ONCE!

Another example with a three-point function:

graph=                   B1:2:3 M3:6 B4:5:6 Q1:4 Q2:5
j,i=0,0, txvertex[j,i]=  2      4    0
j,i=1,0, txvertex[j,i]=  2      5    0
j,i=2,0, txvertex[j,i]=  2      6    0
j,i=0,1, txvertex[j,i]=  3      7    1
j,i=1,1, txvertex[j,i]=  3      8    1
j,i=2,1, txvertex[j,i]=  3      9    1

graph=                  B1:2:3:i M3:6:i:j B4:5:6:i Q1:4:i Q2:5:i
vertex_contract_order=  3        4        0        1      2
trace of active vertices:
 iteration=0:                             B4:5:6:i Q1:4:i        -> B1:5:6:i
 iteration=1:                             B1:5:6:i        Q2:5:i -> B1:2:6:i
 iteration=2:           B1:2:3:i          B1:2:6:i               -> B3:6:i
 iteration=3:                    M3:6:i:j B3:6:i                 -> corr_fun:i:j

*/

// Include this header before the rest to make sure that superbblas is loaded properly
#include "datalib/superb/types.h"
#include "datalib/superb/superb_tensor.h"

// Include the rest
#include "datalib/graph/hadron_adj_map.h"
#include "datalib/graph/hadron_diagram_time.h"

#include "datalib/graph/process_corrs.h"
#include "datalib/graph/single_hadron_coeffs.h"
#include "datalib/superb/twoquark_gamma_util.h"
#include "datalib/superb/twoquark_operators_util.h"
#include "datalib/superb/baryon_superb.h"
#include "datalib/superb/genprop4.h"
#include "datalib/superb/meson_superb.h"
#include "datalib/superb/prop_superb.h"
#include "datalib/superb/superb_aux.h"

#include <array>
#include <functional>
#include <limits>
#include <map>
#include <numeric>
#include "datalib/superb/unsmeared_twoquark_disco.h"
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef _OPENMP
#  include <omp.h>
#endif

namespace Hadron
{
  /// Callback for retrieving distillation objects
  /// \param keys: list of keys
  /// \param perms: list of permutations, one for each key
  /// \param do_conj: list of whether to conjugate the elemental, one for each key
  /// \param ev_from: first eigenvector to return
  /// \param ev_size: number of eigenvectors to return
  /// \param dist_labels: dimensions to be distributed

  template <typename T>
  using distillation_object_callback =
    std::function<SBN::Tensor(const std::vector<T>& keys, const std::vector<SBN::Coor>& perms,
			      const std::vector<bool>& do_conj, const SBN::Coor& ev_from,
			      const SBN::Coor& ev_size, const std::string& dist_labels,
			      const SBN::Tensor& perm)>;

  /// Callback for retrieving meson elementals
  /// \param keys: list of mesons keys
  /// \param perms: list of permutations, one for each meson key
  /// \param do_conj: list of whether to conjugate the elemental, one for each key
  /// \param ev_from: first eigenvector to return
  /// \param ev_size: number of eigenvectors to return
  /// \param dist_labels: dimensions to be distributed, some of "vwi"

  using meson_elementals_callback =
    distillation_object_callback<Hadron::KeyMesonElementalOperator_t>;

  /// Callback for retrieving baryon elementals
  /// \param keys: list of baryon keys
  /// \param perms: list of permutations, one for each baryon key
  /// \param do_conj: list of whether to conjugate the elemental, one for each key
  /// \param ev_from: first eigenvector to return
  /// \param ev_size: number of eigenvectors to return
  /// \param dist_labels: dimensions to be distributed, some of "vwxi"

  using baryon_elementals_callback =
    distillation_object_callback<Hadron::KeyBaryonElementalOperator_t>;

  /// Callback for retrieving prop elementals
  /// \param keys: list of prop keys
  /// \param perms: list of permutations, one for each prop key
  /// \param do_conj: list of whether to conjugate the elemental, one for each key
  /// \param ev_from: first eigenvector to return
  /// \param ev_size: number of eigenvectors to return
  /// \param dist_labels: dimensions to be distributed, some of "vwrsi"

  using prop_elementals_callback =
    distillation_object_callback<Hadron::KeyProp4ElementalOperator_t>;

  /// Callback for retrieving genprop elementals
  /// \param keys: list of genprop keys
  /// \param perms: list of permutations, one for each genprop key
  /// \param do_conj: list of whether to conjugate the elemental, one for each key
  /// \param ev_from: first eigenvector to return
  /// \param ev_size: number of eigenvectors to return
  /// \param dist_labels: dimensions to be distributed, some of "vwrsi"

  using genprop_elementals_callback =
    distillation_object_callback<Hadron::KeyGenProp4ElementalOperator_t>;

  /// Read one complete 16-gamma disconnected block. Called on every rank;
  /// implementations must be local/noncollective and throw on missing data.
  using disco_elementals_callback = std::function<Hadron::ValTwoQuarkDiscoOperator_t(
    const Hadron::KeyTwoQuarkDiscoOperator_t&)>;

  /// Flavor to mass label map
  using FlavorToMassMap = std::map<char, std::string>;

  namespace detail
  {
    /// Auxiliary class for computing the hash for `unordered_set` and `unordered_map`
    template <typename K>
    struct superb_hash {
      std::size_t operator()(const K& k) const
      {
	return SB::getHash(k);
      }
    };

    /// std::unordered_map with hashing from ADAT's SB
    template <typename K, typename V>
    using unordered_map = std::unordered_map<K, V, superb_hash<K>>;

    /// std::unordered_map with hashing from ADAT's SB
    template <typename K, typename V>
    using unordered_multimap = std::unordered_multimap<K, V, superb_hash<K>>;

    /// Return the number of threads in the current context

    inline int get_num_threads()
    {
#ifdef _OPENMP
      return omp_get_num_threads();
#else
      return 1;
#endif
    }

    /// Return the thread index in the current context

    inline int get_thread_num()
    {
#ifdef _OPENMP
      return omp_get_thread_num();
#else
      return 0;
#endif
    }

    /// Return the values given from each thread into a vector
    /// \param t: given value
    /// \return pointer to vector with all values

    template <typename T>
    std::vector<T>* gather_among_threads(const T& t)
    {
#ifdef _OPENMP
      std::vector<T>* all_t_ptr = nullptr;
      const auto num_threads = get_num_threads();
#  pragma omp single copyprivate(all_t_ptr)
      all_t_ptr = new std::vector<T>(num_threads);
      (*all_t_ptr).at(get_thread_num()) = t;
#  pragma omp barrier
      return all_t_ptr;
#else
      return new std::vector<T>{t};
#endif
    }

    /// Move the values given from each thread into a vector
    /// \param t: given value (may be modified afterwards)
    /// \param values: vector with all values

    template <typename T>
    void gather_among_threads(T& t, std::vector<T>* values)
    {
#ifdef _OPENMP
      const auto num_threads = get_num_threads();
#  pragma omp single
      values->resize(num_threads);
      std::swap(t, values->at(get_thread_num()));
#  pragma omp barrier
#else
      values->resize(0);
      values->push_back(std::move(t));
#endif
    }

    /// Reduction of the values given from each thread
    /// \param t_th: given value (may be modified afterwards)
    /// \param t_final: pointer to store the final result
    /// \param f: f(a, b) combines the values of `a` and `b` and stores the result in `a`

    template <typename T, typename func_merge>
    void reduce_among_threads(T& t_th, T* t_final, func_merge f)
    {
#ifdef _OPENMP
      const auto& all_t_ptr = gather_among_threads(&t_th);
      const auto& all_t = *all_t_ptr;
      const auto thread_num = get_thread_num();
      if (get_num_threads() != all_t.size())
	throw std::runtime_error("wtf");
      // Find the largest power of two smaller than the number of threads
      int stride = 1;
      for (; stride < all_t.size(); stride *= 2)
	;
      stride /= 2;
      for (auto n = all_t.size(); stride > 0; n = stride, stride /= 2)
      {
	if (thread_num + stride < n)
	  f(*all_t.at(thread_num), *all_t.at(thread_num + stride));
#  pragma omp barrier
      }
      if (thread_num == 0)
      {
	std::swap(*all_t.at(0), *t_final);
	delete all_t_ptr;
      }
#  pragma omp barrier
#else
      (void)f;
      return std::swap(t_th, *t_final);
#endif
    }

    /// Return the maximum value given from each thread
    /// \param t: given value
    /// \param result: pointer to store the result

    template <typename T>
    void max_among_threads(const T& t, T* result)
    {
      auto copy_of_t = t;
      reduce_among_threads(copy_of_t, result, [](T& a, const T& b) { a = std::max(a, b); });
    }

    /// Combine all the maps given from each thread discarding repeated elements
    /// \param m: given map (may be modified afterwards)
    /// \param result: pointer to store the final result

    template <typename K, typename V>
    void merge_among_threads(unordered_map<K, V>& m, unordered_map<K, V>* result)
    {
      reduce_among_threads(m, result,
			   [](unordered_map<K, V>& a, unordered_map<K, V>& b) { a.merge(b); });
    }

    /// Execute the given function among the local elements sharing the work among threads
    /// \param t: input tensor
    /// \param fun: function executed in a parallel environment that returns a pair of function (iter, end);
    ///        - iter(coor, value) is execute for each value of the given tensor
    ///        - end() executed in the parallel environment after all iterations.

    template <typename Fun>
    void for_each_coor_value_threaded(const SBN::Tensor& t, const Fun& fun)
    {
      if (!t.ptr)
	throw std::runtime_error("for_each_coor_value: unsupported on example tensors");

      const auto& t_local = SBN::get_local_tensor(t);
      const auto& t0_local =
	SBN::make_sure(t_local, {}, SBN::Options::Distribution::Default, SBN::Options::Alloc::Host,
		       SBN::Options::TightView::Default, 0);

      const auto& [from_, size_] =
	SBN::get_local_range(t0_local, SBN::Options::ComplainIfIsntTightView::False);
      const auto size = size_; /// Hack to deal with amdclang++
      const auto from = from_; /// Hack to deal with amdclang++
      const SBN::value_type* ptr = SBN::data(t0_local);
      SBN::IndexType vol = SBN::volume(size);
#ifdef _OPENMP
#  pragma omp parallel
#endif
      {
	auto [iter_, end_] = fun();
	auto iter = iter_; /// Hack to deal with amdclang++
	auto end = end_;   /// Hack to deal with amdclang++
#ifdef _OPENMP
#  pragma omp for schedule(static)
#endif
	for (SBN::IndexType i = 0; i < vol; ++i)
	{
	  using SBN::operator+;
	  using SBN::operator-;
	  const auto& absolute_coor =
	    SBN::norm_coor(SBN::indexToCoor(i, size) + from, t0_local.dim);
	  const auto& relative_coor = SBN::norm_coor(absolute_coor - t0_local.from, t0_local.dim);
	  if (SBN::is_less_than(relative_coor, t0_local.size))
	    iter(relative_coor, ptr[i]);
	}
	end();
      }
    }

    /// Execute the given function passing the coordinate and the value for the local elements
    /// Execute the given function for each value sharing the work among threads
    /// \param values: input vector of values
    /// \param fun: function to execute on each value; signature: fun(value)

    template <typename V, typename Fun>
    void for_each_threaded(const std::vector<V>& values, const Fun& fun)
    {
      const auto size = values.size();
#ifdef _OPENMP
#  pragma omp parallel for schedule(static)
#endif
      for (std::size_t i = 0; i < size; ++i)
	fun(values.at(i));
    }

    /// Fill a map among threads
    /// \param n: number of elements to generate
    /// \param estimated_size: estimated number of unique elements
    /// \param fun: function to generate the i-th element; signature fun(i) -> {key, value}

    template <typename Map, typename Fun>
    Map fill_map_among_threads(std::size_t n, std::size_t estimated_size, Fun fun)
    {
      Map r;
#ifdef _OPENMP
#  pragma omp parallel
#endif
      {
	Map m(estimated_size / (get_thread_num() == 0 ? 1 : get_num_threads()));
#pragma omp for schedule(static)
	for (std::size_t i = 0; i < n; ++i)
	{
	  m.insert(fun(i));
	}
	merge_among_threads(m, &r);
      }
      return r;
    }

    /// Execute a function on each element of the given map among threads
    /// \param m: given map
    /// \param fun: fun(index, map_item), where `index` is an unique numeric index for `map_item`

    template <typename Map, typename Fun>
    void for_each_enumerated_threaded(Map& m, Fun fun)
    {
      std::vector<std::size_t> first_item;
#ifdef _OPENMP
#  pragma omp parallel
#endif
      {
	const std::size_t num_threads = get_num_threads();
	const std::size_t thread_num = get_thread_num();
	const auto local_buckets =
	  m.bucket_count() / num_threads + (thread_num < m.bucket_count() % num_threads ? 1 : 0);
	const auto first_bucket = m.bucket_count() / num_threads * thread_num +
				  std::min(thread_num, m.bucket_count() % num_threads);
	std::size_t local_items = 0;
	for (std::size_t i = 0; i < local_buckets; ++i)
	  local_items += m.bucket_size(first_bucket + i);
	gather_among_threads(local_items, &first_item);
	local_items = 0;
#pragma omp single
	{
	  for (std::size_t i = 0, n = 0; i < first_item.size(); ++i)
	  {
	    const auto next_n = n + first_item.at(i);
	    first_item.at(i) = n;
	    n = next_n;
	  }
	}
	for (std::size_t i = 0, item = first_item.at(get_thread_num()); i < local_buckets; ++i)
	  for (auto it = m.begin(first_bucket + i); it != m.end(first_bucket + i); ++it, ++item)
	    fun(item, *it);
      }
    }

    inline void thread_tests()
    {
      // Get the number of threads
      int num_threads = 1;
#ifdef _OPENMP
#  pragma omp parallel
      num_threads = get_num_threads();
#endif

      // Create a vector from 0 to the number of threads
      std::vector<int> v;
      for (int i = 0; i < num_threads; ++i)
	v.push_back(i);
      const auto copy_of_v = v;

      for_each_threaded(v, [=](const auto vi) {
	auto v_ptr = gather_among_threads(vi);
	if (*v_ptr != copy_of_v)
	  throw std::runtime_error("test failed");
#ifdef _OPENMP
#  pragma omp barrier
#  pragma omp single
#endif
	delete v_ptr;
      });

      {
	std::vector<int> r;
	for_each_threaded(v, [=, &r](auto vi) {
	  auto copy_of_vi = vi;
	  gather_among_threads(copy_of_vi, &r);
	});
	if (r != copy_of_v)
	  throw std::runtime_error("test failed");
      }

      {
	int r;
	for_each_threaded(v, [=, &r](auto vi) {
	  auto copy_of_vi = vi;
	  reduce_among_threads(copy_of_vi, &r, [](int& a, int b) { a += b; });
	});
	if (std::accumulate(v.begin(), v.end(), 0) != r)
	  throw std::runtime_error("test failed");
      }

      {
	int r;
	for_each_threaded(v, [=, &r](auto vi) {
	  auto copy_of_vi = vi;
	  max_among_threads(copy_of_vi, &r);
	});
	if (v.back() != r)
	  throw std::runtime_error("test failed");
      }

      {
	unordered_map<int, int> r;
	for_each_threaded(v, [=, &r](auto vi) {
	  unordered_map<int, int> m{{vi, vi}};
	  merge_among_threads(m, &r);
	});
	if (r.size() != num_threads)
	  throw std::runtime_error("test failed");
	for (const auto& [k, v] : r)
	  if (k < 0 || k >= num_threads || k != v)
	    throw std::runtime_error("test failed");
      }
    }

    /// Return whether a given tensor has some dimensions and the same communicator
    /// \param sc: dimensions to test
    /// \param comm: communicator to test
    /// \param t: tensor to test

    inline bool is_compatible(const SBN::SCoor& sc, const SBN::Comm& comm, const SBN::Tensor& t)
    {
      if (comm.ranks != SBN::get_comm(t).ranks)
	return false;
      bool vol_sc_is_zero = false;
      for (const auto [_, s] : sc)
	if (s == 0)
	  vol_sc_is_zero = true;
      if (vol_sc_is_zero)
	return (SBN::volume(t.size) == 0);
      for (const auto [c, s] : SBN::get_scoor(t.order, t.size))
	if ((sc.count(c) == 0 && s != 1) || (sc.count(c) == 1 && sc.at(c) != s))
	  return false;
      return true;
    }

    /// Return whether a given tensor has some dimensions and the same communicator
    /// \param sc: dimensions to test
    /// \param comm: communicator to test
    /// \param t: tensor to test

    inline SBN::Tensor make_compatible(const SBN::SCoor& sc, const SBN::Tensor& t)
    {
      bool vol_sc_is_zero = false;
      for (const auto [_, s] : sc)
	if (s == 0)
	  vol_sc_is_zero = true;
      if (vol_sc_is_zero)
      {
	if (SBN::volume(t.size) != 0)
	  throw std::runtime_error("make_compatible: incompatible input tensor");
	auto st = SBN::get_scoor(t.order, t.size);
	for (const auto& [c, s] : sc)
	  if (st.count(c) == 1)
	    st.at(c) = s;
	return SBN::like_this(t, {}, SBN::get_coor(t.order, st));
      }
      for (const auto [c, s] : SBN::get_scoor(t.order, t.size))
	if ((sc.count(c) == 0 && s != 1) || (sc.count(c) == 1 && sc.at(c) != s))
	  throw std::runtime_error("make_compatible: incompatible input tensor");
      return t;
    }

    inline const std::string tensor_labels = "vwxrstI";

    using CoeffMap = const Datalib::Model::MapObject<Hadron::HadronDispVertex_t,
					   Hadron::MapSingleHadronQuarkFlavorColorSpin_t>;
    struct Context {
      const CoeffMap& coeff_map;
      prop_elementals_callback prop_callback;
      baryon_elementals_callback baryon_callback;
      meson_elementals_callback meson_callback;
      genprop_elementals_callback genprop_callback;
      FlavorToMassMap flavor_to_mass;
      int num_vecs;
      Array<int> lattice_size;
    };

    using CorrFun = std::vector<std::complex<double>>;

    struct GraphVertex {
      char type;		     ///< vertex type, either B,M,Q,A (smeared) or m,a (unsmeared)
      bool time_moving;		     ///< whether to expect more time slices for this vertex
      std::vector<int> slot_indices; ///< absolute slot index
    };

    /// Return the GraphVertex given the string representation
    /// \param graph: graph representation, eg, B1:2:3:B4:5:6:Q1:4:Q2:6:Q3:5:
    /// \param smearing: a nonzero value for smeared vertices
    /// \param is_time_moving: whether to expect more time slices for this vertex

    inline std::vector<GraphVertex> get_graph_vertex(const std::string& graph,
						     const std::vector<char>& smearing,
						     const std::vector<char>& is_time_moving)
    {
      std::vector<GraphVertex> r;
      const std::map<char, int> num_quarks{{'B', 3}, {'M', 2}, {'Q', 2}, {'A', 2}};
      const std::map<char, char> to_unsmeared{{'M', 'm'}, {'A', 'a'}};
      auto it = graph.begin();
      while (it != graph.end())
      {
	GraphVertex v;
	// Make sure that the letter is valid
	if (num_quarks.count(*it) == 0)
	{
	  throw std::runtime_error("get_graph_vertex: invalid letter");
	}
	v.type = *it;
	++it;
	// Parse the quark indices
	for (int i = 0, n = num_quarks.at(v.type); i < n; ++i)
	{
	  auto e = std::find(it, graph.end(), ':');
	  if (e == graph.end())
	  {
	    throw std::runtime_error("get_graph_vertex: broken graph");
	  }
	  int slot = std::stoi(std::string(it, e));
	  if (slot <= 0)
	  {
	    throw std::runtime_error("get_graph_vertex: broken graph");
	  }
	  // FIXME: The slot number is used as label, so make sure that these labels
	  // don't collide with other labels along the code like, s,S,v,w,x,$
	  if (slot >= ' ' /* 32 */)
	  {
	    throw std::runtime_error("get_graph_vertex: fix this");
	  }
	  v.slot_indices.push_back(slot);
	  it = e + 1;
	}
	r.push_back(v);
      }

      if (r.size() < smearing.size() || r.size() < is_time_moving.size())
	throw std::runtime_error("wtf");

      for (int i = 0; i < r.size(); i++)
      {
	if (i < smearing.size() && smearing.at(i) == 0)
	  r.at(i).type = to_unsmeared.at(r.at(i).type);
	r.at(i).time_moving = (i < is_time_moving.size() && is_time_moving.at(i) != 0);
      }

      return r;
    }

    /// Return a string representation of a graph
    /// \param graph: vertices and slot numbers

    inline std::string graph_to_string(const std::vector<GraphVertex>& graph)
    {
      std::string r;
      for (const auto& it : graph)
      {
	r.push_back(it.type);
	if (it.time_moving)
	  r.push_back('*');
	for (int slot : it.slot_indices)
	  r = r + std::string{':'} + std::to_string(slot);
	r.push_back(' ');
      }
      return r;
    }

    /// Return a map from slot index to vertex and slot number
    /// \param graph: vertices and slot numbers
    /// \param may_ignore_vertex: ignore a particular vertex if other vertices have the same slots indices

    inline std::map<int, std::array<int, 2>>
    get_map_from_slot_index_to_vertex_and_slot(const std::vector<GraphVertex>& graph,
					       int may_ignore_vertex = -1)
    {
      // Fill the adjacency map based on the slot indices
      std::map<int, std::array<int, 2>> r;
      for (int i = 0; i < graph.size(); ++i)
	if (graph.at(i).type != 'Q' && may_ignore_vertex != i)
	  for (int j = 0; j < graph.at(i).slot_indices.size(); ++j)
	    r[graph.at(i).slot_indices.at(j)] = {i, j};
      if (may_ignore_vertex >= 0)
	for (int j = 0; j < graph.at(may_ignore_vertex).slot_indices.size(); ++j)
	  if (r.count(graph.at(may_ignore_vertex).slot_indices.at(j)) == 0)
	    r[graph.at(may_ignore_vertex).slot_indices.at(j)] = {may_ignore_vertex, j};
      return r;
    }

    /// Return a new vertex graph after removing the fake props connecting genprops
    /// \param graph: vertices and slot numbers
    /// \return {graph, related_prop}:
    /// - graph: new graph after removing some props
    /// - related_prop: returns the source and sink prop vertex indices with the flavor and conj info
    /// Example:
    /// - input graph:  M:1:2 m:3:4 Q:1:3 Q:2:4
    /// - output graph: M:1:2 m:1:2 Q:    Q:   , and related_prop[1]={3,2}

    inline std::tuple<std::vector<GraphVertex>, std::map<int, std::array<int, 2>>>
    remove_fake_connections(const std::vector<GraphVertex>& graph)
    {
      std::vector<GraphVertex> new_graph = graph;
      std::map<int, std::array<int, 2>> related_prop;
      for (int vertex_index = 0; vertex_index < new_graph.size(); ++vertex_index)
      {
	const char type = new_graph.at(vertex_index).type;
	if (type != 'a' && type != 'm')
	  continue;
	auto& slot_indices = graph.at(vertex_index).slot_indices;
	std::vector<int> new_slot_indices(slot_indices.size());
	// Invalidate all props with those slots
	for (int j = 0; j < new_graph.size(); ++j)
	{
	  auto& it = new_graph.at(j);
	  if (it.type != 'Q')
	    continue;
	  int found = -1;
	  for (int k = 0; k < slot_indices.size(); ++k)
	  {
	    const int slot_index = slot_indices.at(k);
	    if (std::find(it.slot_indices.begin(), it.slot_indices.end(), slot_index) !=
		it.slot_indices.end())
	    {
	      found = k;
	      new_slot_indices.at(k) =
		it.slot_indices.at(it.slot_indices.at(0) != slot_index ? 0 : 1);
	      break;
	    }
	  }
	  if (found >= 0)
	  {
	    it.slot_indices.resize(0);
	    if (found == 1)
	    {
	      related_prop[vertex_index].at(0) = j;
	      // Assuming that source the unsmeared vertex coincides with the sink of removed prop
	      if (slot_indices.at(1) != graph.at(j).slot_indices.at(0))
		throw std::runtime_error("wtf");
	    }
	    else
	    {
	      related_prop[vertex_index].at(1) = j;
	      // Assuming that source the unsmeared vertex coincides with the sink of removed prop
	      if (slot_indices.at(0) != graph.at(j).slot_indices.at(1))
		throw std::runtime_error("wtf");
	    }
	  }
	}

	// Set the slot indices
	new_graph.at(vertex_index).slot_indices = new_slot_indices;
      }

      return {new_graph, related_prop};
    }

    /// The Turing-like Tensor Contraction Engine (3TCE) description:
    /// a) The "tape" is a list of cells, each containing a single tensor.
    /// b) The program is a list of two pair cell indices indicating the tensors to
    ///    contract; the resulting tensor is stored on the cell with index indicated
    ///    by the "counter".
    /// c) After the execution of each instruction, the "counter" is incremented.
    /// d) The "tape" is initialized with the tensors associated to the graph
    ///    vertices.
    /// e) The "counter" is initialized and the first free cell after the graph
    ///    vertices.

    /// Single tensor contraction instruction for 3TCE
    struct ContractItem {
      // Graph vertex indices of tensors to contract
      std::vector<int> first_tensor_indices;  ///< first tensor indices to contract
      std::vector<int> second_tensor_indices; ///< second tensor indices to contract

      // Contraction instructions:
      //     contract(slice_kv(relabel(t0, first_tensor_relabel), first_tensor_slice)
      //              slice_kv(relabel(t1, second_tensor_relabel), second_tensor_slice)
      //              result_tensor_order)
      SBN::remap first_tensor_relabel;
      SBN::SCoor first_tensor_slice_from;
      SBN::SCoor first_tensor_slice_size;
      SBN::remap second_tensor_relabel;
      SBN::SCoor second_tensor_slice_from;
      SBN::SCoor second_tensor_slice_size;

      // Expected labels of the input and output tensors (just for tracking the
      // contraction program, they are not needed)
      std::string first_tensor_order;
      std::string second_tensor_order;
      std::string result_tensor_order;
    };

    /// Contraction program: a list of contraction instructions
    using ContractionPlan = std::vector<ContractItem>;

    /// Most the nucleon operators only use the 0-1 spin components. So we split the baryons
    /// in up to 8 parts.

    constexpr auto BARYON_SPIN_BLOCK = Datalib::TensorStorage::spin_count / 2;

    /// Get a simple contraction plan following these steps:
    /// 1. Select the rightmost vertex with more slots.
    /// 2. From the current vertex, select another vertex for which the contraction
    ///    results in the smallest number of dimensions.
    /// 3. Concatenate the tensor labels removing the common labels and append the
    ///    new tensor to the end of the "tape"
    ///
    /// \param graph: vertices and slot numbers
    /// \param num_unique_vertices: (optional) number of unique vertices for each vertex
    /// \return tuple{plan, slides, spin_size, color_to_spin}, where
    /// - plan: contraction plan
    /// - slides: distillation basis dimension that can be parallelize
    /// - spin_size: dimension of the spins (2 for the baryons and 4 for the rest)
    /// - color_to_spin: spin label associated to the color label

    inline std::tuple<ContractionPlan, std::vector<int>, std::map<char, int>, std::map<int, int>>
    get_contraction_plan(const std::vector<GraphVertex>& graph,
			 const std::vector<std::size_t>& num_unique_vertices = {})
    {
      // Check input
      if (num_unique_vertices.size() > 0 && graph.size() != num_unique_vertices.size())
	throw std::runtime_error("get_simple_contraction_plan: invalid input");

      // Trivial case
      if (graph.size() == 0)
	return {};

      // Look for the last B or M vertex with more slots
      int vertex_index = -1;
      bool is_all_mesons = true;
      for (int i = 0; i < graph.size(); ++i)
      {
	if (graph.at(i).slot_indices.size() != 2)
	  is_all_mesons = false;
	if (graph.at(i).type == 'Q')
	  continue;
	if (vertex_index < 0 ||
	    graph.at(i).slot_indices.size() > graph.at(vertex_index).slot_indices.size() ||
	    (graph.at(i).slot_indices.size() == graph.at(vertex_index).slot_indices.size() &&
	     ((num_unique_vertices.size() == 0 && !graph.at(i).time_moving) ||
	      (num_unique_vertices.size() > 0 &&
	       num_unique_vertices.at(i) <= num_unique_vertices.at(vertex_index)))))
	{
	  vertex_index = i;
	}
      }

      // Count the number of vertices with time information
      int num_vertices = 0;
      for (const auto& it : graph)
      {
	if (it.type == 'Q')
	  break;
	num_vertices++;
      }
      if (vertex_index >= num_vertices)
	throw std::runtime_error("wtf");

      // Put as potential slices all slots with in baryons
      std::set<int> slides;
      for (const auto& it : graph)
	if (it.slot_indices.size() > 2)
	  for (int slot : it.slot_indices)
	    slides.insert(slot);

      // Associate a spin label for each label
      std::set<int> used_labels;
      for (const auto& it : graph)
	for (const auto& slot : it.slot_indices)
	  if (slot >= 0)
	    used_labels.insert(slot);
      std::map<int, int> spin_label;
      const std::set<int> color_labels = used_labels;
      for (const auto& color_label : color_labels)
      {
	bool label_set = false;
	for (int i = 1; i < int(' '); ++i)
	{
	  if (used_labels.count(i) == 0)
	  {
	    spin_label[color_label] = i;
	    used_labels.insert(i);
	    label_set = true;
	    break;
	  }
	}
	if (!label_set)
	  throw std::runtime_error("wtf: increase the number of labels");
      }

      // Generate the labels for the graph vertices 3D distillation basis for
      // baryons (vwxrst), and 2D for the rest (vwrs)
      std::vector<std::string> labels;
      labels.reserve(graph.size() * 2);
      for (const auto& it : graph)
      {
	labels.push_back(it.type == 'B' ? "vwxrst" : "vwrs");
      }

      // Get a chain of non-prop prop vertices
      std::vector<int> chain;
      if (is_all_mesons)
      {
	for (int k = 0, next_vertex_index = 0; k < graph.size(); ++k)
	{
	  // Check the non-prop/prop even/odd property
	  if ((k % 2 == 0) != (graph.at(next_vertex_index).type != 'Q'))
	  {
	    is_all_mesons = false;
	    chain.clear();
	    break;
	  }

	  // Select slot to contract
	  const int slot_to_contract = graph.at(next_vertex_index).slot_indices.at(1);
	  chain.push_back(next_vertex_index);
	  bool found = false;
	  for (int i = 0; i < graph.size(); ++i)
	  {
	    if (graph.at(i).slot_indices.at(0) == slot_to_contract)
	    {
	      next_vertex_index = i;
	      found = true;
	      break;
	    }
	  }
	  if (!found)
	  {
	    is_all_mesons = false;
	    chain.clear();
	    break;
	  }
	}
	if (is_all_mesons)
	  vertex_index = chain.at(0);
      }

      // Track if the vertices have been contracted already
      std::vector<char> contracted(graph.size() * 2);

      // Start building the plan
      ContractionPlan r;
      r.reserve(graph.size());
      std::vector<GraphVertex> new_graph = graph;
      // NOTE: we need n-1 contractions to contract n tensors, two at a time
      for (std::size_t k = 0; k < graph.size() - 1; ++k)
      {
	// Look for the immediate vertex that the contraction will result in less
	// amount of total labels.
	std::vector<int> new_best_slots;
	int new_best_index = -1;
	// NOTE: Look into the original vertices, we don't need to look into the new
	// ones
	for (int i = 0; i < graph.size(); ++i)
	{
	  if (i == vertex_index || contracted.at(i) != 0 || graph.at(i).slot_indices.size() == 0)
	    continue;
	  if (is_all_mesons && i != chain.at(k + 1))
	    continue;

	  std::vector<int> new_slots = new_graph.at(i).slot_indices;
	  for (int slot : new_graph.at(vertex_index).slot_indices)
	  {
	    auto it = std::find(new_slots.begin(), new_slots.end(), slot);
	    if (it == new_slots.end())
	      new_slots.push_back(slot);
	    else
	      new_slots.erase(it);
	  }
	  if (new_best_index == -1 || new_best_slots.size() > new_slots.size() ||
	      // Avoid contracting with moving vertices
	      (new_best_slots.size() == new_slots.size() &&
	       ((num_unique_vertices.size() == 0 && !graph.at(i).time_moving) ||
		(num_unique_vertices.size() > 0 &&
		 num_unique_vertices.at(i) < num_unique_vertices.at(new_best_index)))))
	  {
	    new_best_index = i;
	    new_best_slots = new_slots;
	  }
	}
	if (new_best_index < 0)
	  break;
	contracted.at(vertex_index) = contracted.at(new_best_index) = 1;
	r.push_back(ContractItem{{new_best_index}, {vertex_index}});
	new_graph.push_back(
	  GraphVertex{0 /* dummy type */, false /* no moving vertex */, new_best_slots});

	const int first_tensor_index = new_best_index;
	const int second_tensor_index = vertex_index;
	const auto& labels0 =
	  (r.back().first_tensor_order = labels.at(first_tensor_index) + std::string{'I'});
	const auto& labels1 =
	  (r.back().second_tensor_order = labels.at(second_tensor_index) + std::string{'I'});
	const auto& slots0 = new_graph.at(first_tensor_index).slot_indices;
	const auto& slots1 = new_graph.at(second_tensor_index).slot_indices;
	auto& first_remap = r.back().first_tensor_relabel;
	for (int slot = 0; slot < slots0.size(); ++slot)
	{
	  first_remap[labels0.at(slot)] = slots0.at(slot);
	  first_remap[labels0.at(slots0.size() + slot)] = spin_label.at(slots0.at(slot));
	}
	if (second_tensor_index < graph.size())
	{
	  auto& second_remap = r.back().second_tensor_relabel;
	  for (int slot = 0; slot < slots1.size(); ++slot)
	  {
	    second_remap[labels1.at(slot)] = slots1.at(slot);
	    second_remap[labels1.at(slots1.size() + slot)] = spin_label.at(slots1.at(slot));
	  }
	}
	std::string& new_labels = r.back().result_tensor_order;
	for (int slot : new_best_slots)
	{
	  new_labels.push_back((char)slot);
	  new_labels.push_back((char)spin_label.at(slot));
	}
	labels.push_back(new_labels);
	new_labels.push_back('I');

	// Test the slicing labels
	if (slots0.size() > 2 || slots1.size() > 2)
	{
	  for (int slot : slots1)
	  {
	    if (std::find(slots0.begin(), slots0.end(), slot) != slots0.end())
	      slides.erase(slot);
	  }
	}

	vertex_index = new_graph.size() - 1;
      }

      // If all slots have been contracted, the last tensor should have no slots
      if (new_graph.back().slot_indices.size() > 0)
	throw std::runtime_error("invalid graph");

      // Get the spin dimensions
      std::map<char, int> spin_dims;
      for (const auto& it : graph)
      {
	if (it.type == 'B')
	  for (const auto& slot : it.slot_indices)
	    spin_dims[spin_label.at(slot)] = BARYON_SPIN_BLOCK;
      }
      for (const auto& it : graph)
      {
	if (it.type != 'B')
	  for (const auto& slot : it.slot_indices)
	    if (spin_dims.count(spin_label.at(slot)) == 0)
	      spin_dims[spin_label.at(slot)] = Datalib::TensorStorage::spin_count;
      }

      return {r, std::vector<int>(slides.begin(), slides.end()), spin_dims, spin_label};
    }

    /// Struct with a member of each kind of distillation object

    template <typename T>
    struct Dist {
      T meson;
      T baryon;
      T prop;
      T genprop;

      template <typename std::enable_if<std::is_default_constructible<T>::value, bool>::type = true>
      Dist()
      {
      }

      Dist(const T& t) : meson(t), baryon(t), prop(t), genprop(t)
      {
      }

      Dist(const T& meson, const T& baryon, const T& prop, const T& genprop)
	: meson(meson), baryon(baryon), prop(prop), genprop(genprop)
      {
      }

      T& get(char type)
      {
	switch (type)
	{
	case 'M':
	case 'A': return meson;
	case 'B': return baryon;
	case 'Q': return prop;
	case 'm':
	case 'a': return genprop;
	default: throw std::runtime_error("wtf");
	}
      }

      const T& get(char type) const
      {
	switch (type)
	{
	case 'M':
	case 'A': return meson;
	case 'B': return baryon;
	case 'Q': return prop;
	case 'm':
	case 'a': return genprop;
	default: throw std::runtime_error("wtf");
	}
      }
    };

    /// Return the maximum number of distillation objects if taken batches of a certain size
    /// \param graph: vertices and slot numbers
    /// \param v: v[i][j] index for vertex j on corr fun i which is unique among all v[*][j]
    /// \param perm: arrange of the corr funs indices into a tensor
    /// \param max_corr_fun_on_batch: maximum number of corr fun in a batch

    inline Dist<std::size_t> get_unique_dists(const std::vector<GraphVertex>& graph,
					      const std::vector<std::vector<SBN::IndexType>>& v,
					      const SBN::Tensor& perm,
					      std::size_t max_corr_fun_on_batch)
    {
      Dist<std::size_t> n(0ul);
      auto b = Dist<std::unordered_set<std::size_t>>(
	std::unordered_set<std::size_t>(max_corr_fun_on_batch));
      std::size_t i0 = 0, id = 0;
      std::size_t in = std::min(max_corr_fun_on_batch, SBN::volume(perm.size));
      SBN::for_each_coor_value(perm, [&](const SBN::Coor&, const SBN::value_type& i_cmplx) {
	const SBN::IndexType i = std::real(i_cmplx);

	if (i < v.size())
	{
	  for (std::size_t j = 0; j < graph.size(); ++j)
	    b.get(graph.at(j).type).insert(v.at(i).at(j));
	}
	++id;

	// Record number of distinct distillation objects at the end of the batch
	if (id >= in)
	{
	  n.meson = std::max(n.meson, b.meson.size());
	  n.baryon = std::max(n.baryon, b.baryon.size());
	  n.prop = std::max(n.prop, b.prop.size());
	  n.genprop = std::max(n.genprop, b.genprop.size());
	  b.meson.clear();
	  b.baryon.clear();
	  b.prop.clear();
	  b.genprop.clear();
	  i0 += id;
	  in = std::min(SBN::volume(perm.size) - i0, max_corr_fun_on_batch);
	  id = 0;
	}
      });

      return n;
    }

    /// Return up to how many steps are using cache

    inline unsigned int get_num_steps_with_cache(const std::vector<GraphVertex>& graph,
						 const ContractionPlan& plan)
    {
      unsigned int current_step = 0;
      unsigned int first_step_without_cache = 0;
      for (const auto& step : plan)
      {
	for (const auto& indices :
	     std::array<std::vector<int>, 2>{step.first_tensor_indices, step.second_tensor_indices})
	{
	  if (indices.size() <= 1 || indices.at(0) >= graph.size())
	    continue;
	  first_step_without_cache = current_step + 1;
	}
	++current_step;
      }

      return first_step_without_cache;
    }

    /// From a list of vertex indices to the suborders of `perm`
    using VertexIndices = std::map<std::vector<int>, SBN::Order>;

    /// Return the maximum number of correlation functions that can be computed as once
    /// \param graph: vertices and slot numbers
    /// \param plan: contraction plan
    /// \param color_to_spin: spin label associated to the color label
    /// \param spin_size: dimension of the spins
    /// \param slides: slot indices that can slides
    /// \param num_vecs: number of distillation vectors
    /// \param max_mem: maximum memory use for contractions
    /// \param vertices_indices: corr fun indices for each vertex
    /// \param v: v[i][j] index for vertex j on corr fun i which is unique among all v[*][j]
    /// \param num_unique_vertices: number of unique vertices for each vertex
    /// \param num_unique_dist: number of unique distillation elements
    /// \param factor_mem_build: number of linear combinations of spins on average to construct an element
    /// \param perm: arrange of the corr funs indices into a tensor
    /// \return tuple{batch_size_for_indices, batch_size_for_slides}
    ///    - batch_size_for_cache: maximum elements for each corr fun index
    ///    - batch_size_for_indices: maximum elements for each corr fun index
    ///    - batch_size_for_vertex: maximum elements for slide
    ///
    /// NOTE: the heuristics assume using a `get_simple_contraction_plan`, where only two tensors and
    ///       the result are hold in memory
    /// Rules:
    /// - The memory footprint for the construction of distillation objects should be less than the
    ///   the entire memory
    /// - The memory footprint of the distillation objects should be smaller than the max cache size
    /// - The memory footprint for contractions should be smaller than the total memory minus the cache

    inline std::tuple<SBN::SCoor, SBN::SCoor, SBN::SCoor>
    get_maximum_corr_funs(const std::vector<GraphVertex>& graph, const ContractionPlan& plan,
			  const std::map<int, int>& color_to_spin,
			  const std::map<char, int>& spin_size, const std::vector<int>& slides,
			  int num_vecs, std::size_t max_mem, const VertexIndices& vertex_indices,
			  const std::vector<std::vector<SBN::IndexType>>& v,
			  const std::vector<std::size_t>& num_unique_vertices,
			  const Dist<SBN::IndexType>& num_unique_dist,
			  const Dist<unsigned int>& factor_mem_build, const SBN::Tensor& perm)
    {
      // Get the index labels
      std::set<char> indices_labels;
      for (const auto& [_, order] : vertex_indices)
	indices_labels.insert(order.begin(), order.end());

      // Get the dimension of all baryons, mesons, genprops and props
      SBN::SCoor s; ///< contraction dimensions
      for (const auto& it : graph)
      {
	for (const auto& slot : it.slot_indices)
	{
	  s[slot] = num_vecs;
	  s[color_to_spin.at(slot)] = spin_size.at(color_to_spin.at(slot));
	}
      }
      for (const auto& it : SBN::get_scoor(perm.order, perm.size))
	s[it.first] = it.second;

      for (int self_it = 0; self_it < 2; ++self_it)
      {
	auto get_mem = [&](const SBN::Order& order, const SBN::SCoor& sc) {
	  std::size_t mem = sizeof(SBN::value_type);
	  for (char label : order)
	    mem *= (std::size_t)(sc.count(label) == 0 ? s.at(label) : sc.at(label));
	  return mem;
	};
	auto is_in = [](const auto& str, char c) {
	  return std::find(str.begin(), str.end(), c) != str.end();
	};
	std::size_t free_mem = max_mem;
	for (const auto& step : plan)
	{
	  const auto& sc = SBN::SCoor{{'I', (int)step.first_tensor_indices.size()}};
	  for (char c : perm.order)
	  {
	    for (int self_it = 0; self_it < 2; ++self_it)
	    {
	      // Get the memory footprint of the input and output tensors
	      const auto factor =
		2u; ///< consider an extra factor for auxiliary buffers during contraction
	      const auto mem_first =
		get_mem(SBN::relabel(step.first_tensor_relabel, step.first_tensor_order),
			SBN::complete_scoor(step.first_tensor_slice_size, sc)) *
		factor;
	      const auto mem_second =
		get_mem(SBN::relabel(step.second_tensor_relabel, step.second_tensor_order),
			SBN::complete_scoor(step.second_tensor_slice_size, sc)) *
		factor;
	      const auto mem_result = get_mem(step.result_tensor_order, sc) * factor;

	      // Reduce the dimension `c` such that the total mem is smaller than the free mem
	      auto this_free_mem = free_mem;
	      std::size_t this_total_mem = 0;
	      auto set_mem = [&](const auto& order, const auto& mem) {
		if (is_in(order, c))
		  this_total_mem += mem;
		else if (this_free_mem >= mem)
		  this_free_mem -= mem;
		else
		  this_free_mem = 0;
	      };
	      set_mem(step.first_tensor_order, mem_first);
	      set_mem(step.second_tensor_order, mem_second);
	      set_mem(step.result_tensor_order, mem_result);
	      if (this_total_mem == 0 || mem_first + mem_second + mem_result < free_mem)
		break;

	      auto& si = s.at(c);
	      si = this_free_mem == 0
		     ? 1.0
		     : std::max(1.0, (double)si * (double)this_free_mem / (double)this_total_mem);
	    }
	  }
	}
      }

      SBN::SCoor batch_size_for_indices, batch_size_for_slides;
      const std::set<char> slide_labels(slides.begin(), slides.end());
      for (const auto& it : s)
      {
	if (indices_labels.count(it.first) > 0)
	  batch_size_for_indices[it.first] = it.second;
	else if (slide_labels.count(it.first) > 0)
	  batch_size_for_slides[it.first] = it.second;
      }

      const auto batch_size_for_cache = SBN::SCoor{};
      return {batch_size_for_cache, batch_size_for_indices, batch_size_for_slides};
    }

    /// Print a table with the vertices and time slices given a permutation
    /// \param vertices: unique_vertices[vertices[i][j]] disp vertex for vertex j for corr fun i
    /// \param t: time[i][j] time slice for vertex j for corr fun i
    /// \param p: permutation
    /// \param cout: output stream

    template <typename stream>
    inline void print_vertices_and_t(const std::vector<std::vector<std::size_t>>& vertices,
				     const std::vector<std::vector<int>>& t,
				     const std::vector<std::size_t>& p, stream& cout)
    {
      int num_columns = t.at(0).size();
      for (const auto& i : p)
      {
	for (int j = 0; j < num_columns; ++j)
	{
	  if (i < t.size())
	    cout << t.at(i).at(j) << "," << vertices.at(i).at(j) << " ";
	  else
	    cout << -1 << "," << -1 << " ";
	}
	cout << std::endl;
      }
    }

    /// Return the order in which the vertices are visited
    /// \param plan: contraction plan
    /// \param num_vertices: the first `num_vertices` are tracked

    inline std::vector<std::vector<int>> get_vertices_visiting_order(const ContractionPlan& plan,
								     int num_vertices)
    {
      std::vector<int> contracted(num_vertices);
      std::vector<std::vector<int>> order;
      order.reserve(num_vertices);
      auto check = [&](const std::vector<int>& vertex_indices) {
	std::vector<int> r;
	r.reserve(vertex_indices.size());
	for (auto vertex_index : vertex_indices)
	{
	  if (vertex_index >= num_vertices || contracted.at(vertex_index) != 0)
	    continue;
	  contracted.at(vertex_index) = 1;
	  r.push_back(vertex_index);
	}
	if (r.size() > 0)
	  order.push_back(r);
      };
      for (const auto& it : plan)
      {
	check(it.second_tensor_indices);
	check(it.first_tensor_indices);
      }
      return order;
    }

    /// Return the used labels by the contractions
    /// \param plan: contraction plan

    inline SBN::Order get_used_labels(const ContractionPlan& plan)
    {
      std::set<char> r;
      for (const auto& step : plan)
	for (const auto& order : std::vector<SBN::Order>{
	       SBN::relabel(step.first_tensor_relabel, step.first_tensor_order),
	       SBN::relabel(step.second_tensor_relabel, step.second_tensor_order),
	       step.result_tensor_order})
	  r.insert(order.begin(), order.end());
      return SBN::Order(r.begin(), r.end());
    }

    /// Return the original slot
    /// \param slot: slot index
    /// \param slot_perm: slot permutation

    inline int get_origin_slot(int slot, const SBN::Coor& slot_perm)
    {
      auto it = std::find(slot_perm.begin(), slot_perm.end(), slot);
      if (it == slot_perm.end())
	throw std::runtime_error("wtf");
      return std::distance(slot_perm.begin(), it);
    }

    /// Return the phasing associated to a slot
    /// \param v: disp vertex
    /// \param index: slot index, eg for meson (0 or 1), for baryon (0..2)

    XMLArray::Array<int> get_phasing(const Hadron::HadronDispVertex_t& v, unsigned int index)
    {
      // If the vertex isn't smeared, return empty phasing
      if (!v.smearedP)
      {
	return XMLArray::Array<int>(std::vector<int>{0, 0, 0});
      }

      // Process for baryons
      if (v.type == 'B')
      {
	if (v.phasings.size() == 1)
	  return v.phasings.at(0);
	else if (v.phasings.size() != 3)
	  throw std::runtime_error("Invalid number of phasings for a baryon");
	for (const auto& p : v.phasings)
	  if (p != v.phasings.at(0))
	    throw std::runtime_error("Baryon does not support quarks with different phasing");
	return v.phasings.at(index);
      }

      // Process for mesons
      if (v.type == 'M' || v.type == 'A')
      {
	if (v.phasings.size() == 1)
	  return v.phasings.at(0);
	else if (v.phasings.size() != 2)
	  throw std::runtime_error("Invalid number of phasings for a meson");
	// Switch phases for creating operator
	return v.phasings.at((index + (v.creation_op ? 1 : 0)) % 2);
      }

      // Otherwise, return no phasing
      return XMLArray::Array<int>(std::vector<int>{0, 0, 0});
    }

    using GroupType = unsigned int;
    using SpinsTable = std::vector<std::array<int, 3>>;
    using SpinsFromUniqueVertexIndex = std::vector<SpinsTable>;
    using CorrIndexAndSpinCombo = std::vector<std::array<SBN::IndexType, 2>>;

    /// Return the number of spin combos for each graph vertex for a given correlation function
    /// \param corr_index: correlation function
    /// \param graph: vertices and slot numbers
    /// \param vertices: unique_vertices[vertices[i][j]] disp vertex for vertex j for corr fun i
    /// \param spins_from_unique_vertex_index: return the list of spin combination for a vertex

    inline SBN::Coor
    get_spin_combo_dim(SBN::IndexType corr_index, const std::vector<GraphVertex>& graph,
		       const std::vector<std::vector<std::size_t>>& vertices,
		       const SpinsFromUniqueVertexIndex& spins_from_unique_vertex_index)
    {
      SBN::Coor spin_combo_dim;
      spin_combo_dim.reserve(graph.size());
      for (int j = 0; j < graph.size(); ++j)
      {
	const auto type = graph.at(j).type;
	if (type == 'B')
	{
	  spin_combo_dim.push_back(
	    spins_from_unique_vertex_index.at(vertices.at(corr_index).at(j)).size());
	}
	else
	{
	  spin_combo_dim.push_back(1);
	}
      }
      return spin_combo_dim;
    }

    inline XMLArray::Array<int> get_meson_phasing_source(const Hadron::HadronDispVertex_t& v);
    inline XMLArray::Array<int> get_meson_phasing_sink(const Hadron::HadronDispVertex_t& v);
    inline XMLArray::Array<int> get_baryon_phasing(const Hadron::HadronDispVertex_t& v);

    /// Get the unique vertices for each graph vertex
    /// \param graph: vertices and slot numbers
    /// \param related_prop: returns the source and sink prop vertex indices with the flavor and conj info
    /// \param unique_vertices: vector of disp vertices
    /// \param vertices: unique_vertices[vertices[i][j]] disp vertex for vertex j for corr fun i
    /// \param slots_perm: do_perm(origin_slots, slots_perm[i]) -> target_slots
    /// \param t: time[i][j] time slice for vertex j for corr fun i
    /// \param T: lattice time size
    /// \param comm: communicator
    /// \param cout: output stream
    /// \return tuple{...}, where
    /// - unique_vertices: v[i][j] index for vertex j on corr fun i which is unique among all v[*][j]
    /// - group: group[i][j], values used to group corr fun with shared heavy computation
    /// - spins_from_unique_vertex_index: return the list of spin combination for a vertex
    /// - corr_index_and_spin: return the list of spin combination for a vertex
    /// - num_unique_vertices: number of unique vertices for each vertex
    /// - num_unique_dist: number of unique distillation elements
    /// - factor_mem_build: number of linear combinations of spins on average to construct an element

    template <typename stream>
    inline std::tuple<std::vector<std::vector<SBN::IndexType>>, std::vector<std::vector<GroupType>>,
		      SpinsFromUniqueVertexIndex, CorrIndexAndSpinCombo, std::vector<std::size_t>,
		      Dist<SBN::IndexType>, Dist<unsigned int>>
    get_unique_vertices(const std::vector<GraphVertex>& graph,
			const std::map<int, std::array<int, 2>>& related_prop,
			const CoeffMap& coeff_map,
			const std::vector<Hadron::HadronDispVertex_t>& unique_vertices,
			const std::vector<std::vector<std::size_t>>& vertices,
			const std::vector<std::vector<SBN::Coor>>& slots_perm,
			const std::vector<std::vector<int>>& t, int T, const SBN::Comm& comm,
			stream& cout)
    {
      // Get trivial cases
      if (vertices.size() == 0 || vertices.at(0).size() == 0)
	return {{}, {}, {}, {}, {}, Dist<SBN::IndexType>(0), Dist<unsigned int>(0)};

      // Check input
      if (vertices.size() != slots_perm.size() || vertices.size() != t.size() ||
	  vertices.at(0).size() != slots_perm.at(0).size())
	throw std::runtime_error("get_unique_vertices: number of rows or columns of `vertices`, "
				 "`slots_perm` and `t` does not match");

      // Three cases of uniqueness
      // a) mesons and baryons (M, A, B):
      //    - vertices[i][j], slots_perm[i][j], t[i][j], spin_index
      // b) props (Q):
      //    - vertices[i][j], slots_perm[i][j],
      //    - phasing(vertices[i][j_src], slots_perm[i][j_src]), t[i][j_src], spin_index_src,
      //    - phasing(vertices[i][j_snk], slots_perm[i][j_snk]), t[i][j_snk], spin_index_snk
      // c) genprops (m, a):
      //    - vertices[i][j], vertices[i][j_related_prop], slots_perm[i][j], t[i][j]
      //    - phasing(vertices[i][j_src], slots_perm[i][j_src]), t[i][j_src], spin_index_src,
      //    - phasing(vertices[i][j_snk], slots_perm[i][j_snk]), t[i][j_snk], spin_index_snk

      // Three cases for grouping
      // a) mesons and baryons (M, A, B):
      //    - phasing(vertices[i][j], slots_perm[i][j]),
      //    - mom(vertices[i][j], slots_perm[i][j]),
      //    - t[i][j],
      //    - 2 - longest_derivative(vertices[i][j])
      //    - spin
      // b) props (Q):
      //    - constant
      // c) genprops (m, a):
      //    - longest_derivative(vertices[i][j])

      const GroupType N_spins_slices = 2u;
      const GroupType N_longest_deriv = 4u;
      const GroupType N_mom_mesons =
	std::sqrt((double)(std::numeric_limits<GroupType>::max() / N_longest_deriv / T));
      const GroupType N_mom_baryons =
	std::sqrt((double)(std::numeric_limits<GroupType>::max() / N_spins_slices / N_spins_slices /
			   N_spins_slices / N_longest_deriv / T));
      const GroupType N_phase_mesons = N_mom_mesons;
      const GroupType N_phase_baryons = N_mom_baryons;

      // Find the source and sink for props and genprops
      std::vector<std::array<std::array<int, 2>, 2>> src_snk(
	graph.size()); // [sink/source][vertex/slow]
      for (int j = 0; j < graph.size(); ++j)
      {
	const auto type = graph.at(j).type;
	if ((type != 'Q' && type != 'm' && type != 'a') || graph.at(j).slot_indices.size() == 0)
	  continue;
	const auto& slot_to_vertex = get_map_from_slot_index_to_vertex_and_slot(graph, j);
	src_snk.at(j) = {slot_to_vertex.at(graph.at(j).slot_indices.at(1)),
			 slot_to_vertex.at(graph.at(j).slot_indices.at(0))};
      }

      using Mom = std::array<int, 3>;
      using Phase = std::array<int, 3>;
      using VPT = std::tuple<std::size_t, SBN::Coor, int>;
      using PTS = std::tuple<Phase, int, int>;
      using VPTS = std::tuple<std::size_t, SBN::Coor, int, int>;
      using MesonOrBaryon = VPTS;
      using Prop = std::tuple<VPT, PTS, PTS>;
      using Genprop = std::tuple<Prop, std::size_t>;

      unordered_map<MesonOrBaryon, SBN::IndexType> index_mesons(16);
      SBN::IndexType num_mesons = 0;

      unordered_map<MesonOrBaryon, SBN::IndexType> index_baryons(16);
      SBN::IndexType num_baryons = 0;
      unordered_map<Prop, SBN::IndexType> index_p(16);
      SBN::IndexType num_props = 0;
      unordered_map<Genprop, SBN::IndexType> index_g(16);
      std::vector<std::unordered_set<SBN::IndexType>> n(graph.size(),
							std::unordered_set<SBN::IndexType>(16));
      SBN::IndexType num_genprops = 0;

      using SnkSrcPhase = std::tuple<Phase, Phase>;
      unordered_map<SnkSrcPhase, unsigned int> group_meson_phase(16);
      unsigned int num_meson_phases = 0;
      unordered_map<Mom, unsigned int> group_meson_mom(16);
      unsigned int num_meson_moms = 0;

      unordered_map<Phase, unsigned int> group_baryon_phase(16);
      unsigned int num_baryon_phases = 0;
      unordered_map<Mom, unsigned int> group_baryon_mom(16);
      unsigned int num_baryon_moms = 0;

      unordered_map<SnkSrcPhase, unsigned int> group_props(16);
      unsigned int num_prop_groups = 0;
      unordered_map<SnkSrcPhase, unsigned int> group_genprops(16);
      unsigned int num_genprop_groups = 0;

      SpinsFromUniqueVertexIndex spins_from_unique_vertex_index(unique_vertices.size());
      std::vector<std::pair<unsigned int, unsigned int>> max_spins_and_derivs(
	unique_vertices.size());

      // Return the spin block that corresponds to a given spin for a given blocking

      auto block_spins = []<std::size_t NUM_SPINS>(const auto& spins, int spin_block) {
	std::array<int, NUM_SPINS> r;
	for (int i = 0; i < NUM_SPINS; ++i)
	  r.at(i) = spins.ref().at(i) / spin_block * spin_block;
	return r;
      };

      // Given a vertex index, returns
      // - the number of linear combinations required to construct the distillation object
      // - the longest derivative/displacement
      // NOTE: it also fills spins_from_unique_vertex_index

      auto get_max_spins_and_derivs = [&]<std::size_t NUM_SPINS>(std::size_t vertex_index,
								 char type) {
	if (max_spins_and_derivs.at(vertex_index).first > 0)
	  return max_spins_and_derivs.at(vertex_index);
	const Hadron::HadronDispVertex_t& h = unique_vertices.at(vertex_index);
	const auto& hcoeff = coeff_map[h];
	unsigned int n = 0;
	unsigned int max_length_deriv = 0;
	if constexpr (NUM_SPINS == 3)
	{
	  unordered_map<std::array<int, NUM_SPINS>, unsigned int> spins(16);
	  const int spin_block = (type == 'B' ? BARYON_SPIN_BLOCK : Datalib::TensorStorage::spin_count);
	  const auto& k1 = hcoeff.begin()->second;
	  for (auto k2 = k1.begin(); k2 != k1.end(); ++k2)
	  {
	    for (auto k3 = k2->first.derivs.ref().begin(); k3 != k2->first.derivs.ref().end(); ++k3)
	    {
	      max_length_deriv = std::max(max_length_deriv, (unsigned int)k3->deriv.size());
	    }
	    for (auto k3 = k2->second.begin(); k3 != k2->second.end(); ++k3)
	    {
	      n = std::max(
		n,
		++spins[block_spins.template operator()<NUM_SPINS>(k3->first.diracs, spin_block)]);
	    }
	  }
	  auto& spins_combos = spins_from_unique_vertex_index.at(vertex_index);
	  spins_combos.reserve(spins.size());
	  for (const auto& it : spins)
	    spins_combos.push_back(it.first);
	}
	else
	{
	  const auto& disp_spins = Hadron::convertTwoQuarkDispGamma(hcoeff);
	  for (auto k2 = disp_spins.begin(); k2 != disp_spins.end(); ++k2)
	  {
	    max_length_deriv = std::max(max_length_deriv, (unsigned int)k2->first.deriv.size());
	    n += k2->second.size();
	  }
	}
	return (max_spins_and_derivs.at(vertex_index) = std::make_pair(n, max_length_deriv));
      };

      // Get the unique elements for each vertex
      const auto N = std::numeric_limits<GroupType>::max() / (GroupType)T / (GroupType)T /
		     (GroupType)T / N_spins_slices / N_spins_slices;
      const SBN::IndexType num_corr = t.size();
      std::vector<std::vector<SBN::IndexType>> index;
      index.reserve(t.size());
      std::vector<std::vector<GroupType>> group;
      group.reserve(t.size());
      std::vector<SBN::IndexType> corr_index;
      corr_index.reserve(t.size());
      std::vector<SBN::IndexType> row(graph.size());
      std::vector<GroupType> group_row(graph.size());
      Dist<unsigned int> factor_mem_build(1);
      CorrIndexAndSpinCombo corr_index_and_spin;
      corr_index_and_spin.reserve(num_corr);
      for (SBN::IndexType i = 0; i < num_corr; ++i)
      {
	// Fill spins_from_unique_vertex_index if baryons are involved
	for (int j = 0; j < graph.size(); ++j)
	{
	  if (graph.at(j).type == 'B')
	    get_max_spins_and_derivs.template operator()<3>(vertices.at(i).at(j), 'B');
	}

	// Get the number of spin combos for each vertex
	const auto& spin_combo_dim =
	  get_spin_combo_dim(i, graph, vertices, spins_from_unique_vertex_index);

	for (int spin_combo_index = 0, num_spin_combos = SBN::volume(spin_combo_dim);
	     spin_combo_index < num_spin_combos; ++spin_combo_index)
	{
	  const auto& spin_combo = SBN::indexToCoor(spin_combo_index, spin_combo_dim);
	  for (int j = 0; j < graph.size(); ++j)
	  {
	    const auto type = graph.at(j).type;
	    std::size_t idx;
	    if (type == 'M' || type == 'A')
	    {
	      const auto tj = t.at(i).at(j);
	      MesonOrBaryon k{vertices.at(i).at(j), slots_perm.at(i).at(j), tj, spin_combo.at(j)};
	      if (index_mesons.count(k) == 0)
		index_mesons[k] = num_mesons++;
	      row.at(j) = index_mesons.at(k);

	      const auto [max_spins, max_deriv_length] =
		get_max_spins_and_derivs.template operator()<2>(vertices.at(i).at(j), type);
	      factor_mem_build.meson = std::max(max_spins, factor_mem_build.meson);

	      const Hadron::HadronDispVertex_t& h = unique_vertices.at(vertices.at(i).at(j));
	      const auto& phase =
		std::make_tuple(Datalib::TensorStorage::detail::toCoor(get_meson_phasing_source(h)),
				Datalib::TensorStorage::detail::toCoor(get_meson_phasing_sink(h)));
	      if (group_meson_phase.count(phase) == 0)
		group_meson_phase[phase] = num_meson_phases++;
	      if (num_meson_phases >= N_phase_mesons)
		throw std::runtime_error("too many phases for mesons");

	      const auto& mom = Datalib::TensorStorage::detail::toCoor(h.mom);
	      if (group_meson_mom.count(mom) == 0)
		group_meson_mom[mom] = num_meson_moms++;
	      if (num_meson_moms >= N_mom_mesons)
		throw std::runtime_error("too many moms for mesons");
	      if (spin_combo.at(j) > 0)
		throw std::runtime_error("wtf");

	      group_row.at(j) = ((group_meson_phase.at(phase) //
				    * N_mom_mesons +
				  group_meson_mom.at(mom)) //
				   * T +
				 tj) //
				  * N_longest_deriv +
				std::min(N_longest_deriv - 1, max_deriv_length);
	    }
	    else if (type == 'B')
	    {
	      const auto tj = t.at(i).at(j);
	      MesonOrBaryon k{vertices.at(i).at(j), slots_perm.at(i).at(j), tj, spin_combo.at(j)};
	      if (index_baryons.count(k) == 0)
		index_baryons[k] = num_baryons++;
	      row.at(j) = index_baryons.at(k);

	      const auto [max_spins, max_deriv_length] =
		get_max_spins_and_derivs.template operator()<3>(vertices.at(i).at(j), type);
	      factor_mem_build.baryon = std::max(max_spins, factor_mem_build.baryon);

	      const Hadron::HadronDispVertex_t& h = unique_vertices.at(vertices.at(i).at(j));
	      const auto& phase = Datalib::TensorStorage::detail::toCoor(get_baryon_phasing(h));
	      if (group_baryon_phase.count(phase) == 0)
		group_baryon_phase[phase] = num_baryon_phases++;
	      if (num_baryon_phases >= N_phase_baryons)
		throw std::runtime_error("too many phases for baryons");

	      const auto& mom = Datalib::TensorStorage::detail::toCoor(h.mom);
	      if (group_baryon_mom.count(mom) == 0)
		group_baryon_mom[mom] = num_baryon_moms++;
	      if (num_baryon_moms >= N_mom_baryons)
		throw std::runtime_error("too many moms for baryons");
	      if (spin_combo.at(j) >= N_spins_slices * N_spins_slices * N_spins_slices)
		throw std::runtime_error("wtf");

	      group_row.at(j) = (((group_baryon_phase.at(phase) //
				     * N_mom_baryons +
				   group_baryon_mom.at(mom)) *
				    T						    //
				  + tj)						    //
				   * N_longest_deriv				    //
				 + std::min(N_longest_deriv - 1, max_deriv_length)) //
				  * N_spins_slices * N_spins_slices * N_spins_slices +
				spin_combo.at(j);
	    }
	    else if (type == 'Q')
	    {
	      const bool transpose = slots_perm.at(i).at(j) == SBN::Coor{1, 0};
	      const auto [j_src, target_slot_src] = src_snk.at(j).at(!transpose ? 0 : 1);
	      const auto [j_snk, target_slot_snk] = src_snk.at(j).at(!transpose ? 1 : 0);
	      const auto tj_src = t.at(i).at(j_src);
	      const auto tj_snk = t.at(i).at(j_snk);
	      int slot_src = get_origin_slot(target_slot_src, slots_perm.at(i).at(j_src));
	      int slot_snk = get_origin_slot(target_slot_snk, slots_perm.at(i).at(j_snk));
	      const auto& ph_src = Datalib::TensorStorage::detail::toCoor(
		get_phasing(unique_vertices.at(vertices.at(i).at(j_src)), slot_src));
	      const auto& ph_snk = Datalib::TensorStorage::detail::toCoor(
		get_phasing(unique_vertices.at(vertices.at(i).at(j_snk)), slot_snk));
	      const int spin_src = graph.at(j_src).type != 'B'
				     ? 0
				     : spins_from_unique_vertex_index.at(vertices.at(i).at(j_src))
					   .at(spin_combo.at(j_src))
					   .at(slot_src) /
					 BARYON_SPIN_BLOCK;
	      const int spin_snk = graph.at(j_snk).type != 'B'
				     ? 0
				     : spins_from_unique_vertex_index.at(vertices.at(i).at(j_snk))
					   .at(spin_combo.at(j_snk))
					   .at(slot_snk) /
					 BARYON_SPIN_BLOCK;
	      const Prop k{
		{vertices.at(i).at(j), slots_perm.at(i).at(j), 0},
		{ph_src, tj_src, spin_src},
		{ph_snk, tj_snk, spin_snk},
	      };
	      if (index_p.count(k) == 0)
		index_p[k] = num_props++;
	      row.at(j) = index_p.at(k);
	      const SnkSrcPhase phase{ph_src, ph_snk};
	      if (group_props.count(phase) == 0)
		group_props[phase] = num_prop_groups++;
	      if (num_prop_groups >= N)
		throw std::runtime_error("too many prop groups");
	      const GroupType dist_tj_snk = T - SBN::norm_coor(tj_snk - tj_src, T);
	      group_row.at(j) =
		((group_props.at(phase) * T + tj_src) * T + dist_tj_snk) * N_spins_slices +
		spin_src;
	    }
	    else if (type == 'm' || type == 'a')
	    {
	      const auto [j_src, target_slot_src] = src_snk.at(j).at(0);
	      const auto [j_snk, target_slot_snk] = src_snk.at(j).at(1);
	      const auto tj = t.at(i).at(j);
	      const auto tj_src = t.at(i).at(j_src);
	      const auto tj_snk = t.at(i).at(j_snk);
	      int slot_src = get_origin_slot(target_slot_src, slots_perm.at(i).at(j_src));
	      int slot_snk = get_origin_slot(target_slot_snk, slots_perm.at(i).at(j_snk));
	      const auto& ph_src = Datalib::TensorStorage::detail::toCoor(
		get_phasing(unique_vertices.at(vertices.at(i).at(j_src)), slot_src));
	      const auto& ph_snk = Datalib::TensorStorage::detail::toCoor(
		get_phasing(unique_vertices.at(vertices.at(i).at(j_snk)), slot_snk));
	      const int spin_src = graph.at(j_src).type != 'B'
				     ? 0
				     : spins_from_unique_vertex_index.at(vertices.at(i).at(j_src))
					   .at(spin_combo.at(j_src))
					   .at(slot_src) /
					 BARYON_SPIN_BLOCK;
	      const int spin_snk = graph.at(j_snk).type != 'B'
				     ? 0
				     : spins_from_unique_vertex_index.at(vertices.at(i).at(j_snk))
					   .at(spin_combo.at(j_snk))
					   .at(slot_snk) /
					 BARYON_SPIN_BLOCK;
	      const Genprop k{
		{
		  {vertices.at(i).at(j), slots_perm.at(i).at(j), tj},
		  {ph_src, tj_src, spin_src},
		  {ph_snk, tj_snk, spin_snk},
		},
		vertices.at(i).at(related_prop.at(j).at(0)),
	      };
	      if (index_g.count(k) == 0)
		index_g[k] = num_genprops++;
	      row.at(j) = index_g.at(k);
	      const auto [max_spins, max_deriv_length] =
		get_max_spins_and_derivs.template operator()<2>(vertices.at(i).at(j), type);
	      factor_mem_build.genprop = std::max(max_spins, factor_mem_build.genprop);
	      const SnkSrcPhase phase{ph_src, ph_snk};
	      if (group_genprops.count(phase) == 0)
		group_genprops[phase] = num_genprop_groups++;
	      if (num_genprop_groups >= N)
		throw std::runtime_error("too many genprop groups");
	      const GroupType dist_tj_snk = T - SBN::norm_coor(tj_snk - tj_src, T);
	      const GroupType dist_tj = T - SBN::norm_coor(tj - tj_src, T);
	      group_row.at(j) =
		((((group_genprops.at(phase) * T + tj_src) * T + dist_tj_snk) * T + dist_tj) *
		   N_spins_slices +
		 spin_src) *
		  N_spins_slices +
		spin_snk;
	    }
	    else
	    {
	      throw std::runtime_error("unsupported type");
	    }
	    n.at(j).insert(row.at(j));
	  }
	  index.push_back(row);
	  group.push_back(group_row);
	  corr_index_and_spin.push_back({i, spin_combo_index});
	}
      }

      // Get the number of unique vertices
      std::vector<std::size_t> num_unique_vertices;
      num_unique_vertices.reserve(graph.size());
      for (const auto& it : n)
	num_unique_vertices.push_back(it.size());

      // Print the number of unique elements for each vertex
      if (SBN::get_rank(comm.comm) == 0)
      {
	cout << "  analysis of vertices and time slices:" << std::endl;
	for (int j = 0; j < graph.size(); ++j)
	{
	  cout << "    vertex= " << j << " has " << num_unique_vertices.at(j) << " unique elements"
	       << std::endl;
	}
	cout << "    total unique mesons: " << num_mesons << "  baryons: " << num_baryons
	     << "  props: " << num_props << "  genprops: " << num_genprops << std::endl;
	cout << "    max spins linear combinations for mesons: " << factor_mem_build.meson
	     << "  baryons: " << factor_mem_build.baryon << "  props: " << factor_mem_build.prop
	     << "  genprops: " << factor_mem_build.genprop << std::endl;
      }

      return {index,
	      group,
	      spins_from_unique_vertex_index,
	      corr_index_and_spin,
	      num_unique_vertices,
	      Dist<SBN::IndexType>{num_mesons, num_baryons, num_props, num_genprops},
	      factor_mem_build};
    }

    /// Group the correlation functions with similar vertices
    /// \param graph: vertices and slot numbers
    /// \param plan: contraction plan
    /// \param v: v[i][j] index for vertex j on corr fun i which is unique among all v[*][j]
    /// \param group: group[i][j], values used to group corr fun with shared heavy computation
    /// \param corr_index_and_spin: return the list of spin combination for a vertex
    /// \param num_unique_vertices: number of unique vertices for each vertex
    /// \param comm: communicator
    /// \param cout: output stream
    /// \return tuple{perm, vertex_cf_indices, new_plan}, where
    /// - perm: arrange of the corr funs indices into a tensor
    /// - vertex_cf_indices: indices for each vertex
    /// - new_plan: plan with corr function indices set

    template <typename stream>
    inline std::tuple<SBN::Tensor, VertexIndices, ContractionPlan>
    get_optimal_perm(const std::vector<GraphVertex>& graph, const ContractionPlan& plan,
		     const std::vector<std::vector<SBN::IndexType>>& v,
		     const std::vector<std::vector<GroupType>>& group,
		     const CorrIndexAndSpinCombo& corr_index_and_spin,
		     const std::vector<std::size_t>& num_unique_vertices, const SBN::Comm& comm,
		     stream& cout)
    {
      Datalib::TensorStorage::detail::Tracker t_("get_optimal_perm");

      // Get trivial cases
      if (v.size() == 0 || v.at(0).size() == 0 || plan.size() == 0)
	return {{}, {}, {}};

      // Check input
      if (v.size() != corr_index_and_spin.size() || v.size() != group.size() ||
	  v.at(0).size() != num_unique_vertices.size() ||
	  group.at(0).size() != num_unique_vertices.size())
	throw std::runtime_error("get_optimal_perm: invalid input");

      // Get visiting order of the vertices
      const int num_vertices = v.at(0).size();
      const auto num_corr = v.size();
      const auto& p = get_vertices_visiting_order(plan, num_vertices);

      using vec_v = std::vector<SBN::IndexType>;
      using vec_uv = std::vector<GroupType>;
      using vec_v_map = unordered_map<vec_v, SBN::IndexType>; ///< key -> corr index

      // Return a key
      auto get_key = [&](int first_vertex, int last_vertex, int corr_fun) {
	vec_v k;
	k.reserve(last_vertex + 1 - first_vertex);
	for (int i = first_vertex; i <= last_vertex; ++i)
	{
	  for (const auto& pij : p.at(i))
	  {
	    k.push_back(v.at(corr_fun).at(pij));
	  }
	}
	return k;
      };

      // Return a group
      auto get_group = [&](int first_vertex, int last_vertex, int corr_fun) {
	vec_uv k;
	k.reserve(last_vertex + 1 - first_vertex);
	for (int i = first_vertex; i <= last_vertex; ++i)
	  for (const auto& pij : p.at(i))
	    k.push_back(group.at(corr_fun).at(pij));
	return k;
      };

      // Return unique combination of elements up to a given number of steps
      auto get_unique_vtx = [&](int first_vertex, int last_vertex) {
	if (last_vertex < first_vertex)
	  throw std::runtime_error("wtf");

	// Get the maximum number of unique vertices
	std::size_t n = 1;
	for (int i = first_vertex; i <= last_vertex; ++i)
	  for (const auto pij : p.at(i))
	    n *= num_unique_vertices.at(pij);

	auto r = fill_map_among_threads<vec_v_map>(num_corr, std::min(n, num_corr), [&](auto i) {
	  return std::make_pair(get_key(first_vertex, last_vertex, i), i);
	});

	if (SBN::get_rank(comm.comm) == 0)
	{
	  cout << "    vertices= ";
	  for (int i = first_vertex; i <= last_vertex; ++i)
	    for (const auto pij : p.at(i))
	      cout << pij << " ";
	  cout << " has " << r.size() << " unique elements" << std::endl;
	}

	return r;
      };

      // List of index fragments {first_index, num_indices}
      std::vector<std::array<int, 2>> index_chunks;
      std::vector<vec_v_map> unique_index_chunks;

      const auto overflow_factor_for_cartesian = 1.2;

      // Factorize corr funs: a) full factorization
      bool is_cart = false;
      {
	std::size_t cart_prod = 1;
	for (const auto& pi : p)
	  for (const auto pij : pi)
	    cart_prod *= num_unique_vertices.at(pij);
	if (cart_prod <= num_corr * overflow_factor_for_cartesian)
	{
	  for (int i = 0; i < p.size(); ++i)
	  {
	    index_chunks.push_back({i, 1});
	    unique_index_chunks.push_back(get_unique_vtx(i, i));
	  }
	  is_cart = true;
	}
      }

      // Factorize corr funs: b) recursive division
      if (index_chunks.size() == 0)
      {
	using FirstLastIndexAndParent = std::tuple<int, int>;
	using Result = std::tuple<std::vector<std::array<int, 2>>, std::vector<vec_v_map>,
				  std::size_t, std::size_t, bool>;
	unordered_map<FirstLastIndexAndParent, Result> cache(16);

	/// Return the indices partitions that minimizes the estimated contraction cost
	/// \return {index_fragments, unique_keys, num_keys, cost, do_cart}, where
	///  - index_fragments: list of index sequences
	///  - unique_keys: unique keys for each list of indices
	///  - num_keys: number of unique keys
	///  - cost: estimated contraction cost in number of contractions
	///  - do_cart: whether an all-to-all contraction of the keys is worth it
	///
	/// NOTE: the cost of contraction is estimated as the cost constructing the individual
	///       elements plus the cost of contracting them.
	auto f = [&](int first_index, int last_index, int deep, const auto& f) -> Result {
	  const auto& cache_entry = FirstLastIndexAndParent{first_index, last_index};
	  if (cache.count(cache_entry) > 0)
	    return cache.at(cache_entry);
	  std::vector<std::array<int, 2>> index_chunks;
	  std::vector<vec_v_map> unique_index_chunks;
	  std::size_t cart_prod;
	  std::size_t cost;
	  bool do_cart = false;
	  auto v = get_unique_vtx(first_index, last_index);
	  const auto num_corr_upto = v.size();
	  if (first_index == last_index)
	  {
	    index_chunks.push_back({first_index, 1});
	    unique_index_chunks.push_back(std::move(v));
	    cart_prod = num_corr_upto;
	    cost = num_corr_upto;
	  }
	  else
	  {
	    int least_cost_index = last_index;
	    std::size_t least_cost = 2ul * (last_index - first_index + 1 - 1) * num_corr_upto;
	    /// FIXME: support several partitions
	    if (deep == 0)
	    {
	      for (int i0 = first_index; i0 < last_index; ++i0)
	      {
		const auto& p0 = f(first_index, i0, deep + 1, f);
		const auto& p1 = f(i0 + 1, last_index, deep + 1, f);
		const auto cost = //
		  // cost constructing the individual indices
		  std::get<3>(p0) + std::get<3>(p1) +
		  // cost doing the contractions
		  std::get<2>(p0) + std::min(num_corr_upto, std::get<2>(p0) * std::get<2>(p1));
		if (cost < least_cost)
		{
		  least_cost_index = i0;
		  least_cost = cost;
		  do_cart = (num_corr_upto * overflow_factor_for_cartesian >=
			     std::get<2>(p0) * std::get<2>(p1));
		}
	      }
	    }
	    if (SBN::get_rank(comm.comm) == 0)
	    {
	      cout << "    for i=" << first_index << ".." << last_index << " optimal cut is "
		   << first_index << ".." << least_cost_index << " and " << least_cost_index + 1
		   << ".." << last_index << ", with a cost of " << least_cost << ", speedup "
		   << 2.0 * num_corr_upto * (last_index - first_index + 1 - 1) / least_cost
		   << (do_cart ? " (do Cartesian product)" : "") << std::endl;
	    }
	    if (least_cost_index < last_index)
	    {
	      int i0 = least_cost_index;
	      auto p0 = f(first_index, i0, deep + 1, f);
	      auto p1 = f(i0 + 1, last_index, deep + 1, f);
	      index_chunks.insert(index_chunks.end(),
				  std::make_move_iterator(std::get<0>(p0).begin()),
				  std::make_move_iterator(std::get<0>(p0).end()));
	      unique_index_chunks.insert(unique_index_chunks.end(),
					 std::make_move_iterator(std::get<1>(p0).begin()),
					 std::make_move_iterator(std::get<1>(p0).end()));
	      index_chunks.insert(index_chunks.end(),
				  std::make_move_iterator(std::get<0>(p1).begin()),
				  std::make_move_iterator(std::get<0>(p1).end()));
	      unique_index_chunks.insert(unique_index_chunks.end(),
					 std::make_move_iterator(std::get<1>(p1).begin()),
					 std::make_move_iterator(std::get<1>(p1).end()));
	      cart_prod = std::min(num_corr_upto, std::get<2>(p0) * std::get<2>(p1));
	      do_cart = (num_corr_upto * overflow_factor_for_cartesian >=
			 std::get<2>(p0) * std::get<2>(p1));
	    }
	    else
	    {
	      index_chunks.push_back({first_index, last_index + 1 - first_index});
	      unique_index_chunks.push_back(std::move(v));
	      cart_prod = num_corr_upto;
	      do_cart = false;
	    }
	    cost = least_cost;
	  }
	  cache[cache_entry] = {index_chunks, unique_index_chunks, cart_prod, cost, do_cart};
	  return {index_chunks, unique_index_chunks, cart_prod, cost, do_cart};
	};
	auto t = f(0, p.size() - 1, 0, f);
	index_chunks.insert(index_chunks.end(), std::make_move_iterator(std::get<0>(t).begin()),
			    std::make_move_iterator(std::get<0>(t).end()));
	unique_index_chunks.insert(unique_index_chunks.end(),
				   std::make_move_iterator(std::get<1>(t).begin()),
				   std::make_move_iterator(std::get<1>(t).end()));
	/// FIXME: consider cases with multiple chunks
	if (unique_index_chunks.size() == 1 || std::get<4>(t))
	  is_cart = true;
	if (unique_index_chunks.size() > 2)
	  throw std::runtime_error("get_optimal_perm: unsupported case");
      }

      // Get the number of indices
      const int num_indices = index_chunks.size();

      // Find free labels for the indices labels
      SBN::Order used_labels = get_used_labels(plan);
      SBN::Order indices_labels;
      for (int i = 0; i < num_indices; ++i)
      {
	char new_label = SBN::get_new_label(used_labels);
	used_labels.push_back(new_label);
	indices_labels.push_back(new_label);
      }

      // Sort the vertices in lexicographic order using the previous permutation
      for (std::size_t index_chunk = 0; index_chunk < index_chunks.size(); ++index_chunk)
      {
	auto& unique_index_chunk = unique_index_chunks.at(index_chunk);
	const auto [first_vertex, size_vertex] = index_chunks.at(index_chunk);
	std::vector<vec_v> k;
	k.reserve(unique_index_chunk.size());
	std::vector<vec_uv> v;
	v.reserve(unique_index_chunk.size());
	for (const auto& [ki, corr] : unique_index_chunk)
	{
	  k.push_back(ki);
	  v.push_back(get_group(first_vertex, first_vertex + size_vertex - 1, corr));
	}
	std::vector<int> perm;
	perm.reserve(unique_index_chunk.size());
	for (int i = 0; i < unique_index_chunk.size(); ++i)
	  perm.push_back(i);
	std::sort(perm.begin(), perm.end(), [&](int i, int j) { return v.at(i) < v.at(j); });
	for (int i = 0; i < unique_index_chunk.size(); ++i)
	  unique_index_chunk[k.at(perm.at(i))] = i;
      }

      // Dimensions of the corr fun tensor
      SBN::Coor perm_coor;
      SBN::IndexType max_k = 0;
      std::unordered_multimap<SBN::IndexType, std::size_t> n(16);
      if (is_cart)
      {
	for (const auto& it : unique_index_chunks)
	  perm_coor.push_back(it.size());
      }
      else
      {
	n.reserve(num_corr);
	for (std::size_t i = 0; i < num_corr; ++i)
	{
	  SBN::IndexType j = unique_index_chunks.at(0).at(get_key(
	    index_chunks.at(0).at(0), index_chunks.at(0).at(0) + index_chunks.at(0).at(1) - 1, i));
	  n.insert({j, i});
	}

	const auto unique_keys = unique_index_chunks.at(0).size();
	max_k = SBN::IndexType((num_corr + unique_keys - 1ul) / unique_keys);

	SBN::IndexType i = 0;
	for (std::size_t j = 0; j < unique_keys; ++j)
	  i += (n.count(j) + max_k - 1) / max_k;

	perm_coor = {i, max_k};
      }

      // Get the distribution: get the earliest label that has as many entries as processes
      std::string distribution;
      {
	int index = 0;
	for (int i = 0; i < indices_labels.size(); ++i)
	{
	  if (perm_coor.at(i) >= SBN::get_nprocs())
	  {
	    index = i;
	    break;
	  }
	}
	distribution = SBN::Order{indices_labels.at(index)};
      }

      // Get the index combinations
      std::vector<SBN::Order> vertex_indices_order(graph.size());
      for (int i = 0; i < num_indices; ++i)
	for (int j = 0; j < index_chunks.at(i).at(1); ++j)
	  for (const auto& pik : p.at(index_chunks.at(i).at(0) + j))
	    vertex_indices_order.at(pik) =
	      is_cart ? SBN::Order{indices_labels.at(i)}
		      : SBN::Order{indices_labels.begin(), indices_labels.begin() + i + 1};

      // Create all corr fun tensors
      const auto& perm_size = SBN::get_scoor(indices_labels, perm_coor);
      const auto& vtx_perm =
	SBN::create_tensor(perm_coor, indices_labels, distribution, SBN::Options::Alloc::Host, 0, 0,
			   SBN::Options::IsEg::False, {}, comm);
      SBN::fill_for_each_coor(vtx_perm,
			      [=](const SBN::Coor&) { return SBN::value_type((double)num_corr); });

      if (is_cart)
      {
	if (num_indices > 1)
	{
	  for (std::size_t i = 0; i < num_corr; ++i)
	  {
	    SBN::Coor coor;
	    coor.reserve(perm_coor.size());
	    for (int j = 0; j < num_indices; ++j)
	    {
	      const auto coor_j = unique_index_chunks.at(j).at(
		get_key(index_chunks.at(j).at(0),
			index_chunks.at(j).at(0) + index_chunks.at(j).at(1) - 1, i));
	      SBN::Coor slice_coor(num_indices);
	      slice_coor.at(j) = coor_j;
	      SBN::set(
		vtx_perm, slice_coor,
		SBN::value_type(corr_index_and_spin.at(i).at(0), corr_index_and_spin.at(i).at(1)));
	    }
	  }
	}
	for (std::size_t i = 0; i < num_corr; ++i)
	{
	  SBN::Coor coor;
	  coor.reserve(perm_coor.size());
	  for (int j = 0; j < num_indices; ++j)
	  {
	    coor.push_back(unique_index_chunks.at(j).at(
	      get_key(index_chunks.at(j).at(0),
		      index_chunks.at(j).at(0) + index_chunks.at(j).at(1) - 1, i)));
	  }
	  SBN::set(
	    vtx_perm, coor,
	    SBN::value_type(corr_index_and_spin.at(i).at(0), corr_index_and_spin.at(i).at(1)));
	}
      }
      else
      {
	for (SBN::IndexType j = 0, j0 = 0; j < unique_index_chunks.at(0).size(); ++j, ++j0)
	{
	  SBN::IndexType k = 0;
	  const auto range = n.equal_range(j);
	  for (auto p = range.first; p != range.second; ++p)
	  {
	    if (k >= max_k)
	    {
	      k = 0;
	      ++j0;
	    }
	    if (j0 >= perm_coor.at(0) || k >= perm_coor.at(1))
	      throw std::runtime_error("wtf");
	    const SBN::Coor& coor = {j0, k++};
	    SBN::set(vtx_perm, coor,
		     SBN::value_type(corr_index_and_spin.at(p->second).at(0),
				     corr_index_and_spin.at(p->second).at(1)));
	  }
	}
      }

      VertexIndices vertex_cf_indices;
      for (int i = 0; i < p.size(); ++i)
	vertex_cf_indices[p.at(i)] = vertex_indices_order.at(p.at(i).at(0));

      ContractionPlan new_plan;
      new_plan.reserve(plan.size());
      std::map<std::vector<int>, std::string> indices;
      for (std::size_t i = 0; i < plan.size(); ++i)
      {
	auto it = plan.at(i);
	const auto& first_index_order = vertex_cf_indices.count(it.first_tensor_indices) == 1
					  ? vertex_cf_indices.at(it.first_tensor_indices)
					  : indices.at(it.first_tensor_indices);
	const auto& second_index_order = vertex_cf_indices.count(it.second_tensor_indices) == 1
					   ? vertex_cf_indices.at(it.second_tensor_indices)
					   : indices.at(it.second_tensor_indices);
	std::set<char> result_order_set;
	result_order_set.insert(first_index_order.begin(), first_index_order.end());
	result_order_set.insert(second_index_order.begin(), second_index_order.end());
	std::string result_index_order(result_order_set.begin(), result_order_set.end());
	indices[{(int)(graph.size() + i)}] = result_index_order;

	it.first_tensor_order += first_index_order;
	it.second_tensor_order += second_index_order;
	it.result_tensor_order += result_index_order;
	new_plan.push_back(it);
      }

      return {vtx_perm, vertex_cf_indices, new_plan};
    }

    /// Group the correlation functions with similar vertices
    /// \param graph: vertices and slot numbers
    /// \param plan: contraction plan
    /// \param v: v[i][j] index for vertex j on corr fun i which is unique among all v[*][j]
    /// \param num_unique_vertices: number of unique vertices for each vertex
    /// \param comm: communicator
    /// \return tuple{perm, vertex_cf_indices, new_plan}, where
    /// - perm: arrange of the corr funs indices into a tensor
    /// - vertex_cf_indices: indices for each vertex
    /// - new_plan: plan with corr function indices set
    ///
    /// NOTE: trivial implementation of `get_optimal_perm`, it can be used for debugging

    inline std::tuple<SBN::Tensor, VertexIndices, ContractionPlan>
    get_trivial_perm(const std::vector<GraphVertex>& graph, const ContractionPlan& plan,
		     const std::vector<std::vector<SBN::IndexType>>& v, const SBN::Comm& comm)
    {
      Datalib::TensorStorage::detail::Tracker t_("get_trivial_perm");

      // Get trivial cases
      if (v.size() == 0 || v.at(0).size() == 0 || plan.size() == 0)
	return {{}, {}, {}};

      SBN::Order used_labels = get_used_labels(plan);
      const char i_label = SBN::get_new_label(used_labels);

      const int num_vertices = v.at(0).size();
      const auto num_corr = v.size();
      auto perm = SBN::create_tensor(SBN::Coor{(SBN::IndexType)num_corr}, SBN::Order{i_label},
				     SBN::Order{i_label}, SBN::Options::Alloc::Host, 0, 0,
				     SBN::Options::IsEg::False, {}, comm);
      SBN::fill_for_each_coor(perm,
			      [=](const SBN::Coor& c) { return SBN::value_type((double)c.at(0)); });

      VertexIndices vertex_cf_indices;
      for (int i = 0; i < num_vertices; ++i)
	vertex_cf_indices[{i}] = SBN::Order{i_label};

      ContractionPlan new_plan;
      new_plan.reserve(plan.size());
      for (std::size_t i = 0; i < plan.size(); ++i)
      {
	auto it = plan.at(i);
	it.first_tensor_order += SBN::Order{i_label};
	it.second_tensor_order += SBN::Order{i_label};
	it.result_tensor_order += SBN::Order{i_label};
	new_plan.push_back(it);
      }

      return {perm, vertex_cf_indices, new_plan};
    }

    /// Return the volume of the given tensor across all processes
    /// \param t: given tensor

    inline std::size_t total_volume_among_processes(const SBN::Tensor& t)
    {
      std::size_t vol = 0;
      for (const auto& [from, size] : SBN::get_distribution(t))
	vol += SBN::volume(size);
      return vol;
    }

    /// Return the quotient of the volume of the given tensor across all processes and the tensor volume
    /// \param t: given tensor

    inline double get_tensor_distribution_efficiency(const SBN::Tensor& t)
    {
      return (double)total_volume_among_processes(t) / SBN::volume(t.size);
    }

    /// Print an analysis of the common vertices for each graph vertex
    /// \param graph: vertices and slot numbers
    /// \param plan: contraction plan
    /// \param batch_size_for_cache: maximum elements for each corr fun index
    /// \param batch_size_for_indices: maximum elements for each corr fun index
    /// \param batch_size_for_slides: maximum elements for slide
    /// \param num_corr: total number of corr fun
    /// \param cout: output stream

    template <typename stream>
    inline void
    print_plan(const std::vector<GraphVertex>& graph, const ContractionPlan& plan,
	       const VertexIndices& vertex_cf_indices, const SBN::SCoor& batch_size_for_cache,
	       const SBN::SCoor& batch_size_for_indices, const SBN::SCoor& batch_size_for_slides,
	       std::size_t num_corr, const SBN::Tensor& perm, stream& cout)
    {
      const auto& scoor_indices = SBN::get_scoor(perm.order, perm.size);

      std::map<char, char> print_labels;
      char l = 'i';
      for (const auto& it : scoor_indices)
	print_labels[it.first] = l++;

      if (SBN::get_rank(SBN::get_comm(perm).comm) == 0)
      {
	cout << "  contraction plan:" << std::endl;
	std::size_t total_tensors = 0, num_vertices = 0;
	for (const auto& pi : get_vertices_visiting_order(plan, graph.size()))
	{
	  total_tensors += SBN::volume(vertex_cf_indices.at(pi), scoor_indices) * pi.size();
	  cout << "    vertex=";
	  for (int i : pi)
	  {
	    cout << " (" << i << ") " << graph.at(i).type;
	    for (int slot : graph.at(i).slot_indices)
	      cout << ":" << slot;
	  }
	  cout << ", has indices= ";
	  for (char label : vertex_cf_indices.at(pi))
	  {
	    cout << print_labels.at(label) << "= " << scoor_indices.at(label) << " in batches of "
		 << batch_size_for_indices.at(label);
	    if (batch_size_for_cache.count(label) > 0)
	      cout << " (with cache batches of " << batch_size_for_cache.at(label) << "), ";
	    else
	      cout << ", ";
	  }
	  cout << std::endl;
	  num_vertices += pi.size();
	}
	cout << "    total number of tensors: " << total_tensors
	     << "    avg re-usability= " << (double)num_corr * num_vertices / total_tensors
	     << std::endl;
      }
    }

    /// Update the contraction plan considering that some indices will stay the same
    /// \param graph: vertices and slot numbers
    /// \param plan: contraction plan
    /// \param vertex_cf_indices: indices for each vertex
    /// \param perm: arrange of the corr funs indices into a tensor
    /// \param prev_perm_from: first coordinate of `perm` computed last time
    /// \param prev_perm_size: size of `perm` computed last time
    /// \param current_perm_from: first coordinate of `perm` to compute now
    /// \param current_perm_size: size of `perm` to compute now
    /// \param next_perm_from: first coordinate of `perm` to compute next time
    /// \param next_perm_size: size of `perm` to compute next time
    /// \return {do_skip, do_cache}: where
    ///   - do_skip[i] is true if the vertex or the contraction is not needed
    ///   - do_cache[i] is true if the result will be needed on the next iteration

    inline std::tuple<std::vector<bool>, std::vector<bool>> get_skip_and_cache_for_the_plan(
      const std::vector<GraphVertex>& graph, const ContractionPlan& plan,
      const VertexIndices& vertex_cf_indices, const SBN::Tensor& perm,
      const SBN::Coor& prev_perm_from, const SBN::Coor& prev_perm_size,
      const SBN::Coor& current_perm_from, const SBN::Coor& current_perm_size,
      const SBN::Coor& next_perm_from, const SBN::Coor& next_perm_size,
      const std::vector<bool>& prev_cache)
    {
      if (perm.size.size() != prev_perm_from.size() || perm.size.size() != prev_perm_size.size() ||
	  perm.size.size() != current_perm_from.size() ||
	  perm.size.size() != current_perm_size.size() ||
	  perm.size.size() != next_perm_from.size() || perm.size.size() != next_perm_size.size() ||
	  prev_cache.size() != graph.size() + plan.size())
	throw std::runtime_error("wtf");
      auto overlap_directions = [](const auto& order, const auto& size, const auto& from0,
				   const auto& size0, const auto& from1, const auto& size1) {
	std::map<char, bool> r;
	for (std::size_t i = 0; i < order.size(); ++i)
	  r[order.at(i)] =
	    (SBN::norm_coor(from0.at(i), size.at(i)) == SBN::norm_coor(from1.at(i), size.at(i)) &&
	     size0.at(i) == size1.at(i));
	return r;
      };
      const auto& was_cache =
	overlap_directions(perm.order, perm.size, prev_perm_from, prev_perm_size, current_perm_from,
			   current_perm_size);
      const auto& will_cache =
	overlap_directions(perm.order, perm.size, next_perm_from, next_perm_size, current_perm_from,
			   current_perm_size);

      // Stored whether a vertex/intermediate result has the same `perm` slice as before
      std::vector<bool> should_have_been_cached(graph.size() + plan.size(), false);
      // Stored whether a vertex/intermediate result will have the same `perm` slice as the current
      std::vector<bool> can_be_cached(graph.size() + plan.size(), false);

      // Check if the vertex should have been cached and can be cached
      for (std::size_t i = 0; i < graph.size(); ++i)
      {
	if (vertex_cf_indices.count({(int)i}) == 0)
	  continue;

	const auto& result_index_order = vertex_cf_indices.at({(int)i});

	// Skip the contraction if all the dimensions are cache
	bool is_result_on_cache = true;
	for (char label : result_index_order)
	  if (was_cache.count(label) == 0 || !was_cache.at(label))
	    is_result_on_cache = false;
	should_have_been_cached.at(i) = is_result_on_cache;

	// Cache the contraction if all the dimensions will be cached
	bool do_cache_result = true;
	for (char label : result_index_order)
	  if (will_cache.count(label) == 0 || !will_cache.at(label))
	    do_cache_result = false;
	can_be_cached.at(i) = do_cache_result;
      }

      // Check if the intermediate contraction results should have been cached and can be cached
      std::map<std::vector<int>, std::string> indices;
      for (std::size_t i = 0; i < plan.size(); ++i)
      {
	auto it = plan.at(i);
	const auto& first_index_order = vertex_cf_indices.count(it.first_tensor_indices) == 1
					  ? vertex_cf_indices.at(it.first_tensor_indices)
					  : indices.at(it.first_tensor_indices);
	const auto& second_index_order = vertex_cf_indices.count(it.second_tensor_indices) == 1
					   ? vertex_cf_indices.at(it.second_tensor_indices)
					   : indices.at(it.second_tensor_indices);
	std::set<char> result_order_set;
	result_order_set.insert(first_index_order.begin(), first_index_order.end());
	result_order_set.insert(second_index_order.begin(), second_index_order.end());
	std::string result_index_order(result_order_set.begin(), result_order_set.end());
	indices[{(int)(graph.size() + i)}] = result_index_order;

	// Skip the contraction if all the dimensions are cache
	bool is_result_on_cache = true;
	for (char label : result_index_order)
	  if (was_cache.count(label) == 0 || !was_cache.at(label))
	    is_result_on_cache = false;
	should_have_been_cached.at(graph.size() + i) = is_result_on_cache;

	// Cache the contraction if all the dimensions will be cached
	bool do_cache_result = true;
	for (char label : result_index_order)
	  if (will_cache.count(label) == 0 || !will_cache.at(label))
	    do_cache_result = false;
	can_be_cached.at(graph.size() + i) = do_cache_result;
      }

      // Skip a computation if is the result should have been cached and has been cached
      std::vector<bool> do_skip(graph.size() + plan.size(), true);
      {
	std::vector<int> to_visit;
	to_visit.reserve(graph.size() + plan.size());
	to_visit.push_back(graph.size() + plan.size() - 1);
	while (to_visit.size() > 0)
	{
	  const auto index = to_visit.back();
	  to_visit.pop_back();
	  if (do_skip.at(index) && (!prev_cache.at(index) || !should_have_been_cached.at(index)))
	  {
	    do_skip.at(index) = false;
	    if (index >= graph.size())
	    {
	      to_visit.push_back(plan.at(index - graph.size()).first_tensor_indices.at(0));
	      to_visit.push_back(plan.at(index - graph.size()).second_tensor_indices.at(0));
	    }
	  }
	}
      }

      // Cached a result if it will be used in the future and either it is computed or it has been cached
      std::vector<bool> do_cache(graph.size() + plan.size(), false);
      {
	std::vector<bool> visited(graph.size() + plan.size(), false);
	std::vector<int> to_visit;
	to_visit.reserve(graph.size() + plan.size());
	to_visit.push_back(graph.size() + plan.size() - 1);
	while (to_visit.size() > 0)
	{
	  const auto index = to_visit.back();
	  to_visit.pop_back();
	  visited.at(index) = true;
	  do_cache.at(index) =
	    can_be_cached.at(index) & (!do_skip.at(index) | prev_cache.at(index));

	  if (!do_cache.at(index) && index >= graph.size())
	  {
	    if (!visited.at(plan.at(index - graph.size()).first_tensor_indices.at(0)))
	      to_visit.push_back(plan.at(index - graph.size()).first_tensor_indices.at(0));
	    if (!visited.at(plan.at(index - graph.size()).second_tensor_indices.at(0)))
	      to_visit.push_back(plan.at(index - graph.size()).second_tensor_indices.at(0));
	  }
	}
      }

      return {do_skip, do_cache};
    }

    /// Print an analysis of the common vertices for each graph vertex
    /// \param graph: vertices and slot numbers
    /// \param plan: contraction plan
    /// \param do_skip: the ith is true if the vertex or the contraction is not needed
    /// \param do_cache: the ith is true if the result will be needed on the next iteration
    /// \param cout: output stream
    /// \param comm: communicator

    template <typename stream>
    inline void print_plan_details(const std::vector<GraphVertex>& graph,
				   const ContractionPlan& plan, const std::vector<bool>& do_skip,
				   const std::vector<bool>& do_cache, stream& cout,
				   const SBN::Comm& comm)
    {
      auto print_aux = [&](int index) {
	cout << index;
	if (do_skip.at(index) && do_cache.at(index))
	  cout << "~";
	else if (do_skip.at(index))
	  cout << ")";
	else if (do_cache.at(index))
	  cout << "(";
      };
      cout << "  detail contraction plan for proc=" << SBN::get_rank(comm.comm) << ":";
      for (std::size_t i = 0; i < plan.size(); ++i)
      {
	if (do_skip.at(graph.size() + i))
	  continue;
	cout << " ";
	print_aux(plan.at(i).first_tensor_indices.at(0));
	cout << "-";
	print_aux(plan.at(i).second_tensor_indices.at(0));
	cout << "=>";
	print_aux(graph.size() + i);
      }
      cout << std::endl;
    }

    /// Return the source phasing for mesons
    /// \param v: disp vertex

    inline XMLArray::Array<int> get_meson_phasing_source(const Hadron::HadronDispVertex_t& v)
    {
      return get_phasing(v, 1);
    }

    /// Return the sink phasing for mesons
    /// \param v: disp vertex

    inline XMLArray::Array<int> get_meson_phasing_sink(const Hadron::HadronDispVertex_t& v)
    {
      return get_phasing(v, 0);
    }

    /// Return the phasing for baryons
    /// \param v: disp vertex

    inline XMLArray::Array<int> get_baryon_phasing(const Hadron::HadronDispVertex_t& v)
    {
      // NOTE: all quarks have the phasing
      return get_phasing(v, 0);
    }

    /// Return the same value but blows up if there is a zero

    inline std::vector<int> checkZero(const std::vector<int>& v)
    {
      for (int i : v)
	if (i == 0)
	  throw std::runtime_error("wtf");
      return v;
    }

    /// Return the maximum value among all processes
    /// \param v: local value

    template <typename T>
    inline T gather_max(const T& v, const SBN::MpiComm& comm)
    {
      const auto& a = SBN::gather(v, comm);
      return std::accumulate(a.begin(), a.end(), a.at(0), [](T a, T b) { return std::max(a, b); });
    }

    /// Gather tensor axes in the same order as do_perm(origin_slots, perm).
    /// \param t: input tensor
    /// \param order: original order
    /// \param perm: target slot i comes from original slot perm[i]

    inline SBN::Tensor apply_vertex_perm(const SBN::Tensor& t, const SBN::Coor& perm,
					 const SBN::Order& order)
    {
      if (order.size() != perm.size())
	throw std::runtime_error("apply_vertex_perm: invalid input");
      SBN::remap m;
      for (int i = 0; i < order.size(); ++i)
	m[order.at(perm.at(i))] = order.at(i);
      return SBN::relabel(t, m);
    }

    /// Normalize a baryon key after gathering quark slots in `perm` order.
    /// The elemental equals the gathered-key elemental times the color sign.
    inline std::tuple<Hadron::KeyBaryonElementalOperator_t, int>
    permute_baryon_key(const Hadron::KeyBaryonElementalOperator_t& key,
                      const SBN::Coor& perm)
    {
      auto sorted = perm;
      std::sort(sorted.begin(), sorted.end());
      if (sorted != SBN::Coor{0, 1, 2})
        throw std::runtime_error("permute_baryon_key: invalid slot permutation");
      const std::array<std::vector<int>, 3> paths{key.left, key.middle, key.right};
      auto result = key;
      result.left = paths.at(perm.at(0));
      result.middle = paths.at(perm.at(1));
      result.right = paths.at(perm.at(2));
      int sign = 1;
      for (int i = 0; i < 3; ++i)
        for (int j = i + 1; j < 3; ++j)
          if (perm.at(i) > perm.at(j)) sign = -sign;
      return {result, sign};
    }

    /// Return the starting coordinate and the size for a given dimension
    using Slides = std::map<char, std::array<int, 2>>;

    /// Return the mesons without spins
    /// \param db: meson storage
    /// \param meson_keys: list of mesons keys
    /// \param perms: list of permutations, one for each meson key
    /// \param ev_from: first eigenvector to return
    /// \param ev_size: number of eigenvectors to return
    /// \param dist_labels: dimensions to be distributed, some of "vwi"

    inline SBN::Tensor get_meson_elementals_from_storage(
      const Datalib::TensorStorage::StorageMeson& db,
      const std::vector<Hadron::KeyMesonElementalOperator_t>& meson_keys,
      const std::vector<SBN::Coor>& perms, const std::vector<bool>& do_conj,
      const SBN::Coor& ev_from, const SBN::Coor& ev_size, const std::string& dist_labels,
      const SBN::Tensor& perm)
    {
      if (meson_keys.size() != perms.size())
	throw std::runtime_error("invalid input");
      if (ev_from.size() != 2 || ev_size.size() != 2)
	throw std::runtime_error("invalid input");
      if (SBN::detail::get_debug_level() > 0)
      {
	for (const auto& it : perms)
	{
	  if (it.size() != 2)
	    throw std::runtime_error("invalid input");
	  for (const auto& i : it)
	    if (i < 0 || i >= 2)
	      throw std::runtime_error("invalid input");
	}
      }

      // Unsupported to conjugate the mesons
      for (bool it : do_conj)
	if (it)
	  throw std::runtime_error("get_meson_elementals_from_storage: unsupported case");

      SBN::Tensor mesons = SBN::create_tensor_with_local_components(
	SBN::concat(ev_size, {-(int)meson_keys.size()}), "vwi", dist_labels,
	SBN::Options::Alloc::Device, 0, 0, SBN::Options::IsEg::False, perm);
      const auto& local_mesons = SBN::slice_kv(
	get_local_tensor(mesons), SBN::slice_ranges(SBN::get_local_srange(mesons), "i"));
      const int num_vecs = std::max(ev_from.at(0) + ev_size.at(0), ev_from.at(1) + ev_size.at(1));
      for (std::size_t i = 0; i < meson_keys.size(); ++i)
      {
	const auto& meson_i = SBN::slice_kv(local_mesons, {{'i', i}}, {{'i', 1}});
	if (!has_local_support(meson_i))
	  continue;
	const auto& key = meson_keys.at(i);
	Hadron::ValMesonElementalOperator_t val;
	if (db.get(key, val) != 0)
	{
	  std::stringstream ss;
	  ss << "failed to get the meson with key= " << key;
	  throw std::runtime_error(ss.str());
	}
	if (val.op.size1() < num_vecs || val.op.size2() < num_vecs)
	  throw std::runtime_error("got a meson with insufficient number of vectors");
	auto ti = SBN::toTensor(val.op, "vw");
	ti = apply_vertex_perm(ti, perms.at(i), "vw");
	ti = SBN::slice_kv(ti, SBN::get_scoor("vw", ev_from), SBN::get_scoor("vw", ev_size));
	SBN::copyTo(ti, meson_i);
      }

      return mesons;
    }

    /// Return a tensor with all the requested mesons
    /// \param get_meson_elementals: callback to get the meson elementals
    /// \param coeff_map: coeff map
    /// \param unique_vertices: vector of disp vertices
    /// \param vertices: unique_vertices[vertices[i][j]] disp vertex for vertex j for corr fun i
    /// \param slots_perm: do_perm(origin_slots, slots_perm[i]) -> target_slots
    /// \param t: time[i][j] time slice for vertex j for corr fun i
    /// \param vertex_index: graph vertex index to get the baryons
    /// \param perm: indices of vertices and t to visit
    /// \param num_vecs: rank of the distillation basis
    /// \param slides: slices of the distillation basis for some dimensions

    inline std::tuple<SBN::Tensor, SBN::Tensor, std::string>
    get_mesons(const meson_elementals_callback get_meson_elementals, const CoeffMap& coeff_map,
	       const std::vector<Hadron::HadronDispVertex_t>& unique_vertices,
	       const std::vector<std::vector<std::size_t>>& vertices,
	       const std::vector<std::vector<SBN::Coor>>& slots_perm,
	       const std::vector<std::vector<int>>& t, const std::vector<int>& vertex_indices,
	       const SBN::Tensor& perm, int num_vecs, const Slides& slides)
    {
      Datalib::TensorStorage::detail::Tracker t_("get_mesons");

      // List of labels:
      // - i: corr fun index
      // - vw: meson distillation basis indices
      // - rs: meson spin indices
      // - j: unique meson index
      // - k: unique time slice and vertex index
      // Intended contraction:
      //   mesons(vwj) * spins(krs|j) * table(iI|k) -> return(vwrsiI)
      // NOTE: tensor notation: <tensor name>(<dense labels>[|<sparse labels>])

      const char j_label = SBN::get_new_label(std::string("vwrsI") + perm.order);
      const char k_label =
	SBN::get_new_label(std::string("vwrsI") + perm.order + std::string{j_label});

      // The correlation functions are going to be distributed among all processes, but in case
      // the baryons are too big, then the baryons dimensions with slides will be distributed instead
      std::string meson_dist_labels; ///< distributed labels for the mesons (empty for now)

      // Extend slides for all dimensions
      Slides sl;
      for (char c : std::string("vw"))
	sl[c] = slides.count(c) > 0 ? slides.at(c) : std::array<int, 2>{0, num_vecs};

      // Index of unique meson keys
      using meson_key = std::tuple<Hadron::KeyMesonElementalOperator_t, SBN::Coor>;
      const auto num_corr_local = SBN::volume(SBN::get_local_tensor_tight(perm).size);
      unordered_map<meson_key, SBN::IndexType> meson_index(num_corr_local);
      SBN::IndexType num_mesons = 0; ///< number of unique meson keys

      // Index of unique combinations of time and HadronDispVertex_t indices
      using th = std::tuple<int, std::size_t, SBN::Coor>; ///< time, unique vertex index and perm
      unordered_map<th, SBN::IndexType> th_combos(num_corr_local);
      SBN::IndexType num_th_combos =
	0; ///< number of unique combinations of time slice and unique vertex index

      // Construct the list of needed mesons
      SBN::IndexType max_nnz_for_spin = 0;
      SBN::for_each_coor_value(perm, [&](const SBN::Coor&, const SBN::value_type& i_cmplx) {
	const SBN::IndexType i = std::real(i_cmplx);

	// Ignore blank entries
	if (i >= t.size())
	  return;

	for (const int vertex_index : vertex_indices)
	{
	  const auto unique_vertex = vertices.at(i).at(vertex_index);
	  const auto& p = slots_perm.at(i).at(vertex_index);
	  const auto t_slice = t.at(i).at(vertex_index);
	  if (th_combos.count({t_slice, unique_vertex, p}) > 0)
	    continue;
	  th_combos[{t_slice, unique_vertex, p}] = num_th_combos++;

	  const Hadron::HadronDispVertex_t& h = unique_vertices.at(unique_vertex);
	  const auto& k1 = coeff_map[h].begin()->second;
	  std::array<SBN::IndexType, Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count> nnz{{}};
	  for (auto k2 = k1.begin(); k2 != k1.end(); ++k2)
	  {
	    Hadron::KeyMesonElementalOperator_t key;
	    key.t_slice = t_slice;
	    // Flip mom to agree with FT convention \int dx exp(-ipx)
	    // And flip mom when creation operator
	    key.mom = !h.creation_op ? -h.mom : h.mom;
	    key.phasing_source = get_meson_phasing_source(h);
	    key.phasing_sink = -get_meson_phasing_sink(h);
	    key.displacement = checkZero(k2->first.derivs[2].deriv);

	    const auto& k = meson_key{key, p};
	    if (meson_index.count(k) == 0)
	      meson_index[k] = num_mesons++;
	    for (auto k3 = k2->second.begin(); k3 != k2->second.end(); ++k3)
	    {
	      int ls = k3->first.diracs[p.at(0) + 1];
	      int rs = k3->first.diracs[p.at(1) + 1];
	      nnz[ls + Datalib::TensorStorage::spin_count * rs]++;
	    }
	  }
	  for (const auto it : nnz)
	    max_nnz_for_spin = std::max(max_nnz_for_spin, it);
	}
      });

      max_nnz_for_spin = gather_max(max_nnz_for_spin, SBN::get_mpi_comm(perm));

      // Get all unique mesons
      std::vector<Hadron::KeyMesonElementalOperator_t> unique_meson_keys(meson_index.size());
      std::vector<SBN::Coor> unique_perms(meson_index.size());
      std::vector<bool> do_conj(meson_index.size(), false);
      for (const auto& it : meson_index)
      {
	const auto& [key, p] = it.first;
	unique_meson_keys.at(it.second) = key;
	unique_perms.at(it.second) = p;
      }
      auto mesons = SBN::relabel(get_meson_elementals(unique_meson_keys, unique_perms, do_conj,
						      {sl.at('v').at(0), sl.at('w').at(0)},
						      {sl.at('v').at(1), sl.at('w').at(1)},
						      meson_dist_labels + std::string{'i'}, perm),
				 {{'i', j_label}});
      const auto expected_size = SBN::SCoor{{'v', sl.at('v').at(1)},
					    {'w', sl.at('w').at(1)},
					    {j_label, (SBN::IndexType)meson_index.size()}};
      if (!is_compatible(expected_size, SBN::get_comm(perm), mesons))
	throw std::runtime_error("get_mesons: invalid returned tensor from meson's callback");
      mesons = make_compatible(expected_size, mesons);

      // Create the table to decorate the mesons with spins
      SBN::Tensor spins = SBN::create_tensor_with_local_components(
	SBN::Coor{-num_th_combos, slides.at('r')[1], slides.at('s')[1], -num_mesons},
	std::string{k_label} + std::string("rs") + std::string{j_label},
	meson_dist_labels + std::string{k_label, j_label}, SBN::Options::Alloc::Host,
	1 /* j is sparse */, max_nnz_for_spin, SBN::Options::IsEg::False, perm);
      const auto& local_spins = SBN::slice_kv(
	SBN::get_local_tensor(spins),
	SBN::slice_ranges(SBN::get_local_srange(spins), std::string{k_label, j_label}));
      SBN::set_zero(spins);
      for (const auto& it : th_combos)
      {
	const auto [t_slice, unique_vertex, p] = it.first;
	const SBN::IndexType k = it.second;
	const Hadron::HadronDispVertex_t& h = unique_vertices.at(unique_vertex);
	const auto& k1 = coeff_map[h].begin()->second;
	for (auto k2 = k1.begin(); k2 != k1.end(); ++k2)
	{
	  Hadron::KeyMesonElementalOperator_t key;
	  key.t_slice = t_slice;
	  // Flip mom to agree with FT convention \int dx exp(-ipx)
	  // And flip mom when creation operator
	  key.mom = !h.creation_op ? -h.mom : h.mom;
	  key.phasing_source = get_meson_phasing_source(h);
	  key.phasing_sink = -get_meson_phasing_sink(h);
	  key.displacement = checkZero(k2->first.derivs[2].deriv);
	  SBN::IndexType j = meson_index.at({key, p});
	  for (auto k3 = k2->second.begin(); k3 != k2->second.end(); ++k3)
	  {
	    int ls = k3->first.diracs[p.at(0) + 1];
	    int rs = k3->first.diracs[p.at(1) + 1];
	    // NOTE: do NOT conjugate the weight for creation operator
	    auto weight = k3->second;
	    SBN::set(local_spins, SBN::Coor{k, ls, rs, j}, weight);
	  }
	}
      }

      // Construct the table of linear combinations from the unique mesons to the
      // required by the corr funs
      SBN::Tensor table = SBN::create_tensor_with_local_components(
	SBN::concat(perm.size, SBN::Coor{(int)vertex_indices.size(), -num_th_combos}),
	perm.order + std::string{'I', k_label},
	SBN::get_distribution_labels(perm) + std::string{k_label}, SBN::Options::Alloc::Host,
	1 /* k is sparse */, 1 /* one nnz */, SBN::Options::IsEg::False, perm);
      const auto& local_table =
	SBN::slice_kv(SBN::get_local_tensor(table),
		      SBN::slice_ranges(SBN::get_local_srange(table), std::string{k_label}));
      SBN::set_zero(table);
      SBN::for_each_coor_value(perm, [&](const SBN::Coor& coor, const SBN::value_type& i_cmplx) {
	const SBN::IndexType i = std::real(i_cmplx);

	// Ignore blank entries
	if (i >= t.size())
	  return;

	for (SBN::IndexType I = 0; I < vertex_indices.size(); ++I)
	{
	  const int vertex_index = vertex_indices.at(I);
	  const auto unique_vertex = vertices.at(i).at(vertex_index);
	  const auto& p = slots_perm.at(i).at(vertex_index);
	  const auto t_slice = t.at(i).at(vertex_index);
	  SBN::IndexType k = th_combos.at({t_slice, unique_vertex, p});
	  SBN::set(local_table, SBN::concat(coor, SBN::Coor{I, k}), 1.0);
	}
      });

      // Move mesons, spins and table to gpu
      mesons = SBN::make_sure(mesons, {}, SBN::Options::Distribution::Default,
			      SBN::Options::Alloc::Device);
      spins =
	SBN::make_sure(spins, {}, SBN::Options::Distribution::Default, SBN::Options::Alloc::Device);
      table =
	SBN::make_sure(table, {}, SBN::Options::Distribution::Default, SBN::Options::Alloc::Device);

      // Do contraction
      const auto& mesons_spins = SBN::contract(SBN::move(mesons), SBN::move(spins),
					       std::string("vwrs") + std::string{k_label});

      return {mesons_spins, table, std::string("vwrsI") + perm.order};
    }

    /// Return the baryons without spins
    /// \param db: baryon storage
    /// \param baryon_keys: list of baryons keys
    /// \param perms: list of permutations, one for each baryon key
    /// \param do_conj: list of whether to conjugate the elemental, one for each baryon key
    /// \param ev_from: first eigenvector to return
    /// \param ev_size: number of eigenvectors to return
    /// \param dist_labels: dimensions to be distributed, some of "vwxi"

    inline SBN::Tensor get_baryon_elementals_from_storage(
      const Datalib::TensorStorage::StorageBaryon& db,
      const std::vector<Hadron::KeyBaryonElementalOperator_t>& baryon_keys,
      const std::vector<SBN::Coor>& perms, const std::vector<bool>& do_conj,
      const SBN::Coor& ev_from, const SBN::Coor& ev_size, const std::string& dist_labels,
      const SBN::Tensor& perm)
    {
      if (baryon_keys.size() != perms.size() || baryon_keys.size() != do_conj.size())
	throw std::runtime_error("invalid input");
      if (ev_from.size() != 3 || ev_size.size() != 3)
	throw std::runtime_error("invalid input");
      if (SBN::detail::get_debug_level() > 0)
      {
	for (const auto& it : perms)
	{
	  if (it.size() != 3)
	    throw std::runtime_error("invalid input");
	  for (const auto& i : it)
	    if (i < 0 || i >= 3)
	      throw std::runtime_error("invalid input");
	}
      }

      // Get all unique baryons
      SBN::Tensor baryons = SBN::create_tensor_with_local_components(
	SBN::concat(ev_size, {-(int)baryon_keys.size()}), "vwxi", dist_labels,
	SBN::Options::Alloc::Host, 0, 0, SBN::Options::IsEg::False, perm);
      const auto& local_baryons = SBN::slice_kv(
	SBN::get_local_tensor(baryons), SBN::slice_ranges(SBN::get_local_srange(baryons), "i"));
      const int num_vecs =
	std::max(std::max(ev_from.at(0) + ev_size.at(0), ev_from.at(1) + ev_size.at(1)),
		 ev_from.at(2) + ev_size.at(2));
      Datalib::Storage::SerialDBData<Hadron::ValBaryonElementalOperator_t> val;
      for (std::size_t i = 0; i < baryon_keys.size(); ++i)
      {
	const auto& baryon_i = SBN::slice_kv(local_baryons, {{'i', i}}, {{'i', 1}});
	if (!has_local_support(baryon_i))
	  continue;
	const auto& key = baryon_keys.at(i);
	if (db.get(key, val) != 0)
	{
	  std::stringstream ss;
	  ss << "failed to get the baryon with key= " << key;
	  throw std::runtime_error(ss.str());
	}
	if (val.data().op.size1() < num_vecs || val.data().op.size2() < num_vecs ||
	    val.data().op.size3() < num_vecs)
	  throw std::runtime_error("got a baryon with insufficient number of vectors");
	auto ti = SBN::toTensor(val.data().op, "vwx");
	ti = apply_vertex_perm(ti, perms.at(i), "vwx");
	ti = SBN::slice_kv(ti, SBN::get_scoor("vwx", ev_from), SBN::get_scoor("vwx", ev_size));
	SBN::copyTo(do_conj.at(i) ? SBN::conj(ti) : ti, baryon_i);
      }

      return baryons;
    }

    /// Return a tensor with all the requested baryons
    /// \param get_baryon_elementals: callback to get the baryon elementals
    /// \param graph: vertices and slot numbers
    /// \param coeff_map: coeff map
    /// \param unique_vertices: vector of disp vertices
    /// \param vertices: unique_vertices[vertices[i][j]] disp vertex for vertex j for corr fun i
    /// \param slots_perm: do_perm(origin_slots, slots_perm[i]) -> target_slots
    /// \param t: time[i][j] time slice for vertex j for corr fun i
    /// \param spins_from_unique_vertex_index: return the list of spin combination for a vertex
    /// \param vertex_index: graph vertex index to get the baryons
    /// \param perm: indices of vertices and t to visit
    /// \param num_vecs: default rank of the distillation basis
    /// \param slides: slices of the distillation basis for some dimensions

    inline std::tuple<SBN::Tensor, SBN::Tensor, std::string> get_baryons(
      const baryon_elementals_callback get_baryon_elementals, const std::vector<GraphVertex>& graph,
      const CoeffMap& coeff_map, const std::vector<Hadron::HadronDispVertex_t>& unique_vertices,
      const std::vector<std::vector<std::size_t>>& vertices,
      const std::vector<std::vector<SBN::Coor>>& slots_perm, const std::vector<std::vector<int>>& t,
      const SpinsFromUniqueVertexIndex& spins_from_unique_vertex_index,
      const std::vector<int>& vertex_indices, const SBN::Tensor& perm, int num_vecs,
      const Slides& slides)
    {
      Datalib::TensorStorage::detail::Tracker t_("get_baryons");

      // List of labels:
      // - i: corr fun index
      // - vwx: baryon distillation basis indices
      // - rst: baryon spin indices
      // - j: unique baryon index
      // - k: unique time slice and vertex index
      // Intended contraction:
      //   baryons(vwxj) * table(iIrst|j) -> return(vwxrstiI)
      // NOTE: tensor notation: <tensor name>(<dense labels>[|<sparse labels>])

      const char j_label = SBN::get_new_label(std::string("vwxrstI") + perm.order);
      const char k_label =
	SBN::get_new_label(std::string("vwxrstI") + perm.order + std::string{j_label});

      // The correlation functions are going to be distributed among all processes, but in case
      // the baryons are too big, then the baryons dimensions with slides will be distributed instead
      std::string baryon_dist_labels; ///< distributed labels for the baryons (empty for now)

      // Extend slides for all dimensions
      Slides sl;
      for (char c : std::string("vwx"))
	sl[c] = slides.count(c) > 0 ? slides.at(c) : std::array<int, 2>{0, num_vecs};

      // Index of unique baryons keys
      using KeyPermAndConj = std::tuple<Hadron::KeyBaryonElementalOperator_t, SBN::Coor, bool>;
      const auto num_corr_local = SBN::volume(SBN::get_local_tensor_tight(perm).size);
      unordered_map<KeyPermAndConj, SBN::IndexType> baryon_index(num_corr_local);
      SBN::IndexType num_baryons = 0;

      // Index of unique combinations of time and HadronDispVertex_t indices
      using th = std::tuple<int, std::size_t, SBN::Coor>; ///< time, unique vertex index and perm
      unordered_map<th, SBN::IndexType> th_combos(num_corr_local);
      SBN::IndexType num_th_combos =
	0; ///< number of unique combinations of time slice and unique vertex index
      int max_elementals_per_spin = 0;

      // Construct the list of needed baryons
      SBN::for_each_coor_value(perm, [&](const SBN::Coor&, const SBN::value_type& i_cmplx) {
	const SBN::IndexType i = std::real(i_cmplx);

	// Ignore blank entries
	if (i >= t.size())
	  return;

	for (const int vertex_index : vertex_indices)
	{
	  const auto unique_vertex = vertices.at(i).at(vertex_index);
	  const auto& p = slots_perm.at(i).at(vertex_index);
	  const auto t_slice = t.at(i).at(vertex_index);
	  if (th_combos.count({t_slice, unique_vertex, p}) > 0)
	    continue;
	  th_combos[{t_slice, unique_vertex, p}] = num_th_combos++;

	  const Hadron::HadronDispVertex_t& h = unique_vertices.at(unique_vertex);
	  const auto& k1 = coeff_map[h].begin()->second;
	  int n = 0;
	  for (auto k2 = k1.begin(); k2 != k1.end(); ++k2)
	  {
	    Hadron::KeyBaryonElementalOperator_t key;
	    key.t_slice = t_slice;
	    // Flip mom to agree with FT convention \int dx exp(-ipx)
	    key.mom = -h.mom;
	    key.phasing = get_baryon_phasing(h);
	    key.left = checkZero(k2->first.derivs[1].deriv);
	    key.middle = checkZero(k2->first.derivs[2].deriv);
	    key.right = checkZero(k2->first.derivs[3].deriv);
	    auto k = KeyPermAndConj{key, p, h.creation_op};
	    if (baryon_index.count(k) == 0)
	      baryon_index[k] = num_baryons++;
	    ++n; // Each derivative elemental contributes at most once to a spin row.
	  }
	  max_elementals_per_spin = std::max(max_elementals_per_spin, n);
	}
      });

      max_elementals_per_spin = gather_max(max_elementals_per_spin, SBN::get_mpi_comm(perm));

      // Get all unique baryons
      std::vector<Hadron::KeyBaryonElementalOperator_t> unique_baryon_keys(baryon_index.size());
      std::vector<SBN::Coor> unique_perms(baryon_index.size());
      std::vector<bool> do_conj(baryon_index.size());
      for (const auto& it : baryon_index)
      {
	const auto& [key, p, c] = it.first;
	unique_baryon_keys.at(it.second) = key;
	unique_perms.at(it.second) = p;
	do_conj.at(it.second) = c;
      }
      auto baryons =
	SBN::relabel(get_baryon_elementals(unique_baryon_keys, unique_perms, do_conj,
					   {sl.at('v').at(0), sl.at('w').at(0), sl.at('x').at(0)},
					   {sl.at('v').at(1), sl.at('w').at(1), sl.at('x').at(1)},
					   baryon_dist_labels + std::string{'i'}, perm),
		     {{'i', j_label}});
      const auto expected_size = SBN::SCoor{{'v', sl.at('v').at(1)},
					    {'w', sl.at('w').at(1)},
					    {'x', sl.at('x').at(1)},
					    {j_label, (SBN::IndexType)baryon_index.size()}};
      if (!is_compatible(expected_size, SBN::get_comm(perm), baryons))
	throw std::runtime_error("get_baryons: invalid returned tensor from baryon's callback");
      baryons = make_compatible(expected_size, baryons);

      // Construct the table of linear combinations from the unique baryons to the
      // required by the corr funs
      // Spin indices survive the contraction with baryons(vwxj). Keep them on
      // the row side; BSR multiplication cannot retain part of its column side.
      SBN::Tensor table = SBN::create_tensor_with_local_components(
	SBN::concat(perm.size, SBN::Coor{(int)vertex_indices.size(), slides.at('r')[1],
					 slides.at('s')[1], slides.at('t')[1], -num_baryons}),
	perm.order + std::string("Irst") + std::string{j_label}, std::string{j_label},
	SBN::Options::Alloc::Host, 1 /* j is sparse */, max_elementals_per_spin,
	SBN::Options::IsEg::False, perm);
      const auto& local_table =
	SBN::slice_kv(SBN::get_local_tensor(table),
		      SBN::slice_ranges(SBN::get_local_srange(table), std::string{j_label}));
      SBN::set_zero(table);
      SBN::for_each_coor_value(perm, [&](const SBN::Coor& coor, const SBN::value_type& i_cmplx) {
	const SBN::IndexType i = std::real(i_cmplx);
	const SBN::IndexType spin_combo_index = std::imag(i_cmplx);

	// Ignore blank entries
	if (i >= t.size())
	  return;

	const auto& spin_combo = SBN::indexToCoor(
	  spin_combo_index, get_spin_combo_dim(i, graph, vertices, spins_from_unique_vertex_index));

	for (SBN::IndexType I = 0; I < vertex_indices.size(); ++I)
	{
	  const int vertex_index = vertex_indices.at(I);
	  const auto unique_vertex = vertices.at(i).at(vertex_index);
	  const auto& p = slots_perm.at(i).at(vertex_index);
	  const auto t_slice = t.at(i).at(vertex_index);
	  const Hadron::HadronDispVertex_t& h = unique_vertices.at(unique_vertex);
	  const auto& first_spin =
	    spins_from_unique_vertex_index.at(unique_vertex).at(spin_combo.at(vertex_index));
	  const auto& k1 = coeff_map[h].begin()->second;
	  for (auto k2 = k1.begin(); k2 != k1.end(); ++k2)
	  {
	    Hadron::KeyBaryonElementalOperator_t key;
	    key.t_slice = t_slice;
	    // Flip mom to agree with FT convention \int dx exp(-ipx)
	    key.mom = -h.mom;
	    key.phasing = get_baryon_phasing(h);
	    key.left = checkZero(k2->first.derivs[1].deriv);
	    key.middle = checkZero(k2->first.derivs[2].deriv);
	    key.right = checkZero(k2->first.derivs[3].deriv);
	    SBN::IndexType j = baryon_index.at({key, p, h.creation_op});
	    for (auto k3 = k2->second.begin(); k3 != k2->second.end(); ++k3)
	    {
	      const int ls =
		SBN::norm_coor(k3->first.diracs[p.at(0) + 1] - first_spin.at(p.at(0)), Datalib::TensorStorage::spin_count);
	      const int ms =
		SBN::norm_coor(k3->first.diracs[p.at(1) + 1] - first_spin.at(p.at(1)), Datalib::TensorStorage::spin_count);
	      const int rs =
		SBN::norm_coor(k3->first.diracs[p.at(2) + 1] - first_spin.at(p.at(2)), Datalib::TensorStorage::spin_count);
	      // Check that spin are on the slides
	      if (ls >= BARYON_SPIN_BLOCK || ms >= BARYON_SPIN_BLOCK || rs >= BARYON_SPIN_BLOCK)
		continue;

	      // NOTE: do NOT conjugate the weight for creation operator
	      auto weight = k3->second;
	      SBN::set(local_table, SBN::concat(coor, SBN::Coor{I, ls, ms, rs, j}), weight);
	    }
	  }
	}
      });

      // Move baryons and table to gpu
      baryons = SBN::make_sure(baryons, {}, SBN::Options::Distribution::Default,
			       SBN::Options::Alloc::Device);
      table =
	SBN::make_sure(table, {}, SBN::Options::Distribution::Default, SBN::Options::Alloc::Device);

      // Do contraction
      return {baryons, table, std::string("vwxrstI") + perm.order};
    }

    /// Return the matrix to transform from Euclidean Dirac to Euclidean Degrand-Rossi
    ///
    /// NOTE: the returned matrix, U, is such that  Gamma_{Dirac} = U^\dagger Gamma_{DeGrand-Rossi} U,
    /// or such that  Gamma_{DeGrand-Rossi} = U Gamma_{Dirac} U^\dagger.
    /// U = 1/sqrt(2) * [  0   1   0  -1 ]
    ///                 [ -1   0   1   0 ]
    ///                 [  0   1   0   1 |
    ///                 [ -1   0  -1   0 ]

    inline SBN::Tensor getDiracToDRMat(const std::string& dist = SBN::Options::Distribution::Local,
				       const SBN::Comm& comm = SBN::get_local_comm())
    {
      SBN::Tensor r =
	SBN::create_tensor({Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count}, "sS", dist, SBN::Options::Alloc::Host, 0, 0,
			   SBN::Options::IsEg::False, {}, comm);
      SBN::set_zero(r);
      if (Datalib::TensorStorage::spin_count == 4)
      {
	const double val = 1. / sqrt(2.0);
	for (int i = 0; i < 2; ++i)
	{
	  for (int j = 0; j < 2; ++j)
	  {
	    SBN::set(r, {i * 2 + 1, j * 2}, i == 0 && j == 1 ? val : -val);
	    SBN::set(r, {i * 2, j * 2 + 1}, i == 0 && j == 1 ? -val : val);
	  }
	}
      }
      else
      {
	throw std::runtime_error("Unsupported number of spins");
      }
      return r;
    }

    /// Return the \gamma_5 matrix with Chroma basis

    inline SBN::Tensor chromaGamma5(const std::string& dist = SBN::Options::Distribution::Local,
				    const SBN::Comm& comm = SBN::get_local_comm())
    {
      SBN::Tensor r =
	SBN::create_tensor({Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count}, "sS", dist, SBN::Options::Alloc::Host, 0, 0,
			   SBN::Options::IsEg::False, {}, comm);
      SBN::set_zero(r);
      if (Datalib::TensorStorage::spin_count == 4)
      {
	SBN::set(r, {0, 0}, 1);
	SBN::set(r, {1, 1}, 1);
	SBN::set(r, {2, 2}, -1);
	SBN::set(r, {3, 3}, -1);
      }
      else
      {
	throw std::runtime_error("Unsupported number of spins");
      }
      return r;
    }

    /// Return an implicit matrix with the spin components conjugate-transposed
    /// \param t: tensor to apply the transformation

    inline SBN::Tensor adjForSpins(const SBN::Tensor& t)
    {
      return SBN::conj(SBN::relabel(t, {{'s', 'S'}, {'S', 's'}}));
    }

    /// Contract the spin components of two tensors as if they were matrices

    inline SBN::Tensor contractSpins(const SBN::Tensor& a, const SBN::Tensor& b)
    {
      if (!SBN::contain_labels(a.order, "sS") || !SBN::contain_labels(b.order, "sS"))
	throw std::runtime_error("contractSpins: the input error must have spin components");
      const SBN::Order& result_order = (a.order.size() == 2 ? b.order : a.order);
      return SBN::contract(SBN::relabel(a, {{'S', '$'}}), SBN::relabel(b, {{'s', '$'}}),
			   result_order);
    }

    /// Return an encoding of flavor and conj
    /// \param flavor: input flavor
    /// \param conj: input conj

    inline std::size_t get_vertex_from_flavor_and_conj(char flavor, bool conj)
    {
      if (flavor < 0)
	throw std::runtime_error("get_vertex_from_flavor_and_conj: fix for this case");
      return std::size_t(flavor) + (conj ? 256ul : 0ul);
    }

    /// Return the flavor and conj encoded with `get_vertex_from_flavor_and_conj`
    /// \param v: input code
    /// \return {flavor, conj}

    inline std::tuple<char, bool> get_flavor_and_conj_from_vertex(std::size_t v)
    {
      return {char(v % 256ul), v / 256ul != 0};
    }

    /// Return the props
    /// \param db: prop storage
    /// \param prop_keys: list of props keys
    /// \param perms: list of permutations, one for each prop key, either {0,1,2,3} or {1,0,3,2}
    /// \param do_conj: list of whether to conjugate the elemental, one for each prop key
    /// \param ev_from: first eigenvector and spin to return, "vwrs"
    /// \param ev_size: number of eigenvectors and spins to return, "vwrs"
    /// \param dist_labels: dimensions to be distributed, some of "vwrsi"

    inline SBN::Tensor get_prop_elementals_from_storage(
      const Datalib::TensorStorage::StorageProp4& db,
      const std::vector<Hadron::KeyProp4ElementalOperator_t>& prop_keys,
      const std::vector<SBN::Coor>& perms, const std::vector<bool>& do_conj,
      const SBN::Coor& ev_from, const SBN::Coor& ev_size, const std::string& dist_labels,
      const SBN::Tensor& perm)
    {
      if (prop_keys.size() != perms.size() || prop_keys.size() != do_conj.size())
	throw std::runtime_error("invalid input");
      if (ev_from.size() != 4 || ev_size.size() != 4)
	throw std::runtime_error("invalid input");
      if (SBN::detail::get_debug_level() > 0)
      {
	for (const auto& it : perms)
	{
	  if (it != SBN::Coor{0, 1, 2, 3} && it != SBN::Coor{1, 0, 3, 2})
	    throw std::runtime_error("invalid input");
	}
      }

      // This object is in the DR basis; create matrices to convert it to DP
      const auto& dr_left = SBN::slice_kv(adjForSpins(getDiracToDRMat()), //
					  {{'s', ev_from.at(2)}}, {{'s', ev_size.at(2)}});
      const auto& dr_right =
	SBN::slice_kv(getDiracToDRMat(), {{'S', ev_from.at(3)}}, {{'S', ev_size.at(3)}});

      // Create matrices to convert it to DP and with pre/post applying \gamma_5
      const auto dr_g5_left = contractSpins(dr_left, chromaGamma5());
      const auto dr_g5_right = contractSpins(chromaGamma5(), dr_right);

      // Gather all unique propagators
      SBN::Tensor props = SBN::create_tensor_with_local_components(
	SBN::concat(ev_size, {-(int)prop_keys.size()}), "vwrsi", dist_labels,
	SBN::Options::Alloc::Host, 0, 0, SBN::Options::IsEg::False, perm);
      const auto& local_props =
	SBN::relabel(SBN::slice_kv(SBN::get_local_tensor(props),
				   SBN::slice_ranges(SBN::get_local_srange(props), "i")),
		     {{'r', 's'}, {'s', 'S'}});
      const int num_vecs = std::max(ev_from.at(0) + ev_size.at(0), ev_from.at(1) + ev_size.at(1));
      Hadron::ValProp4ElementalOperator_t val;
      for (std::size_t i = 0; i < prop_keys.size(); ++i)
      {
	const auto& prop_i = SBN::slice_kv(local_props, {{'i', i}}, {{'i', 1}});
	if (!has_local_support(prop_i))
	  continue;
	auto key = prop_keys.at(i);
	bool is_swap = (perms.at(i) != SBN::Coor{0, 1, 2, 3});
	bool this_conj = do_conj.at(i);
	for (int attempt = 0; attempt < 2; ++attempt)
	{
	  if (db.get(key, val) == 0)
	    break;
	  if (attempt == 1)
	  {
	    std::stringstream ss;
	    ss << "failed to get the propagator (or the reversed) key= " << key;
	    throw std::runtime_error(ss.str());
	  }

	  // If it fails, try reversing sink and source
	  std::swap(key.t_slice, key.t_source);
	  key.phasing_sink = -key.phasing_sink;
	  std::swap(key.phasing_source, key.phasing_sink);
	  key.phasing_sink = -key.phasing_sink;
	  is_swap = !is_swap;
	  this_conj = !this_conj;
	}
	if (val.op.size3() < num_vecs || val.op.size4() < num_vecs)
	{
	  throw std::runtime_error("got a propagator with insufficient number of vectors");
	}
	// If sink and source are swapped, then conjugate and apply \gamma_5 left and right:
	// V_t0' D^{-1} V_t1 =
	//          [(V_t0' D^{-1} V_t1)']' =
	//          [V_t1' D^{-\dagger} V_t0]' =
	//          [\g_5 V_t1' D^{-1} V_t0 \g_5]' =
	//          \g_5 [V_t1' D^{-1} V_t0]' \g_5
	const auto& ti0 = SBN::toTensor(val.op, is_swap ? "wvSs" : "vwsS");
	auto ti = SBN::slice_kv(is_swap ? SBN::conj(ti0) : ti0, //
				{{'v', ev_from.at(0)}, {'w', ev_from.at(1)}},
				{{'v', ev_size.at(0)}, {'w', ev_size.at(1)}});

	// If the propagator is conjugated, then applied \gamma_5 left and right
	// NOTE: \g_5 (V' D^{-1} V) \g_5 = V' \g_5 D^{-1} \g_5 V = V' D^{-\dagger} V
	// NOTE: colorvec function gamma5Herm also adjoint the matrix; we do that just above
	if (this_conj)
	{
	  ti = contractSpins(contractSpins(dr_g5_left, ti), dr_g5_right);
	}
	else
	{
	  ti = contractSpins(contractSpins(dr_left, ti), dr_right);
	}

	SBN::copyTo(ti, prop_i);
      }

      return props;
    }

    /// Return a tensor with all the requested propagators
    /// \param get_prop_elementals: callback to get the prop elementals
    /// \param graph: vertices and slot numbers
    /// \param unique_vertices: vector of disp vertices
    /// \param vertices: unique_vertices[vertices[i][j]] disp vertex for vertex j for corr fun i
    /// \param slots_perm: do_perm(origin_slots, slots_perm[i]) -> target_slots
    /// \param t: time[i][j] time slice for vertex j for corr fun i
    /// \param spins_from_unique_vertex_index: return the list of spin combination for a vertex
    /// \param vertex_index: graph vertex index to get the propagators
    /// \param perm: indices of vertices and t to visit
    /// \param num_vecs: default rank of the distillation basis
    /// \param slides: slices of the distillation basis for some dimensions

    inline std::tuple<SBN::Tensor, SBN::Tensor, std::string>
    get_props(const prop_elementals_callback get_prop_elementals,
	      const std::vector<GraphVertex>& graph,
	      const std::vector<Hadron::HadronDispVertex_t>& unique_vertices,
	      const std::vector<std::vector<std::size_t>>& vertices,
	      const std::vector<std::vector<SBN::Coor>>& slots_perm,
	      const FlavorToMassMap& flavor_to_mass, const std::vector<std::vector<int>>& t,
	      const SpinsFromUniqueVertexIndex& spins_from_unique_vertex_index,
	      const std::vector<int>& vertex_indices, const SBN::Tensor& perm, int num_vecs,
	      const Slides& slides)
    {
      Datalib::TensorStorage::detail::Tracker t_("get_props");

      // List of labels:
      // - i: corr fun index
      // - k: unique prop key
      // - v: prop distillation basis index for first graph first slot
      // - w: prop distillation basis index for second graph first slot
      // - s: prop spin for first graph first slot
      // - S: prop spin for first graph second slot
      // Intended contraction:
      //   props(vwrsk) * table(iI|k) -> return(vwrsiI)
      // NOTE: tensor notation: <tensor name>(<dense labels>[|<sparse labels>])

      const char k_label = SBN::get_new_label(std::string("vwxrstI") + perm.order);

      // The correlation functions are going to be distributed among all processes, but in case
      // the baryons are too big, then the baryons dimensions with slides will be distributed instead
      std::string prop_dist_labels; ///< distributed labels for the props (empty for now)

      // Get the vertices associated to the slots
      const auto& slot_to_vertex = get_map_from_slot_index_to_vertex_and_slot(graph);
      // Extend slides for all dimensions
      Slides sl = slides;
      for (char c : std::string("vw"))
	sl[c] = slides.count(c) > 0 ? slides.at(c) : std::array<int, 2>{0, num_vecs};

      using KeyTransAndConj = std::tuple<Hadron::KeyProp4ElementalOperator_t, bool, bool>;

      auto get_key = [&](SBN::IndexType i, int vertex_index, const SBN::Coor& spin_combo) {
	const int target_slot_index_snk = graph.at(vertex_index).slot_indices.at(0);
	const int target_slot_index_src = graph.at(vertex_index).slot_indices.at(1);
	const auto [target_vertex_snk, target_slot_num_snk] =
	  slot_to_vertex.at(target_slot_index_snk);
	const auto [target_vertex_src, target_slot_num_src] =
	  slot_to_vertex.at(target_slot_index_src);

	// If either the source or the sink vertices is unsmeared, then
	// `remove_fake_connections` failed
	if (!unique_vertices.at(vertices.at(i).at(target_vertex_src)).smearedP ||
	    !unique_vertices.at(vertices.at(i).at(target_vertex_snk)).smearedP)
	  throw std::runtime_error("wtf");

	// Get the flavor and the conjugacy and the mass label
	const auto [flavor, conj] =
	  get_flavor_and_conj_from_vertex(vertices.at(i).at(vertex_index));
	if (flavor_to_mass.count(flavor) == 0)
	{
	  std::stringstream ss;
	  ss << "failed to get the mass label for flavor `" << std::string{flavor} << "`";
	  throw std::runtime_error(ss.str());
	}
	const std::string& mass_label = flavor_to_mass.at(flavor);

	// Assign sink and source vertices
	const bool transpose = slots_perm.at(i).at(vertex_index) == SBN::Coor{1, 0};
	const int vertex_src = (!transpose ? target_vertex_src : target_vertex_snk);
	const int vertex_snk = (!transpose ? target_vertex_snk : target_vertex_src);
	const auto& slot_perm_src = slots_perm.at(i).at(vertex_src);
	const auto& slot_perm_snk = slots_perm.at(i).at(vertex_snk);
	const auto& slot_num_src = !transpose ? target_slot_num_src : target_slot_num_snk;
	const auto& slot_num_snk = !transpose ? target_slot_num_snk : target_slot_num_src;
	const int slot_src = get_origin_slot(slot_num_src, slot_perm_src);
	const int slot_snk = get_origin_slot(slot_num_snk, slot_perm_snk);

	const int t_src = t.at(i).at(vertex_src);
	const int t_snk = t.at(i).at(vertex_snk);
	const auto& phasing_source =
	  get_phasing(unique_vertices.at(vertices.at(i).at(vertex_src)), slot_src);
	const auto& phasing_sink =
	  get_phasing(unique_vertices.at(vertices.at(i).at(vertex_snk)), slot_snk);
	const int spin_src = graph.at(vertex_src).type != 'B'
			       ? 0
			       : spins_from_unique_vertex_index.at(vertices.at(i).at(vertex_src))
				     .at(spin_combo.at(vertex_src))
				     .at(slot_src) /
				   BARYON_SPIN_BLOCK;
	const int spin_snk = graph.at(vertex_snk).type != 'B'
			       ? 0
			       : spins_from_unique_vertex_index.at(vertices.at(i).at(vertex_snk))
				     .at(spin_combo.at(vertex_snk))
				     .at(slot_snk) /
				   BARYON_SPIN_BLOCK;
	const int spin_block = (!transpose ? spin_snk + spin_src * 2 : spin_src + spin_snk * 2);
	if (spin_snk > 0 && sl.at(!transpose ? 'r' : 's').at(1) != BARYON_SPIN_BLOCK)
	  throw std::runtime_error("wtf");
	if (spin_src > 0 && sl.at(!transpose ? 's' : 'r').at(1) != BARYON_SPIN_BLOCK)
	  throw std::runtime_error("wtf");
	Hadron::KeyProp4ElementalOperator_t key;
	key.t_slice = t_snk;
	key.t_source = t_src;
	key.mass_label = mass_label;
	key.phasing_sink = -phasing_sink;
	key.phasing_source = phasing_source;
	return std::make_tuple(spin_block, KeyTransAndConj{key, transpose, conj});
      };

      // Construct the list of unique props
      const auto num_corr_local = SBN::volume(SBN::get_local_tensor_tight(perm).size);
      std::vector<unordered_map<KeyTransAndConj, SBN::IndexType>> index(
	4, unordered_map<KeyTransAndConj, SBN::IndexType>(num_corr_local));
      std::vector<SBN::IndexType> num_props(4);
      SBN::for_each_coor_value(perm, [&](const SBN::Coor& coor, const SBN::value_type& i_cmplx) {
	const SBN::IndexType i = std::real(i_cmplx);
	const SBN::IndexType spin_combo_index = std::imag(i_cmplx);

	// Ignore blank entries
	if (i >= t.size())
	  return;

	const auto& spin_combo = SBN::indexToCoor(
	  spin_combo_index, get_spin_combo_dim(i, graph, vertices, spins_from_unique_vertex_index));

	for (const int vertex_index : vertex_indices)
	{
	  const auto& [spin_group, k] = get_key(i, vertex_index, spin_combo);
	  if (index.at(spin_group).count(k) == 0)
	    index.at(spin_group)[k] = num_props.at(spin_group)++;
	}
      });

      // Gather all unique propagators
      const auto first_spin_r = std::vector<int>{0, BARYON_SPIN_BLOCK, 0, BARYON_SPIN_BLOCK};
      const auto first_spin_s = std::vector<int>{0, 0, BARYON_SPIN_BLOCK, BARYON_SPIN_BLOCK};
      auto props_blocks = std::vector<SBN::Tensor>();
      for (int spin_block = 0; spin_block < 4; ++spin_block)
      {
	std::vector<Hadron::KeyProp4ElementalOperator_t> unique_prop_keys(
	  index.at(spin_block).size());
	std::vector<SBN::Coor> unique_perms(index.at(spin_block).size());
	std::vector<bool> do_conj(index.at(spin_block).size());
	for (const auto& it : index.at(spin_block))
	{
	  const auto& [key, t, c] = it.first;
	  unique_prop_keys.at(it.second) = key;
	  unique_perms.at(it.second) = !t ? SBN::Coor{0, 1, 2, 3} : SBN::Coor{1, 0, 3, 2};
	  do_conj.at(it.second) = c;
	}
	auto props = SBN::relabel(
	  get_prop_elementals(
	    unique_prop_keys, unique_perms, do_conj, //
	    {sl.at('v').at(0), sl.at('w').at(0), first_spin_r.at(spin_block),
	     first_spin_s.at(spin_block)},					      //
	    {sl.at('v').at(1), sl.at('w').at(1), sl.at('r').at(1), sl.at('s').at(1)}, //
	    prop_dist_labels + std::string{'i'}, perm),
	  {{'i', k_label}});
	if (index.at(spin_block).size() == 0)
	  continue;
	const auto expected_size =
	  SBN::SCoor{{'v', sl.at('v').at(1)},
		     {'w', sl.at('w').at(1)},
		     {'r', sl.at('r').at(1)},
		     {'s', sl.at('s').at(1)},
		     {k_label, (SBN::IndexType)index.at(spin_block).size()}};
	if (!is_compatible(expected_size, SBN::get_comm(perm), props))
	  throw std::runtime_error("get_props: invalid returned tensor from prop's callback");
	props_blocks.push_back(make_compatible(expected_size, props));
      }
      auto props = props_blocks.size() > 0
		     ? SBN::concat(props_blocks, k_label)
		     : SBN::like_this(perm, {'v', 'w', 'r', 's', k_label},
				      {sl.at('v').at(1), sl.at('w').at(1), sl.at('r').at(1),
				       sl.at('s').at(1), 0});
      props_blocks.clear();
      const auto total_num_props =
	num_props.at(0) + num_props.at(1) + num_props.at(2) + num_props.at(3);

      // Construct the table of linear combinations from the unique props to the
      // required by the corr funs
      SBN::Tensor table = SBN::create_tensor_with_local_components(
	SBN::concat(perm.size, SBN::Coor{(int)vertex_indices.size(), -total_num_props}),
	perm.order + std::string{'I', k_label},
	get_distribution_labels(perm) + std::string{k_label}, SBN::Options::Alloc::Host,
	1 /* k is sparse */, 1 /* one nnz */, SBN::Options::IsEg::False, perm);
      const auto& local_table =
	SBN::slice_kv(SBN::get_local_tensor(table),
		      SBN::slice_ranges(SBN::get_local_srange(table), std::string{k_label}));
      SBN::set_zero(table);
      std::vector<SBN::IndexType> first_k(4);
      for (int i = 1; i < 4; i++)
	first_k.at(i) = first_k.at(i - 1) + index.at(i - 1).size();
      SBN::for_each_coor_value(perm, [&](const SBN::Coor& coor, const SBN::value_type& i_cmplx) {
	const SBN::IndexType i = std::real(i_cmplx);
	const SBN::IndexType spin_combo_index = std::imag(i_cmplx);

	// Ignore blank entries
	if (i >= t.size())
	  return;

	const auto& spin_combo = SBN::indexToCoor(
	  spin_combo_index, get_spin_combo_dim(i, graph, vertices, spins_from_unique_vertex_index));

	for (SBN::IndexType I = 0; I < vertex_indices.size(); ++I)
	{
	  const int vertex_index = vertex_indices.at(I);
	  const auto [spin_group, key] = get_key(i, vertex_index, spin_combo);
	  const auto k = first_k.at(spin_group) + index.at(spin_group).at(key);
	  SBN::set(local_table, SBN::concat(coor, SBN::Coor{I, k}), 1.0);
	}
      });

      // Move table and props to gpu
      props =
	SBN::make_sure(props, {}, SBN::Options::Distribution::Default, SBN::Options::Alloc::Device);
      table =
	SBN::make_sure(table, {}, SBN::Options::Distribution::Default, SBN::Options::Alloc::Device);

      // Do contraction
      return {props, table, std::string("vwrsI") + perm.order};
    }

    /// Return the reverse displacement
    /// \param s1: input displacement
    /// NOTE: the change of sign is still not clear

    std::vector<int> reverseDisp(const std::vector<int>& s1)
    {
      std::vector<int> dest;
      dest.resize(s1.size());
      for (int i = 0; i < dest.size(); i++)
	dest[i] = -s1[dest.size() - i - 1];

      return dest;
    }

    /// Return the genprops
    /// \param db: genprop storage
    /// \param genprop_keys: list of genprops keys
    /// \param perms: list of permutations, one for each genprop key, either {0,1,2,3} or {1,0,3,2}
    /// \param do_conj: list of whether to conjugate the elemental, one for each genprop key
    /// \param ev_from: first eigenvector and spin to return, "vwrs"
    /// \param ev_size: number of eigenvectors and spins to return, "vwrs"
    /// \param dist_labels: dimensions to be distributed, some of "vwrsi"

    inline SBN::Tensor get_genprop_elementals_from_storage(
      const Datalib::TensorStorage::StorageGenprop4& db,
      const std::vector<Hadron::KeyGenProp4ElementalOperator_t>& genprop_keys,
      const std::vector<SBN::Coor>& perms, const std::vector<bool>& do_conj,
      const SBN::Coor& ev_from, const SBN::Coor& ev_size, const std::string& dist_labels,
      const SBN::Tensor& perm)
    {
      // Phases for  gamma_5*dag(Gamma)*gamma_5 -> eta*Gamma
      //            0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
      const int eta[] = {1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, 1, -1, 1, 1, 1};

      if (genprop_keys.size() != perms.size() || genprop_keys.size() != do_conj.size())
	throw std::runtime_error("invalid input");
      if (ev_from.size() != 4 || ev_size.size() != 4)
	throw std::runtime_error("invalid input");
      if (SBN::detail::get_debug_level() > 0)
      {
	for (const auto& it : perms)
	{
	  if (it != SBN::Coor{0, 1, 2, 3} && it != SBN::Coor{1, 0, 3, 2})
	    throw std::runtime_error("invalid input");
	}
      }

      // This object is in the DR basis; create matrices to convert it to DP
      const auto& dr_left = SBN::slice_kv(adjForSpins(getDiracToDRMat()), //
					  {{'s', ev_from.at(2)}}, {{'s', ev_size.at(2)}});
      const auto& dr_right =
	SBN::slice_kv(getDiracToDRMat(), {{'S', ev_from.at(3)}}, {{'S', ev_size.at(3)}});

      // Create matrices to convert it to DP and with pre/post applying \gamma_5
      const auto dr_g5_left = contractSpins(dr_left, chromaGamma5());
      const auto dr_g5_right = contractSpins(chromaGamma5(), dr_right);

      // Gather all unique generalized propagators
      SBN::Tensor genprops = SBN::create_tensor_with_local_components(
	SBN::concat(ev_size, {-(int)genprop_keys.size()}), "vwrsi", dist_labels,
	SBN::Options::Alloc::Host, 0, 0, SBN::Options::IsEg::False, perm);
      const auto& local_genprops =
	SBN::relabel(SBN::slice_kv(SBN::get_local_tensor(genprops),
				   SBN::slice_ranges(SBN::get_local_srange(genprops), "i")),
		     {{'r', 's'}, {'s', 'S'}});
      auto val = SBN::create_tensor({ev_size.at(0), ev_size.at(1), Datalib::TensorStorage::spin_count, Datalib::TensorStorage::spin_count}, "vwsS",
				    SBN::Options::Distribution::Local, SBN::Options::Alloc::Host);
      auto toCoor4 = [](const SBN::Coor& coor) {
	if (coor.size() != 4)
	  throw std::runtime_error("wtf");
	std::array<int, 4> r;
	std::copy_n(coor.begin(), 4, r.begin());
	return r;
      };
      for (std::size_t i = 0; i < genprop_keys.size(); ++i)
      {
	const auto& genprop_i = SBN::slice_kv(local_genprops, {{'i', i}}, {{'i', 1}});
	if (!has_local_support(genprop_i))
	  continue;
	auto key = genprop_keys.at(i);
	bool is_swap = (perms.at(i) != SBN::Coor{0, 1, 2, 3});
	bool this_conj = do_conj.at(i);
	const auto& val_from =
	  toCoor4(SBN::get_coor(val.order, {{'v', ev_from.at(0)}, {'w', ev_from.at(1)}}, 0));
	const auto& val_size = toCoor4(val.size);
	bool failed = false;
	for (int attempt = 0; attempt < 2; ++attempt)
	{
	  const auto& val_order =
	    SBN::relabel(is_swap ? SBN::remap{{'v', 'n'}, {'w', 'N'}, {'s', 's'}, {'S', 'q'}}
				 : SBN::remap{{'v', 'N'}, {'w', 'n'}, {'s', 'q'}, {'S', 's'}},
			 val.order);
	  if (!this_conj && db.get(key, val_from, toCoor4(val.size), val_order, data(val)) == 0)
	    break;
	  if (!this_conj)
	  {
	    std::stringstream ss;
	    ss << "failed to get the genprop (or the reversed) key= " << key;
	    throw std::runtime_error(ss.str());
	  }

	  // If it fails, try reversing sink and source
	  // If sink and source are swapped, then conjugate and apply \gamma_5 left and right:
	  // V_t0' D^{-1} G D^{-1} V_t1 =
	  //          [(V_t0' D^{-1} G D^{-1} V_t1)']' =
	  //          [V_t1' D^{-\dagger} G^\dagger D^{-\dagger} V_t0]' =
	  //          [\g_5 V_t1' D^{-1} G`^\dagger  D^{-1} V_t0 \g_5]' eta(G) =
	  //          \g_5 [V_t1' D^{-1} G`^\dagger D^{-1} V_t0]' \g_5 eta(G)
	  // NOTE: \g_5 G^\dagger \g_5 = G`^\dagger * eta(G), where G`^\dagger is the dagger of G
	  //       excepting the spin components
	  std::swap(key.t_source, key.t_sink);
	  key.phasing_sink = -key.phasing_sink;
	  std::swap(key.phasing_source, key.phasing_sink);
	  key.phasing_sink = -key.phasing_sink;
	  key.displacement = reverseDisp(key.displacement);
	  key.mom = -key.mom;
	  is_swap = !is_swap;
	  this_conj = !this_conj;
	  failed = true;
	}

	auto ti = failed ? SBN::conj(val) : val;

	if (this_conj)
	{
	  // FIXME!
	  throw std::runtime_error("unsupported case");
#if 0
	  // NOTE: the weight should not be negated. It's outside the
	  // elemental
	  auto s = double(eta[key.g]) * Hadron::momPhase(Hadron::constructMom4d(key.mom),
							 Hadron::dispMomMask(key.displacement),
							 lattice_size);

	  // If the propagator is conjugated, then applied \gamma_5 left and right
	  // NOTE: \g_5 (V' D^{-1} G D^{-1} V) \g_5
	  //         = V' \g_5 D^{-1} \g_5 \g_5 G \g_5 \g_5 D^{-1} \g_5 V
	  //         = eta(G) * V' D^{-\dagger} D^{-\dagger} V
	  ti = contractSpins(contractSpins(dr_g5_left, scale(ti, s)), dr_g5_right);
#endif
	}
	else
	{
	  ti = contractSpins(contractSpins(dr_left, ti), dr_right);
	}

	SBN::copyTo(ti, genprop_i);
      }

      return genprops;
    }

    /// Return a tensor with all the requested genprops
    /// \param get_genprop_elementals: callback to get the prop elementals
    /// \param graph: vertices and slot numbers
    /// \param related_prop: returns the source and sink prop vertex indices with the flavor and conj info
    /// \param coeff_map: coeff map
    /// \param unique_vertices: vector of disp vertices
    /// \param vertices: unique_vertices[vertices[i][j]] disp vertex for vertex j for corr fun i
    /// \param slots_perm: do_perm(origin_slots, slots_perm[i]) -> target_slots
    /// \param t: time[i][j] time slice for vertex j for corr fun i
    /// \param spins_from_unique_vertex_index: return the list of spin combination for a vertex
    /// \param vertex_index: graph vertex index to get the genprops
    /// \param perm: indices of vertices and t to visit
    /// \param num_vecs: default rank of the distillation basis
    /// \param slides: slices of the distillation basis for some dimensions
    /// \param lattice_size: lattice size

    inline std::tuple<SBN::Tensor, SBN::Tensor, std::string>
    get_genprops(const genprop_elementals_callback get_genprop_elementals,
		 const std::vector<GraphVertex>& graph,
		 const std::map<int, std::array<int, 2>>& related_prop, const CoeffMap& coeff_map,
		 const std::vector<Hadron::HadronDispVertex_t>& unique_vertices,
		 const std::vector<std::vector<std::size_t>>& vertices,
		 const std::vector<std::vector<SBN::Coor>>& slots_perm,
		 const FlavorToMassMap& flavor_to_mass, const std::vector<std::vector<int>>& t,
		 const SpinsFromUniqueVertexIndex& spins_from_unique_vertex_index,
		 const std::vector<int>& vertex_indices, const SBN::Tensor& perm, int num_vecs,
		 const Slides& slides, const Array<int>& lattice_size)
    {
      Datalib::TensorStorage::detail::Tracker t_("get_genprops");

      // List of labels:
      // - i: corr fun index
      // - k: unique genprop key
      // - v: genprop distillation basis index for first graph first slot
      // - w: genprop distillation basis index for second graph first slot
      // - s: genprop spin for first graph first slot
      // - S: genprop spin for first graph second slot
      // Intended contraction:
      //   genprops(vwrsk) * table(iI|k) -> return(vwrsiI)
      // NOTE: tensor notation: <tensor name>(<dense labels>[|<sparse labels>])

      const char k_label = SBN::get_new_label(std::string("vwxrst") + perm.order);

      // The correlation functions are going to be distributed among all processes, but in case
      // the baryons are too big, then the baryons dimensions with slides will be distributed instead
      std::string genprop_dist_labels; ///< distributed labels for the genprops (empty for now)

      // Extend slides for all dimensions
      Slides sl = slides;
      for (char c : std::string("vw"))
	sl[c] = slides.count(c) > 0 ? slides.at(c) : std::array<int, 2>{0, num_vecs};

      using KeyTransAndConj = std::tuple<Hadron::KeyGenProp4ElementalOperator_t, bool, bool>;
      auto get_almost_key = [&](SBN::IndexType i, int vertex_index, const SBN::Coor& spin_combo) {
	// Get the vertices associated to the slots
	const auto& slot_to_vertex =
	  get_map_from_slot_index_to_vertex_and_slot(graph, vertex_index);
	const int target_slot_index_snk = graph.at(vertex_index).slot_indices.at(0);
	const int target_slot_index_src = graph.at(vertex_index).slot_indices.at(1);
	const auto [target_vertex_snk_, target_slot_num_snk_] =
	  slot_to_vertex.at(target_slot_index_snk);
	const auto [target_vertex_src_, target_slot_num_src_] =
	  slot_to_vertex.at(target_slot_index_src);
	/// Hack to deal with amdclang++
	const auto target_vertex_src = target_vertex_src_;
	const auto target_vertex_snk = target_vertex_snk_;
	const auto target_slot_num_src = target_slot_num_src_;
	const auto target_slot_num_snk = target_slot_num_snk_;
	const int prop_index = related_prop.at(vertex_index).at(0);

	// Get the flavor and the conjugacy and the mass label
	const auto [flavor, conj] = get_flavor_and_conj_from_vertex(vertices.at(i).at(prop_index));
	if (conj != false)
	  throw std::runtime_error("get_genprops: untested case");
	if (flavor_to_mass.count(flavor) == 0)
	{
	  std::stringstream ss;
	  ss << "failed to get the mass label for flavor `" << std::string{flavor} << "`";
	  throw std::runtime_error(ss.str());
	}
	const std::string& mass_label = flavor_to_mass.at(flavor);

	// Assign sink and source vertices
	const bool transpose = slots_perm.at(i).at(vertex_index) == SBN::Coor{1, 0};
	const int vertex_src = (!transpose ? target_vertex_src : target_vertex_snk);
	const int vertex_snk = (!transpose ? target_vertex_snk : target_vertex_src);
	const auto& slot_perm_src = slots_perm.at(i).at(vertex_src);
	const auto& slot_perm_snk = slots_perm.at(i).at(vertex_snk);
	const int slot_src =
	  get_origin_slot(!transpose ? target_slot_num_src : target_slot_num_snk, slot_perm_src);
	const int slot_snk =
	  get_origin_slot(!transpose ? target_slot_num_snk : target_slot_num_src, slot_perm_snk);

	const int t_src = t.at(i).at(vertex_src);
	const int t_vtx = t.at(i).at(vertex_index);
	const int t_snk = t.at(i).at(vertex_snk);
	const auto& phasing_source =
	  get_phasing(unique_vertices.at(vertices.at(i).at(vertex_src)), slot_src);
	const auto& phasing_sink =
	  get_phasing(unique_vertices.at(vertices.at(i).at(vertex_snk)), slot_snk);
	const int spin_src = graph.at(vertex_src).type != 'B'
			       ? 0
			       : spins_from_unique_vertex_index.at(vertices.at(i).at(vertex_src))
				     .at(spin_combo.at(vertex_src))
				     .at(slot_src) /
				   BARYON_SPIN_BLOCK;
	const int spin_snk = graph.at(vertex_snk).type != 'B'
			       ? 0
			       : spins_from_unique_vertex_index.at(vertices.at(i).at(vertex_snk))
				     .at(spin_combo.at(vertex_snk))
				     .at(slot_snk) /
				   BARYON_SPIN_BLOCK;
	if (spin_snk > 0 && sl.at(!transpose ? 'r' : 's').at(1) != BARYON_SPIN_BLOCK)
	  throw std::runtime_error("wtf");
	if (spin_src > 0 && sl.at(!transpose ? 's' : 'r').at(1) != BARYON_SPIN_BLOCK)
	  throw std::runtime_error("wtf");
	const int spin_block = (!transpose ? spin_snk + spin_src * 2 : spin_src + spin_snk * 2);

	const Hadron::HadronDispVertex_t& h = unique_vertices.at(vertices.at(i).at(vertex_index));
	// Flip mom to agree with FT convention \int dx exp(-ipx)
	auto mom = -h.mom;

	Hadron::KeyGenProp4ElementalOperator_t key;
	key.t_sink = t_snk;
	key.t_slice = t_vtx;
	key.t_source = t_src;
	key.phasing_sink = -phasing_sink;
	key.phasing_source = phasing_source;
	key.mass = mass_label;
	key.mom = (conj xor h.creation_op) ? -mom : mom;
	return std::make_tuple(spin_block, KeyTransAndConj{key, transpose, conj});
      };

      // Construct the list of needed genprops
      const auto num_corr_local = SBN::volume(SBN::get_local_tensor_tight(perm).size);
      using Index = unordered_map<KeyTransAndConj, SBN::IndexType>;
      std::vector<Index> index(4, Index(num_corr_local));
      SBN::IndexType max_nnz_for_table = 0;
      using Table =
	unordered_map<SBN::Coor,
		      std::vector<std::tuple<SBN::IndexType, KeyTransAndConj, SBN::value_type>>>;
      std::vector<std::vector<Table>> local_table_nnz(4);
      for_each_coor_value_threaded(perm, [&] {
	// Thread local variables
	auto index_th = std::make_shared<std::vector<Index>>(
	  4, Index(num_corr_local / (get_thread_num() == 0 ? 1 : get_num_threads())));
	auto local_table_nnz_th =
	  std::make_shared<std::vector<Table>>(4, Table(num_corr_local / get_num_threads()));
	auto max_nnz_for_table_th = std::make_shared<SBN::IndexType>(0);

	return std::make_pair(
	  // Main iteration
	  [&, index_th, local_table_nnz_th, max_nnz_for_table_th](const SBN::Coor& coor,
								  const SBN::value_type& i_cmplx) {
	    const SBN::IndexType i = std::real(i_cmplx);
	    const SBN::IndexType spin_combo_index = std::imag(i_cmplx);

	    // Ignore blank entries
	    if (i >= t.size())
	      return;

	    const auto& spin_combo = SBN::indexToCoor(
	      spin_combo_index,
	      get_spin_combo_dim(i, graph, vertices, spins_from_unique_vertex_index));

	    for (SBN::IndexType I = 0; I < vertex_indices.size(); ++I)
	    {
	      const int vertex_index = vertex_indices.at(I);

	      // Check that the current vertex is indeed unsmeared
	      if (unique_vertices.at(vertices.at(i).at(vertex_index)).smearedP)
		throw std::runtime_error("wtf");

	      // Check that current vertex has no phasing
	      if (unique_vertices.at(vertices.at(i).at(vertex_index)).phasings.size() != 0)
		throw std::runtime_error("unsmeared vertex cannot have phasing");

	      // Get an almost filled key
	      auto [spin_block, keyTrConj] = get_almost_key(i, vertex_index, spin_combo);
	      auto& [key, transpose, conj] = keyTrConj;

	      // Get the DR basis
	      const auto& disp_spins = Hadron::convertTwoQuarkDispGamma(
		coeff_map[unique_vertices.at(vertices.at(i).at(vertex_index))]);
	      int nnz = 0;
	      for (auto k2 = disp_spins.begin(); k2 != disp_spins.end(); ++k2)
	      {
		for (auto k3 = k2->second.begin(); k3 != k2->second.end(); ++k3)
		{
		  int gamma = k3->first.gamma;
		  auto deriv = checkZero(k2->first.deriv);
		  auto weight = k3->second;
		  key.g = gamma;
		  key.displacement = (conj ? reverseDisp(deriv) : deriv);
		  const auto k = KeyTransAndConj{key, transpose, conj};
		  index_th->at(spin_block).try_emplace(k, 0);
		  auto& t = local_table_nnz_th->at(spin_block)[coor];
		  t.reserve(*max_nnz_for_table_th);
		  t.push_back({I, k, weight});
		  nnz++;
		}
	      }
	      *max_nnz_for_table_th = std::max(*max_nnz_for_table_th, nnz);
	    }
	  },

	  // finalize
	  [&, index_th, local_table_nnz_th, max_nnz_for_table_th] {
	    max_among_threads(*max_nnz_for_table_th, &max_nnz_for_table);
	    for (int i = 0; i < 4; ++i)
	    {
	      merge_among_threads(index_th->at(i), &index.at(i));
	      gather_among_threads(local_table_nnz_th->at(i), &local_table_nnz.at(i));
	    }
	  });
      });

      max_nnz_for_table = gather_max(max_nnz_for_table, SBN::get_mpi_comm(perm));

      // Gather all genprops
      const auto first_spin_r = std::vector<int>{0, BARYON_SPIN_BLOCK, 0, BARYON_SPIN_BLOCK};
      const auto first_spin_s = std::vector<int>{0, 0, BARYON_SPIN_BLOCK, BARYON_SPIN_BLOCK};
      std::vector<SBN::Tensor> genprops_blocks;
      for (int spin_block = 0; spin_block < 4; ++spin_block)
      {
	std::vector<Hadron::KeyGenProp4ElementalOperator_t> unique_genprop_keys(
	  index.at(spin_block).size());
	std::vector<SBN::Coor> unique_perms(index.at(spin_block).size());
	std::vector<bool> do_conj(index.at(spin_block).size());
	for_each_enumerated_threaded(
	  index.at(spin_block), [&](SBN::IndexType genprop_index, auto& it) {
	    const auto& [key, t, c] = it.first;
	    it.second = genprop_index;
	    unique_genprop_keys.at(it.second) = key;
	    unique_perms.at(it.second) = !t ? SBN::Coor{0, 1, 2, 3} : SBN::Coor{1, 0, 3, 2};
	    do_conj.at(it.second) = c;
	  });
	auto genprops = SBN::relabel(
	  get_genprop_elementals(
	    unique_genprop_keys, unique_perms, do_conj, //
	    {sl.at('v').at(0), sl.at('w').at(0), first_spin_r.at(spin_block),
	     first_spin_s.at(spin_block)},					      //
	    {sl.at('v').at(1), sl.at('w').at(1), sl.at('r').at(1), sl.at('s').at(1)}, //
	    genprop_dist_labels + std::string{'i'}, perm),
	  {{'i', k_label}});
	if (index.at(spin_block).size() == 0)
	  continue;
	const auto expected_size =
	  SBN::SCoor{{'v', sl.at('v').at(1)},
		     {'w', sl.at('w').at(1)},
		     {'r', sl.at('r').at(1)},
		     {'s', sl.at('s').at(1)},
		     {k_label, (SBN::IndexType)index.at(spin_block).size()}};
	if (!is_compatible(expected_size, SBN::get_comm(perm), genprops))
	  throw std::runtime_error("get_genprops: invalid returned tensor from genprop's callback");
	genprops_blocks.push_back(make_compatible(expected_size, genprops));
      }
      auto genprops = genprops_blocks.size() > 0
			? SBN::concat(genprops_blocks, k_label)
			: SBN::like_this(perm, {'v', 'w', 'r', 's', k_label},
					 {sl.at('v').at(1), sl.at('w').at(1), sl.at('r').at(1),
					  sl.at('s').at(1), 0});
      genprops_blocks.clear();
      const auto total_num_genprops =
	index.at(0).size() + index.at(1).size() + index.at(2).size() + index.at(3).size();

      // Construct the table of linear combinations from the unique genprops to the
      // required by the corr funs
      SBN::Tensor table = SBN::create_tensor_with_local_components(
	SBN::concat(perm.size, SBN::Coor{(SBN::IndexType)vertex_indices.size(),
					 -(SBN::IndexType)total_num_genprops}),
	perm.order + std::string{'I', k_label},
	get_distribution_labels(perm) + std::string{k_label}, SBN::Options::Alloc::Host,
	1 /* k is sparse */, max_nnz_for_table, SBN::Options::IsEg::False, perm);
      const auto& local_table =
	SBN::slice_kv(SBN::get_local_tensor(table),
		      SBN::slice_ranges(SBN::get_local_srange(table), std::string{k_label}));
      SBN::set_zero(table);
      std::vector<SBN::IndexType> first_k(4);
      for (int i = 1; i < 4; i++)
	first_k.at(i) = first_k.at(i - 1) + index.at(i - 1).size();
      for (int spin_block = 0; spin_block < 4; ++spin_block)
      {
	for_each_threaded(local_table_nnz.at(spin_block), [&](const auto& local_table_nnz_items) {
	  for (const auto& it : local_table_nnz_items)
	  {
	    const auto& coor = it.first;
	    for (const auto& [I, keyTransConj, weight] : it.second)
	    {
	      const SBN::IndexType k =
		first_k.at(spin_block) + index.at(spin_block).at(keyTransConj);
	      SBN::set(local_table, SBN::concat(coor, SBN::Coor{I, k}), weight);
	    }
	  }
	});
	local_table_nnz.at(spin_block).clear();
      }

      // Move table and genprops to gpu
      genprops = SBN::make_sure(genprops, {}, SBN::Options::Distribution::Default,
				SBN::Options::Alloc::Device);
      table =
	SBN::make_sure(table, {}, SBN::Options::Distribution::Default, SBN::Options::Alloc::Device);

      // Do contraction
      return {genprops, table, std::string("vwrsI") + perm.order};
    }

    /// Return the tensor associated to a vertex for each corr fun
    /// \param ctx: context
    /// \param graph: vertices and slot numbers
    /// \param related_prop: returns the source and sink prop vertex indices with the flavor and conj info
    /// \param unique_vertices: vector of disp vertices
    /// \param vertices: unique_vertices[vertices[i][j]] disp vertex for vertex j for corr fun i
    /// \param slots_perm: do_perm(origin_slots, slots_perm[i]) -> target_slots
    /// \param t: time[i][j] time slice for vertex j for corr fun i
    /// \param spins_from_unique_vertex_index: return the list of spin combination for a vertex
    /// \param vertex_index: graph vertex index to get the baryons, props, genprops...
    /// \param slides: slices of the distillation basis for some dimensions

    inline std::tuple<SBN::Tensor, SBN::Tensor, std::string> get_tensor_from_vertex(
      const Context& ctx, const std::vector<GraphVertex>& graph,
      const std::map<int, std::array<int, 2>>& related_prop,
      const std::vector<Hadron::HadronDispVertex_t>& unique_vertices,
      const std::vector<std::vector<std::size_t>>& vertices,
      const std::vector<std::vector<SBN::Coor>>& slots_perm, const std::vector<std::vector<int>>& t,
      const SpinsFromUniqueVertexIndex& spins_from_unique_vertex_index,
      const std::vector<int>& vertex_indices, const SBN::Tensor& perm, const Slides& slides)
    {

      const auto normalized_type = std::map<char, char>{{'Q', 'Q'}, {'M', 'M'}, {'A', 'M'},
							{'B', 'B'}, {'m', 'm'}, {'a', 'm'}};
      const char type = graph.at(vertex_indices.at(0)).type;
      for (const auto i : vertex_indices)
	if (normalized_type.at(graph.at(i).type) != normalized_type.at(type))
	  throw std::runtime_error("wtf");

      if (type == 'Q')
      {
	return get_props(ctx.prop_callback, graph, unique_vertices, vertices, slots_perm,
			 ctx.flavor_to_mass, t, spins_from_unique_vertex_index, vertex_indices,
			 perm, ctx.num_vecs, slides);
      }
      else if (type == 'M' || type == 'A')
      {
	return get_mesons(ctx.meson_callback, ctx.coeff_map, unique_vertices, vertices, slots_perm,
			  t, vertex_indices, perm, ctx.num_vecs, slides);
      }
      else if (type == 'B')
      {
	return get_baryons(ctx.baryon_callback, graph, ctx.coeff_map, unique_vertices, vertices,
			   slots_perm, t, spins_from_unique_vertex_index, vertex_indices, perm,
			   ctx.num_vecs, slides);
      }
      else if (type == 'm' || type == 'a')
      {
	return get_genprops(ctx.genprop_callback, graph, related_prop, ctx.coeff_map,
			    unique_vertices, vertices, slots_perm, ctx.flavor_to_mass, t,
			    spins_from_unique_vertex_index, vertex_indices, perm, ctx.num_vecs,
			    slides, ctx.lattice_size);
      }
      throw std::runtime_error("unsupported vertex type");
    }

    /// Return a slide relabelling the dimensions
    /// \param slides: slides to relabel
    /// \param m: input mapping

    inline Slides relabel(const Slides& slides, const SBN::remap& m)
    {
      Slides r;
      for (const auto it : slides)
	if (m.count(it.first) > 0)
	  r[m.at(it.first)] = it.second;
      return r;
    }

    /// Return encoded flavor and conj information of the propagators
    /// \param graph: vertices and slot numbers
    /// \param adj_map: list of propagators, vertex -> {slot -> {vertex, slot, flavor, conj}}

    inline std::vector<std::size_t> get_prop_vertices(const std::vector<GraphVertex>& graph,
						      const Hadron::HadronGraphAdjMap_t& adj_map)
    {
      // Extract the flavor and conj from HadronGraphAdjMap_t
      const auto& slot_index_to_vertex_and_slot = get_map_from_slot_index_to_vertex_and_slot(graph);
      std::vector<std::size_t> prop_vertices;
      for (int i = 0; i < graph.size(); ++i)
      {
	if (graph.at(i).type != 'Q')
	  continue;
	const auto& [vertex_src, slot_src] =
	  slot_index_to_vertex_and_slot.at(graph.at(i).slot_indices.at(1));
	const auto& n = adj_map.at(vertex_src + 1).at(slot_src + 1);
	prop_vertices.push_back(get_vertex_from_flavor_and_conj(n.flavor, n.conj));

	// Sanity check for fun
	const auto& [vertex_snk, slot_snk] =
	  slot_index_to_vertex_and_slot.at(graph.at(i).slot_indices.at(0));
	if (n.vertex_num != vertex_snk + 1 || n.slot_num != slot_snk + 1)
	  throw std::runtime_error("wtf");
      }

      return prop_vertices;
    }

    /// Return the characterization of the graph contraction
    /// \param graph_str: vertices and slot numbers
    /// \param adj_map: list of propagators, vertex -> {slot -> {vertex, slot,
    ///                 flavor, conj}}
    /// \param is_time_moving: whether to expect more time slices for this vertex
    /// \param smearing: a nonzero value for smeared vertices
    /// \return {types, slots, fake_graph, prop_vertices}

    inline std::tuple<std::string, std::vector<std::vector<int>>, std::vector<GraphVertex>,
		      std::vector<std::size_t>>
    get_fingerprint(const std::string& graph_str, const Hadron::HadronGraphAdjMap_t& adj_map,
		    const std::vector<char>& smearing, const std::vector<char>& is_time_moving)
    {
      // FIXME: return the disco loops
      if (smearing == std::vector<char>(1))
	throw std::runtime_error("compute_corr_fun: unsupported disco loops");

      // Process graph
      const auto& fake_graph = get_graph_vertex(graph_str, smearing, is_time_moving);
      const auto& [graph, related_prop] = remove_fake_connections(fake_graph);

      // Get the contraction plan
      const auto& [almost_plan, slides_dims, spin_size, color_to_spin] =
	get_contraction_plan(graph);

      std::string types;
      types.reserve(almost_plan.size() * 2);
      std::vector<std::vector<int>> slots;
      slots.reserve(almost_plan.size() * 2);
      for (const auto& step : almost_plan)
      {
	for (const auto& tensor_indices :
	     std::array<std::vector<int>, 2>{step.first_tensor_indices, step.second_tensor_indices})
	{
	  for (int tensor_index : tensor_indices)
	  {
	    if (tensor_index < graph.size())
	    {
	      types.push_back(graph.at(tensor_index).type);
	      slots.push_back(graph.at(tensor_index).slot_indices);
	    }
	  }
	}
      }

      // Add the slots lost by `remove_fake_connections`
      for (const auto& it : related_prop)
	for (int vertex : std::array<int, 3>{it.first, it.second.at(0), it.second.at(1)})
	  slots.push_back(fake_graph.at(vertex).slot_indices);

      // Compute the prop vertices
      const auto& prop_vertices = get_prop_vertices(fake_graph, adj_map);

      return {types, slots, fake_graph, prop_vertices};
    }

    /// Return the reordering of the vertices and the permutation of the slots on each vertex
    /// \param slots_target: sequence of slots of the target contraction
    /// \param slots: sequence of slots of the current graph
    /// \param graph: current graph
    /// \return {feasible, vertex_reordering, slots_perm}
    /// - feasible: whether there is a mapping between the given graphs
    /// - vertex_reordering: v[target vertex index]=origin vertex index
    /// - slots_perm: do_perm(origin_slots, slots_perm[i]) -> target_slots

    inline std::tuple<bool, std::vector<int>, std::vector<std::vector<int>>>
    get_vertex_reordering_and_permutation(const std::vector<std::vector<int>>& slots_target,
					  const std::vector<std::vector<int>>& slots,
					  const std::vector<GraphVertex>& fake_target_graph,
					  const std::vector<GraphVertex>& fake_graph)
    {
      // Trivial check
      if (slots_target.size() != slots.size() || fake_target_graph.size() != fake_graph.size())
	return {false, {}, {}};

      // Construct the map from current graph to target graph
      std::map<int, std::set<int>> potential_targets;
      std::map<int, int> origin_assigned;
      std::set<int> target_assigned;
      auto set_translation = [&](int origin_slot, int target_slot, bool* progress) {
	origin_assigned[origin_slot] = target_slot;
	target_assigned.insert(target_slot);
	while (true)
	{
	  bool redo = false;
	  for (auto& it : potential_targets)
	  {
	    if (origin_assigned.count(it.first) > 0)
	      continue;
	    std::set<int> new_set;
	    for (int r : it.second)
	      if (target_assigned.count(r) == 0)
		new_set.insert(r);
	    if (new_set.size() == 0)
	      return false;
	    if (new_set.size() == 1)
	    {
	      origin_assigned[it.first] = *new_set.begin();
	      target_assigned.insert(*new_set.begin());
	      redo = true;
	    }
	    if (it.second != new_set)
	    {
	      *progress = true;
	      it.second = new_set;
	    }
	  }
	  if (!redo)
	    break;
	}
	return true;
      };
      while (true)
      {
	bool progress = false;
	for (std::size_t i = 0; i < slots.size(); ++i)
	{
	  const auto& target_slot = slots_target.at(i);
	  const auto& current_slot = slots.at(i);
	  if (target_slot.size() != current_slot.size())
	    return {false, {}, {}};

	  // Make all target slot candidates of each current slot
	  for (int s : current_slot)
	  {
	    if (origin_assigned.count(s) > 0)
	      continue;
	    const auto& old_set = potential_targets.count(s) == 0
				    ? std::set<int>(target_slot.begin(), target_slot.end())
				    : potential_targets.at(s);
	    std::set<int> new_set;
	    for (int r : target_slot)
	      if (old_set.count(r) > 0 && target_assigned.count(r) == 0)
		new_set.insert(r);
	    if (new_set.size() == 0)
	      return {false, {}, {}};
	    if (potential_targets.count(s) == 0 || new_set != old_set)
	    {
	      potential_targets[s] = new_set;
	      progress = true;
	    }
	    if (new_set.size() == 1)
	    {
	      if (!set_translation(s, *new_set.begin(), &progress))
		return {false, {}, {}};
	    }
	  }
	}
	bool missing = false;
	for (const auto& it : potential_targets)
	  if (it.second.size() > 1)
	    missing = true;
	if (!missing)
	  break;
	if (progress)
	  continue;
	for (auto& it : potential_targets)
	{
	  if (it.second.size() > 1)
	  {
	    const int target_slot = it.second.count(it.first) == 1 ? it.first : *it.second.begin();
	    it.second = std::set<int>{target_slot};
	    if (!set_translation(it.first, target_slot, &progress))
	      return {false, {}, {}};
	    break;
	  }
	}
      }

      const auto& to_target = origin_assigned;

      const auto& [graph0, related_prop] = remove_fake_connections(fake_graph);
      const auto& [target_graph0, target_related_prop] = remove_fake_connections(fake_target_graph);
      if (related_prop.size() != target_related_prop.size())
	return {false, {}, {}};
      const auto& graph = fake_graph;
      const auto& target_graph = fake_target_graph;

      // Get a map of the target graph
      std::map<std::set<int>, std::vector<int>> vertex_index;
      for (int i = target_graph.size() - 1; i >= 0; --i)
      {
	const auto& slot_indices = target_graph.at(i).slot_indices;
	vertex_index[std::set<int>(slot_indices.begin(), slot_indices.end())].push_back(i);
      }

      // Get the reordering and the permutation
      std::vector<int> target_to_origin(graph.size(), -1);
      std::vector<std::vector<int>> slots_perm(graph.size());
      for (int i = 0; i < graph.size(); ++i)
      {
	const auto& original_slot_indices = graph.at(i).slot_indices;
	std::vector<int> translate_slot_indices;
	translate_slot_indices.reserve(original_slot_indices.size());
	for (int s : original_slot_indices)
	  translate_slot_indices.push_back(to_target.at(s));
	const auto& sorted_translate_slot_indices =
	  std::set<int>(translate_slot_indices.begin(), translate_slot_indices.end());
	if (vertex_index.count(sorted_translate_slot_indices) == 0)
	  return {false, {}, {}};
	auto& potential_target_vertex_index = vertex_index.at(sorted_translate_slot_indices);
	if (potential_target_vertex_index.size() == 0)
	  return {false, {}, {}};
	const int target_vertex_index = potential_target_vertex_index.back();
	potential_target_vertex_index.pop_back();
	target_to_origin.at(target_vertex_index) = i;
	slots_perm.at(i) =
	  SBN::get_perm(translate_slot_indices, target_graph.at(target_vertex_index).slot_indices);
      }

      // Check that the ordering is consistent
      for (int i = 0; i < graph.size(); ++i)
      {
	if (target_graph.at(i).type != graph.at(target_to_origin.at(i)).type)
	  throw std::runtime_error("wtf");
      }

      return {true, target_to_origin, slots_perm};
    }

    /// Return the number of times a vertex is read
    /// \param graph: vertices and slot numbers
    /// \param plan: contraction plan

    inline std::map<std::vector<int>, int>
    get_number_of_occurrences(const std::vector<GraphVertex>& graph, const ContractionPlan& plan)
    {
      std::map<std::vector<int>, int> r;
      int result_index = graph.size();
      for (const auto& step : plan)
      {
	for (const auto& tensor_indices :
	     std::array<std::vector<int>, 2>{step.first_tensor_indices, step.second_tensor_indices})
	{
	  if (r.count(tensor_indices) == 0)
	    r[tensor_indices] = 1;
	  else
	    r.at(tensor_indices)++;
	}
	r[{result_index++}] = 0;
      }
      return r;
    }

    /// Return the dimensions of the graph vertex indices for each distillation object
    /// \param graph: vertices and slot numbers
    /// \param slides: distillation basis dimension that can be parallelize
    /// \param spin_size: dimension of the spins
    /// \param color_to_spin: spin label associated to the color label
    /// \param num_vecs: default number of vectors
    /// \return list of pairs of indices and slides

    inline std::vector<std::tuple<std::vector<int>, Slides>>
    get_graph_vertices_size(const std::vector<GraphVertex>& graph, const Slides& slides,
			    const SBN::SCoor& spin_size, const std::map<int, int>& color_to_spin,
			    const std::vector<int>& vertices, int num_vecs)
    {
      std::vector<std::tuple<std::vector<int>, Slides>> r;
      const std::string color_labels = "vwx";
      const std::string spin_labels = "rst";
      Slides sizes, last_sizes;
      std::vector<int> v;
      for (int vertices_i = 0; vertices_i < vertices.size(); ++vertices_i)
      {
	const auto vertex_index = vertices.at(vertices_i);
	const auto& slots = graph.at(vertex_index).slot_indices;
	bool success = true;
	for (int i = 0; i < slots.size(); ++i)
	{
	  // Do color
	  int label_size =
	    (slides.count(slots.at(i)) > 0 ? slides.at(slots.at(i)).at(1) : num_vecs);
	  if (sizes.count(color_labels.at(i)) == 0)
	  {
	    sizes[color_labels.at(i)] = {0, label_size};
	  }
	  else if (sizes.at(color_labels.at(i)).at(1) != label_size)
	  {
	    throw std::runtime_error("wtf");
	  }

	  // Do spin
	  label_size = spin_size.at(color_to_spin.at(slots.at(i)));
	  if (sizes.count(spin_labels.at(i)) == 0)
	  {
	    sizes[spin_labels.at(i)] = {0, label_size};
	  }
	  else if (sizes.at(spin_labels.at(i)).at(1) != label_size)
	  {
	    success = false;
	    break;
	  }
	}
	if (success)
	{
	  v.push_back(vertex_index);
	  last_sizes = sizes;
	}
	else
	{
	  if (v.size() == 0)
	    throw std::runtime_error("wtf");
	  r.push_back({v, last_sizes});
	  vertices_i--;
	  v.clear();
	  sizes.clear();
	  last_sizes.clear();
	  continue;
	}
      }
      r.push_back({v, sizes});
      return r;
    }

    /// Remove labels from the input order that are on `order0` but not on `order1`
    /// \parm order: input order
    /// \param order0: labels to look for
    /// \param order1: labels to keep

    SBN::Order restrict(const SBN::Order& order, const SBN::Order& order0, const SBN::Order& order1)
    {
      if (!SBN::contain_labels(order0, order1))
	throw std::runtime_error("restrict: invalid input");

      // Quick shortcut
      if (SBN::contain_labels(order1, order0))
	return order;

      // Get new order
      const auto& s0 = std::set<char>(order0.begin(), order0.end());
      const auto& s1 = std::set<char>(order1.begin(), order1.end());
      SBN::Order new_order;
      new_order.reserve(order.size());
      for (char c : order)
	if (s0.count(c) == 0 || s1.count(c) > 0)
	  new_order.push_back(c);

      return new_order;
    }

    /// Remove labels from the input tensors that are on `order0` but not on `order1`
    /// \parm order: input order
    /// \param order0: labels to look for
    /// \param order1: labels to keep

    SBN::Tensor restrict(const SBN::Tensor& t, const SBN::Order& order0, const SBN::Order& order1)
    {
      if (!SBN::contain_labels(order0, order1))
	throw std::runtime_error("restrict: invalid input");

      // Quick shortcut
      if (SBN::contain_labels(order1, order0))
	return t;

      // Get new order and new distribution
      const auto& new_order = restrict(t.order, order0, order1);

      // Avoid removing sparse dimensions
      if (new_order.size() <= t.n_sp || SBN::Order(t.order.end() - t.n_sp, t.order.end()) !=
					  SBN::Order(new_order.end() - t.n_sp, new_order.end()))
	throw std::runtime_error("restrict: unsupported the elimination of sparse dimensions");

      // Get new size
      auto s = SBN::get_scoor(t.order, t.size);
      const auto& s1 = std::set<char>(order1.begin(), order1.end());
      for (char c : order0)
	if (s1.count(c) == 0)
	  s.at(c) = std::min(1, s.at(c));

      // Make the copy
      const auto& t_restrict = SBN::slice_kv(t, {}, s);
      return SBN::reshape(t_restrict, new_order);
    }

    /// Return the values of a list of correlation functions with the same graph
    /// \param fake_graph: vertices and slot numbers
    /// \param adj_map: list of propagators, vertex -> {slot -> {vertex, slot,
    ///                 flavor, conj}}
    /// \param unique_vertices: vector of disp vertices
    /// \param vertices: unique_vertices[vertices[i][j]] disp vertex for vertex j for corr fun i
    /// \param slots_perm: do_perm(origin_slots, slots_perm[i]) -> target_slots
    /// \param t: time[i][j] time slice for vertex j for corr fun i
    /// \param moving_vertex: vertex with variable time slices
    /// \param max_mem: maximum memory use for contractions
    /// \param comm: communicator
    /// \param cout: output stream

    template <typename stream>
    inline std::vector<std::complex<double>>
    compute_corr_fun(const Context& ctx, const std::vector<GraphVertex>& fake_graph,
		     const std::vector<Hadron::HadronDispVertex_t>& unique_vertices,
		     const std::vector<std::vector<std::size_t>>& vertices,
		     const std::vector<std::vector<SBN::Coor>>& slots_perm,
		     const std::vector<std::vector<int>>& t, std::size_t max_mem,
		     const SBN::Comm& comm, stream& cout)
    {
      Datalib::TensorStorage::detail::Tracker t_("compute corr fun");

      // Check that SBN::IndexType is good enough for the number of correlation functions to compute
      if (std::numeric_limits<SBN::IndexType>::max() <= t.size())
	throw std::runtime_error(
	  "compute_corr_fun: too many correlation functions computed as once!");

      // FIXME: return the disco loops (single unsmeared meson)
      if (fake_graph.size() == 1 && (fake_graph.at(0).type == 'm' || fake_graph.at(0).type == 'a'))
	throw std::runtime_error("compute_corr_fun: unsupported disco loops");

      // Process graph
      const auto& [graph_, related_prop_] = remove_fake_connections(fake_graph);
      /// Hack to deal with amdclang++
      const auto& graph = graph_;
      const auto& related_prop = related_prop_;

      // Check arguments
      int num_vertices = -1;
      for (const auto& ti : t)
      {
	if (graph.size() < ti.size())
	  throw std::runtime_error("compute_corr_fun: invalid `t` input");
	if (num_vertices >= 0 && ti.size() != num_vertices)
	  throw std::runtime_error("compute_corr_fun: invalid `t` input");
	num_vertices = ti.size();
      }
      for (const auto& it : vertices)
	if (graph.size() != it.size())
	  throw std::runtime_error("compute_corr_fun: invalid `vertices` input");
      for (const auto& it : slots_perm)
	if (graph.size() != it.size())
	  throw std::runtime_error("compute_corr_fun: invalid `slots_perm` input");

      // Get the number of unique vertices
      const auto& [unique_vertex_indices, group, spins_from_unique_vertex_index_,
		   corr_index_and_spin, num_unique_vertex_indices, num_unique_dist,
		   factor_mem_build] =
	get_unique_vertices(graph, related_prop, ctx.coeff_map, unique_vertices, vertices,
			    slots_perm, t, ctx.lattice_size[3], comm, cout);
      const auto& spins_from_unique_vertex_index =
	spins_from_unique_vertex_index_; // NOTE: hack required by amdclang++

      // Get the contraction plan
      const auto& [almost_plan, slides_dims, spin_size_, color_to_spin_] =
	get_contraction_plan(graph, num_unique_vertex_indices);
      const auto& spin_size = spin_size_;	  // NOTE: hack required by amdclang++
      const auto& color_to_spin = color_to_spin_; // NOTE: hack required by amdclang++

      // Sort the graphs to increase the change of batching graphs with the same vertices
      const auto& [perm_, vertex_cf_indices_, plan_] =
	get_optimal_perm(graph, almost_plan, unique_vertex_indices, group, corr_index_and_spin,
			 num_unique_vertex_indices, comm, cout);
      const auto& perm = perm_;				  // NOTE: hack required by amdclang++
      const auto& vertex_cf_indices = vertex_cf_indices_; // NOTE: hack required by amdclang++
      const auto& plan = plan_;				  // NOTE: hack required by amdclang++

      // Get the maximum batch size and the slide size
      // NOTE: this is the only function that depends on a local property of the node, so broadcast the result
      const auto& [max_cache_size_, max_batch_size_, max_slides] = SBN::broadcast(
	get_maximum_corr_funs(graph, plan, color_to_spin, spin_size, slides_dims, ctx.num_vecs,
			      max_mem, vertex_cf_indices, unique_vertex_indices,
			      num_unique_vertex_indices, num_unique_dist, factor_mem_build, perm));
      const auto& max_cache_size = max_cache_size_; // NOTE: hack required by amdclang++
      const auto& max_batch_size = max_batch_size_; // NOTE: hack required by amdclang++

      // Print plan
      print_plan(graph, plan, vertex_cf_indices, max_cache_size, max_batch_size, max_slides,
		 t.size(), perm, cout);

      const auto& total_num_occ = get_number_of_occurrences(graph, plan);

      Slides slides;
      for (const auto& it : spin_size)
	slides[it.first] = {0, it.second};

      // Track of the last slice of `perm` computed for this process
      SBN::Coor last_batch_from(perm.size.size()), last_batch_size(perm.size.size());
      // "Tape" of auxiliary tensors
      std::map<std::vector<int>, SBN::Tensor> tensors;
      // Indicator of the tensors cached from previous iteration
      std::vector<bool> prev_do_cache(graph.size() + plan.size(), false);

      // NOTE: Don't support cache
      if (max_cache_size.size() > 0)
	throw std::runtime_error("wtf");

      const auto& tr = SBN::transform_for_each_block(
	perm, max_batch_size, SBN::Options::Ordering::Lexicographic,
	[&](const auto& batch_perm, const auto& batch_index, const auto& batch_size,
	    const auto& current_perm_from, const auto& next_perm_from,
	    const auto& next_perm_size) -> SBN::Tensor {
	  // Update the contraction plan with the pieces already on cache and the ones that will be
	  const auto& [do_skip_, do_cache_] = get_skip_and_cache_for_the_plan(
	    graph, plan, vertex_cf_indices, perm, last_batch_from, last_batch_size,
	    current_perm_from, batch_perm.size, next_perm_from, next_perm_size, prev_do_cache);
	  const auto& do_skip = do_skip_;   /// Hack to deal with amdclang++
	  const auto& do_cache = do_cache_; /// Hack to deal with amdclang++
	  prev_do_cache = do_cache;
	  print_plan_details(graph, plan, do_skip, do_cache, cout, comm);

	  // Track number of uses of each tensor
	  std::map<std::vector<int>, int> num_occ;

	  // Print progress
	  if (SBN::get_rank(comm.comm) == 0)
	  {
	    cout << "  building distillation objects and doing contractions for ";
	    if (batch_size > 1)
	    {
	      cout << "cache batch: " << batch_index << " (out of " << batch_size << ") ";
	    }
	  }

	  auto get_tensor = [&](const std::vector<int>& tensor_index,
				const SBN::remap& tensor_relabel,
				bool skip_contraction) -> SBN::Tensor {
	    const auto already_on_cache = do_skip.at(tensor_index.at(0));

	    //if (tensors.count(tensor_index) > 0 && tensors.at(tensor_index).size.size() > 0)
	    if (num_occ.count(tensor_index) > 0)
	    {
	      num_occ.at(tensor_index)--;
	      if (tensors.count(tensor_index) == 0 && !skip_contraction)
		throw std::runtime_error("wtf");
	      return tensors.count(tensor_index) > 0 ? tensors.at(tensor_index) : SBN::Tensor{};
	    }
	    num_occ[tensor_index] = total_num_occ.at(tensor_index) - 1;
	    if (!already_on_cache)
	    {
	      const auto& this_perm = SBN::make_sure(
		restrict(batch_perm, batch_perm.order, vertex_cf_indices.at(tensor_index)), {},
		SBN::Options::Distribution::Default, SBN::Options::Alloc::Default,
		SBN::Options::TightView::TrueAndRedistribute);
	      const auto& c =
		get_tensor_from_vertex(ctx, graph, related_prop, unique_vertices, vertices,
				       slots_perm, t, spins_from_unique_vertex_index, tensor_index,
				       this_perm, relabel(slides, SBN::inverse(tensor_relabel)));
	      const auto& n = SBN::contract(std::get<0>(c), std::get<1>(c), std::get<2>(c));
	      tensors[tensor_index] = n;
	      return n;
	    }
	    else if (vertex_cf_indices.count(tensor_index) > 0)
	    {
	      // Invoke `get_tensor_from_vertex` with an empty `perm` because it is compulsory to
	      // invoke the function for all processes but we have already the result
	      const auto& empty_perm = SBN::slice(batch_perm, SBN::Coor(batch_perm.size.size()),
						  SBN::Coor(batch_perm.size.size()));
	      const auto& this_perm = SBN::make_sure(
		restrict(empty_perm, batch_perm.order, vertex_cf_indices.at(tensor_index)), {},
		SBN::Options::Distribution::Default, SBN::Options::Alloc::Default,
		SBN::Options::TightView::TrueAndRedistribute);
	      get_tensor_from_vertex(ctx, graph, related_prop, unique_vertices, vertices,
				     slots_perm, t, spins_from_unique_vertex_index, tensor_index,
				     this_perm, relabel(slides, SBN::inverse(tensor_relabel)));
	    }
	    return !skip_contraction ? tensors.at(tensor_index) : SBN::Tensor{};
	  };

	  auto t0 = SBN::w_time();
	  int last_result_index = (int)graph.size() - 1;
	  for (const auto& it : plan)
	  {
	    const auto do_skip_step = do_skip.at(last_result_index + 1);

	    // Get the tensors
	    const SBN::Tensor& t0 =
	      get_tensor(it.first_tensor_indices, it.first_tensor_relabel, do_skip_step);
	    const SBN::Tensor& t1 =
	      get_tensor(it.second_tensor_indices, it.second_tensor_relabel, do_skip_step);
	    if (!do_skip_step && (!SBN::have_same_labels(t0.order, it.first_tensor_order) ||
				  !SBN::have_same_labels(t1.order, it.second_tensor_order)))
	      throw std::runtime_error("wtf");

	    const auto& t0r =
	      do_skip_step ? SBN::Tensor{}
			   : SBN::slice_kv(SBN::relabel(t0, it.first_tensor_relabel),
					   it.first_tensor_slice_from, it.first_tensor_slice_size);
	    const auto& t1r =
	      do_skip_step
		? SBN::Tensor{}
		: SBN::slice_kv(SBN::relabel(t1, it.second_tensor_relabel),
				it.second_tensor_slice_from, it.second_tensor_slice_size);

	    // Contract the tensors
	    const auto& result_tensor_index = std::vector<int>{++last_result_index};
	    if (!do_skip_step)
	      tensors[result_tensor_index] = SBN::contract(t0r, t1r, it.result_tensor_order);
	    num_occ[result_tensor_index] = total_num_occ.at(result_tensor_index);

	    // Release tape tensors when they aren't going to be used anymore
	    if (!do_cache.at(it.first_tensor_indices.at(0)) &&
		num_occ.at(it.first_tensor_indices) <= 0 &&
		tensors.count(it.first_tensor_indices) > 0)
	      SBN::release(tensors.at(it.first_tensor_indices));
	    if (!do_cache.at(it.second_tensor_indices.at(0)) &&
		num_occ.at(it.second_tensor_indices) <= 0 &&
		tensors.count(it.second_tensor_indices) > 0)
	      SBN::release(tensors.at(it.second_tensor_indices));
	  }
	  t0 = SBN::w_time() - t0;
	  if (SBN::get_rank(comm.comm) == 0)
	  {
	    cout << " took " << t0 << " secs " << std::endl;
	  }

	  // Update perm ranges
	  last_batch_from = current_perm_from;
	  last_batch_size = batch_perm.size;

	  // Return the current correlation functions if it was produced
	  if (tensors.count({last_result_index}) > 0)
	    return tensors.at({last_result_index});

	  // Check that if it wasn't produced then the batch_perm should be empty
	  if (SBN::volume(batch_perm.size) > 0)
	    throw std::runtime_error("wtf");

	  // Return a compatible empty tensor
	  return batch_perm;
	});

      // Get the resulting tensor on cpu
      const auto& tr0 = SBN::make_sure(tr, perm.order, SBN::Options::Distribution::OnMaster,
				       SBN::Options::Alloc::Host, SBN::Options::TightView::True);
      const auto& perm0 = SBN::make_sure(perm, {}, SBN::Options::Distribution::OnMaster,
					 SBN::Options::Alloc::Host, SBN::Options::TightView::True);
      std::vector<std::complex<double>> r(SBN::get_rank(comm.comm) == 0 ? vertices.size()
									: 0); // corr fun results
      SBN::for_each_coor_value(
	perm0, tr0,
	[&](const SBN::Coor&, const SBN::value_type& i_cmplx, const SBN::value_type& value) {
	  SBN::IndexType i = (SBN::IndexType)std::real(i_cmplx);
	  // Ignore blank entries
	  if (i < t.size())
	    r.at(i) += value;
	});

      return r;
    }

    /// Return the first active time-slices used for a correlator
    /// \param corr: input corr graph

    inline int get_t_start(const Hadron::KeyHadronSUNClassNPartNPtCorr_t& corr)
    {
      if (corr.npoint.size() == 1) {
        const int t = corr.npoint[1].t_slice;
        if (t == -1) return 0; // standalone moving point references itself
        if (t >= 0) return t;
        throw std::runtime_error("get_t_start: invalid standalone time reference");
      }
      if (corr.npoint.size() < 2)
	throw std::runtime_error("get_t_start: unsupported case");

      for (int npt = 1; npt <= corr.npoint.size(); ++npt)
      {
	if (corr.npoint[npt].t_slice < 0)
	  return corr.npoint.ref().at(-corr.npoint[npt].t_slice - 1).t_slice;
      }

      throw std::runtime_error("get_t_start: unsupported case");
    }

    /// Return the list of time slices associated to the vertices
    /// \param corr: input corr graph
    /// \param t_origin: global shift for all time slices
    /// \param Nt_corr: number of time slices of the correlation function
    /// \param T: lattice time dimension

    inline std::vector<std::vector<int>>
    get_t_slices(const Hadron::KeyHadronSUNClassNPartNPtCorr_t& corr, int t_origin, int Nt_corr,
		 int T)
    {
      if (corr.npoint.size() == 1) {
        const int t = corr.npoint[1].t_slice;
        if (t < -1) throw std::runtime_error("get_t_slices: invalid standalone time reference");
        std::vector<std::vector<int>> r(Nt_corr, std::vector<int>(1));
        for (int i = 0; i < Nt_corr; ++i)
          r[i][0] = SBN::norm_coor((t == -1 ? i : t) + t_origin, T);
        return r;
      }
      if (corr.npoint.size() < 2)
	throw std::runtime_error("get_t_slices: unsupported case");

      std::vector<int> tslices0;
      int neg_tslice = -1;
      for (int npt = 1; npt <= corr.npoint.size(); ++npt)
      {
	if (corr.npoint[npt].t_slice < 0)
	  neg_tslice = npt - 1;
	tslices0.push_back(corr.npoint[npt].t_slice);
      }

      if (neg_tslice < 0)
	throw std::runtime_error("get_t_slices: unsupported case");

      std::vector<std::vector<int>> r(Nt_corr, tslices0);
      const int t_start = get_t_start(corr);
      for (int i = 0; i < Nt_corr; ++i)
	r.at(i).at(neg_tslice) = t_start + i;

      for (auto& ri : r)
	for (int& tslice : ri)
	  tslice = (tslice + t_origin + T) % T;

      return r;
    }

    /// Return whether to flip sign due to boundary conditions
    /// \param quark_count: quark counts (?)
    /// \param t: time slices
    /// NOTE: code from function flipAntiBC in hadron_corr_util.cc

    inline bool flipAntiBC(const std::vector<int>& quark_count, const std::vector<int>& t)
    {
      if (quark_count.size() != t.size())
	throw std::runtime_error("wtf");

      bool flipP = false;

      for (int npt = static_cast<int>(t.size()) - 2; npt >= 0; --npt)
      {
	// Check if a forward step wrapped around. If so, flip sign if odd number of quarks.
	// If same or positive, then just move forward
	if (t.at(npt) < t.at(npt + 1) && std::abs(quark_count.at(npt + 1)) % 2 == 1)
	  flipP = !flipP;
      }
      return flipP;
    }

    template <typename stream>
    struct ProgressBar {
      const std::size_t n;
      std::size_t i;
      const std::size_t num_stops;
      bool actually_do_print;
      stream& cout;
      ProgressBar(std::size_t n, stream& cout, bool actually_do_print = true)
	: n(n), i(0), num_stops(std::min(n, 10ul)), actually_do_print(actually_do_print), cout(cout)
      {
	print();
      }

      // Prefix increment ++i
      ProgressBar& operator++()
      {
	++i;
	print();
	return *this;
      }

      // Postfix increment i--
      ProgressBar operator++(int)
      {
	auto old = *this;
	operator++();
	return old;
      }

      void print() const
      {
	if (!actually_do_print)
	  return;
	if (n == 0)
	  return;
	auto num_irreg_element = n % num_stops;
	auto first_reg_element = (n / num_stops + 1) * num_irreg_element;
	bool do_print = ((i - (i < first_reg_element ? 0 : first_reg_element)) %
			   (n / num_stops + (i < first_reg_element ? 1 : 0)) ==
			 0);
	if (do_print)
	{
	  if (i > 0)
	    cout << " ";
	  cout << i * 100 / n << "%";
	  if (i >= n)
	    cout << " done!" << std::endl;
	  cout.flush();
	}
      }
    };

    /// Represent into a string a set of phases
    /// \param phases: set of phases

    inline std::string phases_to_string(const std::set<std::array<int, 3>>& phases)
    {
      std::stringstream s;
      std::size_t i = 0;
      for (const auto phase : phases)
      {
	s << phase[0] << " " << phase[1] << " " << phase[2];
	if (i + 1 < phases.size())
	  s << ",";
	++i;
      }
      return s.str();
    }

    /// Represent into a string a list of timeslices
    /// \param timeslices: list to timeslices

    inline std::string timeslices_to_string(const std::vector<int> timeslices)
    {
      std::stringstream s;
      std::size_t i = 0;
      for (const auto t : timeslices)
      {
	s << t;
	if (i + 1 < timeslices.size())
	  s << " ";
	++i;
      }
      return s.str();
    }
  }

  template <typename stream = decltype(std::cout)>
  inline MapObject<KeyHadronSUNClassNPartNPtCorr_t, std::vector<std::complex<double>>>
  evaluate_graphs_with_superb(const CorrGraphMap_t& corr_graph, bool zeroUnsmearedGraphsP,
                              bool average_1pt_diagrams,
			      const prop_elementals_callback& prop_callback,
			      const baryon_elementals_callback& baryon_callback,
			      const meson_elementals_callback& meson_callback,
			      const genprop_elementals_callback& genprop_callback,
			      const FlavorToMassMap& flavor_to_mass, int num_vecs, int t_origin,
			      int Nt_corr, stream& cout = std::cout,
			      SBN::MpiComm mpi_comm = SBN::get_global_mpi_comm(),
                              const disco_elementals_callback& disco_callback = {})
  {
    if (average_1pt_diagrams)
      throw std::runtime_error("evaluate_graphs_with_superb: average_1pt_diagrams=true is unsupported");
    Datalib::TensorStorage::detail::Tracker t_("evaluate corr fun");

    // Get the maximum memory available for contractions
    std::size_t max_mem = 0;
    const char* l = std::getenv("REDSTAR_SB_MEM_GB");
    if (l)
      max_mem = std::max(0, std::atoi(l)) * 1024ul * 1024ul * 1024ul;
    if (max_mem == 0)
    {
#ifdef SUPERBBLAS_USE_GPU
      // If no value is suggested, use 90% of the GPU 0 available memory discounting the cache
      const auto cache_mem = superbblas::detail::getMaxGpuCacheSize();
      max_mem = superbblas::detail::totalGpuMemory(0);
      if (cache_mem >= max_mem)
	throw std::runtime_error(
	  "Invalid value of cache memory: too large; check `SB_CACHEGB_GPU'");
      max_mem = (max_mem - cache_mem) * 0.9;
#else
      // If no value is suggested, use 10% of the CPU available memory
      max_mem = std::size_t(sysconf(_SC_PAGESIZE)) * sysconf(_SC_PHYS_PAGES) / 10;
#endif
    }

    using namespace Hadron::detail;
    const auto lattice_size = corr_graph.layout.latt_size;
    const int T = lattice_size[3];
    const auto& corr_graph_outer_coeffs = corr_graph.vertex_coeff_map;
    Context ctx{
      corr_graph.vertex_coeff_map,
      prop_callback,
      baryon_callback,
      meson_callback,
      genprop_callback,
      flavor_to_mass,
      num_vecs,
      lattice_size,
    };

    if (Nt_corr == 0)
      Nt_corr = std::min(corr_graph.Nt_corr, T);
    if (Nt_corr > T)
      throw std::runtime_error("evaluate_graphs_with_superb: invalid value of `Nt_corr`");
    t_origin = SBN::norm_coor(t_origin, T);
    if (SBN::get_rank(mpi_comm) == 0)
    {
      cout << "using lattice= " << lattice_size.at(0) << " " << lattice_size.at(1) << " "
	   << lattice_size.at(2) << " " << lattice_size.at(3) << " ; " //
	   << " Nt_coor= " << Nt_corr << " ; "			       //
	   << " num_vecs= " << num_vecs << " ; "		       //
	   << " t_origin= " << t_origin << std::endl;
    }

    const MapObject<KeyHadronSUNClassNPartNPtCorr_t, ValCorrIntGraphMap_t>& corr_graph_map =
      corr_graph.corr_graph_map;
    const std::vector<KeyHadronClassNPartNPtConnGraph_t>& int_graph_list = corr_graph.int_graph_list;
    // struct KeyHadronClassNPartNPtConnGraph_t
    // {
    //   std::string                                         graph;             /*!< Label for the graph */
    //   std::string                                         ensemble;          /*!< Label for the ensemble */
    //   HadronGraphAdjMap_t                                 adj_map;           /*!< Adjacency map of vertex connections */
    //   std::map<int, std::pair<HadronDispVertex_t,int> >   vertices;          /*!< Vertex info [vertex_num -> {vertex,npt}] */
    //   HadronDiagramTimeSlices_t                           diag_time_slices;  /*!< Diagram time-slice structure */
    // }

    // NOTE: genprops time slides stores source, current, and sink time slices with a single number
    if ((std::size_t)T * (std::size_t)T * (std::size_t)T > std::numeric_limits<int>::max())
      throw std::runtime_error(
	"evaluate_graphs_with_superb: unsupported that big time extension, fix it");

    unordered_map<Hadron::HadronDispVertex_t, std::size_t> vertex_to_index(int_graph_list.size());
    std::size_t num_vertices = 0;

    using GraphSmearingMovingAndAdj =
      std::tuple<std::string, std::vector<char>, std::vector<char>, Hadron::HadronGraphAdjMap_t>;

    auto get_smearing = [](const std::map<int, std::pair<HadronDispVertex_t, int>>& vertices) {
      std::vector<char> r(vertices.size());
      for (const auto& it : vertices)
	r.at(it.first - 1) = it.second.first.smearedP;
      return r;
    };
    auto has_unsmeared_graph = [&](const std::vector<int>& corr_graph_indices) {
      for (int corr_graph_index : corr_graph_indices)
	if (get_smearing(int_graph_list.at(corr_graph_index).vertices) ==
	    std::vector<char>(1))
	  return true;
      return false;
    };
    auto get_phases_from_vertex = [](const HadronDispVertex_t& h) { return h.phasings; };
    auto get_time_slices_for_sorting = [](const std::vector<int>& tt, int T, int Nt_corr) {
      if (tt.size() == 0)
	throw std::runtime_error("wtf");
      // The goal is to avoid recomputing inversions, so sort the graphs by the sources and the maximum distance
      // to the sink. Consider two cases:
      // a) for 2pt, return (time-source, -Nt_corr)
      // b) for 3pt, return (time-source, time-sink - time_source)
      if (tt.size() == 2 && tt.at(0) == -1)
      {
	return std::vector<int>{tt.at(1), -Nt_corr};
      }
      else if (tt.size() == 3 && tt.at(1) == -1)
      {
	return std::vector<int>{tt.at(2), -SBN::norm_coor(tt.at(2) - tt.at(0), T)};
      }
      else
      {
	std::vector<int> r;
	r.reserve(tt.size());
	int tref = -1;
	for (auto t = tt.rbegin(); t != tt.rend(); ++t)
	{
	  if (*t < 0)
	    continue;
	  if (tref < 0)
	    r.push_back(tref = *t);
	  else
	    r.push_back(-SBN::norm_coor(*t - tref, T));
	}
	return r;
      }
    };

    using VertexPhases = std::set<std::array<int, 3>>;

    using Timeslices = std::vector<int>;
    using TimeslicesMap = unordered_map<Timeslices, std::size_t>;

    using Perm = std::vector<SBN::Coor>;
    using PermAndTimeslicesMap = unordered_map<Perm, TimeslicesMap>;

    // Either unique vertex index (for non-props) or unique prop index (for props)
    using Vertices = std::vector<std::size_t>;
    using VerticesPermAndTimeslicesMap = unordered_map<Vertices, PermAndTimeslicesMap>;

    using GraphFingerprint =
      std::tuple<std::string, std::vector<std::vector<int>>, std::vector<GraphVertex>>;
    using GraphFingerprintOrderingAndPerm =
      std::tuple<GraphFingerprint, std::vector<std::size_t>, std::vector<int>, Perm>;
    unordered_map<GraphSmearingMovingAndAdj, GraphFingerprintOrderingAndPerm> to_group(16);

    using SlotsAndGraph = std::tuple<std::vector<std::vector<int>>, std::vector<GraphVertex>>;
    std::unordered_map<std::string, std::vector<SlotsAndGraph>> fingerprints(16);

    using FingerprintSlotsPhasesAndTimerefs =
      std::tuple<std::string, std::vector<std::vector<int>>, VertexPhases, std::vector<int>>;
    using GraphNumEntriesAndVerticesPermAndTimeslicesMap =
      std::tuple<std::vector<GraphVertex>, std::size_t, VerticesPermAndTimeslicesMap>;
    unordered_map<FingerprintSlotsPhasesAndTimerefs, GraphNumEntriesAndVerticesPermAndTimeslicesMap>
      groups(16);

    std::vector<std::vector<std::size_t>> int_graph_list_index_to_entry(int_graph_list.size());
    std::size_t num_entries = 0;
    std::vector<std::pair<std::size_t, std::complex<double>>> disco_values;
    const auto comm = SBN::get_comm_from_mpi_comm(mpi_comm);
    if (SBN::get_rank(mpi_comm) == 0)
    {
      cout << "classify corr funs... ";
    }
    auto progress_bar =
      detail::ProgressBar(corr_graph_map.size(), cout, SBN::get_rank(mpi_comm) == 0);
    for (const auto& it : corr_graph_map)
    {
      const auto& corr_fun = it.first; // corr fun
      const int t_start = detail::get_t_start(corr_fun);
      // it.second.diagrams =  MapObject< std::vector<int>, std::complex<double> >
      for (const auto& diagram : it.second.diagrams)
      {
	// Skip if the diagram is the product with an unsmeared graph
	if (zeroUnsmearedGraphsP && has_unsmeared_graph(diagram.first))
	  continue;
	for (int corr_graph_index : diagram.first)
	{
	  // Produce the entries for a corr graph only once
	  if (int_graph_list_index_to_entry.at(corr_graph_index).size() > 0)
	    continue;

	  // Get the correlation graph
	  const auto& corr_graph = int_graph_list.at(corr_graph_index);

          // A standalone unsmeared bilinear is a disconnected scalar loop,
          // not a GenProp tensor. Preserve it as a factor in the parent diagram.
          {
            const auto& g = corr_graph;
            if (get_smearing(g.vertices) == std::vector<char>(1) &&
                !g.graph.empty() && (g.graph[0] == 'M' || g.graph[0] == 'A')) {
              if (!disco_callback) throw std::runtime_error("disconnected loop requires a DISCO callback");
              const auto& h = g.vertices.at(1).first;
              if (!h.phasings.empty()) throw std::runtime_error("disconnected loop cannot have quark phasings");
              const char flavor = g.adj_map.at(1).at(1).flavor;
              const auto& mass = flavor_to_mass.at(flavor);
              const auto coeffs = Hadron::convertTwoQuarkDispGamma(corr_graph_outer_coeffs[h]);
            const int count = std::min(Nt_corr, g.diag_time_slices.Nt_corr);
            const bool fixed_onept = false;
              if (count <= 0) throw std::runtime_error("invalid disconnected-loop time extent");
              for (int i = 0; i < count; ++i) {
                auto tt = g.diag_time_slices.t_slices.ref();

                if (g.diag_time_slices.neg_slice_npt > 0)
                  tt.at(g.diag_time_slices.neg_slice_npt - 1) = (t_start + i) % T;
                const int t = SBN::norm_coor(tt.at(g.vertices.at(1).second - 1) + t_origin, T);
                std::complex<double> value = 0;
                for (const auto& disp_spin : coeffs) {
                  Hadron::KeyTwoQuarkDiscoOperator_t key;
                  key.t_slice = t;
                  key.mass_label = mass;
                  key.mom = h.creation_op ? h.mom : -h.mom;
                  key.disp = disp_spin.first.deriv;
                  for (int d : key.disp)
                    if (d == 0) throw std::runtime_error("zero inside disconnected displacement path");
                  if (key.disp.empty()) key.disp.push_back(0);
                  const auto data = disco_callback(key);
                  if (data.op.size() != Datalib::TensorStorage::spin_count * Datalib::TensorStorage::spin_count)
                    throw std::runtime_error("DISCO callback must return all 16 gamma values");
                  for (const auto& z : data.op)
                    if (!std::isfinite(z.real()) || !std::isfinite(z.imag()))
                      throw std::runtime_error("DISCO callback returned a nonfinite value");
                  for (const auto& spin : disp_spin.second)
                    value += spin.second * data.op.at(spin.first.gamma);
                }
                const auto index = num_entries++;
                disco_values.push_back({index, value});
                int_graph_list_index_to_entry.at(corr_graph_index).push_back(index);
              }
              continue;
            }
          }

	  // Get the vertices for each one
	  std::vector<std::size_t> vertex_indices(corr_graph.vertices.size());
	  std::vector<int> tslice_indices(corr_graph.vertices.size());
	  VertexPhases phases;
	  for (const auto& it : corr_graph.vertices)
	  {
	    // Insert the HadronDispVertex_t
	    if (vertex_to_index.count(it.second.first) == 0)
	      vertex_to_index[it.second.first] = num_vertices++;
	    vertex_indices.at(it.first - 1) = vertex_to_index.at(it.second.first);
	    tslice_indices.at(it.first - 1) = it.second.second - 1;
	    for (const auto& phase : get_phases_from_vertex(it.second.first))
	      phases.insert(Datalib::TensorStorage::detail::toCoor(phase));
	  }
	  std::vector<char> is_time_moving_vertex(tslice_indices.size());
	  if (corr_graph.diag_time_slices.neg_slice_npt > 0)
	    for (int i = 0; i < tslice_indices.size(); ++i)
	      is_time_moving_vertex.at(i) =
		(corr_graph.diag_time_slices.neg_slice_npt - 1 == tslice_indices.at(i) ? 1
											     : 0);

	  // Get table associated to the group id
	  const auto& smearing = get_smearing(corr_graph.vertices);
	  const auto& graph_str = corr_graph.graph;
	  const auto& adj_map = corr_graph.adj_map;
	  const auto& graph_smearing_and_adj =
	    GraphSmearingMovingAndAdj{graph_str, smearing, is_time_moving_vertex, adj_map};
	  if (to_group.count(graph_smearing_and_adj) == 0)
	  {
	    const auto& [fingerprint, slots, graph, prop_vertices] =
	      get_fingerprint(graph_str, adj_map, smearing, is_time_moving_vertex);
	    if (fingerprints.count(fingerprint) == 0)
	      fingerprints[fingerprint] = {};
	    for (const auto& [it_slots, it_graph] : fingerprints.at(fingerprint))
	    {
	      const auto& [ok, ordering, slots_perm] =
		get_vertex_reordering_and_permutation(it_slots, slots, it_graph, graph);
	      if (!ok)
		continue;
	      to_group[graph_smearing_and_adj] = {
		{fingerprint, it_slots, it_graph}, prop_vertices, ordering, slots_perm};
	      break;
	    }
	    if (to_group.count(graph_smearing_and_adj) == 0)
	    {
	      // Get trivial ordering and permutations
	      const auto& [ok, ordering, slots_perm] =
		get_vertex_reordering_and_permutation(slots, slots, graph, graph);
	      if (!ok)
		throw std::runtime_error("wtf");
	      to_group[graph_smearing_and_adj] = {
		{fingerprint, slots, graph}, prop_vertices, ordering, slots_perm};
	      fingerprints.at(fingerprint).push_back({slots, graph});
	    }
	  }
	  const auto& [group_things, prop_vertices, ordering, slots_perm] =
	    to_group.at(graph_smearing_and_adj);
	  const auto& [group_fingerprint, group_slots, group_graph] = group_things;

	  // Permute tslice_indices with `ordering`
	  std::vector<int> tslice_indices_perm;
	  tslice_indices_perm.reserve(tslice_indices.size());
	  for (int i = 0; i < tslice_indices.size(); ++i)
	    tslice_indices_perm.push_back(tslice_indices.at(ordering.at(i)));

	  // Append the prop vertices information (flavor and conj)
	  vertex_indices.insert(vertex_indices.end(), prop_vertices.begin(), prop_vertices.end());

	  // Permute vertex_indices and slots_perm
	  const auto& vertex_indices_perm = SBN::do_perm(vertex_indices, ordering);
	  const auto& slots_perm_perm = SBN::do_perm(slots_perm, ordering);

	  // Get the time slices for each vertex
	  int this_Nt_corr = std::min(Nt_corr, corr_graph.diag_time_slices.Nt_corr);
	  std::vector<std::vector<int>> t, npt_t;
	  t.reserve(this_Nt_corr);
	  npt_t.reserve(this_Nt_corr);
	  const auto& npt_tslices = corr_graph.diag_time_slices.t_slices.ref();
	  const auto& timerefs = get_time_slices_for_sorting(npt_tslices, T, this_Nt_corr);
	  for (int i = 0; i < this_Nt_corr; ++i)
	  {
	    auto tt = npt_tslices;
	    if (corr_graph.diag_time_slices.neg_slice_npt > 0)
	      tt.at(corr_graph.diag_time_slices.neg_slice_npt - 1) = (t_start + i) % T;

	    // Shift all time t_origin
	    for (auto& it : tt)
	      it = (it + t_origin) % T;
	    npt_t.push_back(tt);

	    // Set the time slice for each vertex
	    std::vector<int> ti;
	    ti.reserve(tslice_indices_perm.size());
	    for (auto ts : tslice_indices_perm)
	      ti.push_back(tt.at(ts));
	    t.push_back(ti);
	  }

	  // Put all the combinations
	  const auto& group_id =
	    FingerprintSlotsPhasesAndTimerefs{group_fingerprint, group_slots, phases, timerefs};
	  if (groups.count(group_id) == 0)
	    groups[group_id] = {group_graph, 0ul, VerticesPermAndTimeslicesMap(16)};
	  auto& [_, num_entries_in_group, vert_perm_and_ts] = groups.at(group_id);
	  if (vert_perm_and_ts.count(vertex_indices_perm) == 0)
	    vert_perm_and_ts[vertex_indices_perm] = PermAndTimeslicesMap(16);
	  auto& perm_and_ts = vert_perm_and_ts.at(vertex_indices_perm);
	  if (perm_and_ts.count(slots_perm_perm) == 0)
	    perm_and_ts[slots_perm_perm] = TimeslicesMap(16);
	  auto& ts = perm_and_ts.at(slots_perm_perm);

	  for (const auto& ti : t)
	  {
	    if (ts.count(ti) == 0)
	    {
	      ts[ti] = num_entries++;
	      num_entries_in_group++;
	    }
	    int_graph_list_index_to_entry.at(corr_graph_index).push_back(ts.at(ti));
	  }
	}
      }
      progress_bar++;
    }

    // Get the table of unique vertices
    std::vector<Hadron::HadronDispVertex_t> unique_vertices(vertex_to_index.size());
    for (const auto& it : vertex_to_index)
      unique_vertices.at(it.second) = it.first;

    // Order the groups by phases and time-slices
    std::vector<FingerprintSlotsPhasesAndTimerefs> group_ids;
    group_ids.reserve(groups.size());
    for (const auto& it : groups)
      group_ids.push_back(it.first);
    std::sort(group_ids.begin(), group_ids.end(), [](const auto& a, const auto& b) {
      return std::make_tuple(std::ref(std::get<2>(a)), std::ref(std::get<3>(a))) <
	     std::make_tuple(std::ref(std::get<2>(b)), std::ref(std::get<3>(b)));
    });

    // Compute the correlation functions for each group
    std::vector<std::complex<double>> values(num_entries, SBN::complex_nan);
    for (const auto& item : disco_values) values.at(item.first) = item.second;
    for (const auto& group_id : group_ids)
    {
      const auto& [graph, num_entries_in_group, vert_perm_and_ts] = groups.at(group_id);

      std::vector<std::vector<std::size_t>> vertices;
      std::vector<std::vector<SBN::Coor>> perm;
      std::vector<std::vector<int>> t;
      std::vector<std::size_t> entries;
      vertices.reserve(num_entries_in_group);
      perm.reserve(num_entries_in_group);
      t.reserve(num_entries_in_group);
      entries.reserve(num_entries_in_group);
      for (const auto& it0 : vert_perm_and_ts)
      {
	for (const auto& it1 : it0.second)
	{
	  for (const auto& ts : it1.second)
	  {
	    vertices.push_back(it0.first);
	    perm.push_back(it1.first);
	    t.push_back(ts.first);
	    entries.push_back(ts.second);
	  }
	}
      }
      if (SBN::get_rank(mpi_comm) == 0)
      {
	cout << "Doing graph= " << graph_to_string(graph)
	     << " with phasings= " << phases_to_string(std::get<2>(group_id))
	     << " and time-slices= " << timeslices_to_string(std::get<3>(group_id)) << " having "
	     << t.size() << " corr funs..." << std::endl;
      }
      auto t0 = SBN::w_time();
      const auto& this_values =
	compute_corr_fun(ctx, graph, unique_vertices, vertices, perm, t, max_mem, comm, cout);
      t0 = SBN::w_time() - t0;
      if (SBN::get_rank(mpi_comm) == 0)
      {
	cout << "... took " << t0 << " secs " << std::endl;
      }
      if (SBN::get_rank(mpi_comm) == 0)
	for (std::size_t i = 0; i < entries.size(); i++)
	  values.at(entries.at(i)) = this_values.at(i);
    }

    // Compose the correlation functions
    // NOTE: they are the addition of the product of graphs
    MapObject<KeyHadronSUNClassNPartNPtCorr_t, std::vector<std::complex<double>>> r;
    if (SBN::get_rank(mpi_comm) == 0)
    {
      for (const auto& it : corr_graph_map)
      {
	const auto& corr_fun = it.first; // corr fun
	const auto& quark_count = it.second.quark_count.ref();
	std::vector<std::complex<double>> this_add_values;
	bool truly_first_time = true;
	// it.second.diagrams =  MapObject< std::vector<int>, std::complex<double> >
	for (const auto& diagram : it.second.diagrams)
	{
	  // Skip if the diagram is the product with an unsmeared graph
	  if (zeroUnsmearedGraphsP && has_unsmeared_graph(diagram.first))
	    continue;

	  bool first_time = true;
	  std::vector<std::complex<double>> this_prod_values;
	  auto weight = diagram.second;
	  for (int corr_graph_index : diagram.first)
	  {
	    // Get the correlation graph
	    const auto& entries = int_graph_list_index_to_entry.at(corr_graph_index);
	    if (first_time)
	    {
	      this_prod_values =
		std::vector<std::complex<double>>(entries.size(), std::complex<double>(1.0));
	    }
	    else if (this_prod_values.size() != entries.size())
	    {
	      if (entries.size() == 1)
	      {
		weight *= values.at(entries.at(0));
		continue;
	      }
	      else if (this_prod_values.size() == 1)
	      {
		weight *= this_prod_values.at(0);
		this_prod_values =
		  std::vector<std::complex<double>>(entries.size(), std::complex<double>(1.0));
	      }
	      else
	      {
		throw std::runtime_error("wtf");
	      }
	    }
	    for (std::size_t i = 0; i < entries.size(); ++i)
	      this_prod_values.at(i) *= values.at(entries.at(i));
	    first_time = false;
	  }
	  if (truly_first_time)
	  {
	    this_add_values.resize(this_prod_values.size());
	  }
	  else
	  {
	    if (this_add_values.size() != this_prod_values.size())
	      throw std::runtime_error("wtf");
	  }
	  for (std::size_t i = 0; i < this_prod_values.size(); ++i)
	    this_add_values.at(i) += this_prod_values.at(i) * weight;
	  truly_first_time = true;
	}
        // A standalone fixed point has one computed value, constant across output time.
        if (corr_fun.npoint.size() == 1 && this_add_values.size() == 1)
          this_add_values.resize(Nt_corr, this_add_values.front());
        else
          this_add_values.resize(Nt_corr);

	// Apply boundary conditions on the correlator
	const auto& tslices = detail::get_t_slices(corr_fun, t_origin, Nt_corr, T);
	for (int i = 0; i < Nt_corr; ++i)
	  if (detail::flipAntiBC(quark_count, tslices.at(i)) != 0)
	    this_add_values.at(i) *= std::complex<double>(-1);

	r.insert(corr_fun, std::move(this_add_values));
      }
    }

    return r;
  }

  template <typename stream = decltype(std::cout)>
  inline MapObject<KeyHadronSUNClassNPartNPtCorr_t, std::vector<std::complex<double>>>
  evaluate_graphs_with_superb(CorrGraphMap_t& corr_graph, bool zeroUnsmearedGraphsP,
                              bool average_1pt_diagrams,
			      Datalib::TensorStorage::StorageProp4& prop_db, Datalib::TensorStorage::StorageBaryon& baryon_db,
			      Datalib::TensorStorage::StorageMeson& meson_db, Datalib::TensorStorage::StorageGenprop4& genprop_db,
			      const FlavorToMassMap& flavor_to_mass, int num_vecs, int t_origin = 0,
			      int Nt_corr = 0, stream& cout = std::cout,
			      SBN::MpiComm mpi_comm = SBN::get_global_mpi_comm())
  {
    const auto meson_callback =
      [&](const std::vector<Hadron::KeyMesonElementalOperator_t>& meson_keys,
	  const std::vector<SBN::Coor>& perms, const std::vector<bool>& do_conj,
	  const SBN::Coor& ev_from, const SBN::Coor& ev_size, const std::string& dist_labels,
	  const SBN::Tensor& perm) {
	return Hadron::detail::get_meson_elementals_from_storage(
	  meson_db, meson_keys, perms, do_conj, ev_from, ev_size, dist_labels, perm);
      };
    const auto baryon_callback =
      [&](const std::vector<Hadron::KeyBaryonElementalOperator_t>& baryon_keys,
	  const std::vector<SBN::Coor>& perms, const std::vector<bool>& do_conj,
	  const SBN::Coor& ev_from, const SBN::Coor& ev_size, const std::string& dist_labels,
	  const SBN::Tensor& perm) {
	return Hadron::detail::get_baryon_elementals_from_storage(
	  baryon_db, baryon_keys, perms, do_conj, ev_from, ev_size, dist_labels, perm);
      };
    const auto prop_callback =
      [&](const std::vector<Hadron::KeyProp4ElementalOperator_t>& prop_keys,
	  const std::vector<SBN::Coor>& perms, const std::vector<bool>& do_conj,
	  const SBN::Coor& ev_from, const SBN::Coor& ev_size, const std::string& dist_labels,
	  const SBN::Tensor& perm) {
	return Hadron::detail::get_prop_elementals_from_storage(
	  prop_db, prop_keys, perms, do_conj, ev_from, ev_size, dist_labels, perm);
      };
    const auto genprop_callback =
      [&](const std::vector<Hadron::KeyGenProp4ElementalOperator_t>& genprop_keys,
	  const std::vector<SBN::Coor>& perms, const std::vector<bool>& do_conj,
	  const SBN::Coor& ev_from, const SBN::Coor& ev_size, const std::string& dist_labels,
	  const SBN::Tensor& perm) {
	return Hadron::detail::get_genprop_elementals_from_storage(
	  genprop_db, genprop_keys, perms, do_conj, ev_from, ev_size, dist_labels, perm);
      };
    return evaluate_graphs_with_superb(corr_graph, zeroUnsmearedGraphsP, average_1pt_diagrams, prop_callback,
				       baryon_callback, meson_callback, genprop_callback,
				       flavor_to_mass, num_vecs, t_origin, Nt_corr, cout, mpi_comm);
  }
}

#endif // __redstar_datalib_superb_contraction_h
