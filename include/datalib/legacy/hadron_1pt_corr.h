// -*- C++ -*-
/*! \file
 * \brief Hadron 1pt correlators
 */

#ifndef __hadron_1pt_corr_h__
#define __hadron_1pt_corr_h__
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
  //! Key for Hadron 1pt corr
  struct KeyHadron1PtCorr_t
  {
    int          num_vecs;    /*!< Number of vectors used in this corr */

    int          creation_op; /*!< Is this a creation ops?  */

    std::string  name;        /*!< Some string label for the operator */
    std::string  smear;       /*!< Some string label for the smearing of this operator */
    XMLArray::Array<int>   lorentz;     /*!< Source Lorentz indices */
    int          spin;        /*!< Source Dirac spin indices */

    XMLArray::Array<int>   mom;         /*!< D-1 momentum of the sink operator */
    std::string  mass;        /*!< Some string label for the mass(es) in the corr */
    std::string  ensemble;    /*!< Label for the ensemble */
  };

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyHadron1PtCorr_t& d);

  //----------------------------------------------------------------------------
  //! KeyHadron1PtCorr reader
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadron1PtCorr_t& param);

  //! KeyHadron1PtCorr writer
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadron1PtCorr_t& param);

  //----------------------------------------------------------------------------
  //! KeyHadron1PtCorr reader
  void read(Datalib::Binary::BinaryReader& bin, KeyHadron1PtCorr_t& param);

  //! KeyHadron1PtCorr writer
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadron1PtCorr_t& param);


  //----------------------------------------------------------------------------
  //! Use a DB


} // namespace ColorVec

#endif
