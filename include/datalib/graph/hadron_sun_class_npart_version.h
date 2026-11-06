// -*- C++ -*-
/*! \file
 * \brief Hadron N-point correlators
 */

#ifndef DATALIB_hadron_sun_class_npart_version_h
#define DATALIB_hadron_sun_class_npart_version_h

#include <cstddef>
namespace Hadron
{
  struct KeyHadronSUNVersion {
    /// Version of KeyHadronSUNNPartNPtCorr_t without disp_list in KeyParticleOp_t
    static constexpr std::size_t Devel = 0;

    /// Version of KeyHadronSUNNPartNPtCorr_t with disp_list in KeyParticleOp_t
    static constexpr std::size_t DevelPdf = 1;

    /// Version of KeyHadronSUNClassNPartNPtCorr_t with:
    /// - disp_list in KeyParticleClassOp_t
    /// - replaced mom_type by mom_class in CGPair_t
    /// - phasing in KeyHadronSUNClassNPartIrrep_t
    static constexpr std::size_t MomClassAndPhasing = 2;

    /// Version of KeyHadronSUNClassNPartNPtCorr_t where phasing is removed from KeyHadronSUNClassNPartIrrep_t,
    /// and added a per-quark phasing (phasings) in KeyParticleClassOp_t
    static constexpr std::size_t Phasings = 3;

    /// Current version
    static constexpr std::size_t Default = Phasings;
  };
}
#endif
