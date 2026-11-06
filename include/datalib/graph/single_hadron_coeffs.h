// -*- C++ -*-
/*! \file
 *  \brief Single hadron coefficients
 */

#ifndef DATALIB_single_hadron_coeffs_h
#define DATALIB_single_hadron_coeffs_h

#include "datalib/model/map_obj.h"

#include "datalib/model/arrays.h"
#include <iostream>
#include <complex>

namespace Hadron
{
  using namespace Datalib::Model;
  using namespace Datalib::Serial;
  using namespace Datalib::Binary;

  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //! All quark fields
  struct KeySingleHadronQuarkSpin_t
  {
    KeySingleHadronQuarkSpin_t() {}
    KeySingleHadronQuarkSpin_t(int s1) {diracs.push_back(s1);}
    KeySingleHadronQuarkSpin_t(int s1, int s2) {diracs.resize(2); diracs[1]=s1; diracs[2]=s2;}
    KeySingleHadronQuarkSpin_t(int s1, int s2, int s3) {diracs.resize(3); diracs[1]=s1; diracs[2]=s2; diracs[3]=s3;}
    KeySingleHadronQuarkSpin_t(const Array1dO<int>& s) : diracs(s) {}

    Array1dO<int>          diracs;   /*!< For each quark, the Dirac index  0..3 */

    bool operator==(const KeySingleHadronQuarkSpin_t&) const = default;
  };

  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeySingleHadronQuarkSpin_t& op);
 
  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeySingleHadronQuarkSpin_t& param);

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeySingleHadronQuarkSpin_t& param);

  //----------------------------------------------------------------------------
  //! Reader
  void read(BinaryReader& bin, KeySingleHadronQuarkSpin_t& op);

  //! Writer
  void write(BinaryWriter& bin, const KeySingleHadronQuarkSpin_t& op);

  //----------------------------------------------------------------------------
  //! Map holding expressions. Key is the dirac, val is the weight
  typedef MapObject< KeySingleHadronQuarkSpin_t, std::complex<double> >  MapSingleHadronQuarkSpin_t;

  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const MapSingleHadronQuarkSpin_t& op);

  //----------------------------------------------------------------------------------
  //! Multiply a number onto an expression
  MapSingleHadronQuarkSpin_t operator*(double s1, const MapSingleHadronQuarkSpin_t& s2);

  //! Multiply a number onto an expression
  MapSingleHadronQuarkSpin_t operator*(const std::complex<double>& s1, const MapSingleHadronQuarkSpin_t& s2);

  //! Negate
  MapSingleHadronQuarkSpin_t operator-(const MapSingleHadronQuarkSpin_t& s1);

  //! Subtract two expressions
  MapSingleHadronQuarkSpin_t operator-(const MapSingleHadronQuarkSpin_t& s1, const MapSingleHadronQuarkSpin_t& s2);

  //! Add two expressionsa
  MapSingleHadronQuarkSpin_t operator+(const MapSingleHadronQuarkSpin_t& s1, const MapSingleHadronQuarkSpin_t& s2);

  //! Conjugate the imaginary parts - down inside spin
  MapSingleHadronQuarkSpin_t conj(const MapSingleHadronQuarkSpin_t& s1);

  // Fold in gamma_4 for creation operators
  MapSingleHadronQuarkSpin_t foldSourceDP(const MapSingleHadronQuarkSpin_t& s1, bool src);

  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //! All quark fields
  struct SingleHadronQuarkDeriv_t
  {
    SingleHadronQuarkDeriv_t() {}
    SingleHadronQuarkDeriv_t(const std::vector<int>& d_) : deriv(d_) {}

    std::vector<int>   deriv;   /*!< Left-right derivative path. The values are 1-based. */
    bool operator==(const SingleHadronQuarkDeriv_t&) const = default;
  };

  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const SingleHadronQuarkDeriv_t& op);
 
  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, SingleHadronQuarkDeriv_t& param);

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const SingleHadronQuarkDeriv_t& param);

  //----------------------------------------------------------------------------
  //! Reader
  void read(BinaryReader& bin, SingleHadronQuarkDeriv_t& op);

  //! Writer
  void write(BinaryWriter& bin, const SingleHadronQuarkDeriv_t& op);


  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //! All quark fields
  struct KeySingleHadronQuarkDeriv_t
  {
    KeySingleHadronQuarkDeriv_t() {}
    KeySingleHadronQuarkDeriv_t(const SingleHadronQuarkDeriv_t& d1) {derivs.push_back(d1);}
    KeySingleHadronQuarkDeriv_t(const SingleHadronQuarkDeriv_t& d1, const SingleHadronQuarkDeriv_t& d2) 
    {
      derivs.push_back(d1);
      derivs.push_back(d2);
    }
    KeySingleHadronQuarkDeriv_t(const SingleHadronQuarkDeriv_t& d1, const SingleHadronQuarkDeriv_t& d2, const SingleHadronQuarkDeriv_t& d3) 
    {
      derivs.push_back(d1);
      derivs.push_back(d2);
      derivs.push_back(d3);
    }
    KeySingleHadronQuarkDeriv_t(const Array1dO<SingleHadronQuarkDeriv_t>& d_) : derivs(d_) {}

    Array1dO<SingleHadronQuarkDeriv_t>   derivs;   /*!< Left-right derivative path. This is 1-based. */

    bool operator==(const KeySingleHadronQuarkDeriv_t&) const = default;
  };

  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeySingleHadronQuarkDeriv_t& op);
 
  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeySingleHadronQuarkDeriv_t& param);

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeySingleHadronQuarkDeriv_t& param);

  //----------------------------------------------------------------------------
  //! Reader
  void read(BinaryReader& bin, KeySingleHadronQuarkDeriv_t& op);

  //! Writer
  void write(BinaryWriter& bin, const KeySingleHadronQuarkDeriv_t& op);

  //----------------------------------------------------------------------------
  //! Map holding expressions. Key is the deriv, val is the weight
  typedef MapObject<KeySingleHadronQuarkDeriv_t,MapSingleHadronQuarkSpin_t>  MapSingleHadronQuarkDeriv_t;

  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const MapSingleHadronQuarkDeriv_t& op);
 
  //----------------------------------------------------------------------------------
  //! Multiply a number onto an expression
  MapSingleHadronQuarkDeriv_t operator*(double s1, const MapSingleHadronQuarkDeriv_t& s2);

  //! Multiply a number onto an expression
  MapSingleHadronQuarkDeriv_t operator*(const std::complex<double>& s1, const MapSingleHadronQuarkDeriv_t& s2);

  //! Negate
  MapSingleHadronQuarkDeriv_t operator-(const MapSingleHadronQuarkDeriv_t& s1);

  //! Subtract two expressions
  MapSingleHadronQuarkDeriv_t operator-(const MapSingleHadronQuarkDeriv_t& s1, const MapSingleHadronQuarkDeriv_t& s2);

  //! Add two expressions
  MapSingleHadronQuarkDeriv_t operator+(const MapSingleHadronQuarkDeriv_t& s1, const MapSingleHadronQuarkDeriv_t& s2);

  //! Divide a number onto an expression
  MapSingleHadronQuarkDeriv_t operator/(const MapSingleHadronQuarkDeriv_t& s1, double s2);

  //----------------------------------------------------------------------------------
  //! Conjugate the imaginary parts - down inside spin
  MapSingleHadronQuarkDeriv_t conj(const MapSingleHadronQuarkDeriv_t& s1);

  //----------------------------------------------------------------------------------
  // Fold in gamma_4 for creation operators
  MapSingleHadronQuarkDeriv_t foldSourceDP(const MapSingleHadronQuarkDeriv_t& s1, bool src);

  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //! Individual quark field
  struct SingleHadronQuarkFlavor_t
  {
    SingleHadronQuarkFlavor_t() : anti(false), flavor(0) {}
    SingleHadronQuarkFlavor_t(bool a, char f) : anti(a), flavor(f) {}

    bool         anti;    /*!< True means quark is barred */
    char         flavor;  /*!< Simple flavor index */

    bool operator==(const SingleHadronQuarkFlavor_t&) const = default;
  };

  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const SingleHadronQuarkFlavor_t& op);
 
  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, SingleHadronQuarkFlavor_t& param);

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const SingleHadronQuarkFlavor_t& param);

  //----------------------------------------------------------------------------
  //! Reader
  void read(BinaryReader& bin, SingleHadronQuarkFlavor_t& op);

  //! Writer
  void write(BinaryWriter& bin, const SingleHadronQuarkFlavor_t& op);

  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //! Hadron field
  struct KeySingleHadronQuarkFlavorColor_t
  {
    KeySingleHadronQuarkFlavorColor_t() {}
    KeySingleHadronQuarkFlavorColor_t(const std::string& c, 
				      const SingleHadronQuarkFlavor_t& f1) : color(c)
    {
      quarks.push_back(f1);
    }
    KeySingleHadronQuarkFlavorColor_t(const std::string& c, 
				      const SingleHadronQuarkFlavor_t& f1,
				      const SingleHadronQuarkFlavor_t& f2) : color(c)
    {
      quarks.push_back(f1);
      quarks.push_back(f2);
    }
    KeySingleHadronQuarkFlavorColor_t(const std::string& c, 
				      const SingleHadronQuarkFlavor_t& f1,
				      const SingleHadronQuarkFlavor_t& f2,
				      const SingleHadronQuarkFlavor_t& f3) : color(c)
    {
      quarks.push_back(f1);
      quarks.push_back(f2);
      quarks.push_back(f3);
    }
    KeySingleHadronQuarkFlavorColor_t(const std::string& c, const Array1dO<SingleHadronQuarkFlavor_t>& f)
      : color(c), quarks(f) {}

    std::string                              color;   /*!< Some kind of flavor designator */
    Array1dO<SingleHadronQuarkFlavor_t>      quarks; /*!< Each quark field */

    bool operator==(const KeySingleHadronQuarkFlavorColor_t&) const = default;
  };

  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeySingleHadronQuarkFlavorColor_t& op);

  //----------------------------------------------------------------------------
  //! Reader 
  void read(XMLReader& xml, const std::string& path, KeySingleHadronQuarkFlavorColor_t& param);

  //! Writer
  void write(XMLWriter& xml, const std::string& path, const KeySingleHadronQuarkFlavorColor_t& op);

  //----------------------------------------------------------------------------
  //! Reader
  void read(BinaryReader& bin, KeySingleHadronQuarkFlavorColor_t& op); 

  //! Writer
  void write(BinaryWriter& bin, const KeySingleHadronQuarkFlavorColor_t& op);

  //----------------------------------------------------------------------------
  //! Map holding expressions. Key is the quark flavor, val is the weight.
  typedef MapObject<KeySingleHadronQuarkFlavorColor_t,MapSingleHadronQuarkDeriv_t>  MapSingleHadronQuarkFlavorColorSpin_t;

  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const MapSingleHadronQuarkFlavorColorSpin_t& op);

  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //! Multiply a number onto an expression
  MapSingleHadronQuarkFlavorColorSpin_t operator*(double s1, const MapSingleHadronQuarkFlavorColorSpin_t& s2);

  //! Multiply a number onto an expression
  MapSingleHadronQuarkFlavorColorSpin_t operator*(const std::complex<double>& s1, const MapSingleHadronQuarkFlavorColorSpin_t& s2);

  //! Negate
  MapSingleHadronQuarkFlavorColorSpin_t operator-(const MapSingleHadronQuarkFlavorColorSpin_t& s1);

  //! Subtract two expressions
  MapSingleHadronQuarkFlavorColorSpin_t operator-(const MapSingleHadronQuarkFlavorColorSpin_t& s1, 
						  const MapSingleHadronQuarkFlavorColorSpin_t& s2);

  //! Add two expressions
  MapSingleHadronQuarkFlavorColorSpin_t operator+(const MapSingleHadronQuarkFlavorColorSpin_t& s1, 
						  const MapSingleHadronQuarkFlavorColorSpin_t& s2);

  //! Divde a number onto an expression
  MapSingleHadronQuarkFlavorColorSpin_t operator/(const MapSingleHadronQuarkFlavorColorSpin_t& s1, double s2);

  //----------------------------------------------------------------------------------
  //! Conjugate the imaginary parts - down inside spin
  MapSingleHadronQuarkFlavorColorSpin_t conj(const MapSingleHadronQuarkFlavorColorSpin_t& s1);

  //----------------------------------------------------------------------------------
  // Fold in gamma_4 for creation operators
  MapSingleHadronQuarkFlavorColorSpin_t foldSourceDP(const MapSingleHadronQuarkFlavorColorSpin_t& s1, bool src);

}  // end namespace Hadron




#endif
