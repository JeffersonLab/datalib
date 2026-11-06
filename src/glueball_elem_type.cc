/*! \file
 * \brief Glueball elemental operators
 */

#include "datalib/superb/glueball_elem_type.h"

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
  std::ostream& operator<<(std::ostream& os, const KeyGlueElementalOperator_t& d)
  {
    os << "KeyGlueElementalOperator_t:"
       << " t_slice= " << d.t_slice
       << " left= " << d.left
       << " right= " << d.right
       << " displacement= " << d.displacement
       << " mom= " << d.mom
       << std::endl;

    return os;
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyGlueElementalOperator_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "t_slice", param.t_slice);
    read(paramtop, "left", param.left);
    read(paramtop, "right", param.right);
    read(paramtop, "displacement", param.displacement);
    read(paramtop, "mom", param.mom);
  }

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyGlueElementalOperator_t& param)
  {
    push(xml, path);

    write(xml, "t_slice", param.t_slice);
    write(xml, "left", param.left);
    write(xml, "right", param.right);
    write(xml, "displacement", param.displacement);
    write(xml, "mom", param.mom);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  //! KeyGlueElementalOperator reader
  void read(BinaryReader& bin, KeyGlueElementalOperator_t& param)
  {
    read(bin, param.t_slice);
    read(bin, param.left);
    read(bin, param.right);
    read(bin, param.displacement);
    read(bin, param.mom);
  }

  //! GlueElementalOperator write
  void write(BinaryWriter& bin, const KeyGlueElementalOperator_t& param)
  {
    write(bin, param.t_slice);
    write(bin, param.left);
    write(bin, param.right);
    write(bin, param.displacement);
    write(bin, param.mom);
  }


  //----------------------------------------------------------------------------
  //! GlueElementalOperator reader
  void read(BinaryReader& bin, ValGlueElementalOperator_t& param)
  {
    int n;
    read(bin, n);    // the size is always written, even if 0
    param.op.resize(n);

    for(int i=0; i < n; ++i)
    {
      read(bin, param.op(i));
    }
  }

  //! GlueElementalOperator write
  void write(BinaryWriter& bin, const ValGlueElementalOperator_t& param)
  {
    int n = param.op.size();  // all sizes the same
    write(bin, n);
    for(int i=0; i < n; ++i)
    {
      write(bin, param.op(i));
    }
  }

} // namespace ColorVec


