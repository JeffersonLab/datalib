/*! Shared version-4 no-Stich graph format, used by Redstar and redstar-datalib. */
#include "datalib/graph/corr_graph.h"
#include <stdexcept>
namespace Hadron {
  //! Read a MapObject via binary
  template<typename K, typename V>
  inline
  void read(BinaryReader& bin, MapObject<K,V>& input)
  {
    input.clear();

    int array_size;
    read(bin, array_size);
      
    // Get the elements one by one
    for(int i=0; i < array_size; i++) 
    {
      K key;
      V val;

      read(bin, key);
      read(bin, val);

      input.insert(key, val);
    } 
  }


  //----------------------------------------------------------------------------
  //! Write a MapObject in binary
  template<typename K, typename V>
  inline
  void write(BinaryWriter& bin, const MapObject<K,V>& param)
  {
    int array_size = param.size();
    write(bin, array_size);

    for(auto iter = param.begin(); iter != param.end(); ++iter)
    {
      write(bin, iter->first);
      write(bin, iter->second);
    }
  }


namespace {
void requireVersion(int version) {
  if(version!=CorrGraphFormatVersion)
    throw std::runtime_error("Unsupported corr graph version " + std::to_string(version) +
      "; regenerate with redstar_corr_graph (version " + std::to_string(CorrGraphFormatVersion) + ").");
}
}
void read(XMLReader& xml,const std::string& path,CorrGraphLayout_t& value) {
 XMLReader in(xml,path);read(in,"lattSize",value.latt_size);
 read(in,in.count("decayDir")?"decayDir":"decay_dir",value.decay_dir);
}
void write(XMLWriter& xml,const std::string& path,const CorrGraphLayout_t& value) {
 push(xml,path);write(xml,"decayDir",value.decay_dir);write(xml,"lattSize",value.latt_size);pop(xml);
}
void read(BinaryReader& bin,CorrGraphLayout_t& value) {read(bin,value.decay_dir);read(bin,value.latt_size);}
void write(BinaryWriter& bin,const CorrGraphLayout_t& value) {write(bin,value.decay_dir);write(bin,value.latt_size);}
  void read(XMLReader& xml, const std::string& path, ValCorrGraphMap_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "quark_count", param.quark_count);
    read(paramtop, "diagrams", param.diagrams);
  }

  // Write a corr graph map
  void write(XMLWriter& xml, const std::string& path, const ValCorrGraphMap_t& param)
  {
    push(xml, path);

    write(xml, "quark_count", param.quark_count);
    write(xml, "diagrams", param.diagrams);

    pop(xml);
  }

  //----------------------------------------------------------------------------
  void read(BinaryReader& bin, ValCorrGraphMap_t& param)
  {
    read(bin, param.quark_count);
    read(bin, param.diagrams);
  }

  void write(BinaryWriter& bin, const ValCorrGraphMap_t& param)
  {
    write(bin, param.quark_count);
    write(bin, param.diagrams);
  }
  void read(XMLReader& xml, const std::string& path, ValCorrIntGraphMap_t& param)
  {
    XMLReader paramtop(xml, path);

    read(paramtop, "quark_count", param.quark_count);
    read(paramtop, "diagrams", param.diagrams);
  }

  // Write a corr graph map
  void write(XMLWriter& xml, const std::string& path, const ValCorrIntGraphMap_t& param)
  {
    push(xml, path);

    write(xml, "quark_count", param.quark_count);
    write(xml, "diagrams", param.diagrams);

    pop(xml);
  }

  //----------------------------------------------------------------------------
  void read(BinaryReader& bin, ValCorrIntGraphMap_t& param)
  {
    read(bin, param.quark_count);
    read(bin, param.diagrams);
  }

  void write(BinaryWriter& bin, const ValCorrIntGraphMap_t& param)
  {
    write(bin, param.quark_count);
    write(bin, param.diagrams);
  }
void read(XMLReader& xml,const std::string& path,CorrGraphMap_t& value) {
 XMLReader in(xml,path);read(in,"version",value.version);requireVersion(value.version);
 read(in,"Nt_corr",value.Nt_corr);read(in,"bc_spec",value.bc_spec);read(in,"layout",value.layout);
 read(in,"ensemble",value.ensemble);read(in,"CorrGraphMap",value.corr_graph_map);
 read(in,"GraphList",value.int_graph_list);read(in,"VertexCoeffMap",value.vertex_coeff_map);
}
void write(XMLWriter& xml,const std::string& path,const CorrGraphMap_t& value) {
 requireVersion(value.version);push(xml,path);write(xml,"version",CorrGraphFormatVersion);
 write(xml,"Nt_corr",value.Nt_corr);write(xml,"bc_spec",value.bc_spec);write(xml,"layout",value.layout);
 write(xml,"ensemble",value.ensemble);write(xml,"CorrGraphMap",value.corr_graph_map);
 write(xml,"GraphList",value.int_graph_list);write(xml,"VertexCoeffMap",value.vertex_coeff_map);pop(xml);
}
void read(BinaryReader& bin,CorrGraphMap_t& value) {
 read(bin,value.version);requireVersion(value.version);read(bin,value.Nt_corr);read(bin,value.bc_spec);
 read(bin,value.layout);readDesc(bin,value.ensemble);read(bin,value.corr_graph_map);
 read(bin,value.int_graph_list);read(bin,value.vertex_coeff_map);
}
void write(BinaryWriter& bin,const CorrGraphMap_t& value) {
 requireVersion(value.version);write(bin,CorrGraphFormatVersion);write(bin,value.Nt_corr);write(bin,value.bc_spec);
 write(bin,value.layout);writeDesc(bin,value.ensemble);write(bin,value.corr_graph_map);
 write(bin,value.int_graph_list);write(bin,value.vertex_coeff_map);
}
}
