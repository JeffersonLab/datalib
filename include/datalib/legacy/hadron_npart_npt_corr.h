// -*- C++ -*-
/*! \file
 * \brief Hadron N-point correlators
 */

#ifndef __hadron_npart_npt_corr_h__
#define __hadron_npart_npt_corr_h__
#include <datalib/io/serial_xml.h>
#include <datalib/codec/binary.h>
#include <datalib/model/arrays.h>


#include <datalib/legacy/hadron_npart_irrep.h>
#include <iostream>

namespace Hadron
{
  using Datalib::Binary::read;
  using Datalib::Binary::write;
  using Datalib::Serial::read;
  using Datalib::Serial::write;

  //----------------------------------------------------------------------------
  //! Key for Hadron NPt correlator
  struct KeyHadronNPartNPtCorr_t
  {
    struct NPoint_t
    {
      int                              t_slice;     /*!< This time-slice. If negative, then varies */
      KeyHadronNPartIrrep_t            irrep;       /*!< Holds each of the operators projected into irreps */
    };

    Datalib::Model::Array1dO<NPoint_t>                 npoint;      /*!< Each time-slice of an n-point function */
    std::string                        ensemble;    /*!< Label for the ensemble */
  };

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyHadronNPartNPtCorr_t::NPoint_t& d);

  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyHadronNPartNPtCorr_t& d);

  //----------------------------------------------------------------------------
  //! KeyHadronNPartNPtCorr reader
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadronNPartNPtCorr_t& param);

  //! KeyHadronNPartNPtCorr writer
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadronNPartNPtCorr_t& param);

  //----------------------------------------------------------------------------
  //! KeyHadronNPartNPtCorr reader
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartNPtCorr_t::NPoint_t& param);

  //! KeyHadronNPartNPtCorr reader
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartNPtCorr_t& param);

  //! KeyHadronNPartNPtCorr writer
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartNPtCorr_t::NPoint_t& param);

  //! KeyHadronNPartNPtCorr writer
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartNPtCorr_t& param);

} // namespace Hadron

#endif
