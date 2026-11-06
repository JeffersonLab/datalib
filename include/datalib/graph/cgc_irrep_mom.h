// -*- C++ -*-
/*! \file
 * \brief CG tables
 *
 * Convention follows  arXiv:0508018
 */

#ifndef DATALIB_cgc_irrep_mom_h
#define DATALIB_cgc_irrep_mom_h

#include <complex>
#include "datalib/codec/binary.h"
#include "datalib/io/serial_xml.h"


#include "datalib/codec/superb_options.h"

namespace Hadron
{
  using namespace SB;
  using namespace Datalib::Serial;
  using namespace Datalib::Binary;

  //---------------------------------
  //! CLEBSCH-GORDAN coefficients for use with rows of irreps and also momentum
  /*!< The irrep rows are 1 based, and follow conventions from arXiv:0508018 */
  struct KeyCGCIrrepMom_t
  {
    KeyCGCIrrepMom_t() : row(0) {}
    KeyCGCIrrepMom_t(int row_, const Array<int>& mom_) : row(row_), mom(mom_) {}
    bool operator==(const KeyCGCIrrepMom_t&) const = default;

    int         row;
    Array<int>  mom;

    using schema = std::tuple<						//
      struct_member<&KeyCGCIrrepMom_t::row, "twoI"_sl>,			//
      struct_member<&KeyCGCIrrepMom_t::mom, "mom"_sl, std::vector<int>> //
      >;
  };


  //---------------------------------
  std::ostream& operator<<(std::ostream& os, const KeyCGCIrrepMom_t& op);

  //----------------------------------------------------------------------------
  //! Read a key
  void read(XMLReader& xml, const std::string& path, KeyCGCIrrepMom_t& param);

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyCGCIrrepMom_t& param);

  //---------------------------------
  //! CGCKeySlotType reader
  void read(BinaryReader& bin, KeyCGCIrrepMom_t& param);

  //! CGCKeySlotType writer
  void write(BinaryWriter& bin, const KeyCGCIrrepMom_t& param);


  //---------------------------------
  //! CLEBSCH-GORDAN for Irreps+mom

}  // end namespace Hadron

#endif
