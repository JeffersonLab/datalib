#ifndef DATALIB_STORAGE_CORR_STORAGE_H
#define DATALIB_STORAGE_CORR_STORAGE_H

/// Specification for storing a list of key-values pairs with string keys and fixed-array double precision values
/// magic_number <u32>: 0x0E0E0E (decimal 921102)
/// version <u32>: version of the format (currently 0)
/// num_columns <u32>: number of columns in the stored matrices
/// num_rows <u32>: number of rows in the stored matrices
/// metadata_size <u32>: length of the metadata in <char>s
/// metadata_content <char*metadata_size>: content of the metadata
/// padding <char*((8 - (metadata_size+5*4) % 8) % 8)>: zero
/// num_token_entries <u32>: number of token entries
/// token_entry: ( x num_token_entries )
///    token_entry_length <u32>: length of the ith entry
///    token_entry_content <char*entry_length>: content of the entry
///    padding <char*((8 - (token_entry_length+4) % 8) % 8)>: zero
/// num_dict_entries <u32>: number of dictionary entries
/// key_entry: ( x num_dict_entries )
///    key_entry_length <u32>: length of the ith entry
///    key_entry_content <u32*entry_length>: content of the entry
///    padding <char*((8 - (key_entry_length*4+4) % 8) % 8)>: zero
/// checksum <u32>: crc32 value of content above
/// padding <u32>: zero
/// value entry: ( x num_dict_entries )
///    value_entry_content <double*2*num_columns*num_rows>: matrix of complex numbers stored in column major
///    checksum <u32>: crc32 value of the value_entry_content
///    padding <u32>: zero

#include <algorithm>
#include <cerrno>
#include <complex>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <sstream>
#include <utility>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace Datalib::Storage
{
  namespace detail
  {
    /// Checksum value type
    using checksum_t = uint32_t;

    /// Magic number
    const int magic_number = 921102;

    /// Open file modes
    enum Mode { ReadWrite, OnlyRead };

    /// Throw a runtime exception with the current libc error
    /// \param error_msg: prepended message to error

    template <typename Str>
    void gen_error(const Str& error_msg)
    {
      std::stringstream ss;
      ss << error_msg << ": " << strerror(errno);
      throw std::runtime_error(ss.str());
    }

    /// Open a file
    /// \param filename: file path
    /// \param mode: for reading or writing

    inline std::FILE* file_open(const char* filename, Mode mode)
    {
      std::FILE* f = nullptr;
      switch (mode)
      {
      case ReadWrite: f = std::fopen(filename, "wb+"); break;
      case OnlyRead: f = std::fopen(filename, "rb"); break;
      }
      if (f == nullptr)
      {
	std::stringstream ss;
	ss << "Error opening file `" << filename << "'";
	gen_error(ss.str());
      }
      return f;
    }

    /// Set the current file cursor
    /// \param f: file handler
    /// \param offset: new absolute position

    inline void seek(std::FILE* f, std::size_t offset)
    {
      if (offset >= (std::size_t)std::numeric_limits<long>::max())
	gen_error("Too small type to represent the displacement");
      if (std::fseek(f, offset, SEEK_SET) != 0)
	gen_error("Error setting file position");
    }

    /// Write some content on a file
    /// \param f: file handler
    /// \param v: pointer to the content to write
    /// \param n: number of elements to write

    template <typename T>
    void write(std::FILE* f, const T* v, std::size_t n)
    {
      if (std::fwrite(v, sizeof(T), n, f) != n)
	gen_error("Error writing in a file");
    }

    /// Read some content from a file
    /// \param f: file handler
    /// \param v: pointer to the buffer
    /// \param n: number of elements to read

    template <typename T>
    void read(std::FILE* f, T* v, std::size_t n)
    {
      if (std::fread(v, sizeof(T), n, f) != n)
	gen_error("Error reading from a file");
    }

    /// Truncate the file to the given size
    /// \param f: file handler
    /// \param n: new sizer of the file

    inline void truncate(std::FILE* f, std::size_t n)
    {
      if (ftruncate(fileno(f), n) != 0)
	gen_error("Error truncating file");
    }

    /// Close the file
    /// \param f: file handler

    inline void close(std::FILE* f)
    {
      if (std::fclose(f) != 0)
	gen_error("Error closing file");
    }

    /// Swap endianness of a bunch of elements
    /// \param v: pointer to the first element to swap endianness
    /// \param n: number of elements to swap

    template <typename T>
    void change_endianness(T* v, std::size_t n = 1)
    {
      for (std::size_t i = 0; i < n; ++i)
      {
	char* c = (char*)&v[i];
	for (std::size_t j = 0; j < sizeof(T) / 2; ++j)
	  std::swap(c[j], c[sizeof(T) - 1 - j]);
      }
    }

    /// Swap endianness of a bunch of elements
    /// \param v: pointer to the first element to swap endianness
    /// \param n: number of elements to swap
    /// NOTE: specialization for std::complex types

    template <typename T>
    void change_endianness(std::complex<T>* v, std::size_t n = 1)
    {
      T* v0 = (T*)v;
      for (std::size_t i = 0; i < n * 2u; ++i)
	change_endianness(&v0[i]);
    }

    /// Compute the CRC32 value of a string
    /// \param crc: initial value
    /// \param buf: pointer to the string
    /// \param len: length of the string

    inline checksum_t crc32(checksum_t crc, const unsigned char* buf, std::size_t len)
    {
      // Table of CRC-32's of all single-byte values (made by make_crc_table)
      static checksum_t crc_table[256] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL, 0xe963a535L,
	0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL,
	0xe7b82d07L, 0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL, 0x1adad47dL,
	0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
	0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
	0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL, 0x35b5a8faL, 0x42b2986cL,
	0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL,
	0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
	0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L, 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL,
	0xb6662d3dL, 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
	0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL,
	0x086d3d2dL, 0x91646c97L, 0xe6635c01L, 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
	0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL,
	0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L, 0x4db26158L, 0x3ab551ceL,
	0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
	0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
	0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L,
	0xce61e49fL, 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L,
	0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL, 0xead54739L,
	0x9dd277afL, 0x04db2615L, 0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L,
	0x6906c2feL, 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L,
	0x10da7a5aL, 0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L, 0xd6d6a3e8L,
	0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
	0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
	0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L, 0xcc0c7795L, 0xbb0b4703L,
	0x220216b9L, 0x5505262fL, 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L,
	0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
	0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L, 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL,
	0x0cb61b38L, 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
	0x68ddb3f8l, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L, 0x88085ae6L,
	0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
	0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L, 0x4969474dL,
	0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L,
	0x47b2cf7fL, 0x30b5ffe9L, 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
	0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
	0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL};

      if (len == 0)
	return crc;
      crc = crc ^ 0xffffffffL;
      for (std::size_t i = 0; i < len; ++i)
	crc = crc_table[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
      return crc ^ 0xffffffffL;
    }

    /// Compute the checksum of a data block
    /// \param str: pointer to the given data
    /// \param size: size of the data in bytes
    /// \param prev_checksum: previous value of the checksum

    template <typename T>
    checksum_t do_checksum(const T* str, std::size_t size = 1, checksum_t prev_checksum = 0)
    {
      return crc32(prev_checksum, (unsigned char*)str, size * sizeof(T));
    }

    struct CompressedDictionary {
    private:
      struct LeafAndBranch {
        std::size_t dict_entry_index;
        std::size_t branch_index;
      };

      struct Branch {
        std::unordered_map<std::string, LeafAndBranch> branches;
        std::size_t branch_size;

        Branch() : branches(4), branch_size(0)
        {
        }

        Branch(const std::string& dict_entry_value, std::size_t dict_entry_index,
               std::size_t branch_index)
          : branches(4)
        {
          branches.emplace(dict_entry_value, LeafAndBranch{dict_entry_index, branch_index});
          branch_size = dict_entry_value.size();
        }
      };

      std::vector<Branch> tree;

      constexpr static const std::size_t npos = -1;

      void insert_entry(std::string::const_iterator entry_begin,
			const std::string::const_iterator entry_end, std::size_t entry_index,
			std::size_t tree_index, std::size_t next_tree_index)
      {
        std::size_t n = std::distance(entry_begin, entry_end);

	if (n == 0)
	  throw std::runtime_error("unexpected state");

	// Rearrange the branch if the given entry is shorter than the branch length
	{
	  auto& branch = tree.at(next_tree_index);
	  if (n < branch.branch_size)
	  {
	    Branch old_branch;
	    std::swap(old_branch, branch);
	    branch.branch_size = n;
	    for (const auto& it : old_branch.branches)
	    {
	      insert_entry(it.first.begin(), it.first.end(), it.second.dict_entry_index,
			   it.second.branch_index, next_tree_index);
	    }
	  }
	}

	// Insert the given string
        auto& branch = tree.at(next_tree_index);
        const std::string this_bite(entry_begin, entry_begin + branch.branch_size);
        const auto it = branch.branches.find(this_bite);
        if (it != branch.branches.end())
        {
          if (n == branch.branch_size)
          {
            if (tree_index != npos)
            {
              if (it->second.branch_index != npos)
        	throw std::runtime_error("inconsistent branch configuration");
              it->second.branch_index = tree_index;
            }
            if (entry_index != npos)
            {
	      if (it->second.dict_entry_index != npos)
		throw std::runtime_error("replicated dictionary entries");
	      it->second.dict_entry_index = entry_index;
	    }
          }
          else
          {
	    insert_entry(entry_begin + branch.branch_size, entry_end, entry_index, tree_index,
			 it->second.branch_index);
	  }
        }
        else
        {
          // A complete token at this branch needs a leaf, even when the
          // branch did not already contain this byte sequence.
          if (n == branch.branch_size)
          {
            branch.branches.emplace(this_bite, LeafAndBranch{entry_index, tree_index});
            return;
          }
          branch.branches.emplace(this_bite, LeafAndBranch{npos, tree.size()});
          tree.push_back(Branch(std::string(entry_begin + branch.branch_size, entry_end),
        			entry_index, tree_index));
        }
      }

    public:
      CompressedDictionary(const std::vector<std::string>& dictionary)
      {
	if (dictionary.size() > std::numeric_limits<std::uint32_t>::max())
	  throw std::runtime_error("dictionary is too large");

	bool first_nonempty_entry = true;
	for (std::size_t i = 0; i < dictionary.size(); ++i)
	{
	  const auto& entry = dictionary[i];
	  if (entry.size() == 0)
	    continue;

	  if (first_nonempty_entry)
	  {
	    // Enter the first entry trivially
	    tree.push_back(Branch(entry, i, npos));
	  }
	  else
	  {
	    // General case
	    insert_entry(entry.begin(), entry.end(), i, npos, 0);
	  }
	  first_nonempty_entry = false;
	}
      }

      std::vector<std::uint32_t> get_indices(const std::string& s)
      {
        if (s.size() == 0)
          return {};
        if (tree.size() == 0)
          throw std::runtime_error("the key cannot be compressed");
        std::vector<std::uint32_t> r;
        std::size_t branch_index = 0;
        for (auto it = s.begin(); it != s.end();)
        {
	  const auto n = std::distance(it, s.end());
	  const auto& branch = tree.at(branch_index);
          if (n < branch.branch_size)
            throw std::runtime_error("the key cannot be compressed");
          const auto branch_it = branch.branches.find(std::string(it, it + branch.branch_size));
	  if (branch_it == branch.branches.end())
	    throw std::runtime_error("the key cannot be compressed");
	  if (branch_it->second.dict_entry_index != npos && branch_it->second.branch_index != npos)
	    throw std::runtime_error("ambiguous branch");
          if (branch_it->second.dict_entry_index != npos)
          {
            r.push_back(branch_it->second.dict_entry_index);
            branch_index = 0;
          }
          else
          {
            branch_index = branch_it->second.branch_index;
          }
          it += branch.branch_size;
        }
        if (branch_index != 0)
          throw std::runtime_error("the key ends in an incomplete dictionary token");
        return r;
      }
    };

    /// Decompose a given string into pieces from a dictionary
    /// \param dictionary: list of pieces
    /// \param key: given string
    /// \return: list of dictionary indices that reconstruct the given string

    inline std::vector<uint32_t> compress_key(const std::vector<std::string>& dictionary,
					      const std::string& key)
    {
      if (dictionary.size() > std::numeric_limits<std::uint32_t>::max())
	throw std::runtime_error("dictionary is too large");
      std::vector<uint32_t> r;
      for (auto it = key.begin(); it != key.end();)
      {
	bool found = false;
	for (std::size_t i = 0; i < dictionary.size(); ++i)
	{
	  const auto& dict_entry = dictionary[i];
	  if (dict_entry.size() > key.end() - it)
	    continue;
	  if (!std::equal(it, it + dict_entry.size(), dict_entry.begin()))
	    continue;
	  it += dict_entry.size();
	  r.push_back(i);
	  found = true;
	  break;
	}
	if (!found)
	  throw std::runtime_error("key cannot be compressed");
      }
      return r;
    }

    /// Reconstruct a string from pieces
    /// \param dictionary: list of pieces
    /// \param compressed_key: list of pieces to reconstruct the original string

    inline std::string decompress_key(const std::vector<std::string>& dictionary,
				      const std::vector<uint32_t>& compressed_key)
    {
      std::size_t key_size = 0;
      for (const auto& compressed_key_item : compressed_key)
	key_size += dictionary.at(compressed_key_item).size();
      std::string r(key_size, char(0));
      auto r_it = r.begin();
      for (const auto& compressed_key_item : compressed_key)
      {
	const auto dict_entry = dictionary.at(compressed_key_item);
	std::copy(dict_entry.begin(), dict_entry.end(), r_it);
	r_it += dict_entry.size();
      }

      return r;
    }

    struct HashVectorUInt {
      std::size_t operator()(const std::vector<std::size_t>& v) const noexcept
      {
	std::size_t r = 0x100000001B3;
	for (std::size_t i : v)
	  r = r ^ std::hash<std::size_t>{}(i);
	return r;
      }
    };
  }

  struct KeyValueDB {
    detail::Mode mode;	       ///< access mode
    unsigned int num_rows;     ///< number of rows of the matrix value
    unsigned int num_columns;  ///< number of columns of the matrix value
    std::string metadata;      ///< metadata content
    std::FILE* f;	       ///< file handler
    bool do_change_endianness; ///< whether to change endianness after reading
    std::unordered_map<std::string, std::size_t> entries_pos; ///< entries positions
    std::vector<std::string> dictionary;	 ///< dictionary used to compress the keys
    std::vector<detail::checksum_t> checksums;	 ///< partial checksums on the entries
    std::vector<unsigned int> last_row_written;	 ///< last row written on the entries
    std::size_t values_base;			 ///< position of the first value in the file

    /// Constructor for writing
    /// \param filename: path to file to write
    /// \param metadata: metadata content
    /// \param dictionary: common substrings on keys
    /// \param keys: list of keys of to store
    /// \param entrie_num_rows: number of rows of the value matrix
    /// \param entrie_num_cols: number of columns of the value matrix

    KeyValueDB(const std::string& filename, const std::string& metadata,
	       const std::vector<std::string>& dictionary, const std::vector<std::string>& keys,
	       int entrie_num_rows, int entrie_num_cols)
    {
      using namespace Datalib::Storage::detail;

      // Create the output file and preallocate its size
      mode = ReadWrite;
      f = file_open(filename.c_str(), mode);
      std::unique_ptr<std::FILE, decltype(&std::fclose)> file(f, &std::fclose);
      std::size_t all_dict_len = 0;
      for (const auto& it : dictionary)
	all_dict_len += it.size();
      truncate(f, 8u * 4u + metadata.size() + dictionary.size() * 4u + all_dict_len +
		    entrie_num_rows * entrie_num_cols * 2u * sizeof(double) * keys.size());

      // Write magic_number
      checksum_t checksum_val = 0;
      values_base = 0;
      uint32_t u32 = magic_number;
      {
	int32_t v = magic_number;
	change_endianness(&v, 1);
	if (v == u32)
	  throw std::runtime_error("choose a magic number that is not a palindrome");
      }
      detail::write(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      values_base += sizeof(uint32_t);
      this->do_change_endianness = false;

      // Write version (the current version is 0)
      u32 = 0;
      detail::write(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      values_base += sizeof(uint32_t);
      static_assert(sizeof(checksum_t) == sizeof(uint32_t), "wtf");

      // Write num_columns entries
      u32 = entrie_num_cols;
      detail::write(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      values_base += sizeof(uint32_t);
      this->num_columns = entrie_num_cols;

      // Write num_rows entries
      u32 = entrie_num_rows;
      detail::write(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      values_base += sizeof(uint32_t);
      this->num_rows = entrie_num_rows;

      // Write metadata
      u32 = metadata.size();
      detail::write(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      values_base += sizeof(uint32_t);
      detail::write(f, metadata.data(), metadata.size());
      checksum_val = do_checksum(metadata.data(), metadata.size(), checksum_val);
      values_base += metadata.size();
      this->metadata = metadata;

      // Write padding
      std::vector<char> padding((8u - (metadata.size() + 5u * sizeof(std::uint32_t)) % 8u) % 8u);
      detail::write(f, padding.data(), padding.size());
      checksum_val = do_checksum(padding.data(), padding.size(), checksum_val);
      values_base += padding.size();

      // Write num_token entries
      u32 = dictionary.size();
      detail::write(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      values_base += sizeof(uint32_t);

      // Write the tokens
      for (const auto& dict_entry : dictionary)
      {
	u32 = dict_entry.size();
	detail::write(f, &u32, 1);
	checksum_val = do_checksum(&u32, 1, checksum_val);
	values_base += sizeof(uint32_t);
	detail::write(f, dict_entry.data(), dict_entry.size());
	checksum_val = do_checksum(dict_entry.data(), dict_entry.size(), checksum_val);
	values_base += dict_entry.size();
	std::vector<char> padding((8u - (dict_entry.size() + 4u) % 8u) % 8u);
	detail::write(f, padding.data(), padding.size());
	checksum_val = do_checksum(padding.data(), padding.size(), checksum_val);
	values_base += padding.size();
      }
      this->dictionary = dictionary;

      // Write num entries
      u32 = keys.size();
      detail::write(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      values_base += sizeof(uint32_t);

      // Write the keys
      CompressedDictionary compressed_dict(dictionary);
      for (const auto& key : keys)
      {
	const auto& compressed_key = compressed_dict.get_indices(key);
	u32 = compressed_key.size();
	detail::write(f, &u32, 1);
	checksum_val = do_checksum(&u32, 1, checksum_val);
	values_base += sizeof(uint32_t);
	detail::write(f, compressed_key.data(), compressed_key.size());
	checksum_val = do_checksum(compressed_key.data(), compressed_key.size(), checksum_val);
	values_base += compressed_key.size() * sizeof(uint32_t);
	std::vector<char> padding(
	  (8u - (compressed_key.size() * sizeof(uint32_t) + sizeof(uint32_t)) % 8u) % 8u);
	detail::write(f, padding.data(), padding.size());
	checksum_val = do_checksum(padding.data(), padding.size(), checksum_val);
	values_base += padding.size();
      }

      // Write checksum
      u32 = checksum_val;
      detail::write(f, &u32, 1);
      values_base += sizeof(uint32_t);

      // Truncate the file to the final size
      std::size_t file_size =
	values_base +
	(entrie_num_rows * entrie_num_cols * sizeof(double) * 2u + 8u) * entries_pos.size();
      truncate(f, file_size);

      // Populate the entries positions
      entries_pos.reserve(keys.size());
      for (std::size_t i = 0; i < keys.size(); ++i)
	entries_pos.emplace(keys[i], i);

      // Allocate the partial checksums
      checksums.resize(keys.size());

      // Allocate the tracking of the last row written
      last_row_written.resize(keys.size());
      file.release();
    }

    /// Write a key-value pair into the file
    /// \param key: key
    /// \param value: matrix associated to the key
    /// \param first_row: first row to write (default 0)

    void write(const std::string& key, const std::vector<std::vector<std::complex<double>>>& value,
	       unsigned int first_row = 0)
    {
      using namespace detail;

      // Do checks and find the key
      if (mode == OnlyRead)
	throw std::runtime_error("write is an invalid operation");
      if (value.size() + first_row > num_rows)
	throw std::runtime_error("invalid number of rows of the input or the first row to write");
      for (const auto& it : value)
	if (it.size() != num_columns)
	  throw std::runtime_error("invalid number of columns of the input");
      auto it = entries_pos.find(key);
      if (it == entries_pos.end())
	throw std::runtime_error("error: writing an unknown key");
      if (last_row_written.at(it->second) != first_row)
	throw std::runtime_error("error: please write the rows consecutively");

      // Concatenate the values and write the content
      std::vector<std::complex<double>> r;
      r.reserve(value.size() * num_columns);
      for (const auto& it : value)
	r.insert(r.end(), it.begin(), it.end());
      seek(f, values_base + (num_rows * num_columns * 2u + 1u) * sizeof(double) * it->second +
		first_row * num_columns * 2u * sizeof(double));
      detail::write(f, r.data(), r.size());

      // Compute checksum
      auto& c = checksums.at(it->second);
      c = do_checksum(r.data(), r.size(), c);

      if (first_row + value.size() == num_rows)
      {
	// Write the checksum
	uint32_t u32 = c;
	detail::write(f, &u32, 1);

	// Write padding
	u32 = 0;
	detail::write(f, &u32, 1);
      }

      // Track the last row written
      last_row_written.at(it->second) = first_row + (unsigned int)value.size();
    }

    /// Constructor for reading
    /// \param filename: path to file to write

    KeyValueDB(const std::string& filename, bool just_read_metadata = false)
    {
      using namespace detail;

      // Create the output file and preallocate its size
      mode = OnlyRead;
      f = file_open(filename.c_str(), mode);
      std::unique_ptr<std::FILE, decltype(&std::fclose)> file(f, &std::fclose);

      // Read magic_number
      checksum_t checksum_val = 0;
      values_base = 0;
      uint32_t u32 = 0;
      detail::read(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      if (u32 != magic_number)
      {
	change_endianness(&u32, 1);
	if (u32 != magic_number)
	{
	  throw std::runtime_error("Unexpected value for the magic number; the file may "
				   "not be a simple key-value storage");
	}
	do_change_endianness = true;
      }
      else
      {
	do_change_endianness = false;
      }
      values_base += sizeof(uint32_t);

      // Read version (the current version is 0)
      detail::read(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      if (do_change_endianness)
	change_endianness(&u32, 1);
      if (u32 != 0)
	throw std::runtime_error("unsupported format version");
      values_base += sizeof(uint32_t);

      // Read num_columns entries
      detail::read(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      if (do_change_endianness)
	change_endianness(&u32, 1);
      values_base += sizeof(uint32_t);
      this->num_columns = u32;

      // Read num_rows entries
      detail::read(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      if (do_change_endianness)
	change_endianness(&u32, 1);
      values_base += sizeof(uint32_t);
      this->num_rows = u32;

      // Read metadata
      detail::read(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      if (do_change_endianness)
	change_endianness(&u32, 1);
      values_base += sizeof(uint32_t);
      metadata.resize(u32);
      detail::read(f, metadata.data(), metadata.size());
      checksum_val = do_checksum(metadata.data(), metadata.size(), checksum_val);
      values_base += metadata.size();
      if (just_read_metadata)
      {
	detail::close(file.release());
	f = nullptr;
	return;
      }

      // Read padding
      std::vector<char> padding((8u - (metadata.size() + 5u * sizeof(std::uint32_t)) % 8u) % 8u);
      detail::read(f, padding.data(), padding.size());
      checksum_val = do_checksum(padding.data(), padding.size(), checksum_val);
      values_base += padding.size();

      // Read num_token entries
      detail::read(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      if (do_change_endianness)
	change_endianness(&u32, 1);
      values_base += sizeof(uint32_t);
      std::size_t num_tokens = u32;
      dictionary.reserve(num_tokens);

      // Read the tokens
      for (std::size_t i = 0; i < num_tokens; ++i)
      {
	detail::read(f, &u32, 1);
	checksum_val = do_checksum(&u32, 1, checksum_val);
	if (do_change_endianness)
	  change_endianness(&u32, 1);
	values_base += sizeof(uint32_t);
	std::string dict_entry(u32, char(0));
	detail::read(f, dict_entry.data(), dict_entry.size());
	checksum_val = do_checksum(dict_entry.data(), dict_entry.size(), checksum_val);
	values_base += dict_entry.size();
	std::vector<char> padding((8u - (dict_entry.size() + 4u) % 8u) % 8u);
	detail::read(f, padding.data(), padding.size());
	checksum_val = do_checksum(padding.data(), padding.size(), checksum_val);
	values_base += padding.size();
	dictionary.push_back(std::move(dict_entry));
      }

      // Read num entries
      detail::read(f, &u32, 1);
      checksum_val = do_checksum(&u32, 1, checksum_val);
      if (do_change_endianness)
	change_endianness(&u32, 1);
      values_base += sizeof(uint32_t);
      std::size_t num_entries = u32;

      // Read the keys
      entries_pos.reserve(num_entries);
      for (std::size_t i = 0; i < num_entries; ++i)
      {
	detail::read(f, &u32, 1);
	checksum_val = do_checksum(&u32, 1, checksum_val);
	if (do_change_endianness)
	  change_endianness(&u32, 1);
	values_base += sizeof(uint32_t);
	std::vector<uint32_t> compressed_key(u32);
	detail::read(f, compressed_key.data(), compressed_key.size());
	checksum_val = do_checksum(compressed_key.data(), compressed_key.size(), checksum_val);
	if (do_change_endianness)
	  change_endianness(compressed_key.data(), compressed_key.size());
	values_base += compressed_key.size() * sizeof(uint32_t);
	entries_pos.emplace(decompress_key(dictionary, compressed_key), i);
	std::vector<char> padding(
	  (8u - (compressed_key.size() * sizeof(uint32_t) + sizeof(uint32_t)) % 8u) % 8u);
	detail::read(f, padding.data(), padding.size());
	checksum_val = do_checksum(padding.data(), padding.size(), checksum_val);
	values_base += padding.size();
      }

      // Read checksum
      detail::read(f, &u32, 1);
      if (do_change_endianness)
	change_endianness(&u32, 1);
      values_base += sizeof(uint32_t);
      if (u32 != checksum_val)
	throw std::runtime_error("checksum failed for the header");
      file.release();
    }

    /// Read the content associated to a key
    /// \param key: key
    /// \param value: (out) content associated to the key
    /// \return: whether the key is in the file

    bool read(const std::string& key, std::vector<std::vector<std::complex<double>>>& value) const
    {
      using namespace detail;

      // Do checks and find the key
      if (mode != OnlyRead)
	throw std::runtime_error("read is an invalid operation");
      auto it = entries_pos.find(key);
      if (it == entries_pos.end())
	return false;
      return read_index(it->second, value);
    }

    /// Read the content associated to a key
    /// \param key_index: key index
    /// \param value: (out) content associated to the key
    /// \return: whether the key is in the file

    bool read_index(std::size_t key_index,
		    std::vector<std::vector<std::complex<double>>>& value) const
    {
      using namespace detail;

      // Do checks and find the key
      if (mode != OnlyRead)
	throw std::runtime_error("read is an invalid operation");

      // Read the values
      std::vector<std::complex<double>> r(num_rows * num_columns);
      seek(f, values_base + (num_rows * num_columns * 2u + 1u) * sizeof(double) * key_index);
      detail::read(f, r.data(), r.size());
      checksum_t checksum_val = do_checksum(r.data(), r.size());

      // Read the checksum
      uint32_t u32 = 0;
      detail::read(f, &u32, 1);
      if (do_change_endianness)
	change_endianness(&u32, 1);
      if (u32 != checksum_val)
	throw std::runtime_error("checksum failed on reading a key-value entry");

      // Copy the content back
      if (do_change_endianness)
	change_endianness(r.data(), r.size());
      value.resize(num_rows);
      for (unsigned int i = 0; i < num_rows; ++i)
      {
	value[i].resize(num_columns);
	std::copy_n(r.data() + i * num_columns, num_columns, value[i].data());
      }

      return true;
    }

    /// Destructor

    ~KeyValueDB()
    {
      if (f)
	detail::close(f);
    }
  };
}
#endif // DATALIB_STORAGE_CORR_STORAGE_H
