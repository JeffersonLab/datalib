// -*- C++ -*-
/*! \file
 * \brief CG tables for SU(3)
 */

#ifndef DATALIB_cgc_su3_h
#define DATALIB_cgc_su3_h


#include <string>
#include <iostream>
#include "datalib/codec/binary.h"
#include "datalib/io/serial_xml.h"
#include "datalib/codec/superb_options.h"


namespace Hadron
{
  using namespace Datalib::Model;
  using namespace SB;
  using namespace Datalib::Serial;
  using namespace Datalib::Binary;

  //---------------------------------
  //! Structure for SU(3)
  /*!< The conventions follow from arXiv:nucl-th/9502037 */
  struct KeyCGCSU3_t
  {
    KeyCGCSU3_t() : twoI(0), threeY(0), twoI_z(0) {}
    KeyCGCSU3_t(int I,int Y, int Iz) : twoI(I), threeY(Y), twoI_z(Iz) {}
    bool operator==(const KeyCGCSU3_t&) const = default;
    int  twoI;
    int  threeY;
    int  twoI_z;

    using schema = std::tuple<				//
      struct_member<&KeyCGCSU3_t::twoI, "twoI"_sl>,	//
      struct_member<&KeyCGCSU3_t::threeY, "threeY"_sl>, //
      struct_member<&KeyCGCSU3_t::twoI_z, "twoI_z"_sl>	//
      >;
  };


  //---------------------------------
  //! A useful conversion
  std::string makeStr(const KeyCGCSU3_t& op);
  
  //---------------------------------
  std::ostream& operator<<(std::ostream& os, const KeyCGCSU3_t& op);

  //---------------------------------
  //! CGCKeyType reader
  void read(XMLReader& bin, const std::string& path, KeyCGCSU3_t& param);

  //! CGCKeyType writer
  void write(XMLWriter& bin, const std::string& path, const KeyCGCSU3_t& param);
  
  std::string write(const KeyCGCSU3_t& param);

  //---------------------------------
  //! CGCKeyType reader
  void read(BinaryReader& bin, KeyCGCSU3_t& param);

  //! CGCKeyType writer
  void write(BinaryWriter& bin, const KeyCGCSU3_t& param);
  
  std::string write(const KeyCGCSU3_t& param);

  //---------------------------------
  //! CLEBSCH-GORDAN for permutation group
  /*!< The conventions follow from arXiv:nucl-th/9502037 */
	

}  // end namespace Hadron

#endif
