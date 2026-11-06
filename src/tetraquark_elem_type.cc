/*! \file
 * \brief Tetraquark elemental operators
 */

#include "datalib/superb/tetraquark_elem_type.h"

#include <iostream>
#include <sstream>

using namespace std; 

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

    //! Error output
    template<typename T>
    std::ostream& operator<<(std::ostream& os, const std::vector<T>& d)
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
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyTetraQuarkElementalOperator_t& d)
  {
    os << "KeyTetraQuarkElementalOperator_t:"
       << " t_slice= " << d.t_slice
       << " color= " << d.color
       << " mom= " << d.mom
       << std::endl;

    return os;
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyTetraQuarkElementalOperator_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "t_slice", param.t_slice);
    read(paramtop, "color", param.color);
    read(paramtop, "mom", param.mom);
  }

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyTetraQuarkElementalOperator_t& param)
  {
    push(xml, path);

    write(xml, "t_slice", param.t_slice);
    write(xml, "color", param.color);
    write(xml, "mom", param.mom);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  //! KeyTetraQuarkElementalOperator reader
  void read(BinaryReader& bin, KeyTetraQuarkElementalOperator_t& param)
  {
    readDesc(bin, param.color);
    read(bin, param.t_slice);
    read(bin, param.mom);
  }

  //! TetraQuarkElementalOperator write
  void write(BinaryWriter& bin, const KeyTetraQuarkElementalOperator_t& param)
  {
    writeDesc(bin, param.color);
    write(bin, param.t_slice);
    write(bin, param.mom);
  }


  //----------------------------------------------------------------------------
  //! TetraQuarkElementalOperator reader
  void read(BinaryReader& bin, ValTetraQuarkElementalOperator_t& param)
  {
    read(bin, param.op);
  }

  //! TetraQuarkElementalOperator write
  void write(BinaryWriter& bin, const ValTetraQuarkElementalOperator_t& param)
  {
    write(bin, param.op);
  }

} // namespace ColorVec


