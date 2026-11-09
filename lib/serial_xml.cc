//
/*! @file
 * @brief XML IO support
 */

#include "datalib/io/serial_xml.h"

namespace Datalib::Serial 
{

  using XMLArray::Array;

  using std::string;
  using std::cerr;

  //--------------------------------------------------------------------------------
  // XML classes
  // XML reader class
  void XMLReader::open(const XMLBufferWriter& mw)
  {
    if (iop)
    {
      std::cerr << "XMLReader already opened: trying to open for xmlbufferwriter" << std::endl;
      exit(1);
    }

    std::istringstream is(mw.str());
    BasicXPathReader::open(is);

    iop = true;
  }

  bool XMLReader::is_derived() const {return derived;}

  void XMLReader::print(std::ostream& os)
  {
    BasicXPathReader::print(os);
  }
   
  void XMLReader::printRoot(std::ostream& os)
  {
    BasicXPathReader::printRoot(os);
  }

  void XMLReader::printCurrentContext(std::ostream& os)
  {
    std::ostringstream newos;

    if (is_derived())
      BasicXPathReader::printChildren(newos);
    else
      BasicXPathReader::printRoot(newos);

    os << newos.str();
  }
   
  int XMLReader::count(const string& xpath)
  {
    int n;
    n = BasicXPathReader::count(xpath);
    return n;
  }
  
  //-------------- EXTENSIONS FOR Datalib::Model--------------------
  //! This is not in the QDP++ XMLReader - expose this function
  void XMLReader::printXPathNode(std::ostream& os, const std::string& xpath_to_node) {
    Detail::BasicXPathReader::printXPathNode(os, xpath_to_node);
  }
  //-------------- END OF EXTENSIONS FOR Datalib::Model--------------------


  // Overloaded Reader Functions
 
  void read(XMLReader& xml, const std::string& xpath, string& result)
  {
    xml.get(xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, char& result)
  {
    // Gruesome hack. For some inexplicable reason we don't have read of a char in xpath_reader.
    std::string d;
    xml.get(xpath, d);
    result = d[0];
  }
  void read(XMLReader& xml, const std::string& xpath, int& result)
  {
    xml.get(xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, unsigned int& result)
  {
    xml.get(xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, short int& result)
  {
    xml.get(xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, unsigned short int& result)
  {
    xml.get(xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, long int& result)
  {
    xml.get(xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, unsigned long int& result)
  {
    xml.get(xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, float& result)
  {
    xml.get(xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, double& result)
  {
    xml.get(xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, bool& result)
  {
    xml.get(xpath, result);
  }


  //! Complex reader
  void read(XMLReader& xml, const std::string& xpath, std::complex<float>& param)
  {
    // NOTE: the C++-11 standard says a "real()" function returns a r-value - not an "l-value", so
    // cannot simply read into a param.real()
    XMLReader paramtop(xml, xpath);
    
    float re, im;
    read(paramtop, "re", re);
    read(paramtop, "im", im);
    param = std::complex<float>(re,im);
  }

  
  void read(XMLReader& xml, const std::string& xpath, std::complex<double>& param)
  {
    // NOTE: the C++-11 standard says a "real()" function returns a r-value - not an "l-value", so
    // cannot simply read into a param.real()
    XMLReader paramtop(xml, xpath);
    
    double re, im;
    read(paramtop, "re", re);
    read(paramtop, "im", im);
    param = std::complex<double>(re,im);
  }



  //! Read a XML Array element
  template<typename T>
  void readArrayPrimitive(XMLReader& xml, const std::string& xpath, XMLArray::Array<T>& result)
  {
    std::ostringstream error_message;
  
    // Try reading the list as a string
    std::string list_string;
    read(xml, xpath, list_string);

    // Count the number of elements
    std::istringstream list_stream(list_string);
	
    int array_size = 0;
    T dummy;
    while(list_stream >> dummy)
      ++array_size;

    if ((! list_stream.eof()) && list_stream.fail())
    {
      error_message << "Error in reading array " << xpath << std::endl;
      throw error_message.str();
    }

    // Now resize the array to hold the no of elements.
    result.resize(array_size);

    // Get the elements one by one
    // I do not understand why, but use a new stringstream
    //  list_stream.str(list_string);
    std::istringstream list_stream2(list_string);

    for(int i=0; i < result.size(); i++) 
    {
      // read the element.
      list_stream2 >> result[i];
    }
  }

  //! Read a XML Array element
  void read(XMLReader& xml, const std::string& xpath, Array<int>& result)
  {
    readArrayPrimitive<int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Array<unsigned int>& result)
  {
    readArrayPrimitive<unsigned int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Array<short int>& result)
  {
    readArrayPrimitive<short int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Array<unsigned short int>& result)
  {
    readArrayPrimitive<unsigned short int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Array<long int>& result)
  {
    readArrayPrimitive<long int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Array<unsigned long int>& result)
  {
    readArrayPrimitive<unsigned long int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Array<float>& result)
  {
    readArrayPrimitive<float>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Array<double>& result)
  {
    readArrayPrimitive<double>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Array<bool>& result)
  {
    readArrayPrimitive<bool>(xml, xpath, result);
  }


  //! Read a XML vector element
  template<typename T>
  void readVectorPrimitive(XMLReader& xml, const std::string& xpath, std::vector<T>& result)
  {
    std::ostringstream error_message;
  
    // Try reading the list as a string
    std::string list_string;
    read(xml, xpath, list_string);

    // Count the number of elements
    std::istringstream list_stream(list_string);
	
    int array_size = 0;
    T dummy;
    while(list_stream >> dummy)
      ++array_size;

    if ((! list_stream.eof()) && list_stream.fail())
    {
      error_message << "Error in reading array " << xpath << std::endl;
      throw error_message.str();
    }

    // Now resize the array to hold the no of elements.
    result.resize(array_size);

    // Get the elements one by one
    // I do not understand why, but use a new stringstream
    //  list_stream.str(list_string);
    std::istringstream list_stream2(list_string);

    for(int i=0; i < result.size(); i++) 
    {
      // read the element.
      list_stream2 >> result[i];
    }
  }


//! Read a XML Array element
  void read(XMLReader& xml, const std::string& xpath, std::vector<int>& result)
  {
    readVectorPrimitive<int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::vector<unsigned int>& result)
  {
    readVectorPrimitive<unsigned int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::vector<short int>& result)
  {
    readVectorPrimitive<short int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::vector<unsigned short int>& result)
  {
    readVectorPrimitive<unsigned short int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::vector<long int>& result)
  {
    readVectorPrimitive<long int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::vector<unsigned long int>& result)
  {
    readVectorPrimitive<unsigned long int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::vector<float>& result)
  {
    readVectorPrimitive<float>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::vector<double>& result)
  {
    readVectorPrimitive<double>(xml, xpath, result);
  }
//void read(XMLReader& xml, const std::string& xpath, std::vector<bool>& result)
//{
//  readVectorPrimitive<bool>(xml, xpath, result);
//}


  //! Read a XML list element
  template<typename T>
  void readListPrimitive(XMLReader& xml, const std::string& xpath, std::list<T>& result)
  {
    std::ostringstream error_message;
  
    // Try reading the list as a string
    std::string list_string;
    read(xml, xpath, list_string);

    // Count the number of elements
    std::istringstream list_stream(list_string);
	
    int array_size = 0;
    T dummy;
    while(list_stream >> dummy)
      ++array_size;

    if ((! list_stream.eof()) && list_stream.fail())
    {
      error_message << "Error in reading array " << xpath << std::endl;
      throw error_message.str();
    }

    // Clear out elements
    result.clear();

    // Get the elements one by one
    // I do not understand why, but use a new stringstream
    //  list_stream.str(list_string);
    std::istringstream list_stream2(list_string);

    for(int i=0; i < array_size; i++) 
    {
      // read the element.
      T thingy;
      list_stream2 >> thingy;
      result.push_back(thingy);
    }
  }


//! Read a XML Array element
  void read(XMLReader& xml, const std::string& xpath, std::list<int>& result)
  {
    readListPrimitive<int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::list<unsigned int>& result)
  {
    readListPrimitive<unsigned int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::list<short int>& result)
  {
    readListPrimitive<short int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::list<unsigned short int>& result)
  {
    readListPrimitive<unsigned short int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::list<long int>& result)
  {
    readListPrimitive<long int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::list<unsigned long int>& result)
  {
    readListPrimitive<unsigned long int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::list<float>& result)
  {
    readListPrimitive<float>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, std::list<double>& result)
  {
    readListPrimitive<double>(xml, xpath, result);
  }

  

  //! Read a XML Array1dO element
  template<typename T>
  void readArray1dOPrimitive(XMLReader& xml, const std::string& xpath, Datalib::Model::Array1dO<T>& result)
  {
    std::ostringstream error_message;
  
    // Try reading the list as a string
    std::string list_string;
    read(xml, xpath, list_string);

    // Count the number of elements
    std::istringstream list_stream(list_string);
	
    int array_size = 0;
    T dummy;
    while(list_stream >> dummy)
      ++array_size;

    if ((! list_stream.eof()) && list_stream.fail())
    {
      error_message << "Error in reading array " << xpath << std::endl;
      throw error_message.str();
    }

    // Now resize the array to hold the no of elements.
    result.resize(array_size);

    // Get the elements one by one
    // I do not understand why, but use a new stringstream
    //  list_stream.str(list_string);
    std::istringstream list_stream2(list_string);

    for(int i=1; i <= result.size(); i++) 
    {
      // read the element.
      list_stream2 >> result[i];
    }
  }


//! Read a XML Array1dO element
  void read(XMLReader& xml, const std::string& xpath, Datalib::Model::Array1dO<int>& result)
  {
    readArray1dOPrimitive<int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Datalib::Model::Array1dO<unsigned int>& result)
  {
    readArray1dOPrimitive<unsigned int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Datalib::Model::Array1dO<short int>& result)
  {
    readArray1dOPrimitive<short int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Datalib::Model::Array1dO<unsigned short int>& result)
  {
    readArray1dOPrimitive<unsigned short int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Datalib::Model::Array1dO<long int>& result)
  {
    readArray1dOPrimitive<long int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Datalib::Model::Array1dO<unsigned long int>& result)
  {
    readArray1dOPrimitive<unsigned long int>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Datalib::Model::Array1dO<float>& result)
  {
    readArray1dOPrimitive<float>(xml, xpath, result);
  }
  void read(XMLReader& xml, const std::string& xpath, Datalib::Model::Array1dO<double>& result)
  {
    readArray1dOPrimitive<double>(xml, xpath, result);
  }
//void read(XMLReader& xml, const std::string& xpath, Datalib::Model::Array1dO<bool>& result)
//{
//  readArray1dOPrimitive<bool>(xml, xpath, result);
//}


  //--------------------------------------------------------------------------------
  // XML writer base class
  XMLWriter::XMLWriter()
  {
  }

  XMLWriter::~XMLWriter()
  {
  }

  void XMLWriter::openSimple(const string& tagname)
  {
    openTag(tagname);
  }

  void XMLWriter::closeSimple()
  {
    closeTag();
  }

  void XMLWriter::openStruct(const string& tagname)
  {
    openTag(tagname);
  }

  void XMLWriter::closeStruct()
  {
    closeTag();
  }

  void XMLWriter::openTag(const string& tagname)
  {
    XMLSimpleWriter::openTag(tagname);
  }

  void XMLWriter::openTag(const string& nsprefix, const string& tagname)
  {
    XMLSimpleWriter::openTag(nsprefix,tagname);
  }

  void XMLWriter::openTag(const string& tagname, AttributeList& al)
  {
    XMLSimpleWriter::openTag(tagname,al);
  }

  void XMLWriter::openTag(const string& nsprefix, 
			  const string& tagname, 
			  AttributeList& al)
  {
    XMLSimpleWriter::openTag(nsprefix,tagname,al);
  }

  void XMLWriter::closeTag()
  {
    XMLSimpleWriter::closeTag();
  }

  void XMLWriter::emptyTag(const string& tagname)
  {
    XMLSimpleWriter::emptyTag(tagname);
  }
  void XMLWriter::emptyTag(const string& tagname,  AttributeList& al)
  {
    XMLSimpleWriter::emptyTag(tagname,al);
  }

  void XMLWriter::emptyTag(const string& nsprefix, 
			   const string& tagname, 
			   AttributeList& al)
  {
    XMLSimpleWriter::emptyTag(nsprefix,tagname,al);
  }


// Overloaded Writer Functions
  void XMLWriter::write(const string& output)
  {
    XMLSimpleWriter::write(output);
  }
  void XMLWriter::write(const int& output)
  {
    XMLSimpleWriter::write(output);
  }
  void XMLWriter::write(const unsigned int& output)
  {
    XMLSimpleWriter::write(output);
  }
  void XMLWriter::write(const short int& output)
  {
    XMLSimpleWriter::write(output);
  }
  void XMLWriter::write(const unsigned short int& output)
  {
    XMLSimpleWriter::write(output);
  }
  void XMLWriter::write(const long int& output)
  {
    XMLSimpleWriter::write(output);
  }
  void XMLWriter::write(const unsigned long int& output)
  {
    XMLSimpleWriter::write(output);
  }
  void XMLWriter::write(const float& output)
  {
    XMLSimpleWriter::write(output);
  }
  void XMLWriter::write(const double& output)
  {
    XMLSimpleWriter::write(output);
  }
  void XMLWriter::write(const bool& output)
  {
    XMLSimpleWriter::write(output);
  }
   
  // Write XML string
  void XMLWriter::writeXML(const string& output)
  {
    XMLSimpleWriter::writeXML(output);
  }


  // Push a group name
  void push(XMLWriter& xml, const string& s) {xml.openStruct(s);}

  // Pop a group name
  void pop(XMLWriter& xml) {xml.closeStruct();}

  // Write something from a reader
  void write(XMLWriter& xml, const std::string& s, const XMLReader& d)
  {
    xml.openTag(s);
    xml << d;
    xml.closeTag();
  }

  XMLWriter& operator<<(XMLWriter& xml, const XMLReader& d)
  {
    std::ostringstream os;
    const_cast<XMLReader&>(d).printRoot(os);
    xml.writeXML(os.str());
    return xml;
  }

  // Write something from a XMLBufferWriter
  void write(XMLWriter& xml, const std::string& s, const XMLBufferWriter& d)
  {
    xml.openTag(s);
    xml << d;
    xml.closeTag();
  }

  XMLWriter& operator<<(XMLWriter& xml, const XMLBufferWriter& d)
  {
    xml.writeXML(const_cast<XMLBufferWriter&>(d).printRoot());
    return xml;
  }

  // Time to build a telephone book of basic primitives
  template<typename T>
  void writePrimitive(XMLWriter& xml, const string& s, const T& d)
  {
    xml.openTag(s);
    xml.write(d);
    xml.closeTag();
  }

  void write(XMLWriter& xml, const string& s, const string& d)
  {
    writePrimitive<string>(xml, s, d);
  }

  void write(XMLWriter& xml, const string& s, const char* d)
  {
    writePrimitive<string>(xml, s, string(d));
  }

  void write(XMLWriter& xml, const string& s, const char& d)
  {
    // Gruesome hack. For some inexplicable reason we don't have a proper write of a char in xpath_reader.
    writePrimitive<string>(xml, s, string(1,d));
  }

  void write(XMLWriter& xml, const string& s, const int& d)
  {
    writePrimitive<int>(xml, s, d);
  }

  void write(XMLWriter& xml, const string& s, const unsigned int& d)
  {
    writePrimitive<unsigned int>(xml, s, d);
  }

  void write(XMLWriter& xml, const string& s, const short int& d)
  {
    writePrimitive<short int>(xml, s, d);
  }

  void write(XMLWriter& xml, const string& s, const unsigned short int& d)
  {
    writePrimitive<unsigned short int>(xml, s, d);
  }

  void write(XMLWriter& xml, const string& s, const long int& d)
  {
    writePrimitive<long int>(xml, s, d);
  }

  void write(XMLWriter& xml, const string& s, const unsigned long int& d)
  {
    writePrimitive<unsigned long int>(xml, s, d);
  }

  void write(XMLWriter& xml, const string& s, const float& d)
  {
    writePrimitive<float>(xml, s, d);
  }

  void write(XMLWriter& xml, const string& s, const double& d)
  {
    writePrimitive<double>(xml, s, d);
  }

  void write(XMLWriter& xml, const string& s, const bool& d)
  {
    writePrimitive<bool>(xml, s, d);
  }

  // Versions that do not print a name
  XMLWriter& operator<<(XMLWriter& xml, const string& d) {xml.write(d);return xml;}
  XMLWriter& operator<<(XMLWriter& xml, const char* d) {xml.write(string(d));return xml;}
  XMLWriter& operator<<(XMLWriter& xml, const char& d) {xml.write(d);return xml;}
  XMLWriter& operator<<(XMLWriter& xml, const int& d) {xml.write(d);return xml;}
  XMLWriter& operator<<(XMLWriter& xml, const unsigned int& d) {xml.write(d);return xml;}
  XMLWriter& operator<<(XMLWriter& xml, const short int& d) {xml.write(d);return xml;}
  XMLWriter& operator<<(XMLWriter& xml, const unsigned short int& d) {xml.write(d);return xml;}
  XMLWriter& operator<<(XMLWriter& xml, const long int& d) {xml.write(d);return xml;}
  XMLWriter& operator<<(XMLWriter& xml, const unsigned long int& d) {xml.write(d);return xml;}
  XMLWriter& operator<<(XMLWriter& xml, const float& d) {xml.write(d);return xml;}
  XMLWriter& operator<<(XMLWriter& xml, const double& d) {xml.write(d);return xml;}
  XMLWriter& operator<<(XMLWriter& xml, const bool& d) {xml.write(d);return xml;}


  // Write an array of basic types
  template<typename T>
  void writeArrayPrimitive(XMLWriter& xml, const std::string& s, const Array<T>& s1)
  {
    std::ostringstream output;

    if (s1.size() > 0)
    {
      output << s1[0];
      for(unsigned index=1; index < s1.size(); index++) 
	output << " " << s1[index];
    }
    
    // Write the array - do not use a normal string write
    xml.openTag(s);
    xml << output.str();
    xml.closeTag();
  }


  //! Complex writer
  void write(XMLWriter& xml, const std::string& xpath, const std::complex<float>& param)
  {
    push(xml, xpath);
    write(xml, "re", param.real());
    write(xml, "im", param.imag());
    pop(xml);
  }

  
  void write(XMLWriter& xml, const std::string& xpath, const std::complex<double>& param)
  {
    push(xml, xpath);
    write(xml, "re", param.real());
    write(xml, "im", param.imag());
    pop(xml);
  }



  void write(XMLWriter& xml, const std::string& xpath, const Array<int>& output)
  {
    writeArrayPrimitive<int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array<unsigned int>& output)
  {
    writeArrayPrimitive<unsigned int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array<short int>& output)
  {
    writeArrayPrimitive<short int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array<unsigned short int>& output)
  {
    writeArrayPrimitive<unsigned short int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array<long int>& output)
  {
    writeArrayPrimitive<long int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array<unsigned long int>& output)
  {
    writeArrayPrimitive<unsigned long int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array<float>& output)
  {
    writeArrayPrimitive<float>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& s, const Array<double>& s1)
  {
    std::ostringstream os;

    if (s1.size() > 0)
    {
      std::streamsize initPrec = os.precision();
      os.precision(15);

      os << s1[0];
      for(unsigned index=1; index < s1.size(); index++) 
	os << " " << s1[index];

      os.precision(initPrec);
    }
    
    // Write the array - do not use a normal string write
    xml.openTag(s);
    xml << os.str();
    xml.closeTag();
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array<bool>& output)
  {
    writeArrayPrimitive<bool>(xml, xpath, output);
  }



  // Write a vector of basic types
  template<typename T>
  void writeVectorPrimitive(XMLWriter& xml, const std::string& s, const std::vector<T>& s1)
  {
    std::ostringstream output;

    if (s1.size() > 0)
    {
      output << s1[0];
      for(unsigned index=1; index < s1.size(); index++) 
	output << " " << s1[index];
    }
    
    // Write the array - do not use a normal string write
    xml.openTag(s);
    xml << output.str();
    xml.closeTag();
  }


  void write(XMLWriter& xml, const std::string& xpath, const std::vector<int>& output)
  {
    writeVectorPrimitive<int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::vector<unsigned int>& output)
  {
    writeVectorPrimitive<unsigned int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::vector<short int>& output)
  {
    writeVectorPrimitive<short int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::vector<unsigned short int>& output)
  {
    writeVectorPrimitive<unsigned short int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::vector<long int>& output)
  {
    writeVectorPrimitive<long int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::vector<unsigned long int>& output)
  {
    writeVectorPrimitive<unsigned long int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::vector<float>& output)
  {
    writeVectorPrimitive<float>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& s, const std::vector<double>& s1)
  {
    std::ostringstream os;

    if (s1.size() > 0)
    {
      std::streamsize initPrec = os.precision();
      os.precision(15);

      os << s1[0];
      for(unsigned index=1; index < s1.size(); index++) 
	os << " " << s1[index];

      os.precision(initPrec);
    }
    
    // Write the array - do not use a normal string write
    xml.openTag(s);
    xml << os.str();
    xml.closeTag();
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::vector<bool>& output)
  {
    writeVectorPrimitive<bool>(xml, xpath, output);
  }


  // Write a list of basic types
  template<typename T>
  void writeListPrimitive(XMLWriter& xml, const std::string& s, const std::list<T>& s1)
  {
    std::ostringstream output;

    if (s1.size() > 0)
    {
      bool first = true;
      for(typename std::list<T>::const_iterator p = s1.begin(); p != s1.end(); ++p)
      {
	if (! first)
	{
	  output << " ";
	}

	output << *p;
	first = false;
      }
    }
    
    // Write the array - do not use a normal string write
    xml.openTag(s);
    xml << output.str();
    xml.closeTag();
  }


  void write(XMLWriter& xml, const std::string& xpath, const std::list<int>& output)
  {
    writeListPrimitive<int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::list<unsigned int>& output)
  {
    writeListPrimitive<unsigned int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::list<short int>& output)
  {
    writeListPrimitive<short int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::list<unsigned short int>& output)
  {
    writeListPrimitive<unsigned short int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::list<long int>& output)
  {
    writeListPrimitive<long int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::list<unsigned long int>& output)
  {
    writeListPrimitive<unsigned long int>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::list<float>& output)
  {
    writeListPrimitive<float>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::list<double>& output)
  {
    writeListPrimitive<double>(xml, xpath, output);
  }
  void write(XMLWriter& xml, const std::string& xpath, const std::list<bool>& output)
  {
    writeListPrimitive<bool>(xml, xpath, output);
  }


  // Write an array of basic types
  void write(XMLWriter& xml, const std::string& xpath, const Array1dO<int>& output)
  {
    writeVectorPrimitive<int>(xml, xpath, output.ref());
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array1dO<unsigned int>& output)
  {
    writeVectorPrimitive<unsigned int>(xml, xpath, output.ref());
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array1dO<short int>& output)
  {
    writeVectorPrimitive<short int>(xml, xpath, output.ref());
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array1dO<unsigned short int>& output)
  {
    writeVectorPrimitive<unsigned short int>(xml, xpath, output.ref());
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array1dO<long int>& output)
  {
    writeVectorPrimitive<long int>(xml, xpath, output.ref());
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array1dO<unsigned long int>& output)
  {
    writeVectorPrimitive<unsigned long int>(xml, xpath, output.ref());
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array1dO<float>& output)
  {
    writeVectorPrimitive<float>(xml, xpath, output.ref());
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array1dO<double>& output)
  {
    writeVectorPrimitive<double>(xml, xpath, output.ref());
  }
  void write(XMLWriter& xml, const std::string& xpath, const Array1dO<bool>& output)
  {
    writeVectorPrimitive<bool>(xml, xpath, output.ref());
  }




  //--------------------------------------------------------------------------------
  // XML writer to a buffer
  XMLBufferWriter::XMLBufferWriter() {indent_level=0;}

  string XMLBufferWriter::str() const
  {
    std::ostringstream s;
  
    writePrologue(s);
    s << output_stream.str();
    s << "\n";

    return s.str();
  }

  string XMLBufferWriter::printRoot() const {return output_stream.str();}

  XMLBufferWriter::~XMLBufferWriter() {}

  //-------------- EXTENSIONS FOR Datalib::Model--------------------
  //! Print buffer onto an ostream
  void XMLBufferWriter::print(std::ostream& os) const
  {
    writePrologue(os);
    os << output_stream.str();
    os << "\n";
  }
  //-------------- END OF EXTENSIONS FOR Datalib::Model--------------------



  //--------------------------------------------------------------------------------
  // XML Writer to a file
  XMLFileWriter::XMLFileWriter() {indent_level=0;}

  void XMLFileWriter::close()
  {
    if (is_open()) 
    {
      output_stream.close();
    }
  }

  // Propagate status to all nodes
  bool XMLFileWriter::is_open()
  {
    return output_stream.is_open();
  }


  // Flush the buffer
  void XMLFileWriter::flush()
  {
    if (is_open()) 
    {
      output_stream.flush();
    }
  }

  // Propagate status to all nodes
  bool XMLFileWriter::fail()
  {
    return output_stream.fail();
  }

  XMLFileWriter::~XMLFileWriter() {close();}



  //--------------------------------------------------------------------------------
  // XML handle class for arrays
  XMLArrayWriter::~XMLArrayWriter()
  {
    if (initP)
      closeArray();
  }

  void XMLArrayWriter::openArray(const string& tagname)
  {
    if (initP)
    {
      cerr << "XMLArrayWriter: calling openArray twice\n";
      exit(1);
    }

    if (arrayTag)
    {
      cerr << "XMLArrayWriter: internal error - array tag already written\n";
      exit(1);
    }

    if (! contextStack.empty())
    {
      cerr << "XMLArrayWriter: context stack not empty\n";
      exit(1);
    }

    qname = tagname;
    elem_qname = "elem";    // for now fix the name - maintains internal consistency

    openTag(qname);   // array tagname

    initP = false;          // not fully initialized yet
    arrayTag = true;
  }

  void XMLArrayWriter::closeArray()
  {
    if (! initP)
    {
      cerr << "XMLArrayWriter: calling closeArray but not initialized\n";
      exit(1);
    }

    if (! contextStack.empty())
    {
      cerr << "XMLArrayWriter: context stack not empty\n";
      exit(1);
    }

    closeTag();   // array tagname

    if (array_size > 0 && elements_written != array_size)
      fprintf(stderr, "XMLArrayWriter: failed to write all the %d required elements: instead = %d",
	      array_size,elements_written);

    initP = arrayTag = false;
    elements_written = 0;
    indent_level = 0;
    simpleElements = false; // do not know this yet
  }

  void XMLArrayWriter::openStruct(const string& tagname)
  {
    if (! arrayTag)
    {
      openArray(tagname);
      return;
    }

    if (! initP)
    {
      if (elements_written == 0)
      {
	// This is the first time this is called
	// From now on, all elements must be STRUCT
	simpleElements = false;
      }
      else
      {
	cerr << "XMLArrayWriter: internal error - data written but state not initialized\n";
	exit(1);
      }

      initP = true;
    }

    if (simpleElements)
    {
      cerr << "XMLArrayWriter: suppose to write simple types but asked to write a struct\n";
      exit(1);
    }


    if (contextStack.empty())
      openTag(elem_qname);   // ignore user provided name and use default name
    else
      openTag(tagname);  // use user provided name

    ElementType el = STRUCT;
    contextStack.push(el);
  }

  void XMLArrayWriter::closeStruct()
  {
    if (! initP)
    {
      cerr << "XMLArrayWriter: calling closeStruct but not initialized\n";
      exit(1);
    }

    if (contextStack.empty())
    {
      closeArray();
      return;
    }

    ElementType topval = contextStack.top();
    if (topval != STRUCT)
    {
      cerr << "XMLArrayWriter: found closeStruct without corresponding openStruct\n";
      exit(1);
    }

    contextStack.pop();

    closeTag();   // struct (or elem_qname)  tagname

    if (contextStack.empty())
    {
      elements_written++;
    }
  }

  // Push a group name
  void push(XMLArrayWriter& xml) {xml.openStruct("");}

  // Pop a group name
  void pop(XMLArrayWriter& xml) {xml.closeStruct();}


}  // end namespace Datalib::Model
