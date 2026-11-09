// -*- C++ -*-
/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep
 */

#ifndef __hadron_npart_irrep_op_h__
#define __hadron_npart_irrep_op_h__
#include <datalib/io/serial_xml.h>
#include <datalib/codec/binary.h>


#include <datalib/legacy/particle_op.h>
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
  //! Time slice collection of operators projected onto a definite irrep
  struct KeyHadronNPartIrrepOp_t
  {
    struct CGPair_t
    {
      struct Slot_t
      {
	std::string                  slot;           /*!< At first level, the HadronOp index, at higher levels, the temporary */
	std::string                  irrep;          /*!< Target Octahedral/little-group irrep */
	int                          embed;          /*!< Target irrep embedding */
	int                          twoI;           /*!< Target 2*Isospin irrep */
	XMLArray::Array<int>                   mom_type;       /*!< Target D-1 Canonical momentum type */
      };

      std::string                    left;           /*!< Left side of CG */
      std::string                    right;          /*!< Right side of CG */
      Slot_t                         target;         /*!< Target of CG */
    };

    Datalib::Model::Array1dO<CGPair_t>               CGs;            /*!< Each pair of CG contractions */
    Datalib::Model::Array1dO<KeyParticleOp_t>        ops;            /*!< Each operator */
  };


  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyHadronNPartIrrepOp_t& d);

  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadronNPartIrrepOp_t::CGPair_t::Slot_t& param);

  // Write a key
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadronNPartIrrepOp_t::CGPair_t::Slot_t& param);

  // Read a key
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadronNPartIrrepOp_t::CGPair_t& param);

  // Write a key
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadronNPartIrrepOp_t::CGPair_t& param);

  //! KeyHadronNPartIrrep reader
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadronNPartIrrepOp_t& param);

  //! KeyHadronNPartIrrep writer
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadronNPartIrrepOp_t& param);

  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartIrrepOp_t::CGPair_t::Slot_t& param);

  // Write a key
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartIrrepOp_t::CGPair_t::Slot_t& param);

  // Read a key
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartIrrepOp_t::CGPair_t& param);

  // Write a key
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartIrrepOp_t::CGPair_t& param);

  //! KeyHadronNPartIrrep reader
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartIrrepOp_t& param);

  //! KeyHadronNPartIrrep writer
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartIrrepOp_t& param);


} // namespace Hadron

#endif
