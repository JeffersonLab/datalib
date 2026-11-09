// -*- C++ -*-
// $Id: hadron_2pt_corr.h,v 2.6 2009/03/21 21:33:45 edwards Exp $
/*! \file
 * \brief Hadron 2pt correlators
 */

#ifndef __hadron_2pt_corr_h__
#define __hadron_2pt_corr_h__
#include <datalib/io/serial_xml.h>
#include <datalib/codec/binary.h>


#include <datalib/model/map_obj.h>



#include <iostream>

namespace FF
{
  using Datalib::Binary::read;
  using Datalib::Binary::write;
  using Datalib::Serial::read;
  using Datalib::Serial::write;

  //----------------------------------------------------------------------------
  //! Key for Hadron 2pt corr
  struct KeyHadron2PtCorr_t
  {
    int          num_vecs;    /*!< Number of vectors used in this corr */

    std::string  src_name;    /*!< Some string label for the operator */
    std::string  src_smear;   /*!< Some string label for the smearing of this operator */
    XMLArray::Array<int>   src_lorentz; /*!< Source Lorentz indices */
    int          src_spin;    /*!< Source Dirac spin indices */

    std::string  snk_name;    /*!< Some string label for the operator */
    std::string  snk_smear;   /*!< Some string label for the smearing of this operator */
    XMLArray::Array<int>   snk_lorentz; /*!< Sink Lorentz indices */
    int          snk_spin;    /*!< Sink Dirac spin indices */

    XMLArray::Array<int>   mom;         /*!< D-1 momentum of the sink operator */
    std::string  mass;        /*!< Some string label for the mass(es) in the corr */
    std::string  ensemble;    /*!< Label for the ensemble */
  };

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyHadron2PtCorr_t& d);

  //----------------------------------------------------------------------------
  //! KeyHadron2PtCorr reader
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadron2PtCorr_t& param);

  //! KeyHadron2PtCorr writer
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadron2PtCorr_t& param);

  //----------------------------------------------------------------------------
  //! KeyHadron2PtCorr reader
  void read(Datalib::Binary::BinaryReader& bin, KeyHadron2PtCorr_t& param);

  //! KeyHadron2PtCorr writer
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadron2PtCorr_t& param);

  //----------------------------------------------------------------------------
  //! Use an in-memory map


  //! Use a DB


  //! Use a DB


} // namespace ColorVec

#endif
