/*! \file
 * \brief Read a corr_graph bin file and extract/dump xml pieces from it
 */
#include <datalib/codec/binary.h>



#include <datalib/io/serial_xml.h>
#include <datalib/model/map_obj.h>




#include <datalib/graph/corr_graph.h>
#include <chrono>
struct GraphTimer {std::chrono::steady_clock::time_point t;double elapsed=0;void reset(){elapsed=0;}void start(){t=std::chrono::steady_clock::now();}void stop(){elapsed+=std::chrono::duration<double>(std::chrono::steady_clock::now()-t).count();}double getTimeInSeconds()const{return elapsed;}};

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
  if (argc != 3 )
  {
    std::cerr << "Usage:"
	      << argv[0] << ": <input bin file> <output XML file>" << std::endl;
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
    std::cout << "Time to read number of corrs = " << corr_graph.corr_graph_map.size()
	      << "  from corr graph bin= " << snarf.getTimeInSeconds() << " secs " << "\n";

    // Write the XML
    std::cout << "\nWrite xml of corr graph map = " << argv[2] << std::endl;

    snarf.reset(); snarf.start();

    Datalib::Serial::XMLFileWriter xml_corr(argv[2]);
    write(xml_corr, "Correlators", corr_graph);
    xml_corr.close();

    snarf.stop();
    std::cout << "Time to write number of corrs = " << corr_graph.corr_graph_map.size()
	      << "  into corr graph xml= " << snarf.getTimeInSeconds() << " secs " << "\n";

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
