#include "datalib/graph/corr_graph.h"

#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace Binary = Datalib::Binary;
namespace Model = Datalib::Model;
namespace Serial = Datalib::Serial;

void require(bool condition, const char* message)
{
  if (!condition)
    throw std::runtime_error(message);
}

template<class Graph>
std::string encode(const Graph& graph)
{
  Binary::BinaryBufferWriter writer;
  write(writer, graph);
  return writer.str();
}

template<class Graph>
void rejects(const std::string& bytes, const char* diagnostic = nullptr)
{
  Binary::BinaryBufferReader reader(bytes);
  Graph graph;
  try
  {
    read(reader, graph);
  }
  catch (const std::exception& error)
  {
    if (diagnostic)
      require(std::string(error.what()).find(diagnostic) != std::string::npos,
              "Unexpected graph rejection diagnostic");
    return;
  }
  throw std::runtime_error("Invalid graph was accepted");
}

std::string load(const std::string& path);

template<class Value>
std::string canonical(const Value& value)
{
  return encode(value);
}

std::string canonical(const Hadron::ValCorrIntGraphMap_t& value);

template<class Key, class Value>
std::string canonical(const Model::MapObject<Key, Value>& values)
{
  std::map<std::string, std::string> sorted;
  for (const auto& entry : values)
    sorted.emplace(encode(entry.first), canonical(entry.second));

  Binary::BinaryBufferWriter writer;
  Binary::write(writer, static_cast<int>(sorted.size()));
  for (const auto& entry : sorted)
  {
    Binary::writeDesc(writer, entry.first);
    Binary::writeDesc(writer, entry.second);
  }
  return writer.str();
}

std::string canonical(const Hadron::ValCorrIntGraphMap_t& value)
{
  return encode(value.quark_count) + canonical(value.diagrams);
}

std::string encodeDescription(const std::string& text)
{
  Binary::BinaryBufferWriter writer;
  Binary::writeDesc(writer, text);
  return writer.str();
}

template<class Graph>
std::string canonicalGraph(const Graph& graph)
{
  return encode(graph.version) + encode(graph.Nt_corr) + encode(graph.bc_spec)
      + encode(graph.layout) + encodeDescription(graph.ensemble) + canonical(graph.corr_graph_map)
      + encode(graph.int_graph_list) + canonical(graph.vertex_coeff_map);
}

template<class Graph>
void checkGraph(const std::string& bytes, int version, const std::string& legacy)
{
  Graph graph;
  Binary::BinaryBufferReader reader(bytes);
  read(reader, graph);
  require(graph.version == version, "Version changed");
  require(graph.layout.latt_size == std::vector<int>({4, 4, 4, 16}), "Layout changed");
  require(graph.int_graph_list.size() == 1, "Missing pion graph");
  require(graph.corr_graph_map.size() > 0, "Missing correlation keys");
  require(graph.vertex_coeff_map.size() > 0, "Missing vertex coefficients");

  require(encode(graph) == load(legacy + ".bin"), "Legacy binary writer output changed");

  Serial::XMLBufferWriter xml;
  write(xml, "CorrGraph", graph);
  require(xml.str() == load(legacy + ".xml"), "Legacy XML writer output changed");
  std::istringstream input(xml.str());
  Serial::XMLReader xml_reader(input);
  Graph roundtrip;
  read(xml_reader, "/CorrGraph", roundtrip);
  const auto legacy_xml_bytes = load(legacy + ".xml.bin");
  Binary::BinaryBufferReader legacy_xml_reader(legacy_xml_bytes);
  Graph legacy_xml_graph;
  read(legacy_xml_reader, legacy_xml_graph);
  require(canonicalGraph(roundtrip) == canonicalGraph(legacy_xml_graph),
          "Legacy XML reader result changed");
  if (canonicalGraph(legacy_xml_graph) != canonicalGraph(graph))
    std::cout << "Format " << version << " retains legacy XML precision/representation differences\n";

  rejects<Graph>(bytes.substr(0, bytes.size() - 1));
  rejects<Graph>(bytes.substr(0, 3));
  std::cout << "PASS format " << version << " legacy writer bytes and XML roundtrip\n";
}

std::string load(const std::string& path)
{
  std::ifstream file(path, std::ios::binary);
  require(bool(file), "Cannot open frozen graph");
  return {std::istreambuf_iterator<char>(file), {}};
}

int main(int argc, char** argv)
{
  require(argc == 2, "Pass the frozen fixture directory");
  const std::string dir = argv[1];
  const auto v3 = load(dir + "/graph-v3.bin");
  const auto v4 = load(dir + "/graph-v4.bin");
  checkGraph<Hadron::CorrGraphMap_t>(v4, 4, dir + "/legacy-v4");
  rejects<Hadron::CorrGraphMap_t>(v3, "Unsupported corr graph version 3");

  std::istringstream legacy_xml(load(dir + "/legacy-v3.xml"));
  Serial::XMLReader reader(legacy_xml);
  Hadron::CorrGraphMap_t graph;
  try
  {
    read(reader, "/CorrGraph", graph);
  }
  catch (const std::exception& error)
  {
    require(std::string(error.what()).find("Unsupported corr graph version 3") != std::string::npos,
            "Unexpected XML rejection diagnostic");
    return 0;
  }
  throw std::runtime_error("Format-3 XML graph was accepted");
}
