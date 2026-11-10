/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep
 */
#include <datalib/codec/binary.h>
#include <datalib/io/serial_xml.h>
#include <datalib/model/arrays.h>

#include <datalib/graph/naming.h>
#include <datalib/legacy/hadron_sun_npart_irrep_op.h>

namespace Hadron {
  namespace {
    //! Error output
    std::ostream &operator<<(std::ostream &os, const XMLArray::Array<int> &d) {
      if (d.size() > 0) {
        os << d[0];

        for (int i = 1; i < d.size(); ++i)
          os << " " << d[i];
      }

      return os;
    }

  } // namespace

  //----------------------------------------------------------------------------
  //! Used for output
  std::ostream &
  operator<<(std::ostream &os,
             const KeyHadronSUNNPartIrrepOp_t::CGPair_t::Slot_t &d) {
    os << " slot= " << d.slot;
    os << " F" << d.F;
    os << " irrep= " << d.irrep;
    os << " momtype= " << d.mom_type;

    return os;
  }

  //! Used for output
  std::ostream &operator<<(std::ostream &os,
                           const KeyHadronSUNNPartIrrepOp_t::CGPair_t &d) {
    os << " Left: " << d.left << " Right: " << d.right
       << " Target: " << d.target;

    return os;
  }

  //! Used for output
  std::ostream &operator<<(
      std::ostream &os,
      const Datalib::Model::Array1dO<KeyHadronSUNNPartIrrepOp_t::CGPair_t> &d) {
    for (int j = 1; j <= d.size(); ++j) {
      os << " Pair" << j << d[j];
    }

    return os;
  }

  //! Used for output
  std::ostream &operator<<(std::ostream &os,
                           const KeyHadronSUNNPartIrrepOp_t &d) {
    os << " Operators:" << d.ops;
    os << " CGs:";
    os << d.CGs;

    return os;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Serial::XMLReader &xml, const std::string &path,
            KeyHadronSUNNPartIrrepOp_t::CGPair_t::Slot_t &param) {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "slot", param.slot);
    read(paramtop, "F", param.F);
    read(paramtop, "irrep", param.irrep);
    XMLArray::Array<int> mom_type;
    read(paramtop, "mom_type", mom_type);
    param.mom_type = mom_type;
  }

  // Write a key
  void write(Datalib::Serial::XMLWriter &xml, const std::string &path,
             const KeyHadronSUNNPartIrrepOp_t::CGPair_t::Slot_t &param) {
    push(xml, path);

    write(xml, "slot", param.slot);
    write(xml, "F", param.F);
    write(xml, "irrep", param.irrep);
    write(xml, "mom_type", param.mom_type);
    pop(xml);
  }

  // Read a key
  void read(Datalib::Serial::XMLReader &xml, const std::string &path,
            KeyHadronSUNNPartIrrepOp_t::CGPair_t &param) {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "left", param.left);
    read(paramtop, "right", param.right);
    read(paramtop, "target", param.target);
  }

  // Write a key
  void write(Datalib::Serial::XMLWriter &xml, const std::string &path,
             const KeyHadronSUNNPartIrrepOp_t::CGPair_t &param) {
    push(xml, path);

    write(xml, "left", param.left);
    write(xml, "right", param.right);
    write(xml, "target", param.target);

    pop(xml);
  }

  // Read a key
  void read(Datalib::Serial::XMLReader &xml, const std::string &path,
            KeyHadronSUNNPartIrrepOp_t &param) {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "Operators", param.ops);
    read(paramtop, "CGs", param.CGs);
  }

  // KeyHadronSUNNPartIrrep writer
  void write(Datalib::Serial::XMLWriter &xml, const std::string &path,
             const KeyHadronSUNNPartIrrepOp_t &param) {
    push(xml, path);

    write(xml, "Operators", param.ops);
    write(xml, "CGs", param.CGs);

    pop(xml);
  }

  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartIrrepOp_t::CGPair_t::Slot_t &param) {
    Datalib::Binary::readDesc(bin, param.slot);
    Datalib::Binary::readDesc(bin, param.irrep);
    Datalib::Binary::readDesc(bin, param.F);
    read(bin, param.mom_type);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartIrrepOp_t::CGPair_t::Slot_t &param) {
    Datalib::Binary::writeDesc(bin, param.slot);
    Datalib::Binary::writeDesc(bin, param.irrep);
    Datalib::Binary::writeDesc(bin, param.F);
    write(bin, param.mom_type);
  }

  // Read a key
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartIrrepOp_t::CGPair_t &param) {
    Datalib::Binary::readDesc(bin, param.left);
    Datalib::Binary::readDesc(bin, param.right);
    read(bin, param.target);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartIrrepOp_t::CGPair_t &param) {
    Datalib::Binary::writeDesc(bin, param.left);
    Datalib::Binary::writeDesc(bin, param.right);
    write(bin, param.target);
  }

  // Read a key
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartIrrepOp_t &param) {
    read(bin, param.ops);
    read(bin, param.CGs);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartIrrepOp_t &param) {
    write(bin, param.ops);
    write(bin, param.CGs);
  }

} // namespace Hadron
