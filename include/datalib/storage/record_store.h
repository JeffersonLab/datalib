#pragma once
#include <ConfDataStoreDBSimple.h>
#include <memory>
#include <string>
#include <vector>
namespace Datalib::TensorStorage {
struct RecordOptions {
  unsigned int cache_bytes = 0;
  unsigned int metadata_capacity = 0;
  unsigned int configurations = 0;
  unsigned int buckets = 32;
};
// Rank-local storage. The consumer owns serialization and communication.
// Encoded records are stored verbatim, preserving all historical key/value formats.
class StorageRecords {
  struct Bytes {
    std::string value;
    void writeObject(std::string& out) const { out = value; }
    void readObject(const std::string& in) { value = in; }
  };
  RecordOptions options_;
  std::shared_ptr<FILEDB::ConfDataStoreDBSimple> records_;
public:
  RecordOptions& record_options() { return options_; }
  static bool record_file_exists(const std::string& file) {
    return FILEDB::ConfDataStoreDBSimple::fileExists(file);
  }
  int open_records(const std::string& file, int flags, int mode) {
    close_records();
    auto next = std::make_shared<FILEDB::ConfDataStoreDBSimple>();
    next->setCacheSize(options_.cache_bytes);
    next->setMaxNumberConfigs(options_.configurations);
    next->setNumberBuckets(options_.buckets);
    next->setMaxUserInfoLen(options_.metadata_capacity);
    int status = next->open(file, flags, mode);
    if (status == 0) records_ = std::move(next);
    return status;
  }
  int write_metadata(const std::string& metadata) {
    return records_ ? records_->insertUserdata(metadata) : -1;
  }
  int read_metadata(std::string& metadata) const {
    return records_ ? records_->getUserdata(metadata) : -1;
  }
  int put_record(const std::string& key, const std::string& value) {
    return records_ ? records_->insert(Bytes{key}, Bytes{value}) : -1;
  }
  int get_record(const std::string& key, std::string& value) const {
    if (!records_) return -1;
    Bytes k{key}, v;
    int status = records_->get(k, v);
    if (status == 0) value = std::move(v.value);
    return status;
  }
  std::vector<std::string> record_keys() const {
    std::vector<std::string> result;
    if (records_) {
      std::vector<Bytes> keys;
      records_->keys<Bytes,Bytes>(keys);
      for (auto& key : keys) result.push_back(std::move(key.value));
    }
    return result;
  }
  int close_records() {
    if (!records_) return 0;
    int status = records_->close();
    records_.reset();
    return status;
  }
};
}
