/*! \file
 * \brief Time-slice value for graphs
 */

#include "datalib/graph/hadron_timeslice.h"

namespace Hadron
{

  //----------------------------------------------------------------------------
  //! Used for output
  std::ostream& operator<<(std::ostream& os, const ValTimeSlice_t& d)
  {
    os << "ValTimeSlice:\n";
    for(auto mm = d.val.begin(); mm != d.val.end(); ++mm)
    {
      os << "  t=" << mm->first << "  " << mm->second << "\n";
    }

    return os;
  }


  //----------------------------------------------------------------------------
  // Hide this
  namespace
  {
    struct ValTimeSliceArray_t
    {
      int                   t;
      std::complex<double>  v;
    };


    void read(XMLReader& xml, const std::string& path, ValTimeSliceArray_t& param)
    {
      XMLReader paramtop(xml, path);

      read(paramtop, "t", param.t);
      read(paramtop, "v", param.v);
    }

    void write(XMLWriter& xml, const std::string& path, const ValTimeSliceArray_t& param)
    {
      push(xml, path);

      write(xml, "t", param.t);
      write(xml, "v", param.v);
    }
      
  }


  //----------------------------------------------------------------------------
  // Read a key
  void read(XMLReader& xml, const std::string& path, ValTimeSlice_t& param)
  {
    // Read/write the map as an array for XML
    std::vector<ValTimeSliceArray_t> val;
    read(xml, path, val);

    // Convert to a map
    param.val.clear();

    for(int i=0; i < val.size(); ++i)
    {
      param.val.insert(std::make_pair(val[i].t, val[i].v));
    }
  }

  // ValTimeSlice writer
  void write(XMLWriter& xml, const std::string& path, const ValTimeSlice_t& param)
  {
    std::vector<ValTimeSliceArray_t> val;

    for(auto mm = param.val.begin(); mm != param.val.end(); ++mm)
    {
      ValTimeSliceArray_t f;
      f.t = mm->first;
      f.v = mm->second;

      val.push_back(f);
    }

    // Read/write the map as an array for XML
    write(xml, path, val);
  }



  //----------------------------------------------------------------------------
  // Read a key
  void read(BinaryReader& bin, ValTimeSlice_t& param)
  {
    param.val.clear();

    int siz;
    read(bin, siz);

    for(int i=0; i < siz; ++i)
    {
      int                  t;
      std::complex<double> v;

      read(bin, t);
      read(bin, v);

      param.val.insert(std::make_pair(t, v));
    }
  }

  // Write a key
  void write(BinaryWriter& bin, const ValTimeSlice_t& param)
  {
    int siz = param.val.size();
    write(bin, siz);

    for(auto mm = param.val.begin(); mm != param.val.end(); ++mm)
    {
      write(bin, mm->first);
      write(bin, mm->second);
    }
  }

} // namespace Hadron


