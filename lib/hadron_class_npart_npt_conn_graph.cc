/*! \file
 * \brief Hadron N-point connected graphs
 */

#include "datalib/graph/hadron_class_npart_npt_conn_graph.h"

namespace Hadron
{

  namespace
  {
    //! Error output
    std::ostream& operator<<(std::ostream& os, const Array<int>& d)
    {
      if (d.size() > 0)
      {
	os << d[0];

	for(int i=1; i < d.size(); ++i)
	  os << " " << d[i];
      }

      return os;
    }

  }


  //----------------------------------------------------------------------------
  //! Used for output
  std::ostream& operator<<(std::ostream& os, const KeyHadronClassNPartNPtConnGraph_t& d)
  {
    // Invert the npt-map
    // [npt -> {vertex_num-s}]
    std::map<int, std::list<int> > npt_vert;

    for(auto v = d.vertices.begin(); v != d.vertices.end(); ++v)
    {
      npt_vert[v->second.second].push_back(v->first);
    }

    os << "graph= " << d.graph << "\n";

    for(auto npt = npt_vert.begin(); npt != npt_vert.end(); ++npt)
    {
      os << "  Npt[";
      os.width(2);
      os << npt->first << "]: t_slice= ";

      if (npt->first == d.diag_time_slices.neg_slice_npt)
      {
	os << -d.diag_time_slices.source_slice_npt << "\n";
      }
      else
      {
	os << d.diag_time_slices.t_slices[npt->first] << "\n";
      }

      for(auto op = npt->second.begin(); op != npt->second.end(); ++op)
      {
	os << "    V[";
	os.width(2);
	os << *op << "]:" << d.vertices.at(*op).first << "\n";
	os << "           ";

	for(auto slot = d.adj_map.at(*op).begin(); slot != d.adj_map.at(*op).end(); ++slot)
	{
	  if (slot == d.adj_map.at(*op).begin()) {os << "[";} else {os << " ";}

	  if (! slot->second.conj)
	  {
	    os << char(toupper(slot->second.flavor));
	  }
	  else
	  {
	    os << slot->second.flavor;
	  }

	  os << "[s" << slot->first;

	  os << " -> " << "v" << slot->second.vertex_num << "s" << slot->second.slot_num;
	  os << "]";
	}

	os << "]\n";
      }
    }

    os << "  " << d.diag_time_slices << std::endl;

    if (! d.ensemble.empty())
      os << " ensemble= " << d.ensemble << std::endl;

    return os;
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyHadronClassNPartNPtConnGraph_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "graph", param.graph);
    read(paramtop, "AdjMap", param.adj_map);
    read(paramtop, "Vertices", param.vertices);
    read(paramtop, "DiagramTimeSlices", param.diag_time_slices);
    if (paramtop.count("ensemble") > 0)
      read(paramtop, "ensemble", param.ensemble);
  }

  // Writer 
  void write(XMLWriter& xml, const std::string& path, const KeyHadronClassNPartNPtConnGraph_t& param)
  {
    push(xml, path);

    write(xml, "graph", param.graph);
    write(xml, "AdjMap", param.adj_map);
    write(xml, "Vertices", param.vertices);
    write(xml, "DiagramTimeSlices", param.diag_time_slices);
    if (! param.ensemble.empty())
      write(xml, "ensemble", param.ensemble);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  // Reader
  void read(BinaryReader& bin, KeyHadronClassNPartNPtConnGraph_t& param)
  {
    read(bin, param.adj_map);
    read(bin, param.vertices);
    read(bin, param.diag_time_slices);
    readDesc(bin, param.graph);
    readDesc(bin, param.ensemble);
  }

  // Writer
  void write(BinaryWriter& bin, const KeyHadronClassNPartNPtConnGraph_t& param)
  {
    write(bin, param.adj_map);
    write(bin, param.vertices);
    write(bin, param.diag_time_slices);
    writeDesc(bin, param.graph);
    writeDesc(bin, param.ensemble);
  }

} // namespace Hadron
