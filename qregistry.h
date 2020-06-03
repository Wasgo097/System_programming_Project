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
    //MainWindow * window;
    std::fstream str;
    std::shared_ptr<std::list<std::shared_ptr<RegField>>> full_registry;
    std::shared_ptr<std::list<std::shared_ptr<RegField>>> one_key;
public:
    QRegistry(bool log);
    ~QRegistry();
    std::shared_ptr<std::list<std::shared_ptr<RegField>>> get_full_registry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags);
    std::shared_ptr<std::list<std::shared_ptr<RegField>>> get_one_key(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey,std::vector<string>&subKeys);
private:
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName);
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName,std::shared_ptr<std::list<std::shared_ptr<RegField>>>& output);
    BOOL WriteSubKey(LPTSTR keyName, LPTSTR subKeyName);
    BOOL TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags);
    BOOL FindKey(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, BOOL next,std::queue<string>&subKeys){
        std::shared_ptr<std::list<std::shared_ptr<RegField>>> temp(new std::list<std::shared_ptr<RegField>>);
        HKEY hSubKey;
        DWORD valueType, index;
        DWORD numSubKeys, maxSubKeyLen, numValues, maxValueNameLen, maxValueLen;
        DWORD subKeyNameLen, valueNameLen, valueLen;
        FILETIME lastWriteTime;
        LPTSTR subKeyName, valueName;
        LPBYTE value;
        BOOL flag;
        // Having a large array such as fullSubKeyName on the stack is a bad idea or, if you prefer, an extemely poor programming practice, even a crime (I plead no contest)
        /* 1) It can consume lots of space as you traverse the directory
              * 2) You risk a stack overflow, which is a security risk
              * 3) You cannot deal with long paths (> MAX_PATH), using the \\?\ prefix
              * SUGGESTION: See lsW (Chapter 3) for a better implementation and fix this program accordingly.
              */
        TCHAR fullSubKeyName[MAX_PATH + 1];
        /* Open up the key handle. */
        if (RegOpenKeyEx(hKey, subKey, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS){
            if(log_on)
            str<<"exc3 cannot open key"<<std::endl;
            return false;
        }
        /*  Find max size info regarding the key and allocate storage */
        if (RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &numSubKeys,&maxSubKeyLen,NULL,&numValues,&maxValueNameLen,&maxValueLen,NULL,&lastWriteTime) != ERROR_SUCCESS){
            if(log_on)
            str<<"exc4 search info about key"<<std::endl;
            return false;
        }
        subKeyName =(LPTSTR) malloc(TSIZE * (maxSubKeyLen + 1));   /* size in bytes */
        valueName =(LPTSTR) malloc(TSIZE * (maxValueNameLen + 1));
        value =(LPBYTE) malloc(maxValueLen);      /* size in bytes */
        /*First pass for key-value pairs.
            Important assumption: No one edits the registry under this subkey
            during this loop. Doing so could change add new values */
        swprintf_s(fullSubKeyName, _T("%s\\%s"), fullKeyName, subKeyName);
        if(!next){
            for (index = 0; index < numValues; index++) {
                valueNameLen = maxValueNameLen + 1; /* A very common bug is to forget to set */
                valueLen = maxValueLen + 1;     /* these values; both are in/out params  */
                RegEnumValue(hSubKey, index, valueName, &valueNameLen, NULL, &valueType, value, &valueLen);
                WriteValue(valueName, valueType, value, valueLen,fullSubKeyName,temp);
                /*  If you wanted to change a value, this would be the place to do it.
                        RegSetValueEx(hSubKey, valueName, 0, valueType, pNewValue, NewValueSize); */
            }
        }
        //Second pass for subkeys
        for (index = 0; index < numSubKeys; index++) {
            subKeyNameLen = maxSubKeyLen + 1;
            RegEnumKeyEx(hSubKey, index, subKeyName, &subKeyNameLen, NULL,NULL, NULL, &lastWriteTime);
            //WriteSubKey(fullKeyName, subKeyName);
            /*  Display subkey components if -R is specified */
            if (next) {
                swprintf_s(fullSubKeyName, _T("%s\\%s"), fullKeyName, subKeyName);
                FindKey(hSubKey, fullSubKeyName, subKeyName, flag,subKeys);
            }
        }
        free(subKeyName);
        free(valueName);
        free(value);
        RegCloseKey(hSubKey);
        return true;
    }
};
#endif // QREGISTRY_H
