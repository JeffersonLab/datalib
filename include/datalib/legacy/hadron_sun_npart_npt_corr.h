// -*- C++ -*-
/*! \file
 * \brief Hadron N-point correlators
 */

#ifndef __hadron_sun_npart_npt_corr_h__
#define __hadron_sun_npart_npt_corr_h__
#include <datalib/codec/binary.h>
#include <datalib/io/serial_xml.h>
#include <datalib/model/arrays.h>

#include <datalib/legacy/hadron_sun_npart_irrep.h>
#include <iostream>

namespace Hadron {
  using namespace SB;

  //----------------------------------------------------------------------------
  //! Key for HadronSUN NPt correlator
  struct KeyHadronSUNNPartNPtCorr_t {
    struct NPoint_t {
      int t_slice; /*!< This time-slice. If negative, then varies */
      KeyHadronSUNNPartIrrep_t
          irrep; /*!< Holds each of the operators projected into irreps */

      bool operator==(const NPoint_t &) const = default;

      using schema = std::tuple<                           //
          struct_member<&NPoint_t::t_slice, "t_slice"_sl>, //
          struct_member<&NPoint_t::irrep, "Irrep"_sl>      //
          >;
    };

    Datalib::Model::Array1dO<NPoint_t>
        npoint; /*!< Each time-slice of an n-point function */

    bool operator==(const KeyHadronSUNNPartNPtCorr_t &) const = default;

    using schema = std::tuple< //
        struct_member<&KeyHadronSUNNPartNPtCorr_t::npoint, "NPoint"_sl,
                      std::vector<NPoint_t>> //
        >;
  };

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream &operator<<(std::ostream &os,
                           const KeyHadronSUNNPartNPtCorr_t::NPoint_t &d);

  //! Used for error output
  std::ostream &operator<<(std::ostream &os,
                           const KeyHadronSUNNPartNPtCorr_t &d);

  //----------------------------------------------------------------------------
  //! KeyHadronSUNNPartNPtCorr reader
  void read(Datalib::Serial::XMLReader &xml, const std::string &path,
            KeyHadronSUNNPartNPtCorr_t &param);

  //! KeyHadronSUNNPartNPtCorr writer
  void write(Datalib::Serial::XMLWriter &xml, const std::string &path,
             const KeyHadronSUNNPartNPtCorr_t &param);

  //----------------------------------------------------------------------------
  //! KeyHadronSUNNPartNPtCorr reader
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartNPtCorr_t::NPoint_t &param);

  //! KeyHadronSUNNPartNPtCorr reader
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartNPtCorr_t &param);

  //! KeyHadronSUNNPartNPtCorr writer
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartNPtCorr_t::NPoint_t &param);

  //! KeyHadronSUNNPartNPtCorr writer
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartNPtCorr_t &param);

} // namespace Hadron
#endif
