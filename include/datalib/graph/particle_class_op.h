// -*- C++ -*-
/*! \file
 * \brief Hadron-like particle operators
 */

#ifndef DATALIB_particle_class_op_h
#define DATALIB_particle_class_op_h

#include "datalib/model/arrays.h"

#include "datalib/graph/hadron_disp_vertex.h"
#include "datalib/graph/hadron_sun_class_npart_version.h"
#include "datalib/codec/superb_options.h"


#include <iostream>
#include <type_traits>

namespace Hadron
{
  using namespace Datalib::Binary;
  using namespace Datalib::Model;
  using namespace Datalib::Serial;
  using namespace SB;

  //----------------------------------------------------------------------------
  Array<int> referenceMom(const Array<int>& mom);

  //! Key for a hadron-like particle operator
  template <std::size_t V>
  struct KeyParticleClassOp_tGen
  {
    KeyParticleClassOp_tGen() {}
    KeyParticleClassOp_tGen(const std::string& n, const std::string& s, const Array<int>& m)
      : name(n), smear(s), mom_class(m)
    {
    }
    KeyParticleClassOp_tGen(const std::string& n, const std::string& s, const Array<int>& m,
			    const std::vector<int>& d, const std::vector<Array<int>>& p)
      : name(n), smear(s), mom_class(m), disp_list(d), phasings(p)
    {
    }
    KeyParticleClassOp_tGen(const HadronDispVertex_t& vert)
      : name(vert.name),
	smear(vert.smear),
	mom_class(referenceMom(vert.mom)),
	disp_list(vert.disp_list),
	phasings(vert.phasings)
    {
    }
    bool operator==(const KeyParticleClassOp_tGen&) const = default;

    std::string          name;           /*!< Some string label for the operator */
    std::string          smear;          /*!< Some string label for the smearing of this operator */
    Array<int>           mom_class;      /*!< D-1 Canonical momentum type */
    struct Empty {
      constexpr bool operator==(const Empty&) const
      {
	return true;
      }
    };
    typename std::conditional_t<(V >= KeyHadronSUNVersion::Phasings), std::vector<Array<int>>,
				Empty>
      phasings;				 /*!< phasing associated to the quarks (for version >= 1) */
    std::vector<int> disp_list;		 /*!< 1-based displacement list */

    using schema = std::tuple<					  //
      struct_member<&KeyParticleClassOp_tGen::name, "name"_sl>,	  //
      struct_member<&KeyParticleClassOp_tGen::smear, "smear"_sl>, //
      struct_member<&KeyParticleClassOp_tGen::mom_class, "mom_class"_sl,
		    std::vector<int>>, //
      struct_member<&KeyParticleClassOp_tGen::phasings, "phasings"_sl,
		    std::vector<std::vector<int>>, NoDefault,
		    (V >= KeyHadronSUNVersion::Phasings)>, //
      struct_member<&KeyParticleClassOp_tGen::disp_list, "disp_list"_sl, None, ""_sl,
		    (V >= KeyHadronSUNVersion::DevelPdf)> //
      >;

    // Constructor from another version
    template <std::size_t V0, typename std::enable_if_t<(V != V0), bool> = true>
    KeyParticleClassOp_tGen(const KeyParticleClassOp_tGen<V0>& s)
      : name(s.name), smear(s.smear), mom_class(s.mom_class), disp_list(s.disp_list)
    {
      // NOTE: deal with phasing in KeyHadronSUNClassNPartIrrep_tGen
    }
  };

  using KeyParticleClassOp_t = KeyParticleClassOp_tGen<KeyHadronSUNVersion::Default>;

  //----------------------------------------------------------------------------------
  //! Enforce referenceMom order
  KeyParticleClassOp_t referenceOrder(const KeyParticleClassOp_t& d);

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyParticleClassOp_t& d);
	
  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const Array1dO<KeyParticleClassOp_t>& d);
	
  //----------------------------------------------------------------------------
  //! KeyParticleOp reader
  void read(XMLReader& xml, const std::string& path, KeyParticleClassOp_t& param);

  //! KeyParticleOp writer
  void write(XMLWriter& xml, const std::string& path, const KeyParticleClassOp_t& param);

  //----------------------------------------------------------------------------
  //! KeyParticleOp reader
  template <std::size_t V>
  void read(BinaryReader& bin, KeyParticleClassOp_tGen<V>& param)
  {
    readDesc(bin, param.name);
    readDesc(bin, param.smear);
    read(bin, param.mom_class);
    if constexpr (V >= KeyHadronSUNVersion::DevelPdf)
      read(bin, param.disp_list);
    if constexpr (V >= KeyHadronSUNVersion::Phasings)
      read(bin, param.phasings);
  }

  //! KeyParticleOp writer
  template <std::size_t V>
  void write(BinaryWriter& bin, const KeyParticleClassOp_tGen<V>& param)
  {
    writeDesc(bin, param.name);
    writeDesc(bin, param.smear);
    write(bin, param.mom_class);
    if constexpr (V >= KeyHadronSUNVersion::DevelPdf)
      write(bin, param.disp_list);
    if constexpr (V >= KeyHadronSUNVersion::Phasings)
      write(bin, param.phasings);
  }

} // namespace Hadron

#endif
