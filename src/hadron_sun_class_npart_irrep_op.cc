/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep
 */

#include "datalib/graph/hadron_sun_class_npart_irrep_op.h"


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

  }


  //----------------------------------------------------------------------------
  //! Used for output
  std::ostream& operator<<(std::ostream& os, const KeyHadronSUNClassNPartIrrepOp_t::CGPair_t::Slot_t& d)
  {
    os << " slot= " << d.slot;
    os << " F" << d.F;
    os << " irrep= " << d.irrep;
    os << " momclass= " << d.mom_class;

    return os;
  }

  //! Used for output
  std::ostream& operator<<(std::ostream& os, const KeyHadronSUNClassNPartIrrepOp_t::CGPair_t& d)
  {
    os << " Left: " << d.left
       << " Right: " << d.right
       << " Target: " << d.target;

    return os;
  }

  //! Used for output
  std::ostream& operator<<(std::ostream& os, const Array1dO<KeyHadronSUNClassNPartIrrepOp_t::CGPair_t>& d)
  {
    for(int j=1; j <= d.size(); ++j)
    {
      os << " Pair" << j << d[j];
    }

    return os;
  }

  //! Used for output
  std::ostream& operator<<(std::ostream& os, const KeyHadronSUNClassNPartIrrepOp_t& d)
  {
    os << " Operators:" << d.ops;
    os << " CGs:";
    os << d.CGs;

    return os;
  }


  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyHadronSUNClassNPartIrrepOp_t::CGPair_t::Slot_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "slot", param.slot);
    read(paramtop, "F", param.F);
    read(paramtop, "irrep", param.irrep);
    Array<int> mom_class;
    read(paramtop, "mom_class", mom_class);
    param.mom_class = mom_class;
  }

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyHadronSUNClassNPartIrrepOp_t::CGPair_t::Slot_t& param)
  {
    push(xml, path);

    write(xml, "slot", param.slot);
    write(xml, "F", param.F);
    write(xml, "irrep", param.irrep);
    write(xml, "mom_class", param.mom_class);
    pop(xml);
  }

  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyHadronSUNClassNPartIrrepOp_t::CGPair_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "left", param.left);
    read(paramtop, "right", param.right);
    read(paramtop, "target", param.target);
  }

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyHadronSUNClassNPartIrrepOp_t::CGPair_t& param)
  {
    push(xml, path);

    write(xml, "left", param.left);
    write(xml, "right", param.right);
    write(xml, "target", param.target);

    pop(xml);
  }

  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyHadronSUNClassNPartIrrepOp_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "Operators", param.ops);
    read(paramtop, "CGs", param.CGs);
  }

  // KeyHadronSUNNPartIrrep writer
  void write(XMLWriter& xml, const std::string& path, const KeyHadronSUNClassNPartIrrepOp_t& param)
  {
    push(xml, path);

    write(xml, "Operators", param.ops);
    write(xml, "CGs", param.CGs);

    pop(xml);
  }



  //----------------------------------------------------------------------------
  // Read a key
  void read(BinaryReader& bin, KeyHadronSUNClassNPartIrrepOp_t::CGPair_t::Slot_t& param)
  {
    readDesc(bin, param.slot);
    readDesc(bin, param.irrep);
    readDesc(bin, param.F);
    read(bin, param.mom_class);
  }

  // Write a key
  void write(BinaryWriter& bin, const KeyHadronSUNClassNPartIrrepOp_t::CGPair_t::Slot_t& param)
  {
    writeDesc(bin, param.slot);
    writeDesc(bin, param.irrep);
    writeDesc(bin, param.F);
    write(bin, param.mom_class);
  }

  // Read a key
  void read(BinaryReader& bin, KeyHadronSUNClassNPartIrrepOp_t::CGPair_t& param)
  {
    readDesc(bin, param.left);
    readDesc(bin, param.right);
    read(bin, param.target);
  }

  // Write a key
  void write(BinaryWriter& bin, const KeyHadronSUNClassNPartIrrepOp_t::CGPair_t& param)
  {
    writeDesc(bin, param.left);
    writeDesc(bin, param.right);
    write(bin, param.target);
  }

} // namespace Hadron


