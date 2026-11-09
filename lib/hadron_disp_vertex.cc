/*! \file
 * \brief Reduced set of hadron vertex info
 */

#include "datalib/graph/hadron_disp_vertex.h"

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

    std::ostream& operator<<(std::ostream& os, const std::vector<Array<int>>& d)
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
    std::string makeStr(const HadronDispVertex_t& s1)
    {
      std::stringstream os;

      os << s1.name << s1.smear << s1.type << s1.piece_num;
      os << ((s1.Cconj) ? '1' : '0');
      os << s1.twoI_z << s1.row << s1.mom;
      for (const auto& p : s1.phasings)
	os << p;
      os << ((s1.creation_op) ? '1' : '0');
      os << ((s1.smearedP) ? '1' : '0');
      os << s1.disp_list;

      return os.str();
    }
  }



  //------------------------------------------------------------------------------
  // Comparison
  bool operator<(const HadronDispVertex_t& s1, const HadronDispVertex_t& s2)
  {
    return (makeStr(s1) < makeStr(s2));
  }


  //------------------------------------------------------------------------------
  // Comparison
  bool operator>(const HadronDispVertex_t& s1, const HadronDispVertex_t& s2)
  {
    return (makeStr(s1) > makeStr(s2));
  }


  //----------------------------------------------------------------------------
  //! Used for output
  std::ostream& operator<<(std::ostream& os, const HadronDispVertex_t& d)
  {
    os << " name= " << d.name;

    if (! d.smear.empty())
      os << " smear= " << d.smear;

    os << " type= " << d.type
       << " piece_num= " << d.piece_num
       << " Cconj= " << d.Cconj
       << " twoI_z= " << d.twoI_z
       << " row= " << d.row
       << " mom= " << d.mom
       << " phasings= " << d.phasings
       << " creation_op= " << d.creation_op
       << " smearedP= " << d.smearedP;
    
    if (! d.disp_list.empty())
      os << " disp_list= " << d.disp_list;

    return os;
  }
  
  //----------------------------------------------------------------------------
  //! HadronVertex reader
  void read(XMLReader& xml, const std::string& path, HadronDispVertex_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "name", param.name);
    if (paramtop.count("smear") > 0)
      read(paramtop, "smear", param.smear);
    read(paramtop, "type", param.type);
    read(paramtop, "piece_num", param.piece_num);
    read(paramtop, "Cconj", param.Cconj);
    read(paramtop, "twoI_z", param.twoI_z);
    read(paramtop, "row", param.row);
    read(paramtop, "mom", param.mom);
    read(paramtop, "phasings", param.phasings);
    read(paramtop, "creation_op", param.creation_op);
    read(paramtop, "smearedP", param.smearedP);
    if (paramtop.count("disp_list") > 0)
      read(paramtop, "disp_list", param.disp_list);
  }

  //! HadronVertex writer
  void write(XMLWriter& xml, const std::string& path, const HadronDispVertex_t& param)
  {
    push(xml, path);

    write(xml, "name", param.name);
    if (! param.smear.empty())
      write(xml, "smear", param.smear);
    write(xml, "type", param.type);
    write(xml, "piece_num", param.piece_num);
    write(xml, "Cconj", param.Cconj);
    write(xml, "twoI_z", param.twoI_z);
    write(xml, "row", param.row);
    write(xml, "mom", param.mom);
    write(xml, "phasings", param.phasings);
    write(xml, "creation_op", param.creation_op);
    write(xml, "smearedP", param.smearedP);
    if (! param.disp_list.empty())
      write(xml, "disp_list", param.disp_list);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  void read(BinaryReader& bin, HadronDispVertex_t& op)
  {
    readDesc(bin, op.name);
    readDesc(bin, op.smear);
    read(bin, op.type);
    read(bin, op.piece_num);
    read(bin, op.Cconj);
    read(bin, op.twoI_z);
    read(bin, op.row);
    read(bin, op.mom);
    read(bin, op.phasings);
    read(bin, op.creation_op);
    read(bin, op.smearedP);
    read(bin, op.disp_list);
  }

  void write(BinaryWriter& bin, const HadronDispVertex_t& op)
  {
    writeDesc(bin, op.name);
    writeDesc(bin, op.smear);
    write(bin, op.type);
    write(bin, op.piece_num);
    write(bin, op.Cconj);
    write(bin, op.twoI_z);
    write(bin, op.row);
    write(bin, op.mom);
    write(bin, op.phasings);
    write(bin, op.creation_op);
    write(bin, op.smearedP);
    write(bin, op.disp_list);
  }

} // namespace Hadron


