/*! \file
 * \brief Convert twoquark ops to gamma basis form
 */


#include "datalib/superb/twoquark_gamma_util.h"

namespace Hadron
{
  // Utility functions
  namespace
  {
    struct GammaEntry
    {
      int column;
      std::complex<double> phase;
    };

    // Nonzero entries by row, in the historical Dirac-Pauli bit-mask order.
    constexpr std::array<std::array<GammaEntry, 4>, 16> gamma_dp {{
      {{{0, {1, 0}}, {1, {1, 0}}, {2, {1, 0}}, {3, {1, 0}}}},
      {{{3, {0, -1}}, {2, {0, -1}}, {1, {0, 1}}, {0, {0, 1}}}},
      {{{3, {-1, 0}}, {2, {1, 0}}, {1, {1, 0}}, {0, {-1, 0}}}},
      {{{0, {0, 1}}, {1, {0, -1}}, {2, {0, 1}}, {3, {0, -1}}}},
      {{{2, {0, -1}}, {3, {0, 1}}, {0, {0, 1}}, {1, {0, -1}}}},
      {{{1, {-1, 0}}, {0, {1, 0}}, {3, {-1, 0}}, {2, {1, 0}}}},
      {{{1, {0, 1}}, {0, {0, 1}}, {3, {0, 1}}, {2, {0, 1}}}},
      {{{2, {1, 0}}, {3, {1, 0}}, {0, {-1, 0}}, {1, {-1, 0}}}},
      {{{0, {1, 0}}, {1, {1, 0}}, {2, {-1, 0}}, {3, {-1, 0}}}},
      {{{3, {0, 1}}, {2, {0, 1}}, {1, {0, 1}}, {0, {0, 1}}}},
      {{{3, {1, 0}}, {2, {-1, 0}}, {1, {1, 0}}, {0, {-1, 0}}}},
      {{{0, {0, 1}}, {1, {0, -1}}, {2, {0, -1}}, {3, {0, 1}}}},
      {{{2, {0, 1}}, {3, {0, -1}}, {0, {0, 1}}, {1, {0, -1}}}},
      {{{1, {-1, 0}}, {0, {1, 0}}, {3, {1, 0}}, {2, {-1, 0}}}},
      {{{1, {0, 1}}, {0, {0, 1}}, {3, {0, -1}}, {2, {0, -1}}}},
      {{{2, {-1, 0}}, {3, {-1, 0}}, {0, {-1, 0}}, {1, {-1, 0}}}}
    }};

    MapTwoQuarkGamma_t convertTwoQuarkGamma(const MapSingleHadronQuarkSpin_t& coefficients)
    {
      std::array<std::array<std::complex<double>, 4>, 4> spin {};
      for (const auto& [key, value] : coefficients)
        spin.at(key.diracs[1]).at(key.diracs[2]) += value;

      MapTwoQuarkGamma_t result;
      for (int gamma = 0; gamma < gamma_dp.size(); ++gamma)
      {
        std::complex<double> weight {};
        for (int row = 0; row < 4; ++row)
        {
          const auto& entry = gamma_dp[gamma][row];
          weight += std::conj(entry.phase) * spin[row][entry.column];
        }
        weight *= 0.25;
        if (std::norm(weight) > 1.0e-11)
          result.insert(KeyTwoQuarkGamma_t(gamma), weight);
      }
      return result;
    }
  }

  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeyTwoQuarkGamma_t& op)
  {
    os << "gamma= " << op.gamma;
   
    return os; 
  }
    

  //----------------------------------------------------------------------------
  //! Reader 
  void read(XMLReader& xml, const std::string& path, KeyTwoQuarkGamma_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "gamma", param.gamma);
  }

  //! Writer
  void write(XMLWriter& xml, const std::string& path, const KeyTwoQuarkGamma_t& param)
  {
    push(xml, path);

    write(xml, "gamma", param.gamma);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  //! Reader
  void read(BinaryReader& bin, KeyTwoQuarkGamma_t& op)
  {
    read(bin, op.gamma);
  }

  //! Writer
  void write(BinaryWriter& bin, const KeyTwoQuarkGamma_t& op)
  {
    write(bin, op.gamma);
  }


  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeyTwoQuarkDisp_t& op)
  {
    os << "deriv=";
    for (int derivative : op.deriv)
      os << ' ' << derivative;
   
    return os; 
  }
    

  //----------------------------------------------------------------------------
  //! Reader 
  void read(XMLReader& xml, const std::string& path, KeyTwoQuarkDisp_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "deriv", param.deriv);
  }

  //! Writer
  void write(XMLWriter& xml, const std::string& path, const KeyTwoQuarkDisp_t& param)
  {
    push(xml, path);

    write(xml, "deriv", param.deriv);

    pop(xml);
  }
 

  //----------------------------------------------------------------------------
  //! Reader
  void read(BinaryReader& bin, KeyTwoQuarkDisp_t& op)
  {
    read(bin, op.deriv);
  }
  

  //! Writer
  void write(BinaryWriter& bin, const KeyTwoQuarkDisp_t& op)
  {
    write(bin, op.deriv);
  }
  

  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //! Convert from single hadron coeff map in the explicit DP basis to one in the GammaDR basis
  MapTwoQuarkDispGamma_t convertTwoQuarkDispGamma(const MapSingleHadronQuarkDeriv_t& fin)
  {
    // Output with derivatives turned into displacements, and phases for left derivatives
    // Note: there should not be any input phases
    MapTwoQuarkDispGamma_t disp_spins;
      
    // Loop over each source term and register the displacement
    for(auto k2 = fin.begin(); k2 != fin.end(); ++k2)
    {
      // Sanity check
      if (k2->first.derivs[1].deriv.size() != 0)
      {
	std::cerr << __func__ << ": unexpectedly found a derivative in the first slot for a map coming from twoquark coeffs\n";
	exit(1);
      }

      // Key for this entry - just the right hand quark derivs
      KeyTwoQuarkDisp_t key(k2->first.derivs[2].deriv);

      // Convert explicit DP gamma basis
      MapTwoQuarkGamma_t val(convertTwoQuarkGamma(k2->second));

      // No need to accumulate
      disp_spins.insert(key, val);
    }

    return disp_spins;
  } // void convertTwoQuarkExpr


  //----------------------------------------------------------------------------------
  //! Convert from single hadron coeff map in the explicit DP basis to one in the GammaDR basis
  MapTwoQuarkDispGamma_t convertTwoQuarkDispGamma(const Hadron::MapSingleHadronQuarkFlavorColorSpin_t& fin)
  {
    // Sanity check - do not expect more than one (generic) flavor key
    if (fin.size() != 1)
    {
      std::cerr << __func__ << ": internal error - expected 1 flavor key, but found k1.size= " << fin.size() << std::endl;
      exit(1);
    }

    // Loop over each source term and do the operation
    auto k1 = fin.begin();

    // Now go to the next level - derivs
    // Build a map with all the derivatives turned into displacements and phases
    return convertTwoQuarkDispGamma(k1->second);
  } // void convertTwoQuarkDispGamma


}  // end namespace Hadron
