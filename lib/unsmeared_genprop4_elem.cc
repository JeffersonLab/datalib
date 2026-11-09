/*! \file
 * \brief Unsmeared genprop elementals
 */

#include "datalib/superb/unsmeared_genprop4_elem.h"

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
  std::ostream& operator<<(std::ostream& os, const KeyGenProp4ElementalOperator_t& d)
  {
    os << "GenProp4:"
       << " t_sink= " << d.t_sink
       << " t_slice= " << d.t_slice
       << " t_source= " << d.t_source
       << " g= " << d.g
       << " displacement= " << d.displacement
       << " mom= " << d.mom
       << " phasing_sink= " << d.phasing_sink
       << " phasing_source= " << d.phasing_source
       << " mass= " << d.mass;

    return os;
  }
    

  //----------------------------------------------------------------------------
  //! KeyGenProp4ElementalOperator reader
  void read(BinaryReader& bin, KeyGenProp4ElementalOperator_t& param)
  {
    read(bin, param.t_sink);
    read(bin, param.t_slice);
    read(bin, param.t_source);
    read(bin, param.g);
    read(bin, param.displacement);
    read(bin, param.mom);
    read(bin, param.phasing_sink);
    read(bin, param.phasing_source);
    readDesc(bin, param.mass);
  }

  //! GenProp4ElementalOperator write
  void write(BinaryWriter& bin, const KeyGenProp4ElementalOperator_t& param)
  {
    write(bin, param.t_sink);
    write(bin, param.t_slice);
    write(bin, param.t_source);
    write(bin, param.g);
    write(bin, param.displacement);
    write(bin, param.mom);
    write(bin, param.phasing_sink);
    write(bin, param.phasing_source);
    writeDesc(bin, param.mass);
  }


  //! UnsmearedGenpropElementalOperator reader
  void read(XMLReader& xml, const std::string& path, KeyGenProp4ElementalOperator_t& param)
  {
    XMLReader paramtop(xml, path);
    
    read(paramtop, "t_sink", param.t_sink);
    read(paramtop, "t_slice", param.t_slice);
    read(paramtop, "t_source", param.t_source);
    read(paramtop, "g", param.g);
    read(paramtop, "displacement", param.displacement);
    read(paramtop, "mom", param.mom);
    read(paramtop, "mass", param.mass);
    if (paramtop.count("phasing_sink") > 0)
      read(paramtop, "phasing_sink", param.phasing_sink);
    if (paramtop.count("phasing_source") > 0)
      read(paramtop, "phasing_source", param.phasing_source);
  }

  //! UnsmearedGenpropElementalOperator writer
  void write(XMLWriter& xml, const std::string& path, const KeyGenProp4ElementalOperator_t& param)
  {
    push(xml, path);

    write(xml, "t_sink", param.t_sink);
    write(xml, "t_slice", param.t_slice);
    write(xml, "t_source", param.t_source);
    write(xml, "g", param.g);
    write(xml, "displacement", param.displacement);
    write(xml, "mom", param.mom);
    write(xml, "phasing_sink", param.phasing_sink);
    write(xml, "phasing_source", param.phasing_source);
    write(xml, "mass", param.mass);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  //! PropElementalOperator reader
  void read(BinaryReader& bin, ValGenProp4ElementalOperator_t& param)
  {
    read(bin, param.op);
  }

  //! GenProp4ElementalOperator write
  void write(BinaryWriter& bin, const ValGenProp4ElementalOperator_t& param)
  {
    write(bin, param.op);
  }

} // namespace Hadron


