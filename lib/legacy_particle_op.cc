/*! \file
 * \brief Hadron-like particle operators
 */
#include <datalib/io/serial_xml.h>
#include <datalib/codec/binary.h>
#include <datalib/model/arrays.h>

#include <datalib/legacy/particle_op.h>
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
  // Key for a hadron-like particle operator
  KeyParticleOp_t::KeyParticleOp_t(const HadronDispVertex_t& vert) :
    name(vert.name), smear(vert.smear), mom_type(canonicalOrder(vert.mom))
  {
    if (vert.disp_list.size() > 0)
    {
      std::cerr << __func__ << ": error - cannot convert a ParticleDispOp to a ParticleOp with a nonzero disp_list" << std::endl;
      std::cerr << __func__ << "HadronDispVertex = " << vert << std::endl;
      exit(1);
    }
  }


  //----------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeyParticleOp_t& param)
  {
    os << " name= " << param.name;
    if (param.smear.size() > 0)
      os << " smear= " << param.smear;
    os << " momtype= " << param.mom_type;

    return os;
  }

  //----------------------------------------------------------------------------
  //! Used for output
  std::ostream& operator<<(std::ostream& os, const Datalib::Model::Array1dO<KeyParticleOp_t>& param)
  {
    for(int i=1; i <= param.size(); ++i)
    {
      os << " Op" << i << ":" << param[i];
    }

    return os;
  }

  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyParticleOp_t& param)
  {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "name", param.name);
    if (paramtop.count("smear") > 0)
      read(paramtop, "smear", param.smear);
    XMLArray::Array<int> mom_type;
    read(paramtop, "mom_type", mom_type);
    param.mom_type = mom_type;
  }

  // Write a key
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyParticleOp_t& param)
  {
    push(xml, path);

    write(xml, "name", param.name);
    if (param.smear.size() > 0)
      write(xml, "smear", param.smear);
    write(xml, "mom_type", param.mom_type);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Binary::BinaryReader& bin, KeyParticleOp_t& param)
  {
    Datalib::Binary::readDesc(bin, param.name);
    Datalib::Binary::readDesc(bin, param.smear);
    read(bin, param.mom_type);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter& bin, const KeyParticleOp_t& param)
  {
    Datalib::Binary::writeDesc(bin, param.name);
    Datalib::Binary::writeDesc(bin, param.smear);
    write(bin, param.mom_type);
  }

} // namespace Hadron
