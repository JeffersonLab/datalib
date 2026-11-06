// -*- C++ -*-
/*! \file
 * \brief Wrapper over maps
 */

#ifndef DATALIB_map_obj_h
#define DATALIB_map_obj_h

#include <unordered_map>
#include <iterator>
#include <vector>
#include <string>
#include <type_traits>

#include "datalib/codec/superb_options.h"
#include "datalib/codec/binary.h"
#include "datalib/io/serial_xml.h"

namespace Datalib::Model
{
  namespace detail
  {
    template <typename T>
    struct AuxHash {
      std::size_t operator()(const T& t) const
      {
	if constexpr (SB::has_schema<T>::value)
	{
	  return SB::getHash(t);
	}
	else if constexpr (SB::detail::is_std_hashable<T>::value)
	{
	  return std::hash<T>{}(t);
	}
	else
	{
	  Datalib::Binary::BinaryBufferWriter bin;
	  write(bin, t);
	  return std::hash<std::string>{}(bin.str());
	}
      }
    };
  }

  //----------------------------------------------------------------------------
  //! A wrapper over maps
  template<typename K, typename V>
  class MapObject
  {
  public:
    //! Re-export these
    typedef K key_type;
    typedef V mapped_type;
    typedef std::pair<K,V> value_type;

    //! Map type convenience
    typedef std::unordered_map<key_type, mapped_type, detail::AuxHash<key_type>> MapType_t;

    typedef MapType_t::const_iterator const_iterator;

    //! Default constructor
    MapObject() {}

    //! Destructor
    ~MapObject() {}

    //! Exists?
    bool exist(const K& key) const 
    {
      return src_map.count(key) > 0;
    }
			
    //! Clear the object
    void clear() {src_map.clear();}

    //! Erase a key-value
    void erase(const K& key) 
    {
      src_map.erase(key);
    }
			
    //! Insert
    void insert(const K& key, const V& val) 
    {
      this->insert(std::make_pair(key,val));
    }
			
    //! Insert from iterators
    template<typename InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
      for(InputIterator iter = first; iter != last; ++iter)
	this->insert(*iter);
    }
			
    //! Insert
    void insert(const value_type& vv) 
    {
      auto iter = src_map.find(vv.first);
      if (iter != src_map.end())
      {
	iter->second = vv.second;
      }
      else
      {
	src_map.insert(std::make_pair(vv.first, vv.second));
      }
    }
			
    //! Getter
    const V& operator[](const K& key) const 
    {
      const auto iter = src_map.find(key);
      if (iter == src_map.end())
      {
	std::cerr << "MapObject: key not found" << std::endl;
	exit(1);
      }

      return iter->second;
    }
			
    //! Setter
    V& operator[](const K& key) 
    {
      auto iter = src_map.find(key);
      if (iter == src_map.end())
      {
	std::cerr << "MapObject: key not found" << std::endl;
	exit(1);
      }

      return iter->second;
    }
			
    //! The number of elements
    size_t size() const {return src_map.size();}

    //! Dump keys
    std::vector<K> keys() const 
    {
      std::vector<K> keys;
      keys.reserve(src_map.size());
      for (const auto& it : src_map)
      {
	keys.push_back(it.first);
      }
      return keys;
    }

    //! Dump keys and values
    void keysAndValues(std::vector<K>& keys, std::vector<V>& vals) const 
    {
      keys.resize(0);
      keys.reserve(src_map.size());
      vals.resize(0);
      vals.reserve(src_map.size());
      for(const auto& it: src_map)
      {
	keys.push_back(it.first);
	vals.push_back(it.second);
      }
    }

    //! Begin loop over keys
    const_iterator begin() const {return src_map.begin();}

    //! End loop over keys
    const_iterator end() const {return src_map.end();}


  protected:  
    //! Map of objects
    MapType_t src_map;
  };



  //----------------------------------------------------------------------------
  //! Read a MapObject via xml
  template<typename K, typename V>
  inline
  void read(Datalib::Serial::XMLReader& xml, const std::string& s, MapObject<K,V>& input)
  {
    Datalib::Serial::XMLReader arraytop(xml, s);

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
	Datalib::Serial::XMLReader xml_elem(arraytop, element_xpath.str());

	K key;
	V val;

	read(xml_elem, std::string("Key"), key);
	read(xml_elem, std::string("Val"), val);

	input.insert(key, val);
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


  //----------------------------------------------------------------------------
  //! Write a MapObject in xml
  template<typename K, typename V>
  inline
  void write(Datalib::Serial::XMLWriter& xml, const std::string& path, const MapObject<K,V>& param)
  {
    push(xml, path);

    for(typename MapObject<K,V>::const_iterator iter = param.begin(); iter != param.end(); ++iter)
    {
      push(xml, "elem");
      write(xml, "Key", iter->first);
      write(xml, "Val", iter->second);
      pop(xml);
    }

    pop(xml);
  }

} // namespace Datalib::Model

#endif
