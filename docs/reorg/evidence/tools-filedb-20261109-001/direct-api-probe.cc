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



#if __has_include(<adat/adat_arraynd.h>)
#error Compatibility header is still available
#endif
#if __has_include(<adat/map_obj.h>)
#error Compatibility header is still available
#endif
#if __has_include(<adat/adat_arrays.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/adat_io.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/disco_superb.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/baryon_superb.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/adat_byteorder.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/colorvec_matelem_type.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/corr_storage.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/adat_xmlio.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/genprop4.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/superb_aux.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/superb_options.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/glueball_superb.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/key_val_db.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/prop_superb.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/tetraquark_superb.h>)
#error Compatibility header is still available
#endif
#if __has_include(<io/meson_superb.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/hadron_diagram_time.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/prop_elem_type.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/single_hadron_coeffs.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/corr_graph.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/baryon_elem_type.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/meson_elem_type.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/hadron_class_npart_npt_conn_graph.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/tetraquark_elem_type.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/particle_class_op.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/glueball_elem_type.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/unsmeared_meson_elem.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/hadron_sun_class_npart_version.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/hadron_timeslice.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/unsmeared_twoquark_disco.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/hadron_disp_vertex.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/unsmeared_genprop4_elem.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/hadron_sun_class_npart_irrep_op.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/hadron_sun_class_npart_irrep.h>)
#error Compatibility header is still available
#endif
#if __has_include(<hadron/hadron_adj_map.h>)
#error Compatibility header is still available
#endif
