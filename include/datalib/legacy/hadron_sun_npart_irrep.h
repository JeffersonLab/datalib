// -*- C++ -*-
/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep and specific
 * row including creation op info
 */

#ifndef __hadron_sun_npart_irrep_h__
#define __hadron_sun_npart_irrep_h__
#include <datalib/codec/binary.h>
#include <datalib/io/serial_xml.h>

#include <datalib/codec/superb_options.h>
#include <datalib/graph/cgc_irrep_mom.h>
#include <datalib/graph/cgc_su3.h>
#include <datalib/legacy/hadron_sun_npart_irrep_op.h>
#include <datalib/model/arrays.h>

#include <iostream>
#include <string>

namespace Hadron {
  using namespace SB;

  //----------------------------------------------------------------------------
  //! Time slice collection of operators projected onto a definite irrep and
  //! specific row
  struct KeyHadronSUNNPartIrrep_t {
    bool creation_op; /*!< Are these creation ops on this time-slice? */
    bool smearedP;    /*!< Do these operators use distillation/distillution? */
    KeyCGCSU3_t flavor;         /*!< Target flavor component */
    KeyCGCIrrepMom_t irrep_mom; /*!< Target irrep row and D-1 momentum of this
                                   N-particle op */

    KeyHadronSUNNPartIrrepOp_t op; /*!< Operator */

    bool operator==(const KeyHadronSUNNPartIrrep_t &) const = default;

    using schema = std::tuple< //
        struct_member<&KeyHadronSUNNPartIrrep_t::creation_op,
                      "creation_op"_sl>,                                   //
        struct_member<&KeyHadronSUNNPartIrrep_t::smearedP, "smearedP"_sl>, //
        struct_member<&KeyHadronSUNNPartIrrep_t::flavor, "flavor"_sl>,     //
        struct_member<&KeyHadronSUNNPartIrrep_t::irrep_mom, "irmom"_sl>,   //
        struct_member<&KeyHadronSUNNPartIrrep_t::op, "Op"_sl>              //
        >;
  };

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream &operator<<(std::ostream &os, const KeyHadronSUNNPartIrrep_t &d);

  //----------------------------------------------------------------------------
  //! KeyHadronSUNNPartIrrep reader
  void read(Datalib::Serial::XMLReader &xml, const std::string &path,
            KeyHadronSUNNPartIrrep_t &param);

  //! KeyHadronSUNNPartIrrep writer
  void write(Datalib::Serial::XMLWriter &xml, const std::string &path,
             const KeyHadronSUNNPartIrrep_t &param);

  //----------------------------------------------------------------------------
  //! KeyHadronSUNNPartIrrep reader
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartIrrep_t &param);

  //! KeyHadronSUNNPartIrrep writer
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartIrrep_t &param);

} // namespace Hadron

#endif
