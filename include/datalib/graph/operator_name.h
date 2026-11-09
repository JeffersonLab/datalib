// -*- C++ -*-
/*! \file
 * \brief Generate array of flavor strings for a particular hadron
 */

#ifndef __operator_name_util_h__
#define __operator_name_util_h__

#include <string>

namespace Hadron
{
  //----------------------------------------------------------------------------------
  //! Get irrep name from operator name
  std::string getIrrep(const std::string& opName);

  //----------------------------------------------------------------------------------
  //! Extract continuum op name from subduced op name
  std::string extractContOpName(const std::string& opName);

  //----------------------------------------------------------------------------------
  //! Get parity from an irrep name
  int getIrrepParity(const std::string& irrep);

  //----------------------------------------------------------------------------------
  //! Get G-parity from an irrep name
  int getIrrepGParity(const std::string& irrep);

  //----------------------------------------------------------------------------------
  //! Get the N in SU(N)
  int getSUNF(const std::string& opName);

  //! Get flavor irrep
  std::string getFlavorIrrep(const std::string& opName);

  //! Get flavor of particle id
  std::string getFlavor(const std::string& opName);

  //! Get flavor of particle id
  int getTwoIsospin(const std::string& opName);

  //! Get threeY of id
  int getThreeY(const std::string& opName);

  //! Get strangeness of particle id
  int getStrangeness(const std::string& opName);

  //! Get number of quarks
  int getNumQuarks(const std::string& opName);

  //  //! Get flavor of particle id
  //  int getTwoIsospin(const std::string& opName);

  //! Get strangeness of particle id
  int getStrangeness(const std::string& opName);

  //! Get rescaling from old normalization
  double getRescalingFactor(const std::string& opName);

  //! Get rescaling for projected ops
  double getProjOpRescalingFactor(const std::string& opName);

  // Get irrep with parity
  std::string getIrrepNoParity(const std::string& opName);

  // Get irrep with parity
  std::string getIrrepWithParity(const std::string& opName);

} // namespace Hadron

#endif
