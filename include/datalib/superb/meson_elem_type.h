// -*- C++ -*-
/*! \file
 * \brief Meson elemental operators
 */

#ifndef __redstar_datalib_meson_elem_type_h__
#define __redstar_datalib_meson_elem_type_h__

#include "datalib/superb/types.h"
#include <datalib/model/xml_array.h>
#include <datalib/model/xml_array2d.h>

#include "datalib/codec/superb_options.h"

#include <iostream>
#include <complex>

namespace Hadron
{
  using namespace XMLArray;
  using namespace SB;

  //----------------------------------------------------------------------------
  //! Key for Meson operator
  struct KeyMesonElementalOperator_t
  {
    int              t_slice;             /*!< Meson operator time slice */ 
    std::vector<int> displacement;        /*!< Displacement dirs of right colorvector */
    Array<int>       mom;                 /*!< D-1 momentum of this operator */
    Array<int>       phasing_sink;        /*!< phasing sink */
    Array<int>       phasing_source;      /*!< phasing source */

    using schema = std::tuple<							    //
      struct_member<&KeyMesonElementalOperator_t::t_slice, "t_slice"_sl>,	    //
      struct_member<&KeyMesonElementalOperator_t::displacement, "displacement"_sl>, //
      struct_member<&KeyMesonElementalOperator_t::mom, "mom"_sl, std::vector<int>>, //
      struct_member<&KeyMesonElementalOperator_t::phasing_sink, "phasing_sink"_sl,
		    std::vector<int>>, //
      struct_member<&KeyMesonElementalOperator_t::phasing_source, "phasing_source"_sl,
		    std::vector<int>> //
      >;

    bool operator==(const KeyMesonElementalOperator_t&) const = default;
  };

  //! Value for Meson operator
  struct ValMesonElementalOperator_t
  {
    int                              type_of_data; /*!< Flag indicating type of data (maybe trivial) */
    Array2d< std::complex<double> >  op;           /*!< Colorvector source and sink with momentum projection */
  };


  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyMesonElementalOperator_t& d);
	
  //----------------------------------------------------------------------------
  //! Read a key
  void read(XMLReader& xml, const std::string& path, KeyMesonElementalOperator_t& param);

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyMesonElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! KeyMesonElementalOperator reader
  void read(BinaryReader& bin, KeyMesonElementalOperator_t& param);

  //! KeyMesonElementalOperator writer
  void write(BinaryWriter& bin, const KeyMesonElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! ValMesonElementalOperator reader
  void read(BinaryReader& bin, ValMesonElementalOperator_t& param);

  //! ValMesonElementalOperator reader
  void write(BinaryWriter& bin, const ValMesonElementalOperator_t& param);

} // namespace ColorVec

#endif
