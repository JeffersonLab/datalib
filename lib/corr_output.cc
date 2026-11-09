#include "datalib/storage/corr_output.h"
#include "datalib/storage/corr_keys.h"
#include "datalib/storage/corr_storage.h"

#include <unordered_set>

namespace Datalib::Storage
{
  void writeCorrelators(const Correlators& correlators, const std::string& ensemble,
                        const Hadron::CorrGraphLayout_t& layout, int time_origin,
                        const std::string& filename)
  {
    Datalib::Serial::XMLBufferWriter metadata;
    Datalib::Serial::push(metadata, "DBMetaData");
    Datalib::Serial::write(metadata, "id", "hadronSUNClassNPartNPtCorr");
    Datalib::Serial::write(metadata, "lattSize", layout.latt_size);
    Datalib::Serial::write(metadata, "decay_dir", layout.decay_dir);
    Datalib::Serial::write(metadata, "t_origin", time_origin);
    Datalib::Serial::write(metadata, "version", 4);
    Datalib::Serial::write(metadata, "ensemble", ensemble);
    Datalib::Serial::pop(metadata);

    const auto samples = correlators.size() == 0 ? 0 : correlators.begin()->second.size();
    std::vector<std::string> keys;
    keys.reserve(correlators.size());
    std::vector<std::string> dictionary;
    std::unordered_set<std::string> seen;
    for (const auto& [key, values] : correlators)
    {
      if (values.size() != samples)
        throw std::runtime_error("Inconsistent correlator time extents");
      keys.push_back(SB::getXML(key, "key"));
      for (auto& fragment : CorrKeyDetail::detail::get_xml_fragments(key, "key"))
        if (seen.insert(fragment).second)
          dictionary.push_back(std::move(fragment));
    }

    KeyValueDB database(filename, metadata.str(), dictionary, keys, 1, samples);
    std::size_t index = 0;
    for (const auto& entry : correlators)
      database.write(keys.at(index++), {entry.second}, 0);
  }
}
