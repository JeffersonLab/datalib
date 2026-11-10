#include <complex>
// Make a two-key SUNClass fixture from one real correlator for replica
// integration tests.
#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>
#include <datalib/storage/corr_reader.h>
#include <stdexcept>
int main(int argc, char **argv) {
  if (argc != 3)
    return 1;
  using Key = Hadron::KeyHadronSUNClassNPartNPtCorr_t;
  Datalib::Storage::StorageCorrFunc<Key> input, output;
  input.open_for_reading(argv[1]);
  auto keys = input.keys();
  if (input.num_configs != 1 || keys.size() != 1)
    return 1;
  std::vector<std::vector<std::complex<double>>> rows;
  if (input.get(keys[0], rows) != 0 || rows.size() != 1 || rows[0].size() < 2)
    return 1;
  Key second = keys[0];
  second.npoint[1].irrep.irrep_mom.row = 2;
  second.npoint[3].irrep.irrep_mom.row = 2;
  keys.push_back(second);
  output.open_for_writing(argv[2], 1, input.metadata, rows[0].size());
  output.insert_keys(keys);
  if (output.insert(keys[0], rows) != 0)
    return 1;
  rows[0][0] = {0.0, 0.0};
  rows[0][1] = {0.0, 0.25};
  if (output.insert(keys[1], rows) != 0)
    return 1;
  output.close();
}
