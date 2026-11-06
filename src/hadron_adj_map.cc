/*! \file
 * \brief Adjacency map for graphs
 */

#include "datalib/graph/hadron_adj_map.h"

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
  std::ostream& operator<<(std::ostream& os, const HadronAdjMapTarget_t& d)
  {
    os << d.vertex_num;
    if (d.conj)
    {
      os << char(toupper(d.flavor));
    }
    else
    {
      os << d.flavor;
    }

    os << d.slot_num;

    return os;
  }


  //----------------------------------------------------------------------------
  //! Diagnostic output
  std::ostream& operator<<(std::ostream& os, const HadronGraphAdjMap_t& adj_map)
  {
    for(auto vert = adj_map.begin(); vert != adj_map.end(); ++vert)
    {
      if (vert != adj_map.begin()) {os << " ";}

      os << "v" << vert->first << "[";

      for(auto slot = vert->second.begin(); slot != vert->second.end(); ++slot)
      {
	if (slot != vert->second.begin()) {os << " ";}

	os << "[s" << slot->first << "->" << "v" << slot->second.vertex_num << "s" << slot->second.slot_num << "]";
      }

      os << "]";
    }

    return os;
  }


  //----------------------------------------------------------------------------
  // Reader
  void read(XMLReader& xml, const std::string& path, HadronAdjMapTarget_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "vertex_num", param.vertex_num);
    read(paramtop, "slot_num", param.slot_num);
    read(paramtop, "flavor", param.flavor);
    read(paramtop, "conj", param.conj);
  }

  // Writer
  void write(XMLWriter& xml, const std::string& path, const HadronAdjMapTarget_t& param)
  {
    push(xml, path);

    write(xml, "vertex_num", param.vertex_num);
    write(xml, "slot_num", param.slot_num);
    write(xml, "flavor", param.flavor);
    write(xml, "conj", param.conj);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  // Reader
  void read(BinaryReader& bin, HadronAdjMapTarget_t& param)
  {
    read(bin, param.vertex_num);
    read(bin, param.slot_num);
    read(bin, param.conj);
    read(bin, param.flavor);
  }

  // Writer
  void write(BinaryWriter& bin, const HadronAdjMapTarget_t& param)
  {
    write(bin, param.vertex_num);
    write(bin, param.slot_num);
    write(bin, param.conj);
    write(bin, param.flavor);
  }

} // namespace Hadron
