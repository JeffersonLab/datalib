/*! \file
 * \brief View a corr_graph bin file
 */
#include <datalib/codec/binary.h>



#include <datalib/io/serial_xml.h>
#include <datalib/model/map_obj.h>


#include <datalib/graph/corr_graph.h>
#include <chrono>
struct GraphTimer {std::chrono::steady_clock::time_point t;double elapsed=0;void reset(){elapsed=0;}void start(){t=std::chrono::steady_clock::now();}void stop(){elapsed+=std::chrono::duration<double>(std::chrono::steady_clock::now()-t).count();}double getTimeInSeconds()const{return elapsed;}};
#include <datalib/graph/filenames.h>

#include <iostream>

namespace Redstar
{
  using namespace XMLArray;

  using namespace Hadron;

  namespace
  {
    //! Error output
    std::ostream& operator<<(std::ostream& os, const XMLArray::Array<int>& d)
    {
      if (d.size() > 0)
      {
	os << d[0];

	for(int i=1; i < d.size(); ++i)
	  os << " " << d[i];
      }

      return os;
    }

    //! Error output
    template<typename T>
    std::ostream& operator<<(std::ostream& os, const std::vector<T>& d)
    {
      if (d.size() > 0)
      {
	os << d[0];

	for(int i=1; i < d.size(); ++i)
	  os << " " << d[i];
      }

      return os;
    }
  }

} // namespace Redstar


//----------------------------------------------------------------------------
using namespace Redstar;

//----------------------------------------------------------------------------
//! Converter

int main(int argc, char *argv[])
{
  if (argc != 2 )
  {
    std::cerr << "Usage:"
	      << argv[0] << ": <input bin file>" << std::endl;
    exit(1);
  }

  try
  {
    // Big structure with the info on the correlators and their graphs
    CorrGraphMap_t corr_graph;

    GraphTimer snarf;
    snarf.reset(); snarf.start();

    std::cout << "\nOpen corr graph bin = " << argv[1] << std::endl;

    // Read the DB
    Datalib::Binary::BinaryFileReader bin(argv[1]);
    read(bin, corr_graph);
    bin.close();

    snarf.stop();
    std::cout << "Time to read from graph bin= " << snarf.getTimeInSeconds() << " secs " << "\n\n";

    std::cout << "Version = " << corr_graph.version << "\n"
	      << "Nt_corr = " << corr_graph.Nt_corr << "\n"
	      << "bc_spec = " << corr_graph.bc_spec << "\n"
	      << "Lattice size = " << corr_graph.layout.latt_size << "\n"
	      << "ensemble= " << corr_graph.ensemble << "\n"
	      << "Number of corrs = " << corr_graph.corr_graph_map.size() << "\n"
	      << "Number of graphs = " << corr_graph.int_graph_list.size() << "\n"
	      << "Number of vertex coeffs = " << corr_graph.vertex_coeff_map.size() << std::endl;

    // View the correlators
    std::cout << "\nHere are the correlators" << std::endl;
    int corr_cnt = 0;

    for(auto corr = corr_graph.corr_graph_map.begin(); corr != corr_graph.corr_graph_map.end(); ++corr)
    {
      // Diagnostic info
      std::cout << "View corr= " << ++corr_cnt << " with ";
      std::cout.width(7);
      std::cout << corr->second.diagrams.size() << "  diagrams ";

      for(int npt = 1; npt <= corr->first.npoint.size(); ++npt)
      {
	std::cout << "  npt[" << npt << "]= " << ensemFileName(corr->first.npoint[npt].irrep);
      }

      std::cout << "\n";

    }
  }
  catch(std::bad_alloc)
  {
    std::cerr << argv[0] << ": caught bad memory allocation" << std::endl;
    exit(1);
  }
  catch(const std::string& e)
  {
    std::cerr << argv[0] << ": Caught Exception: " << e << std::endl;
    exit(1);
  }
  catch(std::exception& e)
  {
    std::cerr << argv[0] << ": Caught standard library exception: " << e.what() << std::endl;
    exit(1);
  }
  catch(...)
  {
    std::cerr << argv[0] << ": caught generic exception" << std::endl;
    exit(1);
  }

  exit(0);
}
