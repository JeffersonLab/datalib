/*! \file
 * \brief Print only the metadata of a db
 */

#include <datalib/storage/dbtype.h>

int main(int argc, char **argv) {
  // Parse the arguments
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <dbasename>" << std::endl;
    return -1;
  }

  // Print the metadata
  std::cout << Hadron::getMetaData(argv[1]) << std::endl;

  return 0;
}
