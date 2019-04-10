#include "record.h"
#include "espexceptions.h"

ESP::Record::Record()
  : m_Header()
  , m_Data()
  , m_OblivionStyle(false)
{
}

size_t ESP::Record::size() const
{
  return sizeof(Header) + m_Data.size() + (m_OblivionStyle ? 0 : 4);
}

bool ESP::Record::flagSet(ESP::Record::EFlag flag) const
{
  return (m_Header.flags & flag) != 0;
}

void ESP::Record::setFlag(ESP::Record::EFlag flag, bool enable)
{
  if (enable) {
    m_Header.flags |= flag;
  } else {
    m_Header.flags &= ~flag;
  }
}

bool ESP::Record::readFrom(std::istream &stream)
{
  if (!stream.read(reinterpret_cast<char*>(&m_Header), sizeof(Header))) {
    if (stream.gcount() == 0) {
      return false;
    } else {
      throw ESP::InvalidRecordException("record incomplete");
    }
  }

  stream.read(m_VersionInfo, 4);
  if (memcmp(m_VersionInfo, "HEDR", 4) == 0) {
    m_OblivionStyle = true;
    stream.seekg(-4, std::istream::cur);
  } // skyrim has some version data here I don't know how to interpret anyway

  m_Data.resize(m_Header.dataSize);
  stream.read(reinterpret_cast<char*>(&m_Data[0]), m_Header.dataSize);
  if (!stream) {
    throw ESP::InvalidRecordException("record incomplete");
  }
  return true;
}

void ESP::Record::writeTo(std::ostream &stream)
{
  stream.write(reinterpret_cast<const char*>(&m_Header), sizeof(Header));
  if (!m_OblivionStyle) {
    stream.write(m_VersionInfo, 4);
  }
  stream.write(reinterpret_cast<const char*>(&m_Data[0]), m_Data.size());
}

