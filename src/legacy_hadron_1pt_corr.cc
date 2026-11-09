/*! \file
 * \brief Hadron 1pt correlators
 */
#include <datalib/io/serial_xml.h>
#include <datalib/codec/binary.h>

#include <datalib/legacy/hadron_1pt_corr.h>

#include <sstream>

using namespace std;

namespace FF
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

    //! Error output


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
  std::ostream& operator<<(std::ostream& os, const KeyHadron1PtCorr_t& d)
  {
    os << "KeyHadron1PtCorr_t:"
       << " name= " << d.name
       << " smear= " << d.smear
       << " lorentz= " << d.lorentz
       << " spin= " << d.spin
       << " mass= " << d.mass
       << " mom= " << d.mom
       << " creation_op= " << d.creation_op
       << " ensemble= " << d.ensemble
       << " num_vecs= " << d.num_vecs
       << std::endl;

    return os;
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadron1PtCorr_t& param)
  {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "num_vecs", param.num_vecs);
    read(paramtop, "name", param.name);
    read(paramtop, "smear", param.smear);
    read(paramtop, "lorentz", param.lorentz);
    read(paramtop, "spin", param.spin);
    read(paramtop, "mom", param.mom);
    read(paramtop, "creation_op", param.creation_op);
    read(paramtop, "mass", param.mass);
    read(paramtop, "ensemble", param.ensemble);
  }

  //! KeyHadron1PtCorr writer
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadron1PtCorr_t& param)
  {
    push(xml, path);

    write(xml, "num_vecs", param.num_vecs);
    write(xml, "name", param.name);
    write(xml, "smear", param.smear);
    write(xml, "lorentz", param.lorentz);
    write(xml, "spin", param.spin);
    write(xml, "mom", param.mom);
    write(xml, "creation_op", param.creation_op);
    write(xml, "mass", param.mass);
    write(xml, "ensemble", param.ensemble);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  //! KeyHadron1PtCorr reader
  void read(Datalib::Binary::BinaryReader& bin, KeyHadron1PtCorr_t& param)
  {
    read(bin, param.name, 128);
    read(bin, param.smear, 128);
    read(bin, param.lorentz);
    read(bin, param.spin);
    read(bin, param.mom);
    read(bin, param.creation_op);
    read(bin, param.num_vecs);
    read(bin, param.mass, 128);
    read(bin, param.ensemble, 1024);
  }

  //! Hadron1PtCorr write
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadron1PtCorr_t& param)
  {
    write(bin, param.name);
    write(bin, param.smear);
    write(bin, param.lorentz);
    write(bin, param.spin);
    write(bin, param.mom);
    write(bin, param.creation_op);
    write(bin, param.num_vecs);
    write(bin, param.mass);
    write(bin, param.ensemble);
  }

} // namespace ColorVec
