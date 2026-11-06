// -*- C++ -*-
/*! \file
 * \brief Hadron N-point connected graphs
 */

#ifndef DATALIB_hadron_class_npart_npt_conn_graph_class_h
#define DATALIB_hadron_class_npart_npt_conn_graph_class_h

#include "datalib/graph/hadron_adj_map.h"
#include "datalib/graph/hadron_diagram_time.h"
#include "datalib/graph/hadron_disp_vertex.h"
#include "datalib/codec/superb_options.h"
#include <map>
#include <list>
#include <iostream>

namespace Hadron
{
  using namespace Datalib::Binary;
  using namespace Datalib::Model;
  using namespace Datalib::Serial;
  using namespace SB;

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //! Key for Hadron NPt connected graph
  struct KeyHadronClassNPartNPtConnGraph_t
  {
    std::string                                         graph;             /*!< Label for the graph */
    std::string                                         ensemble;          /*!< Label for the ensemble */
    HadronGraphAdjMap_t                                 adj_map;           /*!< Adjacency map of vertex connections */
    std::map<int, std::pair<HadronDispVertex_t,int> >   vertices;          /*!< Vertex info [vertex_num -> {vertex,npt}] */
    HadronDiagramTimeSlices_t                           diag_time_slices;  /*!< Diagram time-slice structure */

    using schema = std::tuple<									 //
      struct_member<&KeyHadronClassNPartNPtConnGraph_t::graph, "graph"_sl>,			 //
      struct_member<&KeyHadronClassNPartNPtConnGraph_t::ensemble, "ensemble"_sl>,		 //
      struct_member<&KeyHadronClassNPartNPtConnGraph_t::adj_map, "adj_map"_sl>,			 //
      struct_member<&KeyHadronClassNPartNPtConnGraph_t::vertices, "vertices"_sl>,		 //
      struct_member<&KeyHadronClassNPartNPtConnGraph_t::diag_time_slices, "diag_time_slices"_sl> //
      >;

    bool operator==(const KeyHadronClassNPartNPtConnGraph_t&) const = default;
  };
  
  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyHadronClassNPartNPtConnGraph_t& d);
	
  //----------------------------------------------------------------------------
  //! KeyHadronClassNPartNPtConnGraph reader
  void read(XMLReader& xml, const std::string& path, KeyHadronClassNPartNPtConnGraph_t& param);

  //! KeyHadronClassNPartNPtConnGraph writer
  void write(XMLWriter& xml, const std::string& path, const KeyHadronClassNPartNPtConnGraph_t& param);

  //----------------------------------------------------------------------------
  //! KeyHadronClassNPartNPtConnGraph reader
  void read(BinaryReader& bin, KeyHadronClassNPartNPtConnGraph_t& param);

  //! KeyHadronClassNPartNPtConnGraph writer
  void write(BinaryWriter& bin, const KeyHadronClassNPartNPtConnGraph_t& param);

} // namespace Hadron

#endif
