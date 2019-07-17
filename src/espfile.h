#ifndef ESPFILE_H
#define ESPFILE_H


#include <string>
#include <cstring>
#include <fstream>
#include <set>
#include "record.h"


namespace ESP {

class SubRecord;

class File
{
public:

  File(const std::string &fileName);

  /**
   * write the modified file back. This can currently only update the header
   **/
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
