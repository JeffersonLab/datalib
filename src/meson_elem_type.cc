/*! \file
 * \brief Meson elemental operators
 */

#include "datalib/superb/meson_elem_type.h"

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
  std::ostream& operator<<(std::ostream& os, const KeyMesonElementalOperator_t& d)
  {
    os << "KeyMesonElementalOperator_t:"
       << " t_slice= " << d.t_slice
       << " displacement= " << d.displacement
       << " mom= " << d.mom
       << " phasing_sink= " << d.phasing_sink
       << " phasing_source= " << d.phasing_source
       << std::endl;

    return os;
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyMesonElementalOperator_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "t_slice", param.t_slice);
    read(paramtop, "displacement", param.displacement);
    read(paramtop, "mom", param.mom);
    if (paramtop.count("phasing_sink") > 0)
      read(paramtop, "phasing_sink", param.phasing_sink);
    if (paramtop.count("phasing_source") > 0)
      read(paramtop, "phasing_source", param.phasing_source);
  }

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyMesonElementalOperator_t& param)
  {
    push(xml, path);

    write(xml, "t_slice", param.t_slice);
    write(xml, "displacement", param.displacement);
    write(xml, "mom", param.mom);
    write(xml, "phasing_sink", param.phasing_sink);
    write(xml, "phasing_source", param.phasing_source);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  //! KeyMesonElementalOperator reader
  void read(BinaryReader& bin, KeyMesonElementalOperator_t& param)
  {
    read(bin, param.t_slice);
    read(bin, param.displacement);
    read(bin, param.mom);
    read(bin, param.phasing_sink);
    read(bin, param.phasing_source);
  }

  //! MesonElementalOperator write
  void write(BinaryWriter& bin, const KeyMesonElementalOperator_t& param)
  {
    write(bin, param.t_slice);
    write(bin, param.displacement);
    write(bin, param.mom);
    write(bin, param.phasing_sink);
    write(bin, param.phasing_source);
  }


  //----------------------------------------------------------------------------
  //! MesonElementalOperator reader
  void read(BinaryReader& bin, ValMesonElementalOperator_t& param)
  {
    read(bin, param.type_of_data);

    int n;
    read(bin, n);    // the size is always written, even if 0
    param.op.resize(n,n);
    for(int i=0; i < n; ++i)
    {
      for(int j=0; j < n; ++j)
      {
	read(bin, param.op(i,j));
      }
    }
  }

  //! MesonElementalOperator write
  void write(BinaryWriter& bin, const ValMesonElementalOperator_t& param)
  {
    write(bin, param.type_of_data);

    int n = param.op.size1();  // all sizes the same
    write(bin, n);
    for(int i=0; i < n; ++i)
    {
      for(int j=0; j < n; ++j)
      {
	write(bin, param.op(i,j));
      }
    }
  }

} // namespace ColorVec


