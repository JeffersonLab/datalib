// -*- C++ -*-
#ifndef DATALIB_GRAPH_PROCESS_CORRS_H
#define DATALIB_GRAPH_PROCESS_CORRS_H
#include "datalib/model/map_obj.h"
#include "datalib/graph/hadron_class_npart_npt_conn_graph.h"
#include "datalib/graph/hadron_sun_class_npart_npt_corr.h"
#include "datalib/graph/hadron_timeslice.h"
#include "datalib/graph/single_hadron_coeffs.h"
namespace Hadron {
using namespace Datalib::Model;
using namespace Datalib::Binary;
using namespace Datalib::Serial;
inline constexpr int CorrGraphFormatVersion = 4;
// Wire layout of a correlation graph; independent of evaluator runtime state.
struct CorrGraphLayout_t {
  std::vector<int> latt_size;
  int decay_dir = 3;
};
struct ValCorrGraphMap_t {
  MapObject<std::vector<KeyHadronClassNPartNPtConnGraph_t>,std::complex<double>> diagrams;
  Array1dO<int> quark_count;
};
struct ValCorrIntGraphMap_t {
  MapObject<std::vector<int>,std::complex<double>> diagrams;
  Array1dO<int> quark_count;
};
struct CorrGraphMap_t {
  int version = CorrGraphFormatVersion;
  int Nt_corr = 0;
  int bc_spec = -1;
  CorrGraphLayout_t layout;
  std::string ensemble;
  MapObject<KeyHadronSUNClassNPartNPtCorr_t,ValCorrIntGraphMap_t> corr_graph_map;
  std::vector<KeyHadronClassNPartNPtConnGraph_t> int_graph_list;
  MapObject<HadronDispVertex_t,MapSingleHadronQuarkFlavorColorSpin_t> vertex_coeff_map;
};
void read(XMLReader&, const std::string&, CorrGraphLayout_t&);
void write(XMLWriter&, const std::string&, const CorrGraphLayout_t&);
void read(BinaryReader&, CorrGraphLayout_t&);
void write(BinaryWriter&, const CorrGraphLayout_t&);
void read(XMLReader&, const std::string&, ValCorrGraphMap_t&);
void write(XMLWriter&, const std::string&, const ValCorrGraphMap_t&);
void read(BinaryReader&, ValCorrGraphMap_t&);
void write(BinaryWriter&, const ValCorrGraphMap_t&);
void read(XMLReader&, const std::string&, ValCorrIntGraphMap_t&);
void write(XMLWriter&, const std::string&, const ValCorrIntGraphMap_t&);
void read(BinaryReader&, ValCorrIntGraphMap_t&);
void write(BinaryWriter&, const ValCorrIntGraphMap_t&);
void read(XMLReader&, const std::string&, CorrGraphMap_t&);
void write(XMLWriter&, const std::string&, const CorrGraphMap_t&);
void read(BinaryReader&, CorrGraphMap_t&);
void write(BinaryWriter&, const CorrGraphMap_t&);
}
#endif
