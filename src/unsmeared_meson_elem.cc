/*! \file
 * \brief Unsmeared meson elementals
 */

#include "datalib/superb/unsmeared_meson_elem.h"

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
  std::ostream& operator<<(std::ostream& os, const KeyUnsmearedMesonElementalOperator_t& d)
  {
    os << "KeyUnsmearedMesonElementalOperator_t:"
       << " t_sink= " << d.t_sink
       << " t_slice= " << d.t_slice
       << " t_source= " << d.t_source
       << " colorvec_src= " << d.colorvec_src
       << " gamma= " << d.gamma
       << " derivP= " << d.derivP
       << " mass= " << d.mass
       << " displacement= " << d.displacement
       << " mom= " << d.mom
       << std::endl;

    return os;
  }


  //----------------------------------------------------------------------------
  //! KeyUnsmearedMesonElementalOperator reader
  void read(BinaryReader& bin, KeyUnsmearedMesonElementalOperator_t& param)
  {
    read(bin, param.t_sink);
    read(bin, param.t_slice);
    read(bin, param.t_source);
    read(bin, param.colorvec_src);
    read(bin, param.gamma);
    read(bin, param.derivP);
    read(bin, param.displacement);
    read(bin, param.mom);
    readDesc(bin, param.mass);
  }

  //! UnsmearedMesonElementalOperator write
  void write(BinaryWriter& bin, const KeyUnsmearedMesonElementalOperator_t& param)
  {
    write(bin, param.t_sink);
    write(bin, param.t_slice);
    write(bin, param.t_source);
    write(bin, param.colorvec_src);
    write(bin, param.gamma);
    write(bin, param.derivP);
    write(bin, param.displacement);
    write(bin, param.mom);
    writeDesc(bin, param.mass);
  }

  //! UnsmearedMesonElementalOperator reader
  void read(XMLReader& xml, const std::string& path, KeyUnsmearedMesonElementalOperator_t& param)
  {
    XMLReader paramtop(xml, path);
    
    read(paramtop, "t_sink", param.t_sink);
    read(paramtop, "t_slice", param.t_slice);
    read(paramtop, "t_source", param.t_source);
    read(paramtop, "colorvec_src", param.colorvec_src);
    read(paramtop, "gamma", param.gamma);
    read(paramtop, "derivP", param.derivP);
    read(paramtop, "mass", param.mass);
    read(paramtop, "displacement", param.displacement);
    read(paramtop, "mom", param.mom);
  }

  //! UnsmearedMesonElementalOperator writer
  void write(XMLWriter& xml, const std::string& path, const KeyUnsmearedMesonElementalOperator_t& param)
  {
    push(xml, path);

    write(xml, "t_sink", param.t_sink);
    write(xml, "t_slice", param.t_slice);
    write(xml, "t_source", param.t_source);
    write(xml, "colorvec_src", param.colorvec_src);
    write(xml, "gamma", param.gamma);
    write(xml, "derivP", param.derivP);
    write(xml, "mass", param.mass);
    write(xml, "displacement", param.displacement);
    write(xml, "mom", param.mom);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  //! UnsmearedMesonElementalOperator reader
  void read(BinaryReader& bin, ValUnsmearedMesonElementalOperator_t& param)
  {
    read(bin, param.op);    // the size is always written, even if 0
  }

  //! UnsmearedMesonElementalOperator write
  void write(BinaryWriter& bin, const ValUnsmearedMesonElementalOperator_t& param)
  {
    write(bin, param.op);
  }

} // namespace Hadron


