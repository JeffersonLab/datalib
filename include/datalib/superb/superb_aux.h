// -*- C++ -*-
/*! \file
 * \brief Auxiliary functions for superbblas sparse tensor database
 */

#ifndef __redstar_datalib_aux_superbblas_h__
#define __redstar_datalib_aux_superbblas_h__

#ifndef USE_SUPERBBLAS_WITH_GPU_SUPPORT
// Don't activate the GPU API of superbblas
#  ifndef SUPERBBLAS_NOT_USE_GPU
#    define SUPERBBLAS_NOT_USE_GPU
#  endif
#endif

#ifdef USE_SUPERBBLAS
#  ifdef USE_MPI
#    define SUPERBBLAS_USE_MPI
#  endif
#  include <superbblas.h>
#endif

#include "datalib/superb/types.h"
#include "datalib/codec/superb_options.h"
#include <datalib/model/xml_array.h>
#include <algorithm>
#include <complex>

namespace Datalib::TensorStorage
{
  namespace detail
  {
    /// Return a std::array<3,int> from Array
    /// \param v: vector to convert

    template <typename T>
    std::array<int, 3> toCoor(const XMLArray::Array<T>& v)
    {
      if (v.size() != 3)
	throw std::runtime_error("wtf");
      std::array<int, 3> r;
      for (int i = 0; i < v.size(); ++i)
	r[i] = v[i];
      return r;
    }

    /// Return a Array<int> from std::array<int, 3>
    /// \param v: vector to convert

    template <typename T, std::size_t N>
    XMLArray::Array<T> toXMLArray(const std::array<T, N>& v)
    {
      XMLArray::Array<T> r(v.size());
      for (int i = 0; i < v.size(); ++i)
	r[i] = v[i];
      return r;
    }

    /// Return a std::array<int, N> from key-value dimensional map
    /// \param order: labels order to return
    /// \param m: map from labels to dimensions
    /// \param missing: dimension value for missing labels in `m`

    template <std::size_t N>
    std::array<int, N> kvcoors(const std::string& order, const std::map<char, int>& m,
			       int missing = 0)
    {
      std::array<int, N> r;
      unsigned int found = 0;
      for (std::size_t i = 0; i < N; ++i)
      {
	auto it = m.find(order[i]);
	if (it != m.end())
	{
	  r[i] = it->second;
	  ++found;
	}
	else
	{
	  r[i] = missing;
	}
      }

      return r;
    }

    /// Return the negative of all elements in a vector
    /// \param v: vector to negate

    inline std::array<int, 3> negate(const std::array<int, 3>& v)
    {
      return {-v[0], -v[1], -v[2]};
    }

    /// Return the negative of all elements in a vector
    /// \param v: vector to negate

    inline XMLArray::Array<int> negate(const XMLArray::Array<int>& v)
    {
      return -v;
    }

    /// Return the negative of all elements in a vector
    /// \param v: vector to negate

    inline std::vector<int> negate(const std::vector<int>& v)
    {
      std::vector<int> r;
      r.reserve(v.size());
      for (const auto& it : v)
	r.push_back(-it);
      return r;
    }

    /// Return the reverse of the vector
    /// \param v: input vector

    inline std::vector<int> reverse(const std::vector<int>& v)
    {
      auto r = v;
      std::reverse(r.begin(), r.end());
      return r;
    }

    /// Return a key-value dimensional map from a coordinate
    /// \param order: labels order of the given coordinates
    /// \param c: giving coordinates

    template <typename Coor>
    std::map<char, int> to_kvdim(const std::string& order, const Coor& c)
    {
      if (order.size() != c.size())
	throw std::runtime_error("to_kvdim: invalid arguments, both should have the same size");

      // Create dictionary with the dimensions
      std::map<char, int> kvdim;
      for (int i = 0; i < order.size(); ++i)
	kvdim[order[i]] = c[i];
      return kvdim;
    }

    /// Return the coordinate in the interval [0, dim) for a periodic dimension.
    /// \param coor: coordinate to normalize
    /// \param dim: length of the periodic dimension

    inline int normalize_coor(int coor, int dim)
    {
      return (dim == 0 ? 0 : (coor + dim * (coor < 0 ? -coor / dim + 1 : 0)) % dim);
    }

    /// Extract default phasing from a filename
    /// \param filename: filename possibly containing the suffix @phase=x_y_z
    /// \return [filename, phasing]: filename is the given filename without the suffix
    ///              and phasing is the phasing indicated in the suffix, or empty if
    ///              not given.

    inline std::pair<std::string, XMLArray::Array<int>>
    get_filename_and_phasing(const std::string& filename)
    {
      // Look for the suffix `@phase=`
      const std::string suffix = "@phase=";
      auto s = std::search(filename.begin(), filename.end(), suffix.begin(), suffix.end());
      if (s == filename.end())
	return {filename, XMLArray::Array<int>()};

      // Get the content of the string before the suffix
      const auto& filename_out = std::string(filename.begin(), s);

      // Skip "@phase=" and get the integer values between the "_"
      s += suffix.size();
      std::vector<int> phase;
      phase.reserve(3);
      while (s != filename.end())
      {
	const auto n = std::find(s, filename.end(), '_');
	try
	{
	  std::size_t pos = 0;
	  const auto& number = std::string(s, n);
	  int v = std::stoi(number, &pos);
	  if (pos != number.size())
	    throw std::runtime_error("the content of the array isn't a number");
	  phase.push_back(v);
	} catch (...)
	{
	  return {filename, XMLArray::Array<int>()};
	}
	if (n == filename.end())
	  break;
	s = n + 1;
      }
      if (phase.size() != 3)
	return {filename, XMLArray::Array<int>()};
      return {filename_out, XMLArray::Array<int>(phase)};
    }

    /// Return whether the given filename is dummry

    inline bool is_filename_dummy(const std::string& filename)
    {
      return filename == std::string("~dummy~");
    }

#ifdef USE_SUPERBBLAS
    /// Wrapper around superbblas time tracking

    struct Tracker
        : public superbblas::detail::tracker<superbblas::detail::Cpu> {
      Tracker(const std::string &funcName)
          : superbblas::detail::tracker<superbblas::detail::Cpu>(
                funcName, superbblas::detail::Cpu{}) {}
    };
#else
    /// Wrapper around superbblas time tracking
    /// NOTE: dummy implementation

    struct Tracker {
      Tracker(const std::string &) {}
      void stop()
      {
      }
    };
#endif

    /// NaN<T>::get() returns a NaN value
    /// NaN<T>::is(v) returns whether `v` is NaN

    template <typename T>
    struct NaN;

    /// Specialization for float
    template <>
    struct NaN<float> {
      static float get()
      {
	return std::nanf("");
      }
      static bool is(float v)
      {
	return std::isnan(v);
      }
    };

    /// Specialization for double
    template <>
    struct NaN<double> {
      static double get()
      {
	return std::nan("");
      }
      static bool is(double v)
      {
	return std::isnan(v);
      }
    };

    /// Specialization for std::complex
    template <typename T>
    struct NaN<std::complex<T>> {
      static std::complex<T> get()
      {
	return std::complex<T>{NaN<T>::get(), NaN<T>::get()};
      }
      static bool is(const std::complex<T>& v)
      {
	return std::isnan(std::real(v)) | std::isnan(std::imag(v));
      }
    };
  }
}

#endif
