#ifndef QREGISTRY_H
#define QREGISTRY_H
#include <QMessageBox>
#include <windows.h>
#include <tchar.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <memory>
#include "regfield.h"
class MainWindow;
#define TSIZE sizeof (TCHAR)
class QRegistry{
    bool log_on;
    //MainWindow * window;
    std::fstream str;
    std::shared_ptr<std::list<std::shared_ptr<RegField>>> full_registry;
public:
    QRegistry(bool log);
    ~QRegistry();
    std::shared_ptr<std::list<std::shared_ptr<RegField>>> get_full_registry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags);
private:
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName);
    BOOL WriteSubKey(LPTSTR keyName, LPTSTR subKeyName);
    BOOL TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags);
};
#endif // QREGISTRY_H
