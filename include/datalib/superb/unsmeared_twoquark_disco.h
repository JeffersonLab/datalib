// -*- C++ -*-
/*! \file
 * \brief Twoquark disconnected blocks
 */

#ifndef __redstar_datalib_unsmeared_twoquark_disco_h__
#define __redstar_datalib_unsmeared_twoquark_disco_h__

#include "datalib/superb/types.h"
#include "datalib/codec/binary.h"
#include "datalib/model/arrays.h"
#include <datalib/model/xml_array.h>


#include <string>
#include <vector>
#include <complex>

namespace Hadron
{
  using namespace XMLArray;

  //----------------------------------------------------------------------------
  //! Twoquark disco operator     
  struct KeyTwoQuarkDiscoOperator_t
  {
    int t_slice                ; /*!< Meson operator time slice */
    std::vector<int>   disp    ;  /*!< Displacement dirs of right colorstd::vector */
    Array<int>         mom     ; /*!< D-1 momentum of this operator */
    std::string  mass_label    ; /*!< Mass label */
  };

  bool operator<(const KeyTwoQuarkDiscoOperator_t& a, const KeyTwoQuarkDiscoOperator_t& b);
  std::ostream& operator<<(std::ostream& os, const KeyTwoQuarkDiscoOperator_t& d);
  
  //----------------------------------------------------------------------------
  //! Twoquark disco operator     
  class ValTwoQuarkDiscoOperator_t
  {
  public:
    std::vector< std::complex<double> > op;  
    ValTwoQuarkDiscoOperator_t() {op.resize(Datalib::TensorStorage::spin_count*Datalib::TensorStorage::spin_count);} // Here go the 16 gamma matrices
    ~ValTwoQuarkDiscoOperator_t() {}
  };

  //----------------------------------------------------------------------------
  //! KeyOperator reader    
  void read(BinaryReader& bin, KeyTwoQuarkDiscoOperator_t& d);

  //! KeyOperator writer
  void write(BinaryWriter& bin, const KeyTwoQuarkDiscoOperator_t& d);

  //----------------------------------------------------------------------------
  //!  reader
  void read(XMLReader& xml, const std::string& path, KeyTwoQuarkDiscoOperator_t& param);

  //!  writer
  void write(XMLWriter& xml, const std::string& path, const KeyTwoQuarkDiscoOperator_t& param);

  //----------------------------------------------------------------------------
  //! KeyTwoQuarkDiscoOperator_t reader
  void read(BinaryReader& bin, KeyTwoQuarkDiscoOperator_t& param);

  //! KeyHadron1PtCorr writer
  void write(BinaryWriter& bin, const KeyTwoQuarkDiscoOperator_t& param);
 
  //----------------------------------------------------------------------------
  //! ValOperator reader    
  void read(BinaryReader& bin, ValTwoQuarkDiscoOperator_t& d);

  //! ValOperator writer
  void write(BinaryWriter& bin, const ValTwoQuarkDiscoOperator_t& d);

} // namespace Hadron

#endif
