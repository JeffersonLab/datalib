#ifndef DATALIB_DBTOOLS_SUPPORT_H
#define DATALIB_DBTOOLS_SUPPORT_H
#include <cmath>
#include <complex>
#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/legacy/hadron_2pt_corr.h>
#include <datalib/legacy/hadron_3pt_corr.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>
#include <stdexcept>
#include <vector>
namespace Datalib::Tools {
  using Row = std::vector<std::complex<double>>;
  template <class Scalar> void scale(Row &row, Scalar factor) {
    for (auto &value : row)
      value *= factor;
  }
  inline void divide(Row &row, double divisor) {
    for (auto &value : row)
      value /= divisor;
  }
  template <class Scalar>
  void addScaled(Row &dest, const Row &src, Scalar factor) {
    if (dest.size() != src.size())
      throw std::runtime_error("Inconsistent observable counts");
    for (std::size_t i = 0; i < dest.size(); ++i)
      dest[i] += factor * src[i];
  }
  // Matches the legacy ascending product for the supported normalization range.
  inline double factorial(int n) {
    if (n < 0)
      throw std::runtime_error("Negative factorial");
    double result = 1.0;
    for (int i = 2; i <= n; ++i)
      result *= i;
    if (!std::isfinite(result))
      throw std::runtime_error("Factorial overflow");
    return result;
  }
  inline double wickDenominator(int n) {
    if (n < 0 || n >= 31)
      throw std::runtime_error("Unsupported Wick normalization exponent");
    return std::ldexp(factorial(n), n);
  }
  template <class F> void dispatch(const std::string &type, F &&fn) {
    if (type == "hadron1Pt")
      return fn.template operator()<FF::KeyHadron1PtCorr_t>();
    if (type == "hadron2Pt")
      return fn.template operator()<FF::KeyHadron2PtCorr_t>();
    if (type == "hadron3Pt")
      return fn.template operator()<FF::KeyHadron3PtCorr_t>();
    if (type == "hadronNPartNPtCorr")
      return fn.template operator()<Hadron::KeyHadronNPartNPtCorr_t>();
    if (type == "hadronSUNNPartNPtCorr")
      return fn.template operator()<Hadron::KeyHadronSUNNPartNPtCorr_t>();
    throw std::runtime_error("Unsupported legacy correlator database type: " +
                             type);
  }
} // namespace Datalib::Tools
#endif
