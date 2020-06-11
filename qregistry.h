#ifndef QREGISTRY_H
#define QREGISTRY_H
#include <QMessageBox>
#include <windows.h>
#include <tchar.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <list>
#include <memory>
#include "regfield.h"
class MainWindow;
#define TSIZE sizeof (TCHAR)
class QRegistry{
    bool log_on;
    std::fstream str;
    std::shared_ptr<std::list<std::shared_ptr<RegField>>> full_registry;
public:
    QRegistry(bool log);
    ~QRegistry();
    std::shared_ptr<std::list<std::shared_ptr<RegField>>> get_full_registry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags);
    std::shared_ptr<std::list<std::shared_ptr<RegField>>> get_one_key(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey);
    BOOL Import(std::shared_ptr<std::queue<std::shared_ptr<RegField>>> & keys){
        bool flag=true;
        while(!keys->empty()){
            RegField actual=*keys->front();
            QString temp=QString::fromStdString((string)actual);
            QStringList field=temp.split("|");
            qDebug()<<field;
            keys->pop();
        }
        return flag;
    }
private:
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName);
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName,std::shared_ptr<std::list<std::shared_ptr<RegField>>>& key);
    BOOL WriteSubKey(LPTSTR keyName, LPTSTR subKeyName);
    BOOL TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags);
    VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printErrorMessage){
        /* General-purpose function for reporting system errors.
                Obtain the error number and convert it to the system error message.
                Display this information and the user-specified message to the standard error device.
                userMessage:                Message to be displayed to standard error device.
                exitCode:                0 - Return.
                                                > 0 - ExitProcess with this code.
                printErrorMessage:        Display the last system error message if this flag is set. */
        DWORD eMsgLen, errNum = GetLastError();
        LPTSTR lpvSysMsg;
        _ftprintf(stderr, _T("%s\n"), userMessage);
        if (printErrorMessage) {
            eMsgLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, errNum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpvSysMsg, 0, NULL);
            std::wstring temp=lpvSysMsg;
            if (eMsgLen > 0){
                //_ftprintf(stderr, _T("%s\n"), lpvSysMsg);
                qDebug()<<temp<<" "<<temp.c_str();
            }
            else{
                //_ftprintf(stderr, _T("Last Error Number; %d.\n"), errNum);
                qDebug()<<"Last Error Number"<<errNum;
            }
            if (lpvSysMsg != NULL) LocalFree(lpvSysMsg); /* Explained in Chapter 5. */
        }
    }
};
#endif // QREGISTRY_H
