// -*- C++ -*-
/*! \file
 * \brief Baryon elemental operators
 */

#ifndef __redstar_datalib_baryon_elem_type_h__
#define __redstar_datalib_baryon_elem_type_h__

#include "datalib/superb/types.h"
#include "xml_array.h"
#include "xml_array2d.h"

#include "datalib/codec/superb_options.h"

#include <iostream>
#include <complex>

namespace Hadron
{
  using namespace XMLArray;
  using namespace SB;

  //----------------------------------------------------------------------------
  //! Key for Baryon operator
  struct KeyBaryonElementalOperator_t
  {
    int                 t_slice;             /*!< Baryon operator time slice */
    std::vector<int>    left;                /*!< Displacement dirs of left colorvector */
    std::vector<int>    middle;              /*!< Displacement dirs of middle colorvector */
    std::vector<int>    right;               /*!< Displacement dirs of right colorvector */
    Array<int>          mom;                 /*!< D-1 momentum of this operator */
    Array<int>          phasing;             /*!< phasing of this operator */

    using schema = std::tuple<								    //
      struct_member<&KeyBaryonElementalOperator_t::t_slice, "t_slice"_sl>,		    //
      struct_member<&KeyBaryonElementalOperator_t::left, "left"_sl>,			    //
      struct_member<&KeyBaryonElementalOperator_t::middle, "middle"_sl>,		    //
      struct_member<&KeyBaryonElementalOperator_t::right, "right"_sl>,			    //
      struct_member<&KeyBaryonElementalOperator_t::mom, "mom"_sl, std::vector<int>>,	    //
      struct_member<&KeyBaryonElementalOperator_t::phasing, "phasing"_sl, std::vector<int>> //
      >;

    bool operator==(const KeyBaryonElementalOperator_t&) const = default;
  };

  //! Value for Baryon operator
  struct ValBaryonElementalOperator_t
  {
    int                              type_of_data; /*!< Flag indicating type of data (maybe trivial) */
    Array3d< std::complex<double> >  op;           /*!< Momentum projected operator */
  };


  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyBaryonElementalOperator_t& d);
	
  //----------------------------------------------------------------------------
  //! Read a key
  void read(XMLReader& xml, const std::string& path, KeyBaryonElementalOperator_t& param);

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyBaryonElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! KeyBaryonElementalOperator reader
  void read(BinaryReader& bin, KeyBaryonElementalOperator_t& param);

  //! KeyBaryonElementalOperator writer
  void write(BinaryWriter& bin, const KeyBaryonElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! ValBaryonElementalOperator reader
  void read(BinaryReader& bin, ValBaryonElementalOperator_t& param);

  //! ValBaryonElementalOperator reader
  void write(BinaryWriter& bin, const ValBaryonElementalOperator_t& param);

} // namespace ColorVec

#endif
