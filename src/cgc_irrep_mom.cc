/*! \file
 * \brief CG tables
 */

#include "datalib/graph/cgc_irrep_mom.h"

namespace Hadron
{
  using namespace Datalib::Model;
  using namespace Datalib::Binary;

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
  std::ostream& operator<<(std::ostream& os, const KeyCGCIrrepMom_t& d)
  {
    os << " r= " << d.row << " mom= " << d.mom;

    return os;
  }
  
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //! Read a key
  void read(XMLReader& xml, const std::string& path, KeyCGCIrrepMom_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "row", param.row);
    read(paramtop, "mom", param.mom);
  }

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyCGCIrrepMom_t& param)
  {
    push(xml, path);

    write(xml, "row", param.row);
    write(xml, "mom", param.mom);

    pop(xml);
  }


  //---------------------------------
  //! CGCKeyType reader
  void read(BinaryReader& bin, KeyCGCIrrepMom_t& param)
  {
    read(bin, param.row);
    read(bin, param.mom);
  }

  //! CGCKeyType writer
  void write(BinaryWriter& bin, const KeyCGCIrrepMom_t& param)
  {
    write(bin, param.row);
    write(bin, param.mom);
  }

}  // end namespace Hadron
