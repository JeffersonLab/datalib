// -*- C++ -*-
/*! \file
 * \brief Unsmeared genprop elementals
 */

#ifndef __redstar_datalib_unsmeared_genprop4_elem_h__
#define __redstar_datalib_unsmeared_genprop4_elem_h__

#include "datalib/superb/types.h"
#include "datalib/codec/binary.h"
#include "datalib/model/arrays.h"
#include <datalib/model/xml_array4d.h>

#include "datalib/codec/superb_options.h"

#include <string>
#include <vector>
#include <iostream>
#include <complex>

namespace Hadron
{
  using namespace XMLArray;
  using namespace SB;

  //----------------------------------------------------------------------------
  //! Generalized propagator operator
  struct KeyGenProp4ElementalOperator_t
  {
    int                t_sink;        /*!< Source time slice */
    int                t_slice;       /*!< Propagator time slice */
    int                t_source;      /*!< Source time slice */
    int                g;             /*!< DR spin matrices */
    std::vector<int>   displacement;  /*!< Displacement dirs of right colorstd::vector */
    Array<int>         mom;           /*!< D-1 momentum of this operator */
    Array<int>         phasing_sink;  /*!< phasing sink */
    Array<int>         phasing_source;/*!< phasing source */
    std::string        mass;          /*!< A mass label */

    using schema = std::tuple<							       //
      struct_member<&KeyGenProp4ElementalOperator_t::t_sink, "t_sink"_sl>,	       //
      struct_member<&KeyGenProp4ElementalOperator_t::t_slice, "t_slice"_sl>,	       //
      struct_member<&KeyGenProp4ElementalOperator_t::t_source, "t_source"_sl>,	       //
      struct_member<&KeyGenProp4ElementalOperator_t::g, "g"_sl>,		       //
      struct_member<&KeyGenProp4ElementalOperator_t::displacement, "displacement"_sl>, //
      struct_member<&KeyGenProp4ElementalOperator_t::mom, "mom"_sl, std::vector<int>>, //
      struct_member<&KeyGenProp4ElementalOperator_t::phasing_sink, "phasing_sink"_sl,
		    std::vector<int>>, //
      struct_member<&KeyGenProp4ElementalOperator_t::phasing_source, "phasing_source"_sl,
		    std::vector<int>>,				      //
      struct_member<&KeyGenProp4ElementalOperator_t::mass, "mass"_sl> //
      >;

    bool operator==(const KeyGenProp4ElementalOperator_t&) const = default;
  };
  
  //! Generalized propagator operator
  struct ValGenProp4ElementalOperator_t
  {
    Array4d< std::complex<double> >  op;              /*!< Colorvector source and sink and spin indices */
  };

  //----------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeyGenProp4ElementalOperator_t& d);

  //----------------------------------------------------------------------------
  //! KeyGenPropElementalOperator reader
  void read(BinaryReader& bin, KeyGenProp4ElementalOperator_t& param);

  //! GenProp4ElementalOperator write
  void write(BinaryWriter& bin, const KeyGenProp4ElementalOperator_t& param);

  //! UnsmearedGenpropElementalOperator reader
  void read(XMLReader& xml, const std::string& path, KeyGenProp4ElementalOperator_t& param);

  //! UnsmearedGenpropElementalOperator writer
  void write(XMLWriter& xml, const std::string& path, const KeyGenProp4ElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! PropElementalOperator reader
  void read(BinaryReader& bin, ValGenProp4ElementalOperator_t& param);

  //! GenProp4ElementalOperator write
  void write(BinaryWriter& bin, const ValGenProp4ElementalOperator_t& param);

} // namespace HadronSUN

#endif
