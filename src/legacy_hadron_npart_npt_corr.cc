/*! \file
 * \brief Hadron N-point correlators
 */
#include <datalib/io/serial_xml.h>
#include <datalib/codec/binary.h>

#include <datalib/legacy/hadron_npart_npt_corr.h>

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
  std::ostream& operator<<(std::ostream& os, const KeyHadronNPartNPtCorr_t::NPoint_t& d)
  {
    os << "t_slice= " << d.t_slice << "\n";
    os << "    Irrep:"<< d.irrep;

    return os;
  }

  //! Used for output
  std::ostream& operator<<(std::ostream& os, const KeyHadronNPartNPtCorr_t& d)
  {
    for(int i=1; i <= d.npoint.size(); ++i)
    {
      os << " Npt[" << i << "]:  " << d.npoint[i];
    }

    os << " ensemble= " << d.ensemble
       << std::endl;

    return os;
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadronNPartNPtCorr_t::NPoint_t& param)
  {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "t_slice", param.t_slice);
    read(paramtop, "Irrep", param.irrep);
  }

  // Read a key
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadronNPartNPtCorr_t& param)
  {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "NPoint", param.npoint);
    read(paramtop, "ensemble", param.ensemble);
  }

  // Write a key
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadronNPartNPtCorr_t::NPoint_t& param)
  {
    push(xml, path);

    write(xml, "t_slice", param.t_slice);
    write(xml, "Irrep", param.irrep);

    pop(xml);
  }

  // Write a key
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadronNPartNPtCorr_t& param)
  {
    push(xml, path);

    write(xml, "NPoint", param.npoint);
    write(xml, "ensemble", param.ensemble);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartNPtCorr_t::NPoint_t& param)
  {
    read(bin, param.t_slice);
    read(bin, param.irrep);
  }

  // Read a key
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartNPtCorr_t& param)
  {
    read(bin, param.npoint);
    Datalib::Binary::readDesc(bin, param.ensemble);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartNPtCorr_t::NPoint_t& param)
  {
    write(bin, param.t_slice);
    write(bin, param.irrep);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartNPtCorr_t& param)
  {
    write(bin, param.npoint);
    Datalib::Binary::writeDesc(bin, param.ensemble);
  }

} // namespace Hadron
