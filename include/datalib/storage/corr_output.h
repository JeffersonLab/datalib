#pragma once

#include "datalib/graph/corr_graph.h"

namespace Datalib::Storage
{
  using Correlators = Datalib::Model::MapObject<Hadron::KeyHadronSUNClassNPartNPtCorr_t,
                                               std::vector<std::complex<double>>>;

  void writeCorrelators(const Correlators& correlators, const std::string& ensemble,
                        const Hadron::CorrGraphLayout_t& layout, int time_origin,
                        const std::string& filename);
}
