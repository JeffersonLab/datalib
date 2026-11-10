/*! \file
 * \brief Hadron N-point correlators
 */
#include <datalib/codec/binary.h>
#include <datalib/io/serial_xml.h>

#include <datalib/legacy/hadron_sun_npart_npt_corr.h>

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
                           const KeyHadronSUNNPartNPtCorr_t::NPoint_t &d) {
    os << "t_slice= " << d.t_slice << "\n";
    os << "    Irrep:" << d.irrep;

    return os;
  }

  //! Used for output
  std::ostream &operator<<(std::ostream &os,
                           const KeyHadronSUNNPartNPtCorr_t &d) {
    for (int i = 1; i <= d.npoint.size(); ++i) {
      os << " Npt[" << i << "]:  " << d.npoint[i];
    }

    return os;
  }

  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Serial::XMLReader &xml, const std::string &path,
            KeyHadronSUNNPartNPtCorr_t::NPoint_t &param) {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "t_slice", param.t_slice);
    read(paramtop, "Irrep", param.irrep);
  }

  // Read a key
  void read(Datalib::Serial::XMLReader &xml, const std::string &path,
            KeyHadronSUNNPartNPtCorr_t &param) {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "NPoint", param.npoint);
  }

  // Write a key
  void write(Datalib::Serial::XMLWriter &xml, const std::string &path,
             const KeyHadronSUNNPartNPtCorr_t::NPoint_t &param) {
    push(xml, path);

    write(xml, "t_slice", param.t_slice);
    write(xml, "Irrep", param.irrep);

    pop(xml);
  }

  // Write a key
  void write(Datalib::Serial::XMLWriter &xml, const std::string &path,
             const KeyHadronSUNNPartNPtCorr_t &param) {
    push(xml, path);

    write(xml, "NPoint", param.npoint);

    pop(xml);
  }

  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartNPtCorr_t::NPoint_t &param) {
    read(bin, param.t_slice);
    read(bin, param.irrep);
  }

  // Read a key
  void read(Datalib::Binary::BinaryReader &bin,
            KeyHadronSUNNPartNPtCorr_t &param) {
    read(bin, param.npoint);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartNPtCorr_t::NPoint_t &param) {
    write(bin, param.t_slice);
    write(bin, param.irrep);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter &bin,
             const KeyHadronSUNNPartNPtCorr_t &param) {
    write(bin, param.npoint);
  }

} // namespace Hadron
