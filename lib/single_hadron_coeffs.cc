/*! \file
 *  \brief Single hadron coefficients
 */

#include "datalib/graph/single_hadron_coeffs.h"

namespace Hadron
{

  // A small, but non-zero, number
  const double fuzz = 1.0e-11;


  // Utility functions
  namespace
  {
    //! Error output
    std::ostream& operator<<(std::ostream& os, const std::vector<int>& d)
    {
      if (d.size() > 0)
      {
        os << d[0];

        for(int i=1; i < d.size(); ++i)
          os << " " << d[i];
      }

      return os;
    }

    //! Zero out bits of a complex number
    std::complex<double> zeroComplex(const std::complex<double>& w)
    {
      // Pretty print the complex number. Replace noise with 0.
      double op_r = real(w);
      double op_i = imag(w);
	  
      if (fabs(op_r) < fuzz)
      {
	op_r = 0;
      }

      if (fabs(op_i) < fuzz)
      {
	op_i = 0;
      }

      return std::complex<double>(op_r,op_i);
    }

  } // anonymous namespace


  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeySingleHadronQuarkSpin_t& op)
  {
    os << "s ";
    os << op.diracs[1];
    for(int n = 2; n <= op.diracs.size(); ++n)
    {
      os << " " << op.diracs[n];
    }

    return os; 
  }

  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeySingleHadronQuarkSpin_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "diracs", param.diracs);
  }

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeySingleHadronQuarkSpin_t& param)
  {
    push(xml, path);

    write(xml, "diracs", param.diracs);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  void read(BinaryReader& bin, KeySingleHadronQuarkSpin_t& op)
  {
    read(bin, op.diracs);
  }

  void write(BinaryWriter& bin, const KeySingleHadronQuarkSpin_t& op)
  {
    write(bin, op.diracs);
  }


  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const MapSingleHadronQuarkSpin_t& s1)
  {
    // Loop over each source term and do the operation
    for(auto k = s1.begin(); k != s1.end(); ++k)
    {
      os << k->first << "  " << k->second << "\n";
    }

    return os;
  }

  //----------------------------------------------------------------------------------
  //! Multiply a number onto an expression
  MapSingleHadronQuarkSpin_t operator*(double s1, const MapSingleHadronQuarkSpin_t& s2)
  {
    return std::complex<double>(s1,0.0) * s2;
  }

  //----------------------------------------------------------------------------------
  //! Multiply a number onto an expression
  MapSingleHadronQuarkSpin_t operator*(const std::complex<double>& s1, const MapSingleHadronQuarkSpin_t& s2)
  {
    MapSingleHadronQuarkSpin_t dest;

    // Loop over each source term
    for(auto k2 = s2.begin(); k2 != s2.end(); ++k2)
    {
      auto w = zeroComplex(s1 * k2->second);

      // (Re)insert
      // Optimization. If the term is zero (or actually just small), then remove it.
      if (norm(w) > fuzz)
      {	  
	dest.insert(k2->first, w);
//	if (norm(w) < 1.0e-8)
//	  std::cerr << __func__ << ": WARNING: in single_hadron_coeffs.cc NOT zeroing (but would have before change) norm= " << norm(w) << std::endl;
      }
//      else if (norm(w) > 1.0e-16)
//	std::cerr << __func__ << ": WARNING: in single_hadron_coeffs.cc zeroing norm= " << norm(w) << std::endl;

    }

    return dest;
  }

  //----------------------------------------------------------------------------------
  //! Negate
  MapSingleHadronQuarkSpin_t operator-(const MapSingleHadronQuarkSpin_t& s2)
  {
    MapSingleHadronQuarkSpin_t dest;

    // Loop over each source term
    std::vector<KeySingleHadronQuarkSpin_t> k2;
    std::vector< std::complex<double> >                   v2;
    s2.keysAndValues(k2,v2);

    for(int i=0; i < k2.size(); ++i)
    {
      dest.insert(k2[i], -v2[i]);
    }

    return dest;
  }

  //----------------------------------------------------------------------------------
  //! Subtract two expressions
  MapSingleHadronQuarkSpin_t operator-(const MapSingleHadronQuarkSpin_t& s1, const MapSingleHadronQuarkSpin_t& s2)
  {
    return s1 + (-s2);
  }

  //----------------------------------------------------------------------------------
  //! Add two expressions
  MapSingleHadronQuarkSpin_t operator+(const MapSingleHadronQuarkSpin_t& s1, const MapSingleHadronQuarkSpin_t& s2)
  {
   // Initialize the destination with the first term
    MapSingleHadronQuarkSpin_t dest = s1;

    // Loop over each s2 term. Insert the term if it does not exist.
    for(auto k2 = s2.begin(); k2 != s2.end(); ++k2)
    {
      std::complex<double> w = k2->second;
       
      // If key exist, add it
      if (dest.exist(k2->first))
      {
	std::complex<double> orig = dest[k2->first];

	// Phases are the same, so just add the weights
	w += orig;
      }

      // Pretty print the complex number. Replace noise with 0.
      w = zeroComplex(w);

      // (Re)insert
      // Optimization. If the term is zero (or actually just small), then remove it.
      if (norm(w) > fuzz)
      {	  
	dest.insert(k2->first, w);
//	if (norm(w) < 1.0e-8)
//	  std::cerr << __func__ << ": WARNING: in single_hadron_coeffs.cc NOT zeroing (but would have before change) norm= " << norm(w) << std::endl;
      }
      else
      {
	// Remove it
	dest.erase(k2->first);
//	if (norm(w) > 1.0e-16)
//	  std::cerr << __func__ << ": WARNING: in single_hadron_coeffs.cc zeroing norm= " << norm(w) << std::endl;
      }
    }

    return dest;
  }


  //----------------------------------------------------------------------------------
  //! Conjugate the imaginary parts - down inside spin
  MapSingleHadronQuarkSpin_t conj(const MapSingleHadronQuarkSpin_t& s1)
  {
    // Output
    MapSingleHadronQuarkSpin_t dest;

    for(auto k1 = s1.begin(); k1 != s1.end(); ++k1)
    {
      dest.insert(k1->first, conj(k1->second));
    }

    return dest;
  }


  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  // Fold in gamma_4 for creation operators
  MapSingleHadronQuarkSpin_t foldSourceDP(const MapSingleHadronQuarkSpin_t& s1, bool src)
  {
    // Bolt if not a creation op
    if (! src)
      return s1;

    // Output
    MapSingleHadronQuarkSpin_t dest;

    // The spin terms are where is the action
    // Check for lower spin components
    for(auto k1 = s1.begin(); k1 != s1.end(); ++k1)
    {
      int isign = +1;

      // Flip sign if this quark has a lower spin component
      for(auto q = 1; q <= k1->first.diracs.size(); ++q)
      {
	if (k1->first.diracs[q] >= 2)
	  isign = -isign;
      }

      // Put this modified value back into a new map
      dest.insert(k1->first, double(isign) * k1->second);
    }

    return dest;
  }


  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const SingleHadronQuarkDeriv_t& op)
  {
    os << op.deriv;

    return os; 
  }

  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, SingleHadronQuarkDeriv_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "deriv", param.deriv);
  }

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const SingleHadronQuarkDeriv_t& param)
  {
    push(xml, path);

    write(xml, "deriv", param.deriv);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  void read(BinaryReader& bin, SingleHadronQuarkDeriv_t& op)
  {
    read(bin, op.deriv);
  }

  void write(BinaryWriter& bin, const SingleHadronQuarkDeriv_t& op)
  {
    write(bin, op.deriv);
  }



  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeySingleHadronQuarkDeriv_t& op)
  {
    os << "   d";
    for(int n = 1; n <= op.derivs.size(); ++n)
    {
      int ss = op.derivs[n].deriv.size();
      os << "  "  << ss;
      if (ss > 0)
      {
	os << " " << op.derivs[n].deriv;
      }
    }

    return os; 
  }

  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeySingleHadronQuarkDeriv_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "derivs", param.derivs);
  }

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeySingleHadronQuarkDeriv_t& param)
  {
    push(xml, path);

    write(xml, "derivs", param.derivs);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  void read(BinaryReader& bin, KeySingleHadronQuarkDeriv_t& op)
  {
    read(bin, op.derivs);
  }

  void write(BinaryWriter& bin, const KeySingleHadronQuarkDeriv_t& op)
  {
    write(bin, op.derivs);
  }


  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const MapSingleHadronQuarkDeriv_t& s1)
  {
    // Now go to the next level - derivs
    for(auto k2 = s1.begin(); k2 != s1.end(); ++k2)
    {
      // Get the spin terms
      for(auto k3 = k2->second.begin(); k3 != k2->second.end(); ++k3)
      {
	// First print the dirac indices
	os << k3->first;

	// Print the derivatives
	os << k2->first;

	// Pretty print the complex number. Replace noise with 0.
	auto op = zeroComplex(k3->second);

	os << "  " << op << "\n";
      } // j
    } // d

    return os;
  }


  //----------------------------------------------------------------------------------
  //! Multiply a number onto an expression
  MapSingleHadronQuarkDeriv_t operator*(double s1, const MapSingleHadronQuarkDeriv_t& s2)
  {
    return std::complex<double>(s1,0) * s2;
  }

  //----------------------------------------------------------------------------------
  //! Multiply a number onto an expression
  MapSingleHadronQuarkDeriv_t operator*(const std::complex<double>& s1, const MapSingleHadronQuarkDeriv_t& s2)
  {
    MapSingleHadronQuarkDeriv_t dest;

    // Loop over each s2 term. Insert the term if it does not exist.
    for(auto k2 = s2.begin(); k2 != s2.end(); ++k2)
    {
      MapSingleHadronQuarkSpin_t w = s1 * k2->second;

      if (w.size() > 0)
	dest.insert(k2->first, w);
    }

    return dest;
  }

  //----------------------------------------------------------------------------------
  //! Negate
  MapSingleHadronQuarkDeriv_t operator-(const MapSingleHadronQuarkDeriv_t& s2)
  {
    MapSingleHadronQuarkDeriv_t dest;

    // Loop over each s2 term. Insert the term if it does not exist.
    for(auto k2 = s2.begin(); k2 != s2.end(); ++k2)
    {
      dest.insert(k2->first, -k2->second);
    }

    return dest;
  }

  //----------------------------------------------------------------------------------
  //! Subtract two expressions
  MapSingleHadronQuarkDeriv_t operator-(const MapSingleHadronQuarkDeriv_t& s1, const MapSingleHadronQuarkDeriv_t& s2)
  {
    return s1 + (-s2);
  }

  //----------------------------------------------------------------------------------
  //! Add two expressions
  MapSingleHadronQuarkDeriv_t operator+(const MapSingleHadronQuarkDeriv_t& s1, const MapSingleHadronQuarkDeriv_t& s2)
  {
    // Initialize the destination with the first term
    MapSingleHadronQuarkDeriv_t dest = s1;

    // Loop over each source term
    std::vector<KeySingleHadronQuarkDeriv_t> k2 = s2.keys();

    // Loop over each s2 term. Insert the term if it does not exist.
    for(std::vector<KeySingleHadronQuarkDeriv_t>::const_iterator k = k2.begin();
	k != k2.end();
	++k)
    {
      MapSingleHadronQuarkSpin_t weight = s2[*k];

      // If key exist, add it
      if (dest.exist(*k))
      {
	weight = weight + dest[*k];
      }

      // Insert
      if (weight.size() > 0)
	dest.insert(*k, weight);
      else
	dest.erase(*k);
    }

    return dest;
  }


  //----------------------------------------------------------------------------------
  //! Divide a number onto an expression
  MapSingleHadronQuarkDeriv_t operator/(const MapSingleHadronQuarkDeriv_t& s1, double s2)
  {
    return double(1.0 / s2) * s1;
  }


  //----------------------------------------------------------------------------------
  //! Conjugate the imaginary parts - down inside spin
  MapSingleHadronQuarkDeriv_t conj(const MapSingleHadronQuarkDeriv_t& s1)
  {
    // Output
    MapSingleHadronQuarkDeriv_t dest;

    for(auto k1 = s1.begin(); k1 != s1.end(); ++k1)
    {
      dest.insert(k1->first, conj(k1->second));
    }

    return dest;
  }


  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  // Fold in gamma_4 for creation operators
  MapSingleHadronQuarkDeriv_t foldSourceDP(const MapSingleHadronQuarkDeriv_t& s1, bool src)
  {
    // Bolt if not a creation op
    if (! src)
      return s1;

    // Output
    MapSingleHadronQuarkDeriv_t dest;

    // Derivative keys
    for(auto k1 = s1.begin(); k1 != s1.end(); ++k1)
    {
      // Fold and insert into the out flavor map
      dest.insert(k1->first, foldSourceDP(k1->second,src));
    }

    return dest;
  }



  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const SingleHadronQuarkFlavor_t& op)
  {
    if (op.anti)
    {
      os << char(toupper(op.flavor));
    }
    else
    {
      os << op.flavor;
    }

    return os; 
  }

  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, SingleHadronQuarkFlavor_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "anti", param.anti);
    read(paramtop, "flavor", param.flavor);
  }

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const SingleHadronQuarkFlavor_t& param)
  {
    push(xml, path);

    write(xml, "anti", param.anti);
    write(xml, "flavor", param.flavor);

    pop(xml);
  }


  //----------------------------------------------------------------------------
  void read(BinaryReader& bin, SingleHadronQuarkFlavor_t& op)
  {
    read(bin, op.anti);
    read(bin, op.flavor);
  }

  void write(BinaryWriter& bin, const SingleHadronQuarkFlavor_t& op)
  {
    write(bin, op.anti);
    write(bin, op.flavor);
  }



  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const KeySingleHadronQuarkFlavorColor_t& op)
  {
    os << "C" << op.color << "  ";

    if (op.quarks.size() > 0)
    {
      for(int i=1; i <= op.quarks.size(); ++i)
	os << op.quarks[i];
    }
   
    return os; 
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, KeySingleHadronQuarkFlavorColor_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "color", param.color);
    read(paramtop, "Quarks", param.quarks);
  }

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeySingleHadronQuarkFlavorColor_t& param)
  {
    push(xml, path);

    write(xml, "color", param.color);
    write(xml, "Quarks", param.quarks);

    pop(xml);
  }

  //----------------------------------------------------------------------------
  void read(BinaryReader& bin, KeySingleHadronQuarkFlavorColor_t& op)
  {
    readDesc(bin, op.color);
    read(bin, op.quarks);
  }

  void write(BinaryWriter& bin, const KeySingleHadronQuarkFlavorColor_t& op)
  {
    writeDesc(bin, op.color);
    write(bin, op.quarks);
  }


  //----------------------------------------------------------------------------------
  std::ostream& operator<<(std::ostream& os, const MapSingleHadronQuarkFlavorColorSpin_t& s1)
  {
    // Loop over each source term and do the operation
    std::vector<KeySingleHadronQuarkFlavorColor_t> k1;
    std::vector<MapSingleHadronQuarkDeriv_t>       v1;
    s1.keysAndValues(k1,v1);

    for(int i=0; i < k1.size(); ++i)
    {
      os << k1[i] << "\n";

      os << v1[i];
    }

    return os;
  }


  //----------------------------------------------------------------------------------
  //! Multiply a number onto an expression
  MapSingleHadronQuarkFlavorColorSpin_t operator*(double s1, const MapSingleHadronQuarkFlavorColorSpin_t& s2)
  {
    return std::complex<double>(s1,0) * s2;
  }

  //----------------------------------------------------------------------------------
  //! Multiply a number onto an expression
  MapSingleHadronQuarkFlavorColorSpin_t operator*(const std::complex<double>& s1, const MapSingleHadronQuarkFlavorColorSpin_t& s2)
  {
    MapSingleHadronQuarkFlavorColorSpin_t dest;

    // Loop over each s2 term. Insert the term if it does not exist.
    for(auto k2 = s2.begin(); k2 != s2.end(); ++k2)
    {
      dest.insert(k2->first, s1 * k2->second);
    }

    return dest;
  }

  //----------------------------------------------------------------------------------
  //! Negate
  MapSingleHadronQuarkFlavorColorSpin_t operator-(const MapSingleHadronQuarkFlavorColorSpin_t& s2)
  {
    MapSingleHadronQuarkFlavorColorSpin_t dest;

    // Loop over each s2 term. Insert the term if it does not exist.
    for(auto k2 = s2.begin(); k2 != s2.end(); ++k2)
    {
      dest.insert(k2->first, -k2->second);
    }

    return dest;
  }

  //----------------------------------------------------------------------------------
  //! Subtract two expressions
  MapSingleHadronQuarkFlavorColorSpin_t operator-(const MapSingleHadronQuarkFlavorColorSpin_t& s1, 
						  const MapSingleHadronQuarkFlavorColorSpin_t& s2)
  {
    return s1 + (-s2);
  }

  //----------------------------------------------------------------------------------
  //! Add two expressions
  MapSingleHadronQuarkFlavorColorSpin_t operator+(const MapSingleHadronQuarkFlavorColorSpin_t& s1, 
						  const MapSingleHadronQuarkFlavorColorSpin_t& s2)
  {
    // Initialize the destination with the first term
    MapSingleHadronQuarkFlavorColorSpin_t dest = s1;

    // Loop over each s2 term. Insert the term if it does not exist.
    for(auto k2 = s2.begin(); k2 != s2.end(); ++k2)
    {
      MapSingleHadronQuarkDeriv_t weight = k2->second;

      // If key exist, add it
      if (dest.exist(k2->first))
      {
	weight = weight + dest[k2->first];
      }

      // Insert
      dest.insert(k2->first, weight);
    }

    return dest;
  }

  //! Divde a number onto an expression
  MapSingleHadronQuarkFlavorColorSpin_t operator/(const MapSingleHadronQuarkFlavorColorSpin_t& s1, double s2)
  {
    return double(1.0 / s2) * s1;
  }


  //----------------------------------------------------------------------------------
  //! Conjugate the imaginary parts - down inside spin
  MapSingleHadronQuarkFlavorColorSpin_t conj(const MapSingleHadronQuarkFlavorColorSpin_t& s1)
  {
    // Output
    MapSingleHadronQuarkFlavorColorSpin_t dest;

    for(auto k1 = s1.begin(); k1 != s1.end(); ++k1)
    {
      dest.insert(k1->first, conj(k1->second));
    }

    return dest;
  }


  //----------------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  // Fold in gamma_4 for creation operators
  MapSingleHadronQuarkFlavorColorSpin_t foldSourceDP(const MapSingleHadronQuarkFlavorColorSpin_t& s2, bool src)
  {
    // Bolt if not a creation op
    if (! src)
      return s2;

    // Output
    MapSingleHadronQuarkFlavorColorSpin_t dest;

    for(auto k2 = s2.begin(); k2 != s2.end(); ++k2)
    {
      // Fold and insert into the out flavor map
      dest.insert(k2->first, foldSourceDP(k2->second,src));
    }

    return dest;
  }


}

