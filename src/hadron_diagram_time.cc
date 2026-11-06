/*! \file
 *  \brief Diagram time-slice info
 */

#include "datalib/graph/hadron_diagram_time.h"
#include <map>

namespace Hadron
{
  using namespace Datalib::Model;

  // Utility functions
  namespace
  {
    //! Error output
    std::ostream& operator<<(std::ostream& os, const Array1dO<int>& d)
    {
      if (d.size() > 0)
      {
	os << d[1];

	for(int i=2; i <= d.size(); ++i)
	  os << " " << d[i];
      }

      return os;
    }
  } // anonymous namespace


  //----------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const HadronDiagramTimeSlices_t& p)
  {
    os << "HadronDiagramTimeSlices: Nt_corr= " << p.Nt_corr 
       << " t_start= " << p.t_start << " t_end= " << p.t_end
       << " neg_slice_npt= " << p.neg_slice_npt << " source_slice_npt= " << p.source_slice_npt
       << " t_slices= " << p.t_slices
       << std::endl;

    return os;
  }


  //----------------------------------------------------------------------------
  void read(XMLReader& xml_in, const std::string& path, HadronDiagramTimeSlices_t& param)
  {
    XMLReader paramtop(xml_in, path);

    read(paramtop, "Nt_corr", param.Nt_corr);
    read(paramtop, "t_start", param.t_start);
    read(paramtop, "t_end", param.t_end);
    read(paramtop, "neg_slice_npt", param.neg_slice_npt);
    read(paramtop, "source_slice_npt", param.source_slice_npt);
    read(paramtop, "t_slices", param.t_slices);
  }

  void write(XMLWriter& xml, const std::string& path, const HadronDiagramTimeSlices_t& param)
  {
    push(xml, path);

    write(xml, "Nt_corr", param.Nt_corr);
    write(xml, "t_start", param.t_start);
    write(xml, "t_end", param.t_end);
    write(xml, "neg_slice_npt", param.neg_slice_npt);
    write(xml, "source_slice_npt", param.source_slice_npt);
    write(xml, "t_slices", param.t_slices);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  void read(BinaryReader& bin, HadronDiagramTimeSlices_t& param)
  {
    read(bin, param.Nt_corr);
    read(bin, param.t_start);
    read(bin, param.t_end);
    read(bin, param.neg_slice_npt);
    read(bin, param.source_slice_npt);
    read(bin, param.t_slices);
  }

  void write(BinaryWriter& bin, const HadronDiagramTimeSlices_t& param)
  {
    write(bin, param.Nt_corr);
    write(bin, param.t_start);
    write(bin, param.t_end);
    write(bin, param.neg_slice_npt);
    write(bin, param.source_slice_npt);
    write(bin, param.t_slices);
  }

} // end namespace Hadron
