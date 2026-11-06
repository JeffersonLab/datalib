// -*- C++ -*-
/*! \file
 * \brief Time-slice value for graphs
 */

#ifndef DATALIB_hadron_timeslice_h
#define DATALIB_hadron_timeslice_h


#include "datalib/codec/binary.h"
#include "datalib/io/serial_xml.h"
#include <map>
#include <iostream>
#include <complex>

namespace Hadron
{
  using namespace Datalib::Binary;
  using namespace Datalib::Serial;

  //----------------------------------------------------------------------------
  //! Map to hold time-slices -> complex values
  struct ValTimeSlice_t
  {
    std::map< int, std::complex<double> >  val;
  };

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const ValTimeSlice_t& d);
	
  //----------------------------------------------------------------------------
  //! Reader
  void read(XMLReader& xml, const std::string& path, ValTimeSlice_t& param);

  //! Writer
  void write(XMLWriter& xml, const std::string& path, const ValTimeSlice_t& param);

  //----------------------------------------------------------------------------
  //! Rreader
  void read(BinaryReader& bin, ValTimeSlice_t& param);

  //! Writer
  void write(BinaryWriter& bin, const ValTimeSlice_t& param);


} // namespace Hadron

#endif
