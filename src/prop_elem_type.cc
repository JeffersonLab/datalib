/*! \file
 * \brief Propagator elemental types
 */

#include "datalib/superb/prop_elem_type.h"

#include <ostream>

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
  std::ostream& operator<<(std::ostream& os, const KeyPropElementalOperator_t& d)
  {
    os << "KeyPropElementalOperator_t:"
       << " t_slice= " << d.t_slice
       << " t_source= " << d.t_source
       << " spin_l= " << d.spin_l
       << " spin_r= " << d.spin_r
       << " mass_label= " << d.mass_label
       << endl;

    return os;
  }

 
  //----------------------------------------------------------------------------
  //! KeyPropElementalOperator reader
  void read(BinaryReader& bin, KeyPropElementalOperator_t& param)
  {
    read(bin, param.t_slice);
    read(bin, param.t_source);
    read(bin, param.spin_r);
    read(bin, param.spin_l);
    read(bin, param.mass_label, 32);
    read(bin, param.phasing_sink);
    read(bin, param.phasing_source);
  }

  //! PropElementalOperator write
  void write(BinaryWriter& bin, const KeyPropElementalOperator_t& param)
  {
    write(bin, param.t_slice);
    write(bin, param.t_source);
    write(bin, param.spin_r);
    write(bin, param.spin_l);
    write(bin, param.mass_label);
    write(bin, param.phasing_sink);
    write(bin, param.phasing_source);
  }

 
  //----------------------------------------------------------------------------
  //! PropElementalOperator reader
  void read(BinaryReader& bin, ValPropElementalOperator_t& param)
  {
    int n1;
    int n2;
    read(bin, n2);    // the size is always written, even if 0
    read(bin, n1);    // the size is always written, even if 0
    param.mat.resize(n2,n1);
  
    for(int i=0; i < param.mat.size1(); ++i)
    {
      for(int j=0; j < param.mat.size2(); ++j)
      {
	read(bin, param.mat[j][i]);
      }
    }
  }

  //! PropElementalOperator write
  void write(BinaryWriter& bin, const ValPropElementalOperator_t& param)
  {
    write(bin, param.mat.size2());    // always write the size
    write(bin, param.mat.size1());    // always write the size

    for(int i=0; i < param.mat.size1(); ++i)
    {
      for(int j=0; j < param.mat.size2(); ++j)
      {
	write(bin, param.mat[j][i]);
      }
    }
  }

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyProp4ElementalOperator_t& d)
  {
    os << "KeyProp4ElementalOperator_t:"
       << " t_slice= " << d.t_slice
       << " t_source= " << d.t_source
       << " mass_label= " << d.mass_label
       << " phasing_source= " << d.phasing_source
       << " phasing_sink= " << d.phasing_sink
       << endl;

    return os;
  }

  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyProp4ElementalOperator_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "t_slice", param.t_slice);
    read(paramtop, "t_source", param.t_source);
    read(paramtop, "mass_label", param.mass_label);
    if (paramtop.count("phasing_sink") > 0)
      read(paramtop, "phasing_sink", param.phasing_sink);
    if (paramtop.count("phasing_source") > 0)
      read(paramtop, "phasing_source", param.phasing_source);
  }

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyProp4ElementalOperator_t& param)
  {
    push(xml, path);

    write(xml, "t_slice", param.t_slice);
    write(xml, "t_source", param.t_source);
    write(xml, "mass_label", param.mass_label);
    write(xml, "phasing_sink", param.phasing_sink);
    write(xml, "phasing_source", param.phasing_source);

    pop(xml);
  }

} // namespace ColorVec

