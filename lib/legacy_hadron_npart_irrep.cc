/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep and specific row including creation op info
 */
#include <datalib/io/serial_xml.h>
#include <datalib/codec/binary.h>

#include <datalib/legacy/hadron_npart_irrep.h>

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
  std::ostream& operator<<(std::ostream& os, const KeyHadronNPartIrrep_t& d)
  {
    os << " creat= " << d.creation_op
       << " mom= " << d.mom
       << " phasing= " << d.phasing
       << " row= " << d.row
       << " 2I_z= " << d.twoI_z;
    os << " Op:" << d.op;
    os << std::endl;

    return os;
  }


  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Serial::XMLReader& xml, const std::string& path, KeyHadronNPartIrrep_t& param)
  {
    Datalib::Serial::XMLReader paramtop(xml, path);

    read(paramtop, "CGs", param.op.CGs);
    read(paramtop, "Operators", param.op.ops);
    read(paramtop, "creation_op", param.creation_op);
    read(paramtop, "smearedP", param.smearedP);
    read(paramtop, "mom", param.mom);
    if (paramtop.count("phasing") > 0)
      read(paramtop, "phasing", param.phasing);
    read(paramtop, "row", param.row);
    read(paramtop, "twoI_z", param.twoI_z);
  }

  // KeyHadronNPartIrrep writer
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const KeyHadronNPartIrrep_t& param)
  {
    push(xml, path);

    write(xml, "CGs", param.op.CGs);
    write(xml, "Operators", param.op.ops);
    write(xml, "creation_op", param.creation_op);
    write(xml, "smearedP", param.smearedP);
    write(xml, "mom", param.mom);
    write(xml, "phasing", param.phasing);
    write(xml, "row", param.row);
    write(xml, "twoI_z", param.twoI_z);

    pop(xml);
  }



  //----------------------------------------------------------------------------
  // Read a key
  void read(Datalib::Binary::BinaryReader& bin, KeyHadronNPartIrrep_t& param)
  {
    read(bin, param.op.CGs);
    read(bin, param.op.ops);
    read(bin, param.row);
    read(bin, param.twoI_z);
    read(bin, param.mom);
    read(bin, param.phasing);
    read(bin, param.creation_op);
    read(bin, param.smearedP);
  }

  // Write a key
  void write(Datalib::Binary::BinaryWriter& bin, const KeyHadronNPartIrrep_t& param)
  {
    write(bin, param.op.CGs);
    write(bin, param.op.ops);
    write(bin, param.row);
    write(bin, param.twoI_z);
    write(bin, param.mom);
    write(bin, param.phasing);
    write(bin, param.creation_op);
    write(bin, param.smearedP);
  }

} // namespace Hadron
