#include <cstdlib>
#include <datalib/io/serial_xml.h>
#include <mpi.h>
#include <omp.h>
#include <qdp.h>
#include <qmp.h>
#include <sstream>
#include <stdexcept>
#include <vector>

int main(int argc, char **argv) {
  const int expected = argc > 1 ? std::stoi(argv[1]) : 1;
  QDP::QDP_initialize(&argc, &argv);
  try {
    int world = 0, rank = -1, thread_level = -1;
    MPI_Comm_size(MPI_COMM_WORLD, &world);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Query_thread(&thread_level);
    if (world != expected || QMP_get_number_of_nodes() != world ||
        QMP_get_node_number() != rank)
      throw std::runtime_error("QMP and MPI rank geometry disagree");
    QDP::multi1d<int> lattice(4);
    lattice[0] = lattice[1] = lattice[2] = 4;
    lattice[3] = 16;
    QDP::Layout::setLattSize(lattice);
    QDP::Layout::create();
    {
      QDP::LatticeReal ones = QDP::Real(1);
      const double total = QDP::toDouble(QDP::sum(ones));
      if (total != 1024 || QDP::Layout::sitesOnNode() * world != 1024)
        throw std::runtime_error("Distributed lattice reduction failed");

      // Only the primary node receives valid QDP XML; the read must broadcast.
      std::istringstream collective_input(
          QDP::Layout::primaryNode() ? "<Boundary><value>72</value></Boundary>"
                                     : "INVALID_NONPRIMARY_XML");
      QDP::XMLReader collective(collective_input);
      QDP::XMLReader collective_child(collective, "/Boundary");
      int broadcast_value = -1;
      QDP::read(collective_child, "value", broadcast_value);
      if (broadcast_value != 72)
        throw std::runtime_error("QDP XML did not broadcast the primary value");

      // Datalib serial XML must preserve distinct per-rank scalars and arrays.
      Datalib::Serial::XMLBufferWriter writer;
      Datalib::Serial::push(writer, "Boundary");
      Datalib::Serial::write(writer, "value", rank + 37);
      Datalib::Serial::write(writer, "items",
                             std::vector<int>{rank + 1, rank + 2});
      Datalib::Serial::pop(writer);
      std::istringstream local_input(writer.str());
      Datalib::Serial::XMLReader local(local_input);
      Datalib::Serial::XMLReader local_child(local, "/Boundary");
      int local_value = -1;
      std::vector<int> local_array;
      Datalib::Serial::read(local_child, "value", local_value);
      Datalib::Serial::read(local_child, "items", local_array);
      if (local_value != rank + 37 ||
          local_array != std::vector<int>{rank + 1, rank + 2})
        throw std::runtime_error(
            "Datalib serial XML broadcast or changed local data");
      std::vector<int> local_values(world);
      MPI_Allgather(&local_value, 1, MPI_INT, local_values.data(), 1, MPI_INT,
                    MPI_COMM_WORLD);
      int threads = 0;
#pragma omp parallel
      {
#pragma omp atomic
        ++threads;
      }
      int minimum = 0, maximum = 0;
      MPI_Allreduce(&threads, &minimum, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
      MPI_Allreduce(&threads, &maximum, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
      if (minimum != 2 || maximum != 2)
        throw std::runtime_error("OpenMP team size is not two on every rank");
      if (QDP::Layout::primaryNode()) {
        QDP::QDPIO::cout << "RESULT:{\"mpi_ranks\":" << world
                         << ",\"qmp_nodes\":" << QMP_get_number_of_nodes()
                         << ",\"global_lattice_sum\":" << total
                         << ",\"openmp_threads_per_rank\":" << threads
                         << ",\"qdp_xml_broadcast\":true,\"datalib_rank_local_"
                            "xml\":true,\"mpi_thread_level\":"
                         << thread_level << ",\"local_values\":[";
        for (int i = 0; i < world; ++i) {
          if (i)
            QDP::QDPIO::cout << ',';
          QDP::QDPIO::cout << local_values[i];
        }
        QDP::QDPIO::cout << "]}\n";
      }
    }
  } catch (const std::exception &error) {
    QDP::QDPIO::cerr << error.what() << '\n';
    QDP::QDP_abort(1);
  } catch (const std::string &error) {
    QDP::QDPIO::cerr << error << '\n';
    QDP::QDP_abort(1);
  }
  QDP::QDP_finalize();
}
