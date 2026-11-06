// -*- C++ -*-
/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep and specific row including creation op info
 */

#ifndef DATALIB_hadron_sun_npart_irrep_class_h
#define DATALIB_hadron_sun_npart_irrep_class_h

#include "datalib/model/arrays.h"
#include "datalib/graph/hadron_sun_class_npart_irrep_op.h"
#include "datalib/graph/cgc_su3.h"
#include "datalib/graph/cgc_irrep_mom.h"

#include <iostream>
#include <string>
#include <type_traits>

namespace Hadron
{
  using namespace Datalib::Binary;
  using namespace Datalib::Model;
  using namespace Datalib::Serial;

  //----------------------------------------------------------------------------
  //! Time slice collection of operators projected onto a definite irrep and specific row
  template <std::size_t V>
  struct KeyHadronSUNClassNPartIrrep_tGen {
    bool                                creation_op;    /*!< Are these creation ops on this time-slice? */
    bool                                smearedP;       /*!< Do these operators use distillation/distillution? */
    KeyCGCSU3_t                         flavor;         /*!< Target flavor component */
    KeyCGCIrrepMom_t                    irrep_mom;      /*!< Target irrep row and D-1 momentum of this N-particle op */
    struct Empty {
      constexpr bool operator==(Empty) const
      {
	return true;
      }
    };
    typename std::conditional_t<(V == KeyHadronSUNVersion::MomClassAndPhasing), Array<int>, Empty>
      phasing; /*!< Phasing of the particle (only for version 0) */
    bool operator==(const KeyHadronSUNClassNPartIrrep_tGen&) const = default;

    KeyHadronSUNClassNPartIrrepOp_tGen<V> op; /*!< Operator */

    using schema = std::tuple<								  //
      struct_member<&KeyHadronSUNClassNPartIrrep_tGen<V>::creation_op, "creation_op"_sl>, //
      struct_member<&KeyHadronSUNClassNPartIrrep_tGen<V>::smearedP, "smearedP"_sl>,	  //
      struct_member<&KeyHadronSUNClassNPartIrrep_tGen<V>::flavor, "flavor"_sl>,		  //
      struct_member<&KeyHadronSUNClassNPartIrrep_tGen<V>::irrep_mom, "irmom"_sl>,	  //
      struct_member<&KeyHadronSUNClassNPartIrrep_tGen<V>::phasing, "phasing"_sl, std::vector<int>,
		    ""_sl, (V == KeyHadronSUNVersion::MomClassAndPhasing)>,
      struct_member<&KeyHadronSUNClassNPartIrrep_tGen<V>::op, "Op"_sl> //
      >;

    // Default constructor
    KeyHadronSUNClassNPartIrrep_tGen()
    {
    }

    // Constructor from an earlier version
    template <std::size_t V0, typename std::enable_if_t<(V > V0), bool> = true>
    KeyHadronSUNClassNPartIrrep_tGen(const KeyHadronSUNClassNPartIrrep_tGen<V0>& s)
      : creation_op(s.creation_op),
	smearedP(s.smearedP),
	flavor(s.flavor),
	irrep_mom(s.irrep_mom),
	op(s.op)
    {
      // If the object has some phasing, set it to all operators
      if constexpr (V == KeyHadronSUNVersion::MomClassAndPhasing)
      {
	if (s.phasing.size() > 0)
	  for (auto& it : op.ops.ref())
	    it.phasings.push_back(s.phasing);
      }
    }

    // Constructor from a more recent version
    template <std::size_t V0, typename std::enable_if_t<(V < V0), bool> = true>
    KeyHadronSUNClassNPartIrrep_tGen(const KeyHadronSUNClassNPartIrrep_tGen<V0>& s)
      : creation_op(s.creation_op),
	smearedP(s.smearedP),
	flavor(s.flavor),
	irrep_mom(s.irrep_mom),
	op(s.op)
    {
      // If all operators for the object at version > 0 has the same phasing, set it as
      // the phasing of the object at version 0
      if constexpr (V0 == KeyHadronSUNVersion::MomClassAndPhasing)
      {
	bool is_phasing_set = false;
	for (const auto& it : s.op.ops.ref())
	{
	  if (it.phasings.size() == 0)
	  {
	    if (is_phasing_set)
	    {
	      if (phasing.size() != 0)
		throw std::runtime_error("Unconvertible key");
	    }
	    else
	    {
	      is_phasing_set = true;
	    }
	  }
	  else
	  {
	    for (const auto& p : it.phasings)
	    {
	      if (!is_phasing_set)
	      {
		phasing = p;
		is_phasing_set = true;
	      }
	      else if (p != phasing)
	      {
		throw std::runtime_error("Unconvertible key");
	      }
	    }
	  }
	}
      }
    }
  };

  using KeyHadronSUNClassNPartIrrep_t =
    KeyHadronSUNClassNPartIrrep_tGen<KeyHadronSUNVersion::Default>;

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyHadronSUNClassNPartIrrep_t& d);
	
  //----------------------------------------------------------------------------
  //! KeyHadronSUNClassNPartIrrep reader
  void read(XMLReader& xml, const std::string& path, KeyHadronSUNClassNPartIrrep_t& param);

  //! KeyHadronSUNClassNPartIrrep writer
  void write(XMLWriter& xml, const std::string& path, const KeyHadronSUNClassNPartIrrep_t& param);

  //----------------------------------------------------------------------------
  //! KeyHadronSUNClassNPartIrrep reader
  template <std::size_t V>
  void read(BinaryReader& bin, KeyHadronSUNClassNPartIrrep_tGen<V>& param)
  {
    static_assert(V != KeyHadronSUNVersion::MomClassAndPhasing,
		  "don't support binary keys for this version");
    read(bin, param.creation_op);
    read(bin, param.smearedP);
    read(bin, param.flavor);
    read(bin, param.irrep_mom);
    read(bin, param.op);
  }


  //! KeyHadronSUNClassNPartIrrep writer
  template <std::size_t V>
  void write(BinaryWriter& bin, const KeyHadronSUNClassNPartIrrep_tGen<V>& param)
  {
    static_assert(V != KeyHadronSUNVersion::MomClassAndPhasing,
		  "don't support binary keys for this version");
    write(bin, param.creation_op);
    write(bin, param.smearedP);
    write(bin, param.flavor);
    write(bin, param.irrep_mom);
    write(bin, param.op);
  }

} // namespace Hadron

#endif
