// $Id: adat_io.cc,v 2.2 2009/03/09 15:34:14 edwards Exp $
//


#include <string.h>
#include "datalib/codec/binary.h"
#include "datalib/codec/byteorder.h"

using std::string;
using std::cerr;

namespace Datalib::Binary 
{

  // Propagate status to all nodes
  bool BinaryReader::fail()
  {
    return getIstream().fail();
  }

  // Wrappers for read functions
  void readDesc(BinaryReader& bin, std::string& input)
  {
    bin.readDesc(input);
  }

  void read(BinaryReader& bin, std::string& input, size_t maxBytes)
  {
    bin.read(input, maxBytes);
  }

  void read(BinaryReader& bin, char& input)
  {
    bin.read(input);
  }

  void read(BinaryReader& bin, int& input)
  {
    bin.read(input);
  }

  void read(BinaryReader& bin, unsigned int& input)
  {
    bin.read(input);
  }

  void read(BinaryReader& bin, short int& input)
  {
    bin.read(input);
  }

  void read(BinaryReader& bin, unsigned short int& input)
  {
    bin.read(input);
  }

  void read(BinaryReader& bin, long int& input)
  {
    bin.read(input);
  }

  void read(BinaryReader& bin, unsigned long int& input)
  {
    bin.read(input);
  }

  void read(BinaryReader& bin, float& input)
  {
    bin.read(input);
  }

  void read(BinaryReader& bin, double& input)
  {
    bin.read(input);
  }

  void read(BinaryReader& bin, bool& input)
  {
    bin.read(input);
  }

  //! Complex reader
  void read(BinaryReader& bin, std::complex<float>& param)
  {
    // NOTE: the C++-11 standard says a "real()" function returns a r-value - not an "l-value", so
    // cannot simply read into a param.real()
    float re, im;
    read(bin, re);
    read(bin, im);
    param = std::complex<float>(re,im);
  }

  //! Complex reader
  void read(BinaryReader& bin, std::complex<double>& param)
  {
    double re, im;
    read(bin, re);
    read(bin, im);
    param = std::complex<double>(re,im);
  }

  // Different bindings for read functions
  BinaryReader& operator>>(BinaryReader& bin, char& input)
  {
    read(bin, input);
    return bin;
  }

  BinaryReader& operator>>(BinaryReader& bin, int& input)
  {
    read(bin, input);
    return bin;
  }

  BinaryReader& operator>>(BinaryReader& bin, unsigned int& input)
  {
    read(bin, input);
    return bin;
  }

  BinaryReader& operator>>(BinaryReader& bin, short int& input)
  {
    read(bin, input);
    return bin;
  }

  BinaryReader& operator>>(BinaryReader& bin, unsigned short int& input)
  {
    read(bin, input);
    return bin;
  }

  BinaryReader& operator>>(BinaryReader& bin, long int& input)
  {
    read(bin, input);
    return bin;
  }

  BinaryReader& operator>>(BinaryReader& bin, unsigned long int& input)
  {
    read(bin, input);
    return bin;
  }

  BinaryReader& operator>>(BinaryReader& bin, float& input)
  {
    read(bin, input);
    return bin;
  }

  BinaryReader& operator>>(BinaryReader& bin, double& input)
  {
    read(bin, input);
    return bin;
  }

  BinaryReader& operator>>(BinaryReader& bin, bool& input)
  {
    read(bin, input);
    return bin;
  }

  void BinaryReader::readDesc(string& input)
  {
    // Get the length
    int n;
    readPrimitive<int>(n);
    if (n < 0)
      throw BinaryReadException();
    
    // Read
    char* str = new char[n];
    readArray(str, sizeof(char), n);

    input.assign(str, n);

    delete[] str;
  }

  void BinaryReader::read(string& input, size_t maxBytes)
  {
    if (maxBytes >= MAX_ARRAY_READ_SIZE)
      throw BinaryReadException();
    char *str = new char[maxBytes];
    size_t n;

    if (!getIstream().getline(str, maxBytes))
      throw BinaryReadException();

    n = strlen(str);
    setChecksum() = Datalib::Util::crc32(getChecksum(), str, n);   // no string terminator
    ++n;
    setChecksum() = Datalib::Util::crc32(getChecksum(), "\n", 1);   // account for newline written

    input = str;
    delete[] str;
  }

  void BinaryReader::read(char& input) 
  {
    readPrimitive<char>(input);
  }

  void BinaryReader::read(int& input) 
  {
    readPrimitive<int>(input);
  }

  void BinaryReader::read(unsigned int& input)
  {
    readPrimitive<unsigned int>(input);
  }

  void BinaryReader::read(short int& input)
  {
    readPrimitive<short int>(input);
  }

  void BinaryReader::read(unsigned short int& input)
  {
    readPrimitive<unsigned short int>(input);
  }

  void BinaryReader::read(long int& input)
  {
    readPrimitive<long int>(input);
  }

  void BinaryReader::read(unsigned long int& input)
  {
    readPrimitive<unsigned long int>(input);
  }

  void BinaryReader::read(float& input)
  {
    readPrimitive<float>(input);
  }

  void BinaryReader::read(double& input)
  {
    readPrimitive<double>(input);
  }

  void BinaryReader::read(bool& input)
  {
    readPrimitive<bool>(input);
  }

  template< typename T>
  void BinaryReader::readPrimitive(T& input)
  {
    readArray((char*)&input, sizeof(T), 1);
  }

  void BinaryReader::readArray(char* input, size_t size, size_t nmemb)
  {
    if (size == 0 || nmemb == 0)
      return;

    // Read
    // By default, we expect all data to be in big-endian
    if (!getIstream().read(input, size * nmemb))
      throw BinaryReadException();
    setChecksum() = Datalib::Util::crc32(getChecksum(), input, size*nmemb);

    if (! Datalib::Util::big_endian())
    {
      // little-endian
      // Swap
      Datalib::Util::byte_swap(input, size, nmemb);
    }
  }


  //-----------------------------------------
  //! Binary reader support
  BinaryBufferReader::BinaryBufferReader() {checksum = 0;}

  BinaryBufferReader::BinaryBufferReader(const std::string& s) {checksum = 0; open(s);}

  BinaryBufferReader::~BinaryBufferReader() {}

  void BinaryBufferReader::open(const std::string& s) 
  {
    f.str(s);
  }

  // Output the stream
  std::string BinaryBufferReader::str() const
  {
    return f.str();
  }

  // Is it empty?
  bool BinaryBufferReader::eof() const
  {
    return f.eof();
  }


  //-----------------------------------------
  //! Binary file reader support
  BinaryFileReader::BinaryFileReader() {checksum = 0;}

  BinaryFileReader::BinaryFileReader(const std::string& p) {checksum = 0; open(p);}

  void BinaryFileReader::open(const std::string& p) 
  {
    f.open(p.c_str(),std::ifstream::in | std::ifstream::binary);

    if (! is_open())
    {
      cerr << "BinaryFileReader: error opening file: " << p;
      exit(1);
    }
  }

  void BinaryFileReader::close()
  {
    if (is_open())
      f.close();
  }


  // Propagate status to all nodes
  bool BinaryFileReader::is_open()
  {
    return f.is_open();
  }

  // Seek offset from end
  void BinaryFileReader::seekEnd(long int off)
  {
    f.seekg(-off, std::ios_base::end);
  }

  // Rewind
  void BinaryFileReader::rewind()
  {
    f.seekg(0, std::ios_base::beg);
    checksum = 0;
  }

  BinaryFileReader::~BinaryFileReader() {close();}


  //-----------------------------------------
  //! Binary writer support
  BinaryWriter::BinaryWriter() {}

  // Propagate status to all nodes
  bool BinaryWriter::fail()
  {
    return getOstream().fail();
  }

  BinaryWriter::~BinaryWriter() {}

  // Wrappers for write functions
  void writeDesc(BinaryWriter& bin, const std::string& output)
  {
    bin.writeDesc(output);
  }

  void write(BinaryWriter& bin, const std::string& output)
  {
    bin.write(output);
  }

  void write(BinaryWriter& bin, const char* output)
  {
    bin.write(std::string(output));
  }

  void write(BinaryWriter& bin, char output)
  {
    bin.write(output);
  }

  void write(BinaryWriter& bin, int output)
  {
    bin.write(output);
  }

  void write(BinaryWriter& bin, unsigned int output)
  {
    bin.write(output);
  }

  void write(BinaryWriter& bin, short int output)
  {
    bin.write(output);
  }

  void write(BinaryWriter& bin, unsigned short int output)
  {
    bin.write(output);
  }

  void write(BinaryWriter& bin, long int output)
  {
    bin.write(output);
  }

  void write(BinaryWriter& bin, unsigned long int output)
  {
    bin.write(output);
  }

  void write(BinaryWriter& bin, float output)
  {
    bin.write(output);
  }

  void write(BinaryWriter& bin, double output)
  {
    bin.write(output);
  }

  void write(BinaryWriter& bin, bool output)
  {
    bin.write(output);
  }

  //! Complex writer
  void write(BinaryWriter& bin, const std::complex<float>& param)
  {
    write(bin, param.real());
    write(bin, param.imag());
  }

  //! Complex writer
  void write(BinaryWriter& bin, const std::complex<double>& param)
  {
    write(bin, param.real());
    write(bin, param.imag());
  }

  // Different bindings for write functions
  BinaryWriter& operator<<(BinaryWriter& bin, const std::string& output)
  {
    write(bin, output);
    return bin;
  }

  BinaryWriter& operator<<(BinaryWriter& bin, const char* output)
  {
    write(bin, output);
    return bin;
  }

  BinaryWriter& operator<<(BinaryWriter& bin, int output)
  {
    write(bin, output);
    return bin;
  }

  BinaryWriter& operator<<(BinaryWriter& bin, unsigned int output)
  {
    write(bin, output);
    return bin;
  }

  BinaryWriter& operator<<(BinaryWriter& bin, short int output)
  {
    write(bin, output);
    return bin;
  }

  BinaryWriter& operator<<(BinaryWriter& bin, unsigned short int output)
  {
    write(bin, output);
    return bin;
  }

  BinaryWriter& operator<<(BinaryWriter& bin, long int output)
  {
    write(bin, output);
    return bin;
  }

  BinaryWriter& operator<<(BinaryWriter& bin, unsigned long int output)
  {
    write(bin, output);
    return bin;
  }

  BinaryWriter& operator<<(BinaryWriter& bin, float output)
  {
    write(bin, output);
    return bin;
  }

  BinaryWriter& operator<<(BinaryWriter& bin, double output)
  {
    write(bin, output);
    return bin;
  }

  BinaryWriter& operator<<(BinaryWriter& bin, bool output)
  {
    write(bin, output);
    return bin;
  }

  void BinaryWriter::writeDesc(const string& output)
  {
    // WARNING: CHECK ON NEWLINE IN CHECKSUM
    size_t n = output.length();
    writePrimitive<int>(n);
    writeArray(output.c_str(), sizeof(char), n);
  }

  void BinaryWriter::write(const string& output)
  {
    // WARNING: CHECK ON NEWLINE IN CHECKSUM
    size_t n = output.length();
    writeArray(output.c_str(), sizeof(char), n);
    write('\n');   // Convention is to write a line terminator
  }

  void BinaryWriter::write(const char* output)
  {
    write(string(output));
  }

  void BinaryWriter::write(const char& output) 
  {
    writePrimitive<char>(output);
  }

  void BinaryWriter::write(const int& output) 
  {
    writePrimitive<int>(output);
  }

  void BinaryWriter::write(const unsigned int& output)
  {
    writePrimitive<unsigned int>(output);
  }

  void BinaryWriter::write(const short int& output)
  {
    writePrimitive<short int>(output);
  }

  void BinaryWriter::write(const unsigned short int& output)
  {
    writePrimitive<unsigned short int>(output);
  }

  void BinaryWriter::write(const long int& output)
  {
    writePrimitive<long int>(output);
  }

  void BinaryWriter::write(const unsigned long int& output)
  {
    writePrimitive<unsigned long int>(output);
  }

  void BinaryWriter::write(const float& output)
  {
    writePrimitive<float>(output);
  }

  void BinaryWriter::write(const double& output)
  {
    writePrimitive<double>(output);
  }

  void BinaryWriter::write(const bool& output)
  {
    writePrimitive<bool>(output);
  }

  template< typename T>
  void BinaryWriter::writePrimitive(const T& output)
  {
    writeArray((const char*)&output, sizeof(T), 1);
  }

  void BinaryWriter::writeArray(const char* output, size_t size, size_t nmemb)
  {
    if (Datalib::Util::big_endian())
    {
      /* big-endian */
      /* Write */
      setChecksum() = Datalib::Util::crc32(getChecksum(), output, size*nmemb);
      getOstream().write(output, size*nmemb);
    }
    else
    {
      /* little-endian */
      /* Swap and write and swap */
      Datalib::Util::byte_swap(const_cast<char *>(output), size, nmemb);
      setChecksum() = Datalib::Util::crc32(getChecksum(), output, size*nmemb);
      getOstream().write(output, size*nmemb);
      Datalib::Util::byte_swap(const_cast<char *>(output), size, nmemb);
    }
  }

  //-----------------------------------------
  //! Binary writer support
  BinaryBufferWriter::BinaryBufferWriter() {checksum = 0;}

  BinaryBufferWriter::BinaryBufferWriter(const std::string& s) {checksum = 0; open(s);}

  void BinaryBufferWriter::open(const std::string& s) 
  {
    f.str(s);
  }

  // Output the stream
  std::string BinaryBufferWriter::str() const
  {
    return f.str();
  }

  BinaryBufferWriter::~BinaryBufferWriter() {}


  //-----------------------------------------
  //! Binary writer support
  BinaryFileWriter::BinaryFileWriter() {checksum = 0;}

  BinaryFileWriter::BinaryFileWriter(const std::string& p) {checksum = 0; open(p);}

  void BinaryFileWriter::open(const std::string& p) 
  {
    f.open(p.c_str(),std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

    if (! is_open())
    {
      cerr << "BinaryFileWriter: error opening file: " << p;
      exit(1);
    }
  }

  void BinaryFileWriter::close()
  {
    if (is_open())
      f.close();
  }


  // Propagate status to all nodes
  bool BinaryFileWriter::is_open()
  {
    return f.is_open();
  }

  void BinaryFileWriter::flush()
  {
    if (is_open()) 
      f.flush();
  }

  BinaryFileWriter::~BinaryFileWriter() {close();}


}  // namespace Datalib::Binary
