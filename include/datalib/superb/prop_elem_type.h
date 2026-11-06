// -*- C++ -*-
/*! \file
 * \brief Propagator elemental types
 */

#ifndef __redstar_datalib_prop_elem_type_h__
#define __redstar_datalib_prop_elem_type_h__


#include "datalib/superb/types.h"
#include "datalib/codec/superb_options.h"
#include <complex>
#include <iostream>
#include <xml_array4d.h>

namespace Hadron
{
  using namespace SB;

  //----------------------------------------------------------------------------
  //! Key for prop operator
  struct KeyPropElementalOperator_t
  {
    int                t_slice;       /*!< Propagator time slice */
    int                t_source;      /*!< Source time slice */
    int                spin_l;        /*!< Sink spin index */
    int                spin_r;        /*!< Source spin index */
    std::string        mass_label;    /*!< A mass label */
    Array<int>         phasing_sink;  /*!< phasing sink */
    Array<int>         phasing_source;/*!< phasing source */

    bool operator == (const KeyPropElementalOperator_t& ) const = default;
  };

  //! Value for prop operator
  struct ValPropElementalOperator_t
  {
    Array2d< std::complex<double> >  mat;     /*!< Colorvector source and sink */
  };


  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyPropElementalOperator_t& d);

  //----------------------------------------------------------------------------
  //! KeyPropElementalOperator reader
  void read(BinaryReader& bin, KeyPropElementalOperator_t& param);

  //! KeyPropElementalOperator writer
  void write(BinaryWriter& bin, const KeyPropElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! ValPropElementalOperator reader
  void read(BinaryReader& bin, ValPropElementalOperator_t& param);

  //! ValPropElementalOperator reader
  void write(BinaryWriter& bin, const ValPropElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! Key for prop operator for all spins
  struct KeyProp4ElementalOperator_t
  {
    int                t_slice;       /*!< Propagator time slice */
    int                t_source;      /*!< Source time slice */
    std::string        mass_label;    /*!< A mass label */
    Array<int>         phasing_sink;  /*!< phasing sink */
    Array<int>         phasing_source;/*!< phasing source */

    using schema = std::tuple<						    //
      struct_member<&KeyProp4ElementalOperator_t::t_slice, "t_slice"_sl>,   //
      struct_member<&KeyProp4ElementalOperator_t::t_source, "t_source"_sl>, //
      struct_member<&KeyProp4ElementalOperator_t::mass_label, "mass"_sl>,   //
      struct_member<&KeyProp4ElementalOperator_t::phasing_sink, "phasing_sink"_sl,
		    std::vector<int>>, //
      struct_member<&KeyProp4ElementalOperator_t::phasing_source, "phasing_source"_sl,
		    std::vector<int>> //
      >;

    bool operator==(const KeyProp4ElementalOperator_t&) const = default;
  };

  //! Value for prop operator
  struct ValProp4ElementalOperator_t
  {
    XMLArray::Array4d< std::complex<double> >  op;     /*!< Colorvector source and sink and spin source and sink */
  };

  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyProp4ElementalOperator_t& d);

  //----------------------------------------------------------------------------
  //! Read a key
  void read(XMLReader& xml, const std::string& path, KeyProp4ElementalOperator_t& param);

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyProp4ElementalOperator_t& param);

} // namespace ColorVec

#endif
