// -*- C++ -*-
/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep
 */

#ifndef __hadron_sun_npart_irrep_op_h__
#define __hadron_sun_npart_irrep_op_h__
#include <datalib/codec/binary.h>
#include <datalib/io/serial_xml.h>

#include <datalib/codec/superb_options.h>
#include <datalib/legacy/particle_op.h>
#include <datalib/model/arrays.h>

#include <iostream>
#include <string>

namespace Hadron {
  using namespace SB;

  //----------------------------------------------------------------------------
  //! Time slice collection of operators projected onto a definite irrep
  struct KeyHadronSUNNPartIrrepOp_t {
    struct CGPair_t {
      struct Slot_t {
        std::string slot; /*!< At first level, the HadronOp index, at higher
                             levels, the temporary */
        std::string F; /*!< Target SU(N) flavor irrep - of form for SU(2) "3,1",
                          or for SU(3) "8,1" */
        std::string irrep; /*!< Target Octahedral/little-group irrep - of form
                              "Lambda,embed" */
        XMLArray::Array<int>
            mom_type; /*!< Target D-1 Canonical momentum type */
        bool operator==(const Slot_t &) const = default;

        using schema = std::tuple<                                            //
            struct_member<&Slot_t::slot, "slot"_sl>,                          //
            struct_member<&Slot_t::F, "F"_sl>,                                //
            struct_member<&Slot_t::irrep, "irrep"_sl>,                        //
            struct_member<&Slot_t::mom_type, "mom_type"_sl, std::vector<int>> //
            >;
      };

      std::string left;  /*!< Left side of CG */
      std::string right; /*!< Right side of CG */
      Slot_t target;     /*!< Target of CG */

      using schema = std::tuple<                        //
          struct_member<&CGPair_t::left, "left"_sl>,    //
          struct_member<&CGPair_t::right, "right"_sl>,  //
          struct_member<&CGPair_t::target, "target"_sl> //
          >;
      bool operator==(const CGPair_t &) const = default;
    };

    Datalib::Model::Array1dO<CGPair_t> CGs; /*!< Each pair of CG contractions */
    Datalib::Model::Array1dO<KeyParticleOp_t> ops; /*!< Each operator */

    using schema = std::tuple< //
        struct_member<&KeyHadronSUNNPartIrrepOp_t::ops, "Operators"_sl,
                      std::vector<KeyParticleOp_t>>, //
        struct_member<&KeyHadronSUNNPartIrrepOp_t::CGs, "CGs"_sl,
                      std::vector<CGPair_t>> //
        >;

    bool operator==(const KeyHadronSUNNPartIrrepOp_t &) const = default;
  };

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream &operator<<(std::ostream &os,
                           const KeyHadronSUNNPartIrrepOp_t &d);

  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Serial::XMLReader &xml, const std::string &path,
            KeyHadronSUNNPartIrrepOp_t::CGPair_t::Slot_t &param);

  // Write a key
  void write(Datalib::Serial::XMLWriter &xml, const std::string &path,
             const KeyHadronSUNNPartIrrepOp_t::CGPair_t::Slot_t &param);

  // Read a key
  void read(Datalib::Serial::XMLReader &xml, const std::string &path,
            KeyHadronSUNNPartIrrepOp_t::CGPair_t &param);

  // Write a key
  void write(Datalib::Serial::XMLWriter &xml, const std::string &path,
             const KeyHadronSUNNPartIrrepOp_t::CGPair_t &param);

  //! KeyHadronSUNNPartIrrep reader
  void read(Datalib::Serial::XMLReader &xml, const std::string &path,
            KeyHadronSUNNPartIrrepOp_t &param);

  //! KeyHadronSUNNPartIrrep writer
  void write(Datalib::Serial::XMLWriter &xml, const std::string &path,
             const KeyHadronSUNNPartIrrepOp_t &param);

  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartIrrepOp_t::CGPair_t::Slot_t &param);

  // Write a key
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartIrrepOp_t::CGPair_t::Slot_t &param);

  // Read a key
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartIrrepOp_t::CGPair_t &param);

  // Write a key
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartIrrepOp_t::CGPair_t &param);

  //! KeyHadronSUNNPartIrrep reader
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartIrrepOp_t &param);

  //! KeyHadronSUNNPartIrrep writer
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartIrrepOp_t &param);

} // namespace Hadron

#endif
