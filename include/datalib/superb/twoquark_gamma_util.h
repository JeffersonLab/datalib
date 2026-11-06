// -*- C++ -*-
/*! \file
 * \brief Convert twoquark ops to gamma basis form
 */

#ifndef __redstar_datalib_twoquark_gamma_util_h__
#define __redstar_datalib_twoquark_gamma_util_h__



#include "datalib/superb/types.h"
#include "datalib/graph/single_hadron_coeffs.h"
#include <vector>

namespace Hadron
{

  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //! Twoquark field
  struct KeyTwoQuarkGamma_t
  {
    KeyTwoQuarkGamma_t() {}
    KeyTwoQuarkGamma_t(int g) : gamma(g) {}

    int    gamma;   /*!< The gamma in Gamma(gamma) - a number from [0 ... Datalib::TensorStorage::spin_count*Datalib::TensorStorage::spin_count) */

    bool operator==(const KeyTwoQuarkGamma_t&) const = default;
  };

  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeyTwoQuarkGamma_t& op);

  //----------------------------------------------------------------------------
  //! Reader 
  void read(XMLReader& xml, const std::string& path, KeyTwoQuarkGamma_t& param);

  //! Writer
  void write(XMLWriter& xml, const std::string& path, const KeyTwoQuarkGamma_t& op);

  //----------------------------------------------------------------------------
  //! Reader
  void read(BinaryReader& bin, KeyTwoQuarkGamma_t& op);

  //! Writer
  void write(BinaryWriter& bin, const KeyTwoQuarkGamma_t& op);

  //----------------------------------------------------------------------------
  //! Map holding expressions. Key is the two-quark, val is the weight.
  typedef MapObject< KeyTwoQuarkGamma_t, std::complex<double> >  MapTwoQuarkGamma_t;


  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //! Twoquark field
  struct KeyTwoQuarkDisp_t
  {
    KeyTwoQuarkDisp_t() {}
    explicit KeyTwoQuarkDisp_t(const std::vector<int>& d) : deriv(d) {}

    std::vector<int>   deriv;   /*!< Left-right derivative path. This is 1-based. */

    bool operator==(const KeyTwoQuarkDisp_t&) const = default;
  };

  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeyTwoQuarkDisp_t& op);

  //----------------------------------------------------------------------------
  //! Reader 
  void read(XMLReader& xml, const std::string& path, KeyTwoQuarkDisp_t& param);

  //! Writer
  void write(XMLWriter& xml, const std::string& path, const KeyTwoQuarkDisp_t& op);

  //----------------------------------------------------------------------------
  //! Reader
  void read(BinaryReader& bin, KeyTwoQuarkDisp_t& op);

  //! Writer
  void write(BinaryWriter& bin, const KeyTwoQuarkDisp_t& op);

  //----------------------------------------------------------------------------
  //! Map holding expressions. Key is the two-quark, val is the weight.
  typedef MapObject<KeyTwoQuarkDisp_t, MapTwoQuarkGamma_t>  MapTwoQuarkDispGamma_t;


  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //! Convert from single hadron coeff map in the explicit DP basis to one in the GammaDR basis
  MapTwoQuarkDispGamma_t convertTwoQuarkDispGamma(const Hadron::MapSingleHadronQuarkFlavorColorSpin_t& fin);

}  // end namespace Hadron

#endif
