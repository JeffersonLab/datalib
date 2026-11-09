// -*- C++ -*-
/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep and specific row including creation op info
 */

#ifndef __hadron_npart_irrep_h__
#define __hadron_npart_irrep_h__
#include <datalib/io/serial_xml.h>
#include <datalib/codec/binary.h>


#include <datalib/legacy/hadron_npart_irrep_op.h>
#include <datalib/model/arrays.h>

#include <string>
#include <iostream>

namespace Hadron
{
  using Datalib::Binary::read;
  using Datalib::Binary::write;
  using Datalib::Serial::read;
  using Datalib::Serial::write;

  //----------------------------------------------------------------------------
  //! Time slice collection of operators projected onto a definite irrep and specific row
  struct KeyHadronNPartIrrep_t
  {
    KeyHadronNPartIrrepOp_t          op;             /*!< Operator */

    int                              row;            /*!< Target irrep row */
    int                              twoI_z;         /*!< Target Isospin z component */
    XMLArray::Array<int>                       mom;            /*!< D-1 momentum of this N-particle op */
    XMLArray::Array<int>                       phasing;        /*!< phasing of this N-particle op */
    bool                             creation_op;    /*!< Are these creation ops on this time-slice? */
    bool                             smearedP;       /*!< Do these operators use distillation/distillution? */
  };


  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyHadronNPartIrrep_t& d);

  //----------------------------------------------------------------------------
  //! KeyHadronNPartIrrep reader
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadronNPartIrrep_t& param);

  //! KeyHadronNPartIrrep writer
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadronNPartIrrep_t& param);

  //----------------------------------------------------------------------------
  //! KeyHadronNPartIrrep reader
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartIrrep_t& param);

  //! KeyHadronNPartIrrep writer
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartIrrep_t& param);


} // namespace Hadron

#endif
