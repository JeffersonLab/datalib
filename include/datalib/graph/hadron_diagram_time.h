// -*- C++ -*-
/*! \file
 *  \brief Diagram time-slice info
 */

#ifndef DATALIB_hadron_diagram_time_h
#define DATALIB_hadron_diagram_time_h

#include "datalib/io/serial_xml.h"
#include "datalib/codec/binary.h"
#include "datalib/codec/superb_options.h"
#include "datalib/model/arrays.h"

namespace Hadron
{
  using XMLArray::Array;
  using namespace Datalib::Serial;
  using namespace Datalib::Binary;
  using namespace Datalib::Model;
  using namespace SB;

  //----------------------------------------------------------------------------
  //! Active time slices for a correlator
  struct HadronDiagramTimeSlices_t
  {
    HadronDiagramTimeSlices_t() : 
      Nt_corr(0), t_start(-1), t_end(-1), neg_slice_npt(-1), source_slice_npt(-1) {}
    
    int               Nt_corr;             /*!< Length of correlator */
    int               t_start;             /*!< Starting time-slice value. Beware of wrap-around. */
    int               t_end;               /*!< Ending time-slice value. Beware of wrap-around. */
    int               neg_slice_npt;       /*!< The N-pt value with the negative time-slice */
    int               source_slice_npt;    /*!< Reference N-pt value that will be the source */
    Array1dO<int>     t_slices;            /*!< The fixed time-slices for all the graph   [npt -> t_slice] */

    using schema = std::tuple<								   //
      struct_member<&HadronDiagramTimeSlices_t::Nt_corr, "Nt_corr"_sl>,			   //
      struct_member<&HadronDiagramTimeSlices_t::t_start, "t_start"_sl>,			   //
      struct_member<&HadronDiagramTimeSlices_t::t_end, "t_end"_sl>,			   //
      struct_member<&HadronDiagramTimeSlices_t::neg_slice_npt, "neg_slice_npt"_sl>,	   //
      struct_member<&HadronDiagramTimeSlices_t::source_slice_npt, "source_slice_npt"_sl>,  //
      struct_member<&HadronDiagramTimeSlices_t::t_slices, "t_slices"_sl, std::vector<int>> //
      >;

    bool operator==(const HadronDiagramTimeSlices_t&) const = default;
};

  //----------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const HadronDiagramTimeSlices_t& op);

  //----------------------------------------------------------------------------
  // Read
  void read(XMLReader& xml, const std::string& path, HadronDiagramTimeSlices_t& param);

  // Write
  void write(XMLWriter& xml, const std::string& path, const HadronDiagramTimeSlices_t& param);

  //----------------------------------------------------------------------------
  // Reader
  void read(BinaryReader& bin, HadronDiagramTimeSlices_t& param);

  // Writer
  void write(BinaryWriter& bin, const HadronDiagramTimeSlices_t& param);

}  // end namespace Redstar

#endif
