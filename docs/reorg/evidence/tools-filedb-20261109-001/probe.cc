#include <datalib/codec/binary.h>
#include <datalib/graph/corr_graph.h>
#include <datalib/io/serial_xml.h>
#include <datalib/model/map_obj.h>
#include <datalib/storage/filedb.h>
#include <hadron/cgc_irrep_mom.h>
#include <hadron/cgc_su3.h>
#include <hadron/hadron_sun_class_npart_npt_corr.h>
#include <io/superb_kv.h>

#include <fstream>
#include <iterator>
#include <type_traits>

static_assert(std::is_base_of_v<Hadron::CGCs<Hadron::KeyCGCSU3_t, double>,
                                Hadron::CGCSU3>);

int main(int argc, char** argv)
{
    if (argc != 2)
        return 2;
    Hadron::CorrGraphMap_t graph;
    Datalib::Binary::BinaryFileReader input(argv[1]);
    Hadron::read(input, graph);
    input.close();
    Datalib::Binary::BinaryBufferWriter writer;
    Hadron::write(writer, graph);
    Datalib::Binary::BinaryBufferReader reader(writer.str());
    Hadron::CorrGraphMap_t restored;
    Hadron::read(reader, restored);
    if (restored.version != 4 || restored.int_graph_list.size() != 1)
        return 1;
    Datalib::Serial::XMLBufferWriter xml;
    Hadron::write(xml, "CorrGraph", restored);
    if (xml.str().find("<version>4</version>") == std::string::npos)
        return 1;
    Datalib::Model::MapObject<int, double> values;
    values.insert(3, 4.0);
    Datalib::Storage::SerialDBKey<Hadron::KeyHadronSUNClassNPartNPtCorr_t> key;
    std::cout << "PASS direct datalib graph, XML, container and FILEDB APIs" << std::endl;
}
