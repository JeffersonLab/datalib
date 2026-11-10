/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep and specific
 * row including creation op info
 */
#include <datalib/codec/binary.h>
#include <datalib/io/serial_xml.h>

#include <datalib/legacy/hadron_sun_npart_irrep.h>

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
  std::ostream &operator<<(std::ostream &os,
                           const KeyHadronSUNNPartIrrep_t &d) {
    os << " creat= " << d.creation_op << " f= " << d.flavor
       << " r= " << d.irrep_mom.row << " mom= " << d.irrep_mom.mom;
    os << " Op:" << d.op;
    os << std::endl;

    return os;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Serial::XMLReader &xml, const std::string &path,
            KeyHadronSUNNPartIrrep_t &param) {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "creation_op", param.creation_op);
    read(paramtop, "smearedP", param.smearedP);
    read(paramtop, "flavor", param.flavor);
    read(paramtop, "irmom", param.irrep_mom);
    read(paramtop, "Op", param.op);
  }

  // KeyHadronSUNNPartIrrep writer
  void write(Datalib::Serial::XMLWriter &xml, const std::string &path,
             const KeyHadronSUNNPartIrrep_t &param) {
    push(xml, path);

    write(xml, "creation_op", param.creation_op);
    write(xml, "smearedP", param.smearedP);
    write(xml, "flavor", param.flavor);
    write(xml, "irmom", param.irrep_mom);
    write(xml, "Op", param.op);

    pop(xml);
  }

  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartIrrep_t &param) {
    read(bin, param.creation_op);
    read(bin, param.smearedP);
    read(bin, param.flavor);
    read(bin, param.irrep_mom);
    read(bin, param.op);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartIrrep_t &param) {
    write(bin, param.creation_op);
    write(bin, param.smearedP);
    write(bin, param.flavor);
    write(bin, param.irrep_mom);
    write(bin, param.op);
  }

} // namespace Hadron
