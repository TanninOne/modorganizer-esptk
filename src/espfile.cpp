#include "espfile.h"
#include "subrecord.h"
#include "espexceptions.h"
#include <sstream>
#include <bitset>
#if !defined(_WIN32) || !defined(_WIN64)
#include <memory>
#endif

ESP::File::File(const std::string &fileName)
{
  m_File.open(fileName, std::fstream::in | std::fstream::binary);
  init();
}

#if defined(_WIN32) || defined(_WIN64)
ESP::File::File(const std::wstring &fileName)
{
  m_File.open(fileName, std::fstream::in | std::fstream::binary);
  init();
}
#endif

class membuf : public std::basic_streambuf<char>
{
public:
  membuf(const char *start, size_t size) {
    char *startMod = const_cast<char*>(start);
    setg(startMod, startMod, startMod + size);
  }
};


void ESP::File::init()
{
  if (!m_File.is_open()) {
    throw ESP::InvalidFileException("file not found");
  }
  m_File.exceptions(std::ios_base::badbit);

  uint8_t type[4];
  if (!m_File.read(reinterpret_cast<char*>(type), 4)) {
    throw ESP::InvalidFileException("file incomplete");
  }
  if (memcmp(type, "TES4", 4) != 0) {
    throw ESP::InvalidFileException("invalid file type");
  }
  m_File.seekg(0);

  m_MainRecord = readRecord();

  const std::vector<uint8_t> &data = m_MainRecord.data();
  membuf buf(reinterpret_cast<const char*>(&data[0]), data.size());

  std::istream stream(&buf);
  while (!stream.eof() && !stream.fail()) {
    SubRecord rec;
    bool success = rec.readFrom(stream);
    if (success) {
      if (rec.type() != SubRecord::TYPE_UNKNOWN) {
        switch (rec.type()) {
          case SubRecord::TYPE_HEDR: onHEDR(rec); break;
          case SubRecord::TYPE_MAST: onMAST(rec); break;
          case SubRecord::TYPE_CNAM: onCNAM(rec); break;
          case SubRecord::TYPE_SNAM: onSNAM(rec); break;
        }
      }
    }
  }
}

#if defined(_WIN32) || defined(_WIN64)
void ESP::File::write(const std::wstring &fileName) {
#else
void ESP::File::write(const std::string &fileName) {
#endif
  // wtf? If I use an ofstream here the function crashes (0xc0000005) on destruction
  // of outFile. It even happens if we write absolutely nothing into the file.
  std::fstream outFile(fileName, std::ofstream::out | std::ofstream::binary);

  m_MainRecord.writeTo(outFile);

  m_File.seekg(m_MainRecord.size());

  static const int BUFFER_SIZE = 16 * 1024;
  std::unique_ptr<char> buffer(new char[BUFFER_SIZE]);
  std::streampos start = m_File.tellg();
  bool eof = false;
  while (!eof && !m_File.fail()) {
    m_File.read(buffer.get(), BUFFER_SIZE);
    size_t numBytes = BUFFER_SIZE;
    if (m_File.eof()) {
      eof = true;
      m_File.clear();
#if defined(_WIN32) || defined(_WIN64)
      m_File.seekg(0, SEEK_END);
#else
      m_File.seekg(0, std::fstream::end);
#endif
      numBytes = (m_File.tellg() - start) % BUFFER_SIZE;
    }
    outFile.write(buffer.get(), numBytes);
  }
  outFile.close();
}

void ESP::File::onHEDR(const SubRecord &rec)
{
  if (rec.data().size() != sizeof(m_Header)) {
    printf("invalid header size\n");
    m_Header.version = 0.0f;
    m_Header.numRecords = 1; // prevent this esp appear like a dummy
  } else {
    memcpy(&m_Header, &rec.data()[0], sizeof(m_Header));
  }
}

void ESP::File::onMAST(const SubRecord &rec)
{
  if (rec.data().size() > 0)
    m_Masters.insert(reinterpret_cast<const char*>(&rec.data()[0]));

}

void ESP::File::onCNAM(const SubRecord &rec)
{
  if (rec.data().size() > 0)
    m_Author = reinterpret_cast<const char*>(&rec.data()[0]);
}

void ESP::File::onSNAM(const SubRecord &rec)
{
  if (rec.data().size() > 0)
    m_Description = reinterpret_cast<const char*>(&rec.data()[0]);
}


ESP::Record ESP::File::readRecord()
{
  ESP::Record rec;
  rec.readFrom(m_File);
  return rec;
}

bool ESP::File::isMaster() const
{
  return m_MainRecord.flagSet(Record::FLAG_MASTER);
}

bool ESP::File::isLight() const
{
  return m_MainRecord.flagSet(Record::FLAG_LIGHT);
}

bool ESP::File::isDummy() const
{
  return m_Header.numRecords == 0;
}

void ESP::File::setLight(bool enabled)
{
  m_MainRecord.setFlag(Record::FLAG_LIGHT, enabled);
}
