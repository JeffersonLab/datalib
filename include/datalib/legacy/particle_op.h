// -*- C++ -*-
/*! \file
 * \brief Hadron-like particle operators
 */

#ifndef __particle_op_h__
#define __particle_op_h__
#include <datalib/io/serial_xml.h>
#include <datalib/codec/binary.h>


#include <datalib/model/arrays.h>
#include <datalib/codec/superb_options.h>
#include <datalib/graph/hadron_disp_vertex.h>

#include <iostream>

namespace Hadron
{
  using Datalib::Binary::read;
  using Datalib::Binary::write;
  using Datalib::Serial::read;
  using Datalib::Serial::write;
  using namespace SB;

  //----------------------------------------------------------------------------
  //! Key for a hadron-like particle operator
  struct KeyParticleOp_t
  {
    KeyParticleOp_t() {}
    KeyParticleOp_t(const std::string& n, const std::string& s, const XMLArray::Array<int>& m) : name(n), smear(s), mom_type(m) {}
    KeyParticleOp_t(const HadronDispVertex_t& vert);

    std::string          name;           /*!< Some string label for the operator */
    std::string          smear;          /*!< Some string label for the smearing of this operator */
    XMLArray::Array<int>           mom_type;       /*!< D-1 Canonical momentum type */

    using schema = std::tuple<				  //
      struct_member<&KeyParticleOp_t::name, "name"_sl>,	  //
      struct_member<&KeyParticleOp_t::smear, "smear"_sl>, //
      struct_member<&KeyParticleOp_t::mom_type, "mom_type"_sl,
		    std::vector<int>> //
      >;
      bool operator==(const KeyParticleOp_t &) const = default;
  };


  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyParticleOp_t& d);

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const Datalib::Model::Array1dO<KeyParticleOp_t>& d);

  //----------------------------------------------------------------------------
  //! KeyParticleOp reader
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyParticleOp_t& param);

  //! KeyParticleOp writer
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyParticleOp_t& param);

  //----------------------------------------------------------------------------
  //! KeyParticleOp reader
  void read(Datalib::Binary::BinaryReader& bin, KeyParticleOp_t& param);

  //! KeyParticleOp writer
  void write(Datalib::Binary::BinaryWriter& bin, const KeyParticleOp_t& param);


} // namespace Hadron

#endif
