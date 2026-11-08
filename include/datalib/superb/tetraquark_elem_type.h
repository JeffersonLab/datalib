// -*- C++ -*-
/*! \file
 * \brief Tetraquark elemental operators
 */

#ifndef __tetraquark_elem_type_h__
#define __tetraquark_elem_type_h__

#include "datalib/superb/types.h"
#include <datalib/model/xml_array.h>
#include <datalib/model/xml_array4d.h>


#include <iostream>
#include <complex>

namespace Hadron
{
  using namespace XMLArray;

  //----------------------------------------------------------------------------
  //! Key for TetraQuark operator
  struct KeyTetraQuarkElementalOperator_t
  {
    int              t_slice;             /*!< TetraQuark operator time slice */ 
    std::string      color;               /*!< Type of color construction */
    Array<int>       mom;                 /*!< D-1 momentum of this operator */

    bool operator==(const KeyTetraQuarkElementalOperator_t& ) const = default;
  };

  //! Value for TetraQuark operator
  struct ValTetraQuarkElementalOperator_t
  {
    Array4d< std::complex<double> >  op;           /*!< Colorvector source and sink with momentum projection */
  };


  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyTetraQuarkElementalOperator_t& d);
	
  //----------------------------------------------------------------------------
  //! Read a key
  void read(XMLReader& xml, const std::string& path, KeyTetraQuarkElementalOperator_t& param);

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyTetraQuarkElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! KeyTetraQuarkElementalOperator reader
  void read(BinaryReader& bin, KeyTetraQuarkElementalOperator_t& param);

  //! KeyTetraQuarkElementalOperator writer
  void write(BinaryWriter& bin, const KeyTetraQuarkElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! ValTetraQuarkElementalOperator reader
  void read(BinaryReader& bin, ValTetraQuarkElementalOperator_t& param);

  //! ValTetraQuarkElementalOperator reader
  void write(BinaryWriter& bin, const ValTetraQuarkElementalOperator_t& param);

} // namespace ColorVec

#endif
