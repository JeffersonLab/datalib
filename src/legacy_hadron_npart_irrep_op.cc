/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep
 */
#include <datalib/io/serial_xml.h>
#include <datalib/codec/binary.h>
#include <datalib/model/arrays.h>

#include <datalib/legacy/hadron_npart_irrep_op.h>
#include <datalib/graph/naming.h>

namespace Hadron
{
  namespace
  {
    //! Error output
    std::ostream& operator<<(std::ostream& os, const XMLArray::Array<int>& d)
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
  std::ostream& operator<<(std::ostream& os, const KeyHadronNPartIrrepOp_t::CGPair_t::Slot_t& d)
  {
    os << " slot= " << d.slot << " irrep= " << d.irrep << "," << d.embed
       << " 2I= " << d.twoI << " momtype= " << d.mom_type;

    return os;
  }

  //! Used for output
  std::ostream& operator<<(std::ostream& os, const KeyHadronNPartIrrepOp_t::CGPair_t& d)
  {
    os << " Left: " << d.left
       << " Right: " << d.right
       << " Target: " << d.target;

    return os;
  }

  //! Used for output
  std::ostream& operator<<(std::ostream& os, const Datalib::Model::Array1dO<KeyHadronNPartIrrepOp_t::CGPair_t>& d)
  {
    for(int j=1; j <= d.size(); ++j)
    {
      os << " Pair" << j << d[j];
    }

    return os;
  }

  //! Used for output
  std::ostream& operator<<(std::ostream& os, const KeyHadronNPartIrrepOp_t& d)
  {
    os << " Operators:" << d.ops;
    os << " CGs:";
    os << d.CGs << std::endl;

    return os;
  }


  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadronNPartIrrepOp_t::CGPair_t::Slot_t& param)
  {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "slot", param.slot);
    read(paramtop, "irrep", param.irrep);
    read(paramtop, "twoI", param.twoI);
    read(paramtop, "embed", param.embed);
    XMLArray::Array<int> mom_type;
    read(paramtop, "mom_type", mom_type);

    if (mom_type != canonicalOrder(mom_type))
    {
      std::cerr << __func__ << ": error reading CGPair: mom_type must be canonical\n";
      exit(1);
    }
    param.mom_type = mom_type;
  }

  // Write a key
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadronNPartIrrepOp_t::CGPair_t::Slot_t& param)
  {
    push(xml, path);

    write(xml, "slot", param.slot);
    write(xml, "irrep", param.irrep);
    write(xml, "twoI", param.twoI);
    write(xml, "embed", param.embed);
    write(xml, "mom_type", param.mom_type);
    pop(xml);
  }

  // Read a key
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadronNPartIrrepOp_t::CGPair_t& param)
  {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "left", param.left);
    read(paramtop, "right", param.right);
    read(paramtop, "target", param.target);
  }

  // Write a key
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadronNPartIrrepOp_t::CGPair_t& param)
  {
    push(xml, path);

    write(xml, "left", param.left);
    write(xml, "right", param.right);
    write(xml, "target", param.target);

    pop(xml);
  }

  // Read a key
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadronNPartIrrepOp_t& param)
  {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "CGs", param.CGs);
    read(paramtop, "Operators", param.ops);
  }

  // KeyHadronNPartIrrep writer
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadronNPartIrrepOp_t& param)
  {
    push(xml, path);

    write(xml, "CGs", param.CGs);
    write(xml, "Operators", param.ops);

    pop(xml);
  }



  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartIrrepOp_t::CGPair_t::Slot_t& param)
  {
    Datalib::Binary::readDesc(bin, param.slot);
    Datalib::Binary::readDesc(bin, param.irrep);
    read(bin, param.twoI);
    read(bin, param.embed);
    read(bin, param.mom_type);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartIrrepOp_t::CGPair_t::Slot_t& param)
  {
    Datalib::Binary::writeDesc(bin, param.slot);
    Datalib::Binary::writeDesc(bin, param.irrep);
    write(bin, param.twoI);
    write(bin, param.embed);
    write(bin, param.mom_type);
  }

  // Read a key
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartIrrepOp_t::CGPair_t& param)
  {
    Datalib::Binary::readDesc(bin, param.left);
    Datalib::Binary::readDesc(bin, param.right);
    read(bin, param.target);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartIrrepOp_t::CGPair_t& param)
  {
    Datalib::Binary::writeDesc(bin, param.left);
    Datalib::Binary::writeDesc(bin, param.right);
    write(bin, param.target);
  }

  // Read a key
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartIrrepOp_t& param)
  {
    read(bin, param.CGs);
    read(bin, param.ops);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartIrrepOp_t& param)
  {
    write(bin, param.CGs);
    write(bin, param.ops);
  }

} // namespace Hadron
