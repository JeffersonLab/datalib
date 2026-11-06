// -*- C++ -*-
/*! \file
 * \brief Adjacency map for graphs
 */

#ifndef DATALIB_hadron_adj_map_h
#define DATALIB_hadron_adj_map_h

#include "datalib/graph/hadron_diagram_time.h"
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
  //! Target of adjacancy map: [had -> [slot -> {target had,target slot}]]
  struct HadronAdjMapTarget_t
  {
    HadronAdjMapTarget_t() : vertex_num(0), slot_num(0), conj(false), flavor(0) {}
    HadronAdjMapTarget_t(int v, int s, bool c, char f) : 
      vertex_num(v), slot_num(s), conj(c), flavor(f) {}

    int              vertex_num;  /*!< Target vertex */
    int              slot_num;    /*!< Slot on the target */
    bool             conj;        /*!< Is quark conjugated on the target? */
    char             flavor;      /*!< Quark flavor, something like  d,u,s,c,... */

    using schema = std::tuple<						 //
      struct_member<&HadronAdjMapTarget_t::vertex_num, "vertex_num"_sl>, //
      struct_member<&HadronAdjMapTarget_t::slot_num, "slot_num"_sl>,	 //
      struct_member<&HadronAdjMapTarget_t::conj, "conj"_sl>,		 //
      struct_member<&HadronAdjMapTarget_t::flavor, "flavor"_sl>		 //
      >;

    bool operator==(const HadronAdjMapTarget_t&) const = default;
  };

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const HadronAdjMapTarget_t& d);
	
  //----------------------------------------------------------------------------
  // Reader
  void read(XMLReader& xml, const std::string& path, HadronAdjMapTarget_t& param);

  // Writer
  void write(XMLWriter& xml, const std::string& path, const HadronAdjMapTarget_t& param);
  
  //----------------------------------------------------------------------------
  // Reader
  void read(BinaryReader& bin, HadronAdjMapTarget_t& param);

  // Writer
  void write(BinaryWriter& bin, const HadronAdjMapTarget_t& param);

  //----------------------------------------------------------------------------
  //! Adjacancy map: [vertex_num -> [slot -> {target vertex,target slot}]]
  typedef std::map<int, std::map<int, HadronAdjMapTarget_t> > HadronGraphAdjMap_t;

  //----------------------------------------------------------------------------
  //! Diagnostic output
  std::ostream& operator<<(std::ostream& os, const HadronGraphAdjMap_t& adj_map);

} // namespace Hadron

#endif
