#ifndef QREGISTRY_H
#define QREGISTRY_H
#include <QMessageBox>
#include <windows.h>
#include <tchar.h>
#include <fstream>
#include <sstream>
#include <vector>
class MainWindow;
#define TSIZE sizeof (TCHAR)
class QRegistry{
    MainWindow * window;
    std::fstream str;
    std::vector<const wchar_t *> exception;
public:
    QRegistry(MainWindow* window);
    ~QRegistry();
    BOOL TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags);
private:
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName);
    BOOL WriteSubKey(LPTSTR keyName, LPTSTR subKeyName);
    bool is_exception(const wchar_t * str);
};
#endif // QREGISTRY_H
