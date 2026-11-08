#pragma once

#include "datalib/codec/binary.h"
#include "datalib/io/serial_xml.h"
#include "datalib/storage/filedb.h"
#include "datalib/model/map_obj.h"
#include <datalib/model/xml_array2d.h>
#include <datalib/model/xml_array3d.h>

namespace Datalib::TensorStorage
{
  inline constexpr int spin_count = 4;
  inline constexpr int dimension_count = 4;
  using XMLArray::Array;
  using Datalib::Storage::SerialDBKey;
  using Datalib::Storage::SerialDBData;
  using Datalib::Binary::BinaryReader;
  using Datalib::Binary::BinaryWriter;
  using Datalib::Serial::XMLReader;
  using Datalib::Serial::XMLWriter;
  using Datalib::Serial::XMLBufferWriter;
}

namespace Hadron
{
  using XMLArray::Array;
  using XMLArray::Array2d;
  using XMLArray::Array3d;
  using Datalib::Binary::BinaryReader;
  using Datalib::Binary::BinaryWriter;
  using Datalib::Serial::XMLReader;
  using Datalib::Serial::XMLWriter;
  using Datalib::Serial::XMLBufferWriter;
  using Datalib::Model::MapObject;
}
