#ifndef ESPFILE_H
#define ESPFILE_H


#include <string>
#include <fstream>
#include <set>
#include "record.h"

#if !defined(_WIN32)
#include <cstring>
#endif

namespace ESP {

class SubRecord;

class File
{
public:

  /**
   * write the modified file back. This can currently only update the header
   **/
   File(const std::string &fileName);
#if defined(_WIN32)
  File(const std::wstring &fileName);
  void write(const std::wstring &fileName);
#endif
  void write(const std::string &fileName);

  Record readRecord();

  bool isMaster() const;
  bool isLight() const;
  bool isDummy() const;

  void setLight(bool enabled);

  uint32_t revision() const { return m_MainRecord.revision(); }

  std::string author() const { return m_Author; }
  std::string description() const { return m_Description; }
  std::set<std::string> masters() const { return m_Masters; }

private:

  void init();

  void onHEDR(const SubRecord &rec);
  void onMAST(const SubRecord &rec);
  void onCNAM(const SubRecord &rec);
  void onSNAM(const SubRecord &rec);

private:

  std::ifstream m_File;

  struct {
    float version;
    int32_t numRecords;
    uint32_t nextObjectId;
  } m_Header;

  Record m_MainRecord;

  std::string m_Author;
  std::string m_Description;

  std::set<std::string> m_Masters;

};

}

#endif // ESPFILE_H
