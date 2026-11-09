// -*- C++ -*-
/*! \file
 * \brief Ensemble file names
 */

#ifndef __ensem_filenames_h__
#define __ensem_filenames_h__

#include <datalib/model/xml_array.h>
#include <string>

#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/legacy/hadron_2pt_corr.h>
#include <datalib/legacy/hadron_3pt_corr.h>

#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>

#include <datalib/graph/hadron_class_npart_npt_conn_graph.h>

namespace Hadron
{
  using namespace XMLArray;

  //----------------------------------------------------------------------------
  //! Build a 1pt correlator file name
  std::string ensemFileName(const FF::KeyHadron1PtCorr_t& key);

  //! Build a 2pt correlator file name
  std::string ensemFileName(const FF::KeyHadron2PtCorr_t& key);

  //! Build a 3pt correlator file name
  std::string ensemFileName(const FF::KeyHadron3PtCorr_t& key);

  //----------------------------------------------------------------------------
  //! Build a npt file name
  std::string ensemFileName(const Hadron::KeyHadronClassNPartNPtConnGraph_t& key);

  //----------------------------------------------------------------------------
  //! Build a npt file name
  std::string ensemFileName(const Hadron::KeyHadronNPartNPtCorr_t& key);

  //----------------------------------------------------------------------------
  //! Build a npt file name
  std::string ensemFileName(const Hadron::KeyHadronSUNClassNPartNPtCorr_t& key);

  //----------------------------------------------------------------------------
  //! Build a npt irrep name
  std::string ensemFileName(const KeyHadronSUNClassNPartIrrepOp_t& key);

  //! Build a npt irrep name
  std::string ensemFileName(const Hadron::KeyHadronSUNClassNPartIrrep_t& key);

} // namespace ColorVec

#endif
