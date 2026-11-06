/*! \file
 * \brief Baryon elemental operators
 */

#include "datalib/superb/baryon_elem_type.h"

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
    std::ostream& operator<<(std::ostream& os, const std::vector<int>& d)
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
  std::ostream& operator<<(std::ostream& os, const KeyBaryonElementalOperator_t& d)
  {
    os << "KeyBaryonElementalOperator_t:"
       << " t_slice= " << d.t_slice
       << " left= " << d.left
       << " middle= " << d.middle
       << " right= " << d.right
       << " mom= " << d.mom
       << " phasing= " << d.phasing
       << endl;

    return os;
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyBaryonElementalOperator_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "t_slice", param.t_slice);
    read(paramtop, "left", param.left);
    read(paramtop, "middle", param.middle);
    read(paramtop, "right", param.right);
    if (paramtop.count("phasing") > 0)
      read(paramtop, "phasing", param.phasing);
  }

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyBaryonElementalOperator_t& param)
  {
    push(xml, path);

    write(xml, "t_slice", param.t_slice);
    write(xml, "left", param.left);
    write(xml, "middle", param.middle);
    write(xml, "right", param.right);
    write(xml, "mom", param.mom);
    write(xml, "phasing", param.phasing);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  //! KeyBaryonElementalOperator reader
  void read(BinaryReader& bin, KeyBaryonElementalOperator_t& param)
  {
    read(bin, param.t_slice);
    read(bin, param.left);
    read(bin, param.middle);
    read(bin, param.right);
    read(bin, param.mom);
    read(bin, param.phasing);
  }

  //! BaryonElementalOperator write
  void write(BinaryWriter& bin, const KeyBaryonElementalOperator_t& param)
  {
    write(bin, param.t_slice);
    write(bin, param.left);
    write(bin, param.middle);
    write(bin, param.right);
    write(bin, param.mom);
    write(bin, param.phasing);
  }


  //----------------------------------------------------------------------------
  //! BaryonElementalOperator reader
  void read(BinaryReader& bin, ValBaryonElementalOperator_t& param)
  {
    read(bin, param.type_of_data);

    int n;
    read(bin, n);    // the size is always written, even if 0
    param.op.resize(n,n,n);

    for(int i=0; i < n; ++i)
    {
      for(int j=0; j < n; ++j)
      {
	for(int k=0; k < n; ++k)
	{
	  read(bin, param.op(i,j,k));
	}
      }
    }
  }

  //! BaryonElementalOperator write
  void write(BinaryWriter& bin, const ValBaryonElementalOperator_t& param)
  {
    write(bin, param.type_of_data);

    int n = param.op.size1();  // all sizes the same
    write(bin, n);
    for(int i=0; i < n; ++i)
    {
      for(int j=0; j < n; ++j)
      {
	for(int k=0; k < n; ++k)
	{
	  write(bin, param.op(i,j,k));
	}
      }
    }
  }

} // namespace ColorVec
