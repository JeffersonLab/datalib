// -*- C++ -*-
/*! \file
 * \brief Reduced set of hadron vertex info
 */

#ifndef DATALIB_hadron_disp_vertex_h
#define DATALIB_hadron_disp_vertex_h

#include "datalib/io/serial_xml.h"
#include "datalib/codec/binary.h"
#include "datalib/codec/superb_options.h"

namespace Hadron
{ 
  // Namespace composition
  using XMLArray::Array;
  using namespace Datalib::Serial;
  using namespace Datalib::Binary;
  using namespace SB;

  //----------------------------------------------------------------------------------
  //! The key for the hadron nodes used here
  struct HadronDispVertex_t
  {
    HadronDispVertex_t() : type(0), piece_num(0), Cconj(false), twoI_z(0), row(0), creation_op(false), smearedP(false) {}

    std::string       name;          /*!< Some string label for the operator */
    std::string       smear;         /*!< Some string label for the smearing of this operator */
    std::vector<int>  disp_list;     /*!< 1-based displacement list for this operator */
    char              type;          /*!< Type of hadron, e.g. M=2-quark meson, B=3-quark baryon, etc. */
    int               piece_num;     /*!< 1-based piece number of a hadron */
    bool              Cconj;         /*!< Is the node (gamma matrix structure) C-conjugated? */
    int               twoI_z;        /*!< Isospin_z - here, always set to 0 for compatibility purposes */
    int               row;           /*!< Some row indicator of irrep, etc. */
    Array<int>        mom;           /*!< D-1 momentum of the operator */
    std::vector<Array<int>> phasings; /*!< phasing associated to the quarks */
    bool              creation_op;   /*!< Is this a creation ops?  */
    bool              smearedP;      /*!< Does this operators use distillation/distillution? */

    using schema = std::tuple<									  //
      struct_member<&HadronDispVertex_t::name, "name"_sl>,					  //
      struct_member<&HadronDispVertex_t::smear, "smear"_sl>,					  //
      struct_member<&HadronDispVertex_t::disp_list, "disp_list"_sl>,				  //
      struct_member<&HadronDispVertex_t::type, "type"_sl>,					  //
      struct_member<&HadronDispVertex_t::piece_num, "piece_num"_sl>,				  //
      struct_member<&HadronDispVertex_t::Cconj, "Cconj"_sl>,					  //
      struct_member<&HadronDispVertex_t::twoI_z, "twoI_z"_sl>,					  //
      struct_member<&HadronDispVertex_t::row, "row"_sl>,					  //
      struct_member<&HadronDispVertex_t::mom, "mom"_sl, std::vector<int>>,			  //
      struct_member<&HadronDispVertex_t::phasings, "phasings"_sl, std::vector<std::vector<int>>>, //
      struct_member<&HadronDispVertex_t::creation_op, "creation_op"_sl>,			  //
      struct_member<&HadronDispVertex_t::smearedP, "smearedP"_sl>				  //
      >;

    bool operator==(const HadronDispVertex_t&) const = default;
  };


  //----------------------------------------------------------------------------
  //! Comparison
  bool operator<(const HadronDispVertex_t& a, const HadronDispVertex_t& b);
  
  //----------------------------------------------------------------------------
  //! Comparison
  bool operator>(const HadronDispVertex_t& a, const HadronDispVertex_t& b);
  
  //----------------------------------------------------------------------------
  //! Used for output
  std::ostream& operator<<(std::ostream& os, const HadronDispVertex_t& d);
  
  //----------------------------------------------------------------------------
  //! HadronVertex reader
  void read(XMLReader& xml, const std::string& path, HadronDispVertex_t& param);

  //! HadronVertex writer
  void write(XMLWriter& xml, const std::string& path, const HadronDispVertex_t& param);

  //----------------------------------------------------------------------------
  // Reading
  void read(BinaryReader& bin, HadronDispVertex_t& op);

  // Writing
  void write(BinaryWriter& bin, const HadronDispVertex_t& op);

} // namespace Hadron

#endif


