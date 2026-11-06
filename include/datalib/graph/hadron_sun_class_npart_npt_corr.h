// -*- C++ -*-
/*! \file
 * \brief Hadron N-point correlators
 */

#ifndef DATALIB_hadron_sun_npart_npt_corr_class_h
#define DATALIB_hadron_sun_npart_npt_corr_class_h

#include "datalib/graph/hadron_sun_class_npart_irrep.h"
#include <iostream>

namespace Hadron
{
  using namespace Datalib::Binary;

  //----------------------------------------------------------------------------
  template <std::size_t V>
  struct NPoint_t {
    int t_slice; /*!< This time-slice. If negative, then varies */
    KeyHadronSUNClassNPartIrrep_tGen<V>
      irrep; /*!< Holds each of the operators projected into irreps */
    bool operator==(const NPoint_t&) const = default;

    using schema = std::tuple<			       //
      struct_member<&NPoint_t::t_slice, "t_slice"_sl>, //
      struct_member<&NPoint_t::irrep, "Irrep"_sl>      //
      >;
  };

  //! Key for HadronSUN NPt correlator
  template <std::size_t V>
  struct KeyHadronSUNClassNPartNPtCorr_tGen {
    using NPoint_t = typename Hadron::NPoint_t<V>;
    Array1dO<NPoint_t>               npoint;      /*!< Each time-slice of an n-point function */

    using schema = std::tuple< //
      struct_member<&KeyHadronSUNClassNPartNPtCorr_tGen<V>::npoint, "NPoint"_sl,
		    std::vector<NPoint_t>> //
      >;

    KeyHadronSUNClassNPartNPtCorr_tGen()
    {
    }

    // Constructor from a different version
    template <std::size_t V0, typename std::enable_if_t<(V != V0), bool> = true>
    KeyHadronSUNClassNPartNPtCorr_tGen(const KeyHadronSUNClassNPartNPtCorr_tGen<V0>& s)
    {
      npoint.ref().reserve(s.npoint.size());
      for (const auto& it : s.npoint.ref())
	npoint.ref().push_back(NPoint_t{it.t_slice, it.irrep});
    }

    bool operator==(const KeyHadronSUNClassNPartNPtCorr_tGen&) const = default;

    // Set proper phasing for keys converted from old format
    void set_phasing(const XMLArray::Array<int>& default_phasing)
    {
      if constexpr (V >= KeyHadronSUNVersion::MomClassAndPhasing)
      {
	if (default_phasing.size() == 0)
	  return;
	for (auto& it : npoint.ref())
	{
	  if (!it.irrep.smearedP)
	    continue;
	  if constexpr (V == KeyHadronSUNVersion::MomClassAndPhasing)
	  {
	    it.irrep.phasing = default_phasing;
	  }
	  else
	  {
	    for (auto& op : it.irrep.op.ops.ref())
	    {
	      op.phasings.resize(0);
	      op.phasings.push_back(default_phasing);
	    }
	  }
	}
      }
    }
  };

  using KeyHadronSUNClassNPartNPtCorr_t =
    KeyHadronSUNClassNPartNPtCorr_tGen<KeyHadronSUNVersion::Default>;

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyHadronSUNClassNPartNPtCorr_t::NPoint_t& d);
	
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyHadronSUNClassNPartNPtCorr_t& d);
	
  //----------------------------------------------------------------------------
  //! KeyHadronSUNClassNPartNPtCorr reader
  void read(XMLReader& xml, const std::string& path, KeyHadronSUNClassNPartNPtCorr_t& param);

  //! Combo KeyHadronSUNClassNPartNPtCorr reader

   //! KeyHadronSUNClassNPartNPtCorr writer
  void write(XMLWriter& xml, const std::string& path, const KeyHadronSUNClassNPartNPtCorr_t& param);

  //----------------------------------------------------------------------------
  //! KeyHadronSUNClassNPartNPtCorr reader
  template <std::size_t V>
  void read(BinaryReader& bin, NPoint_t<V>& param)
  {
    read(bin, param.t_slice);
    read(bin, param.irrep);
  }

  //! KeyHadronSUNClassNPartNPtCorr reader
  template <std::size_t V>
  void read(BinaryReader& bin, KeyHadronSUNClassNPartNPtCorr_tGen<V>& param)
  {
    read(bin, param.npoint);
  }

  //! KeyHadronSUNClassNPartNPtCorr writer
  template <std::size_t V>
  void write(BinaryWriter& bin, const NPoint_t<V>& param)
  {
    write(bin, param.t_slice);
    write(bin, param.irrep);
  }


  //! KeyHadronSUNClassNPartNPtCorr writer
  template <std::size_t V>
  void write(BinaryWriter& bin, const KeyHadronSUNClassNPartNPtCorr_tGen<V>& param)
  {
    write(bin, param.npoint);
  }
} // namespace HadronSUN

#endif
