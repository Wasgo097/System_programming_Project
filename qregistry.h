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
#include "RegField.h"
class MainWindow;
#define TSIZE sizeof (TCHAR)
class QRegistry{
    bool log_on;
    MainWindow * window;
    std::fstream str;
    //std::shared_ptr<std::list<std::shared_ptr<RegField>>> full_registry;
    //std::vector<const wchar_t *> exception;
public:
    QRegistry(MainWindow* window,bool log);
    ~QRegistry();
//    std::shared_ptr<std::list<std::shared_ptr<RegField>>> get_full_registry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags){
//        full_registry=std::make_shared<std::list<std::shared_ptr<RegField>>>();
//        TraverseRegistry(hKey,fullKeyName,subKey,flags);
//        return full_registry;
//    }
    BOOL TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags);
private:
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName);
    BOOL WriteSubKey(LPTSTR keyName, LPTSTR subKeyName);
    //bool is_exception(const wchar_t * str);
};
#endif // QREGISTRY_H
