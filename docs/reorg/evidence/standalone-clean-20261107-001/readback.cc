#include <datalib/io/serial_xml.h>
#include <datalib/storage/corr_storage.h>
#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

int main(int argc, char** argv)
{
  try
  {
    if (argc != 5) throw std::runtime_error("expected REFERENCE CANDIDATE KEYS TIMES");
    Datalib::Storage::KeyValueDB reference(argv[1]);
    Datalib::Storage::KeyValueDB candidate(argv[2]);
    const auto keys = std::stoul(argv[3]);
    const auto times = std::stoul(argv[4]);
    if (reference.num_rows != 1 || candidate.num_rows != 1 ||
        reference.num_columns != times || candidate.num_columns != times ||
        reference.entries_pos.size() != keys || candidate.entries_pos.size() != keys)
      throw std::runtime_error("incorrect key/configuration/time coverage");
    if (reference.metadata != candidate.metadata)
      throw std::runtime_error("metadata differs");
    std::istringstream metadata(candidate.metadata);
    Datalib::Serial::XMLReader xml(metadata);
    int version = -1;
    Datalib::Serial::read(xml, "/DBMetaData/version", version);
    if (version != 4) throw std::runtime_error("incorrect graph schema metadata");
    double scale = 0, max_error = 0;
    std::size_t count = 0, failures = 0, zeros = 0;
    std::vector<std::vector<std::complex<double>>> a, b;
    for (const auto& [key, position] : reference.entries_pos)
    {
      if (!reference.read(key, a) || !candidate.read(key, b))
        throw std::runtime_error("missing exact XML key");
      for (const auto& z : a.at(0)) scale = std::max(scale, std::abs(z));
    }
    const double floor = 1e-12 * std::max(scale, 1e-30);
    for (const auto& [key, position] : reference.entries_pos)
    {
      reference.read(key, a);
      candidate.read(key, b);
      bool zero = true;
      for (std::size_t t = 0; t < times; ++t)
      {
        const auto z = a.at(0).at(t), w = b.at(0).at(t);
        if (!std::isfinite(z.real()) || !std::isfinite(z.imag()) ||
            !std::isfinite(w.real()) || !std::isfinite(w.imag()))
          throw std::runtime_error("nonfinite complex value");
        const double error = std::abs(z-w);
        max_error = std::max(max_error, error);
        failures += error > 1e-9*std::max(std::abs(z),std::abs(w)) + floor;
        zero = zero && (w == std::complex<double>{});
        ++count;
      }
      zeros += zero;
    }
    std::cout << "{\"keys\":" << keys << ",\"values\":" << count
              << ",\"configurations\":1,\"times\":" << times
              << ",\"finite\":true,\"failures\":" << failures
              << ",\"zero_channels\":" << zeros
              << ",\"max_abs_difference\":" << max_error << "}\n";
    return failures != 0;
  }
  catch (const std::string& error) { std::cerr << error << '\n'; return 1; }
  catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
