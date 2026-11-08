// -*- C++ -*-
/*! @file
 * @brief XML IO support
 */

#ifndef DATALIB_ADAT_XMLIO_H
#define DATALIB_ADAT_XMLIO_H

#include <string>
#include <sstream>
#include <fstream>
#include <stack>
#include <vector>
#include <map>
#include <complex>

#include "datalib/io/detail/xml_simplewriter.h"
#include "datalib/io/detail/basic_xpath_reader.h"
#include <datalib/model/xml_array.h>
#include "datalib/model/arrays.h"

namespace Datalib::Serial {

  // Namespace composition
  using XMLArray::Array;
  using Detail::Attribute;
  using Detail::AttributeList;
  using Datalib::Model::Array1dO;
    
  // Forward declarations
  class XMLReader;
  class XMLWriter;
  class XMLBufferWriter;
  class XMLFileWriter;
  class XMLArrayWriter;


  /*! @addtogroup io
   *
   * XML File input and output operations on Datalib::Model types
   *
   * @{
   */

  //--------------------------------------------------------------------------------
  //! XML reader class
  
  class XMLReader : protected Detail::BasicXPathReader
  {
  public:
    //! Empty constructor
    XMLReader()  {iop=derived=false;}
    
    //! Construct from contents of file
    /*!
      Opens and reads an XML file.
      \param filename The name of the file.
    */
    XMLReader(const std::string& filename) {
      iop=derived=false;
      open(filename);
    }
    
    //! Construct from contents of stream
    XMLReader(std::istream& is) {
      iop=derived=false;
      open(is);
    }
    
    //! Construct from contents of a XMLBufferWriter
    XMLReader(const XMLBufferWriter& mw) {
      iop=derived=false;
      open(mw);
    }
    
    
    //! Clone and set path -- the new side effect free interface
    XMLReader(XMLReader& old, const std::string& xpath) : Detail::BasicXPathReader() { 
      Detail::BasicXPathReader::open(old, xpath);
      iop = true;
      derived = true;
    }
    
    ~XMLReader() {close();}
    
    
    //! Opens and reads an XML file.
    /*!
      \param filename The name of the file
      \post Any previously opened file is closed.
    */
    void open(const std::string& filename) {
      if (iop)
      {
        std::cerr << "XMLReader already opened: opening for filename=" << filename << std::endl;
        exit(1);
      }
      Detail::BasicXPathReader::open(filename);
      iop = true;
      derived = false;
    }
    
    //! Opens and reads an XML file.
    /*!
      \param id The input stream of the file
      \post Any previously opened file is closed      
    */
    void open(std::istream& is) {
      if (iop)
      {
        std::cerr << "XMLReader already opened: opening for istream" << std::endl;
        exit(1);
      }
      Detail::BasicXPathReader::open(is);
      iop = true;
      derived = false;
    }
    
    //! Reads content of a  XMLBufferWriter
    void open(const XMLBufferWriter& mw);
    
    //! Queries whether the binary file is open
    /*!
      \return true if the binary file is open; false otherwise.
    */
    bool is_open() { return iop; }

    //! Queries whether the XML data has been obtained from another XMLReader
    /*!
      A private method allows this XMLReader to be copy the contents of
      another.
    */
    bool is_derived() const;

    //! Closes the last file opened
    void close() {
      if (is_open()) 
      {
	Detail::BasicXPathReader::close();
	  
	iop = false;
      }
    }
    

    void registerNamespace(const std::string& prefix, const std::string& uri) {
      if( is_open() ) { 
	Detail::BasicXPathReader::registerNamespace(prefix,uri);
      }
    }
    
    /* So should these, there is just a lot of overloading */
    void get(const std::string& s, std::string& result) {
      Detail::BasicXPathReader::get(s, result);
    }
    
    void get(const std::string& s, int& result)  {
      Detail::BasicXPathReader::get(s, result);
    }
    
    void get(const std::string& s, unsigned int& result)  {
      Detail::BasicXPathReader::get(s, result);
    }
    void get(const std::string& s, short int& result) {
      Detail::BasicXPathReader::get(s, result);
    }
    
    void get(const std::string& s, unsigned short int& result)  {
      Detail::BasicXPathReader::get(s, result);
    }
    
    void get(const std::string& s, long int& result)  {
      Detail::BasicXPathReader::get(s, result);
    }
    void get(const std::string& s, unsigned long int& result)  {
      Detail::BasicXPathReader::get(s, result);
    }
    void get(const std::string& s, float& result)  {
      Detail::BasicXPathReader::get(s, result);
    }
    void get(const std::string& s, double& result) {
      Detail::BasicXPathReader::get(s, result);
    }
    
    void get(const std::string& s, bool& result) { 
      Detail::BasicXPathReader::get(s, result);
    }
    
    //! Return the entire contents of the Reader as a stream
    void print(std::ostream& is);
    
    //! Return the root element of the Reader as a stream
    void printRoot(std::ostream& is);
    
    //! Print the current context
    void printCurrentContext(std::ostream& is);
        
    //! Count the number of occurances from the xpath query
    int count(const std::string& xpath);
    
    //-------------- EXTENSIONS FOR Datalib::Model--------------------
    //! Print an element selected by XPath
    void printXPathNode(std::ostream& os, const std::string& xpath_to_node);
    //-------------- END OF EXTENSIONS FOR Datalib::Model--------------------

  private:
    bool  iop;     // file open or closed?
    bool  derived; // is this reader derived from another reader?
  };
  

  // Time to build a telephone book of basic primitives
  void read(XMLReader& xml, const std::string& s, std::string& result);
  void read(XMLReader& xml, const std::string& s, char& result);
  void read(XMLReader& xml, const std::string& s, int& result);
  void read(XMLReader& xml, const std::string& s, unsigned int& result);
  void read(XMLReader& xml, const std::string& s, short int& result);
  void read(XMLReader& xml, const std::string& s, unsigned short int& result);
  void read(XMLReader& xml, const std::string& s, long int& result);
  void read(XMLReader& xml, const std::string& s, unsigned long int& result);
  void read(XMLReader& xml, const std::string& s, float& result); 
  void read(XMLReader& xml, const std::string& s, double& result);
  void read(XMLReader& xml, const std::string& s, bool& result);

  //! Complex reader
  void read(XMLReader& xml, const std::string& s, std::complex<float>& param);
  void read(XMLReader& xml, const std::string& s, std::complex<double>& param);



  //! Read a XML Array element
  template<class T>
  inline
  void read(XMLReader& xml, const std::string& s, XMLArray::Array<T>& input)
  {
    XMLReader arraytop(xml, s);

    std::ostringstream error_message;
    std::string elemName = "elem";
  
    // Count the number of elements
    std::string elem_base_query = elemName;
	
    int array_size;
    try {
      array_size = arraytop.count(elem_base_query);
    }
    catch( const std::string& e) { 
      error_message << "Exception occurred while counting " << elem_base_query 
		    << " during array read " << s << std::endl;
      arraytop.close();
      throw error_message.str();
    }
      
    // Now resize the array to hold the no of elements.
    input.resize(array_size);

    // Get the elements one by one
    for(int i=0; i < input.size(); i++) 
    {
      std::ostringstream element_xpath;

      // Create the query for the element 
      element_xpath << elem_base_query << "[" << (i+1) << "]";

      // recursively try and read the element.
      try {

	read(arraytop, element_xpath.str(), input[i]);

      } 
      catch (const std::string& e) 
      {
	error_message << "Failed to match element " << i
		      << " of array  " << s << "  with query " << element_xpath.str()
		      << std::endl
		      << "Query returned error: " << e;
	arraytop.close();
	throw error_message.str();
      }
    }

    // Arraytop should self destruct but just to be sure.
    arraytop.close();
  }


  // Specialized versions for basic types
  void read(XMLReader& xml, const std::string& s, XMLArray::Array<int>& input);
  void read(XMLReader& xml, const std::string& s, XMLArray::Array<unsigned int>& input);
  void read(XMLReader& xml, const std::string& s, XMLArray::Array<short int>& input);
  void read(XMLReader& xml, const std::string& s, XMLArray::Array<unsigned short int>& input);
  void read(XMLReader& xml, const std::string& s, XMLArray::Array<long int>& input);
  void read(XMLReader& xml, const std::string& s, XMLArray::Array<unsigned long int>& input);
  void read(XMLReader& xml, const std::string& s, XMLArray::Array<float>& input);
  void read(XMLReader& xml, const std::string& s, XMLArray::Array<double>& input);
  void read(XMLReader& xml, const std::string& s, XMLArray::Array<bool>& input);



  //! Read a XML Array element
  template<class T>
  inline
  void read(XMLReader& xml, const std::string& s, std::vector<T>& input)
  {
    XMLReader arraytop(xml, s);

    std::ostringstream error_message;
    std::string elemName = "elem";
  
    // Count the number of elements
    std::string elem_base_query = elemName;
	
    int array_size;
    try {
      array_size = arraytop.count(elem_base_query);
    }
    catch( const std::string& e) { 
      error_message << "Exception occurred while counting " << elem_base_query 
		    << " during array read " << s << std::endl;
      arraytop.close();
      throw error_message.str();
    }
      
    // Now resize the array to hold the no of elements.
    input.resize(array_size);

    // Get the elements one by one
    for(int i=0; i < input.size(); i++) 
    {
      std::ostringstream element_xpath;

      // Create the query for the element 
      element_xpath << elem_base_query << "[" << (i+1) << "]";

      // recursively try and read the element.
      try {
	read(arraytop, element_xpath.str(), input[i]);
      } 
      catch (const std::string& e) 
      {
	error_message << "Failed to match element " << i
		      << " of array  " << s << "  with query " << element_xpath.str()
		      << std::endl
		      << "Query returned error: " << e;
	arraytop.close();
	throw error_message.str();
      }
    }

    // Arraytop should self destruct but just to be sure.
    arraytop.close();
  }


  // Specialized versions for basic types
  void read(XMLReader& xml, const std::string& s, std::vector<int>& input);
  void read(XMLReader& xml, const std::string& s, std::vector<unsigned int>& input);
  void read(XMLReader& xml, const std::string& s, std::vector<short int>& input);
  void read(XMLReader& xml, const std::string& s, std::vector<unsigned short int>& input);
  void read(XMLReader& xml, const std::string& s, std::vector<long int>& input);
  void read(XMLReader& xml, const std::string& s, std::vector<unsigned long int>& input);
  void read(XMLReader& xml, const std::string& s, std::vector<float>& input);
  void read(XMLReader& xml, const std::string& s, std::vector<double>& input);
//  void read(XMLReader& xml, const std::string& s, std::vector<bool>& input);  // does not seem to exist


  //! Read a XML Array element
  template<class T>
  inline
  void read(XMLReader& xml, const std::string& s, std::list<T>& input)
  {
    XMLReader arraytop(xml, s);

    std::ostringstream error_message;
    std::string elemName = "elem";
  
    // Count the number of elements
    std::string elem_base_query = elemName;
	
    int array_size;
    try {
      array_size = arraytop.count(elem_base_query);
    }
    catch( const std::string& e) { 
      error_message << "Exception occurred while counting " << elem_base_query 
		    << " during array read " << s << std::endl;
      arraytop.close();
      throw error_message.str();
    }
      
    // Clear out the original list
    input.clear();

    // Get the elements one by one
    for(int i=0; i < input.size(); i++) 
    {
      std::ostringstream element_xpath;

      // Create the query for the element 
      element_xpath << elem_base_query << "[" << (i+1) << "]";

      // recursively try and read the element.
      try {
	T thingy;
	read(arraytop, element_xpath.str(), thingy);

	input.push_back(thingy);
      } 
      catch (const std::string& e) 
      {
	error_message << "Failed to match element " << i
		      << " of array  " << s << "  with query " << element_xpath.str()
		      << std::endl
		      << "Query returned error: " << e;
	arraytop.close();
	throw error_message.str();
      }
    }

    // Arraytop should self destruct but just to be sure.
    arraytop.close();
  }


  // Specialized versions for basic types
  void read(XMLReader& xml, const std::string& s, std::list<int>& input);
  void read(XMLReader& xml, const std::string& s, std::list<unsigned int>& input);
  void read(XMLReader& xml, const std::string& s, std::list<short int>& input);
  void read(XMLReader& xml, const std::string& s, std::list<unsigned short int>& input);
  void read(XMLReader& xml, const std::string& s, std::list<long int>& input);
  void read(XMLReader& xml, const std::string& s, std::list<unsigned long int>& input);
  void read(XMLReader& xml, const std::string& s, std::list<float>& input);
  void read(XMLReader& xml, const std::string& s, std::list<double>& input);



  //! Read a XML Array1dO element
  template<class T>
  inline
  void read(XMLReader& xml, const std::string& s, Datalib::Model::Array1dO<T>& input)
  {
    XMLReader arraytop(xml, s);

    std::ostringstream error_message;
    std::string elemName = "elem";
  
    // Count the number of elements
    std::string elem_base_query = elemName;
	
    int array_size;
    try {
      array_size = arraytop.count(elem_base_query);
    }
    catch( const std::string& e) { 
      error_message << "Exception occurred while counting " << elem_base_query 
		    << " during array read " << s << std::endl;
      arraytop.close();
      throw error_message.str();
    }
      
    // Now resize the array to hold the no of elements.
    input.resize(array_size);

    // Get the elements one by one
    for(int i=1; i <= input.size(); i++) 
    {
      std::ostringstream element_xpath;

      // Create the query for the element 
      element_xpath << elem_base_query << "[" << i << "]";

      // recursively try and read the element.
      try {
	read(arraytop, element_xpath.str(), input[i]);
      } 
      catch (const std::string& e) 
      {
	error_message << "Failed to match element " << i
		      << " of array  " << s << "  with query " << element_xpath.str()
		      << std::endl
		      << "Query returned error: " << e;
	arraytop.close();
	throw error_message.str();
      }
    }

    // Arraytop should self destruct but just to be sure.
    arraytop.close();
  }


  // Specialized versions for basic types
  void read(XMLReader& xml, const std::string& s, Datalib::Model::Array1dO<int>& input);
  void read(XMLReader& xml, const std::string& s, Datalib::Model::Array1dO<unsigned int>& input);
  void read(XMLReader& xml, const std::string& s, Datalib::Model::Array1dO<short int>& input);
  void read(XMLReader& xml, const std::string& s, Datalib::Model::Array1dO<unsigned short int>& input);
  void read(XMLReader& xml, const std::string& s, Datalib::Model::Array1dO<long int>& input);
  void read(XMLReader& xml, const std::string& s, Datalib::Model::Array1dO<unsigned long int>& input);
  void read(XMLReader& xml, const std::string& s, Datalib::Model::Array1dO<float>& input);
  void read(XMLReader& xml, const std::string& s, Datalib::Model::Array1dO<double>& input);
//  void read(XMLReader& xml, const std::string& s, Datalib::Model::Array1dO<bool>& input);  // does not seem to exist


  //! Read a XML Array element
  template<typename K, typename V>
  inline
  void read(XMLReader& xml, const std::string& s, std::map<K,V>& input)
  {
    XMLReader arraytop(xml, s);

    std::ostringstream error_message;
    std::string elemName = "elem";
  
    int array_size;
    try {
      array_size = arraytop.count(elemName);
    }
    catch( const std::string& e) { 
      error_message << "Exception occurred while counting " << elemName
		    << " during array read " << s << std::endl;
      arraytop.close();
      throw error_message.str();
    }
      
    // Get the elements one by one
    for(int i=0; i < array_size; i++) 
    {
      std::ostringstream element_xpath;

      // Create the query for the element 
      element_xpath << elemName << "[" << (i+1) << "]";

      // recursively try and read the element.
      try {
	XMLReader xml_elem(arraytop, element_xpath.str());

	K key;
	V val;

	read(xml_elem, "Key", key);
	read(xml_elem, "Val", val);

	input.insert(std::make_pair(key, val));
      } 
      catch (const std::string& e) 
      {
	error_message << "Failed to match element " << i
		      << " of array  " << s << "  with query " << element_xpath.str()
		      << std::endl
		      << "Query returned error: " << e;
	arraytop.close();
	throw error_message.str();
      }
    }

    // Arraytop should self destruct but just to be sure.
    arraytop.close();
  }



  //! Read a XML Array element
  template<typename T1, typename T2>
  inline
  void read(XMLReader& xml, const std::string& s, std::pair<T1,T2>& input)
  {
    XMLReader paramtop(xml, s);

    read(paramtop, "First", input.first);
    read(paramtop, "Second", input.second);
  }



  //--------------------------------------------------------------------------------
  //! Metadata output class
  class XMLWriter : protected Detail::XMLSimpleWriter
  {
  public:
    XMLWriter();
    ~XMLWriter();

    virtual void openSimple(const std::string& tagname);
    virtual void closeSimple();

    virtual void openStruct(const std::string& tagname);
    virtual void closeStruct();

    void openTag(const std::string& tagname);
    void openTag(const std::string& nsprefix, const std::string& tagname);
    void openTag(const std::string& tagname, AttributeList& al);

    void openTag(const std::string& nsprefix,
		 const std::string& tagname, 
		 AttributeList& al);

    void closeTag();

    void emptyTag(const std::string& tagname);
    void emptyTag(const std::string& nsprefix, const std::string& tagname);
    void emptyTag(const std::string& tagname, AttributeList& al);

    void emptyTag(const std::string& nsprefix,
		  const std::string& tagname, 
		  AttributeList& al);
    

    // Overloaded Writer Functions
    void write(const std::string& output);
    void write(const int& output);
    void write(const unsigned int& output);
    void write(const short int& output);
    void write(const unsigned short int& output);
    void write(const long int& output);
    void write(const unsigned long int& output);
    void write(const float& output);
    void write(const double& output);
    void write(const bool& output);

    // Write XML std::string
    void writeXML(const std::string& output);

    friend class XMLArrayWriter;
  };


  //! Push a group name
  void push(XMLWriter& xml, const std::string& s);

  //! Pop a group name
  void pop(XMLWriter& xml);

  //! Write something from a reader
  void write(XMLWriter& xml, const std::string& s, const XMLReader& d);
  XMLWriter& operator<<(XMLWriter& xml, const XMLReader& d);

  //! Write something from a XMLBufferWriter
  void write(XMLWriter& xml, const std::string& s, const XMLBufferWriter& d);
  XMLWriter& operator<<(XMLWriter& xml, const XMLBufferWriter& d);

  // Time to build a telephone book of basic primitives
  void write(XMLWriter& xml, const std::string& s, const std::string& output);
  void write(XMLWriter& xml, const std::string& s, const char* output);
  void write(XMLWriter& xml, const std::string& s, const char& output);
  void write(XMLWriter& xml, const std::string& s, const int& output);
  void write(XMLWriter& xml, const std::string& s, const unsigned int& output);
  void write(XMLWriter& xml, const std::string& s, const short int& output);
  void write(XMLWriter& xml, const std::string& s, const unsigned short int& output);
  void write(XMLWriter& xml, const std::string& s, const long int& output);
  void write(XMLWriter& xml, const std::string& s, const unsigned long int& output);
  void write(XMLWriter& xml, const std::string& s, const float& output);
  void write(XMLWriter& xml, const std::string& s, const double& output);
  void write(XMLWriter& xml, const std::string& s, const bool& output);
 
  //! Complex reader
  void write(XMLWriter& xml, const std::string& s, const std::complex<float>& param);
  void write(XMLWriter& xml, const std::string& s, const std::complex<double>& param);

  // Versions that do not print a name
  XMLWriter& operator<<(XMLWriter& xml, const std::string& output);
  XMLWriter& operator<<(XMLWriter& xml, const char* output);
  XMLWriter& operator<<(XMLWriter& xml, const char& output);
  XMLWriter& operator<<(XMLWriter& xml, const int& output);
  XMLWriter& operator<<(XMLWriter& xml, const unsigned int& output);
  XMLWriter& operator<<(XMLWriter& xml, const short int& output);
  XMLWriter& operator<<(XMLWriter& xml, const unsigned short int& output);
  XMLWriter& operator<<(XMLWriter& xml, const long int& output);
  XMLWriter& operator<<(XMLWriter& xml, const unsigned long int& output);
  XMLWriter& operator<<(XMLWriter& xml, const float& output);
  XMLWriter& operator<<(XMLWriter& xml, const double& output);
  XMLWriter& operator<<(XMLWriter& xml, const bool& output);

  //! Write a XML Array element
  template<class T>
  inline
  void write(XMLWriter& xml, const std::string& s, const XMLArray::Array<T>& s1)
  {
    // Write the array name
    xml.openTag(s);

    for(unsigned index=0; index < s1.size(); index++) 
    {
      write(xml, "elem", s1[index]);  // Possibly grab user defines here
    }

    xml.closeTag(); // Array name
  }


  // Writers for arrays of basic types
  void write(XMLWriter& xml, const std::string& s, const XMLArray::Array<int>& output);
  void write(XMLWriter& xml, const std::string& s, const XMLArray::Array<unsigned int>& output);
  void write(XMLWriter& xml, const std::string& s, const XMLArray::Array<short int>& output);
  void write(XMLWriter& xml, const std::string& s, const XMLArray::Array<unsigned short int>& output);
  void write(XMLWriter& xml, const std::string& s, const XMLArray::Array<long int>& output);
  void write(XMLWriter& xml, const std::string& s, const XMLArray::Array<unsigned long int>& output);
  void write(XMLWriter& xml, const std::string& s, const XMLArray::Array<float>& output);
  void write(XMLWriter& xml, const std::string& s, const XMLArray::Array<double>& output);
  void write(XMLWriter& xml, const std::string& s, const XMLArray::Array<bool>& output);


  //! Write a XML vector element
  template<class T>
  inline
  void write(XMLWriter& xml, const std::string& s, const std::vector<T>& s1)
  {
    // Write the array name
    xml.openTag(s);

    for(unsigned index=0; index < s1.size(); index++) 
    {
      write(xml, "elem", s1[index]);  // Possibly grab user defines here
    }

    xml.closeTag(); // Array name
  }


  // Writers for arrays of basic types
  void write(XMLWriter& xml, const std::string& s, const std::vector<int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::vector<unsigned int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::vector<short int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::vector<unsigned short int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::vector<long int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::vector<unsigned long int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::vector<float>& output);
  void write(XMLWriter& xml, const std::string& s, const std::vector<double>& output);
  void write(XMLWriter& xml, const std::string& s, const std::vector<bool>& output);


  //! Write a XML vector element
  template<class T>
  inline
  void write(XMLWriter& xml, const std::string& s, const std::list<T>& s1)
  {
    // Write the array name
    xml.openTag(s);

    for(typename std::list<T>::const_iterator p = s1.begin(); p != s1.end(); ++p)
    {
      write(xml, "elem", *p);  // Possibly grab user defines here
    }

    xml.closeTag(); // Array name
  }


  // Writers for arrays of basic types
  void write(XMLWriter& xml, const std::string& s, const std::list<int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::list<unsigned int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::list<short int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::list<unsigned short int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::list<long int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::list<unsigned long int>& output);
  void write(XMLWriter& xml, const std::string& s, const std::list<float>& output);
  void write(XMLWriter& xml, const std::string& s, const std::list<double>& output);
  void write(XMLWriter& xml, const std::string& s, const std::list<bool>& output);


  //! Write a XML Array1dO element
  template<class T>
  inline
  void write(XMLWriter& xml, const std::string& s, const Datalib::Model::Array1dO<T>& s1)
  {
    write(xml, s, s1.ref());
  }


  // Writers for arrays of basic types
  void write(XMLWriter& xml, const std::string& s, const Datalib::Model::Array1dO<int>& output);
  void write(XMLWriter& xml, const std::string& s, const Datalib::Model::Array1dO<unsigned int>& output);
  void write(XMLWriter& xml, const std::string& s, const Datalib::Model::Array1dO<short int>& output);
  void write(XMLWriter& xml, const std::string& s, const Datalib::Model::Array1dO<unsigned short int>& output);
  void write(XMLWriter& xml, const std::string& s, const Datalib::Model::Array1dO<long int>& output);
  void write(XMLWriter& xml, const std::string& s, const Datalib::Model::Array1dO<unsigned long int>& output);
  void write(XMLWriter& xml, const std::string& s, const Datalib::Model::Array1dO<float>& output);
  void write(XMLWriter& xml, const std::string& s, const Datalib::Model::Array1dO<double>& output);
  void write(XMLWriter& xml, const std::string& s, const Datalib::Model::Array1dO<bool>& output);


  //! Write a map
  template<typename K, typename V>
  inline
  void write(XMLWriter& xml, const std::string& path, const std::map<K,V>& param)
  {
    push(xml, path);

    for(typename std::map<K,V>::const_iterator pp = param.begin(); 
	pp != param.end(); 
	++pp)
    {
      push(xml, "elem");
      write(xml, "Key", pp->first);
      write(xml, "Val", pp->second);
      pop(xml);
    }

    pop(xml);
  }


  //! Write a pair
  template<typename T1, typename T2>
  inline
  void write(XMLWriter& xml, const std::string& path, const std::pair<T1,T2>& param)
  {
    push(xml, path);

    write(xml, "First", param.first);
    write(xml, "Second", param.second);

    pop(xml);
  }


  //--------------------------------------------------------------------------------
  //! Write metadata to a buffer
  class XMLBufferWriter : public XMLWriter
  {
  public:
    //! Constructor
    /*! No prologue written */
    XMLBufferWriter();
  
    //! Destructor
    ~XMLBufferWriter();

    // Return entire stream as a string
    std::string str() const;
        
    // Return root element as a string
    std::string printRoot() const;
        
    //-------------- EXTENSIONS FOR Datalib::Model--------------------
    //! Print buffer onto an ostream
    void print(std::ostream& os) const;
    //-------------- END OF EXTENSIONS FOR Datalib::Model--------------------

  private:
    // The output stream...
    std::ostringstream output_stream;

    // The function that supplies the stream to the parent...
    std::ostream& getOstream(void) {return output_stream;}
  };


  //--------------------------------------------------------------------------------
  //! Write data to a file
  class XMLFileWriter : public XMLWriter
  {
  public:
    //! Empty constructor
    XMLFileWriter();

    //! Constructor from a filename
    explicit XMLFileWriter(const std::string& filename, bool write_prologue=true)
      {
	open(filename, write_prologue);
      }

    //! Destructor
    ~XMLFileWriter();

    bool is_open();
    void open(const std::string& filename, bool write_prologue=true)
      {
	output_stream.open(filename.c_str(), std::ofstream::out);
	if (write_prologue)
	  writePrologue(output_stream);

	indent_level=0;
      }

    //! Flush the buffer
    void flush();

    //! Return true if some failure occurred in previous IO operation
    bool fail();

    //! Close the file
    void close();
        
  private:
    std::ofstream output_stream;
    std::ostream& getOstream(void) {return output_stream;}
  };



  //--------------------------------------------------------------------------------
  //! Writes metadata to an array which serves as a handle for another XML object
  class XMLArrayWriter : public XMLWriter
  {
  public:

    /*! No prologue written
     * @param xml_out  previous XMLWriter object - used for handle source
     * @param size     size of array - default unbounded
     */
    explicit XMLArrayWriter(XMLWriter& xml_out, int size = -1) : 
      output_xml(xml_out), array_size(size)
      {
	initP = arrayTag = false;
	elements_written = 0;
	indent_level = xml_out.indent_level;
	simpleElements = false; // do not know this yet
      }
  

    ~XMLArrayWriter();

    // Flush the buffer
    //  void flush();

    //! Closes the array writer
    /*! It is an error to close before all data is written, unless unbounded */
    void close();
       
    //! Returns the size of the array
    int size() const {return array_size;}

    void openArray(const std::string& tagname);
    void closeArray();

    //  virtual void openSimple(const std::string& tagname);
    //  virtual void closeSimple();

    void openStruct(const std::string& tagname);
    void closeStruct();

  private:
    std::string qname;
    std::string elem_qname;

    bool arrayTag;         // set once array tag is written
    bool initP;            // set once we know how the array is composed
    bool simpleElements;   // true if elements will all be simple types

    // output stream is actually the original stream
    XMLWriter& output_xml; 

    int array_size;        // total array element size
    int elements_written;  // elements written so far

    // A stack to hold context.
    enum ElementType {SIMPLE, STRUCT};
    std::stack<ElementType> contextStack;

    std::ostream& getOstream(void) {return output_xml.getOstream();}
  };

  //! Push a group name
  void push(XMLArrayWriter& xml);

  //! Pop a group name
  void pop(XMLArrayWriter& xml);

  /*! @} */   // end of group io

}  // namespace Datalib::Model

#endif
