// -*- C++ -*-
/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep
 */

#ifndef DATALIB_hadron_sun_npart_irrep_op_class_h
#define DATALIB_hadron_sun_npart_irrep_op_class_h

#include "datalib/graph/particle_class_op.h"
#include "datalib/model/arrays.h"
#include "datalib/codec/superb_options.h"

#include <string>
#include <iostream>

namespace Hadron
{
  using namespace Datalib::Binary;
  using namespace Datalib::Model;
  using namespace Datalib::Serial;
  using namespace SB;

  struct CGPair_t {
    struct Slot_t {
      std::string slot; /*!< At first level, the HadronOp index, at higher levels, the temporary */
      std::string F; /*!< Target SU(N) flavor irrep - of form for SU(2) "3,1", or for SU(3) "8,1" */
      std::string irrep;    /*!< Target Octahedral/little-group irrep - of form   "Lambda,embed" */
      Array<int> mom_class; /*!< Target D-1 Canonical momentum type */
      bool operator==(const Slot_t&) const = default;

      using schema = std::tuple<					    //
	struct_member<&Slot_t::slot, "slot"_sl>,			    //
	struct_member<&Slot_t::F, "F"_sl>,				    //
	struct_member<&Slot_t::irrep, "irrep"_sl>,			    //
	struct_member<&Slot_t::mom_class, "mom_class"_sl, std::vector<int>> //
	>;
    };

    bool operator==(const CGPair_t&) const = default;

    std::string left;  /*!< Left side of CG */
    std::string right; /*!< Right side of CG */
    Slot_t target;     /*!< Target of CG */

    using schema = std::tuple<			    //
      struct_member<&CGPair_t::left, "left"_sl>,    //
      struct_member<&CGPair_t::right, "right"_sl>,  //
      struct_member<&CGPair_t::target, "target"_sl> //
      >;
  };

  //----------------------------------------------------------------------------
  //! Time slice collection of operators projected onto a definite irrep
  template <std::size_t V>
  struct KeyHadronSUNClassNPartIrrepOp_tGen
  {
    using CGPair_t = Hadron::CGPair_t;
    Array1dO<CGPair_t>                     CGs;            /*!< Each pair of CG contractions */
    Array1dO<KeyParticleClassOp_tGen<V>>   ops;            /*!< Each operator */

    using schema = std::tuple< //
      struct_member<&KeyHadronSUNClassNPartIrrepOp_tGen<V>::ops, "Operators"_sl,
		    std::vector<KeyParticleClassOp_tGen<V>>>,					  //
      struct_member<&KeyHadronSUNClassNPartIrrepOp_tGen<V>::CGs, "CGs"_sl, std::vector<CGPair_t>> //
      >;

    bool operator==(const KeyHadronSUNClassNPartIrrepOp_tGen&) const = default;

    KeyHadronSUNClassNPartIrrepOp_tGen()
    {
    }

    // Constructor from another version
    template <std::size_t V0, typename std::enable_if_t<(V != V0), bool> = true>
    KeyHadronSUNClassNPartIrrepOp_tGen(const KeyHadronSUNClassNPartIrrepOp_tGen<V0>& s) : CGs(s.CGs)
    {
      ops.ref().reserve(s.ops.size());
      for (const auto& it : s.ops.ref())
	ops.ref().push_back(it);
    }
  };

  using KeyHadronSUNClassNPartIrrepOp_t = KeyHadronSUNClassNPartIrrepOp_tGen<KeyHadronSUNVersion::Default>;

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyHadronSUNClassNPartIrrepOp_t& d);
	
  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, CGPair_t::Slot_t& param);

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const CGPair_t::Slot_t& param);

  // Read a key
  void read(XMLReader& xml, const std::string& path, CGPair_t& param);

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const CGPair_t& param);

  //! KeyHadronSUNNPartIrrep reader
  void read(XMLReader& xml, const std::string& path, KeyHadronSUNClassNPartIrrepOp_t& param);

  //! KeyHadronSUNNPartIrrep writer
  void write(XMLWriter& xml, const std::string& path, const KeyHadronSUNClassNPartIrrepOp_t& param);

  //----------------------------------------------------------------------------
  // Read a key
  void read(BinaryReader& bin, CGPair_t::Slot_t& param);

  // Write a key
  void write(BinaryWriter& bin, const CGPair_t::Slot_t& param);

  // Read a key
  void read(BinaryReader& bin, CGPair_t& param);

  // Write a key
  void write(BinaryWriter& bin, const CGPair_t& param);

  //! KeyHadronSUNNPartIrrep reader
  template <std::size_t V>
  void read(BinaryReader& bin, KeyHadronSUNClassNPartIrrepOp_tGen<V>& param)
  {
    read(bin, param.ops);
    read(bin, param.CGs);
  }

  //! KeyHadronSUNNPartIrrep writer
  template <std::size_t V>
  void write(BinaryWriter& bin, const KeyHadronSUNClassNPartIrrepOp_tGen<V>& param)
  {
    write(bin, param.ops);
    write(bin, param.CGs);
  }

} // namespace Hadron

#endif
