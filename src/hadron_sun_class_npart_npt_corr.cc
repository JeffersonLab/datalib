/*! \file
 * \brief Hadron N-point correlators
 */

#include "datalib/graph/hadron_sun_class_npart_npt_corr.h"

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
  std::ostream& operator<<(std::ostream& os, const KeyHadronSUNClassNPartNPtCorr_t::NPoint_t& d)
  {
    os << "t_slice= " << d.t_slice << "\n";
    os << "    Irrep:"<< d.irrep;

    return os;
  }

  //! Used for output
  std::ostream& operator<<(std::ostream& os, const KeyHadronSUNClassNPartNPtCorr_t& d)
  {
    for(int i=1; i <= d.npoint.size(); ++i)
    {
      os << " Npt[" << i << "]:  " << d.npoint[i];
    }

    return os;
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyHadronSUNClassNPartNPtCorr_t::NPoint_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "t_slice", param.t_slice);
    read(paramtop, "Irrep", param.irrep);
  }

  // Read a key
  void read(XMLReader& xml, const std::string& path, KeyHadronSUNClassNPartNPtCorr_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "NPoint", param.npoint);
  }

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyHadronSUNClassNPartNPtCorr_t::NPoint_t& param)
  {
    push(xml, path);

    write(xml, "t_slice", param.t_slice);
    write(xml, "Irrep", param.irrep);

    pop(xml);
  }

  // Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyHadronSUNClassNPartNPtCorr_t& param)
  {
    push(xml, path);

    write(xml, "NPoint", param.npoint);

    pop(xml);
  }


} // namespace Hadron
