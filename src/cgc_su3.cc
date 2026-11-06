/*! \file
 * \brief CG tables
 */

#include "datalib/graph/cgc_su3.h"

namespace Hadron
{
  using namespace Datalib::Model;
  using namespace Datalib::Binary;

  //---------------------------------
  //! A useful conversion
  std::string makeStr(const KeyCGCSU3_t& op)
  {
    std::ostringstream os;
    os << "I" << op.twoI << "Y" << op.threeY << "i" << op.twoI_z;
    return os.str();
  }

  //---------------------------------
  std::ostream& operator<<(std::ostream& os, const KeyCGCSU3_t& op)
  {
    os<< "I" << op.twoI  << "Y" << op.threeY << "i" << op.twoI_z;
    return os;
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyCGCSU3_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "twoI", param.twoI);
    read(paramtop, "threeY", param.threeY);
    read(paramtop, "twoI_z", param.twoI_z);
  }

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyCGCSU3_t& param)
  {
    push(xml, path);

    write(xml, "twoI", param.twoI);
    write(xml, "threeY", param.threeY);
    write(xml, "twoI_z", param.twoI_z);

    pop(xml);
  }

  //---------------------------------
  //! CGCKeyType reader
  void read(BinaryReader& bin, KeyCGCSU3_t& param)
  {
    read(bin, param.twoI);
    read(bin, param.threeY);
    read(bin, param.twoI_z);
  }

  //! CGCKeyType writer
  void write(BinaryWriter& bin, const KeyCGCSU3_t& param)
  {
    write(bin, param.twoI);
    write(bin, param.threeY);
    write(bin, param.twoI_z);
  }

  std::string write(const KeyCGCSU3_t& param)
  { 
    std::stringstream ss;
    ss << "twoI = " <<  param.twoI << "\n";
    ss << "threeY = " << param.threeY << "\n";
    ss << "twoI_z = " << param.twoI_z << "\n";

    return ss.str();
  }

}  // end namespace Hadron

