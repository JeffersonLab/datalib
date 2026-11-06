/*! \file
 * \brief Hadron multi-particle operators projected onto an irrep and specific row including creation op info
 */

#include "datalib/graph/hadron_sun_class_npart_irrep.h"

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
  std::ostream& operator<<(std::ostream& os, const KeyHadronSUNClassNPartIrrep_t& d)
  {
    os << " creat= " << d.creation_op
       << " f= " << d.flavor
       << " r= " << d.irrep_mom.row
       << " mom= " << d.irrep_mom.mom;
    os << " Op:" << d.op;
    os << std::endl;

    return os;
  }


  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyHadronSUNClassNPartIrrep_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "creation_op", param.creation_op);
    read(paramtop, "smearedP", param.smearedP);
    read(paramtop, "flavor", param.flavor);
    read(paramtop, "irmom", param.irrep_mom);
    read(paramtop, "Op", param.op);
  }

  // KeyHadronSUNNPartIrrep writer
  void write(XMLWriter& xml, const std::string& path, const KeyHadronSUNClassNPartIrrep_t& param)
  {
    push(xml, path);

    write(xml, "creation_op", param.creation_op);
    write(xml, "smearedP", param.smearedP);
    write(xml, "flavor", param.flavor);
    write(xml, "irmom", param.irrep_mom);
    write(xml, "Op", param.op);

    pop(xml);
  }

} // namespace Hadron


