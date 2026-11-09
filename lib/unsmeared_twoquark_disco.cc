/*! \file
 * \brief Twoquark disconnected blocks
 */

#include "datalib/superb/unsmeared_twoquark_disco.h"

#include <iostream>

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

    // Make a string
    std::string makeStr(const KeyTwoQuarkDiscoOperator_t& s1)
    {
      std::stringstream os;

      os << s1.t_slice << s1.mom << s1.disp << s1.mass_label;

      return os.str();
    }
  }



  //------------------------------------------------------------------------------
  // Comparison
  bool operator<(const KeyTwoQuarkDiscoOperator_t& s1, const KeyTwoQuarkDiscoOperator_t& s2)
  {
    return (makeStr(s1) < makeStr(s2));
  }
  
  //------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeyTwoQuarkDiscoOperator_t& d)
  {
    os << "KeyTwoQuarkDiscoOperator_t:"
       << " t_slice= " << d.t_slice
       << " disp= " << d.disp
       << " mom= " << d.mom
       << " mass= " << d.mass_label;

    return os;
  }

  
  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyTwoQuarkDiscoOperator_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "t_slice", param.t_slice);
    read(paramtop, "mom", param.mom);
    read(paramtop, "disp", param.disp);
    read(paramtop, "mass", param.mass_label);
  }

  //! KeyHadron1PtCorr writer
  void write(XMLWriter& xml, const std::string& path, const KeyTwoQuarkDiscoOperator_t& param)
  {
    push(xml, path);
    
    write(xml, "mom", param.mom);
    write(xml, "t_slice", param.t_slice);
    write(xml, "disp", param.disp);
    write(xml, "mass", param.mass_label);

    pop(xml);
  }

  
  void write(XMLWriter& xml, const std::string& path, const ValTwoQuarkDiscoOperator_t& v)
  {
    push(xml, path);
    
    write(xml, "op", v.op);
    
    pop(xml); 
  }

  //! KeyOperator reader    
  void read(BinaryReader& bin, KeyTwoQuarkDiscoOperator_t& d){
    read(bin,d.t_slice);
    int n ;
    read(bin,n);
    d.disp.resize(n); 
    read(bin,d.disp);
    d.mom.resize(Datalib::TensorStorage::dimension_count-1) ;
    read(bin,d.mom);
    read(bin, d.mass_label, 32);
  }
  
  //! KeyOperator writer
  void write(BinaryWriter& bin, const KeyTwoQuarkDiscoOperator_t& d)
  {
    write(bin,d.t_slice);
    int n ;
    n = d.disp.size();
    write(bin,n);
    write(bin,d.disp);
    write(bin,d.mom);
    write(bin,d.mass_label);
  }
  
  //! ValOperator reader    
  void read(BinaryReader& bin, ValTwoQuarkDiscoOperator_t& d)
  {
    d.op.resize(Datalib::TensorStorage::spin_count*Datalib::TensorStorage::spin_count);
    read(bin,d.op);
  }
  
  //! ValOperator writer
  void write(BinaryWriter& bin, const ValTwoQuarkDiscoOperator_t& d)
  {
    write(bin,d.op);
  }

} // namespace FF


