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
    std::shared_ptr<std::list<std::shared_ptr<RegField>>> get_one_key(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey){
        std::shared_ptr<std::list<std::shared_ptr<RegField>>> registry(new std::list<std::shared_ptr<RegField>>());
        HKEY hsubkey;
        DWORD valueType, index;
        DWORD numSubKeys, maxSubKeyLen, numValues, maxValueNameLen, maxValueLen;
        DWORD valueNameLen, valueLen;
        FILETIME lastWriteTime;
        LPTSTR subKeyName, valueName;
        LPBYTE value;
        TCHAR fullSubKeyName[MAX_PATH + 1];
        if(RegOpenKeyEx(hKey,subKey,0,KEY_ALL_ACCESS,&hsubkey)!=ERROR_SUCCESS){
            throw "Nie otworzono";
        }
        else{
            qDebug()<<"Otworzono";
        }
        if (RegQueryInfoKey(hsubkey, NULL, NULL, NULL, &numSubKeys,&maxSubKeyLen,NULL,&numValues,&maxValueNameLen,&maxValueLen,NULL,&lastWriteTime) != ERROR_SUCCESS){
            throw"Nie przeszukano";
        }
        else{
            qDebug()<<"Wydobyto info";
        }
        subKeyName =(LPTSTR) malloc(TSIZE * (maxSubKeyLen + 1));
        valueName =(LPTSTR) malloc(TSIZE * (maxValueNameLen + 1));
        value =(LPBYTE) malloc(maxValueLen);
        swprintf_s(fullSubKeyName, _T("%s\\%s"), fullKeyName, subKeyName);
        for (index = 0; index < numValues; index++) {
            valueNameLen = maxValueNameLen + 1;
            valueLen = maxValueLen + 1;
            RegEnumValue(hsubkey, index, valueName, &valueNameLen, NULL, &valueType, value, &valueLen);
            WriteValue(valueName, valueType, value, valueLen,fullSubKeyName,registry);
        }
        free(subKeyName);
        free(valueName);
        free(value);
        RegCloseKey(hsubkey);
        return registry;
    }
private:
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName);
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName,std::shared_ptr<std::list<std::shared_ptr<RegField>>>& key){
        LPBYTE pV = value;
        QString qkey=QString::fromWCharArray(fullKeyName);
        string skey=qkey.toStdString();
        unsigned long i;
        int namesize=wcslen(valueName);
        if(namesize==0){
            return false;
        }
        string name="";
        for(int i=0;i<namesize;i++)
            name+=(char)valueName[i];
        std::shared_ptr<RegField> row(new RegField());
        row->key(skey);
        row->value_name(name);
        string svalue;
        std::stringstream sstream;
        switch (valueType) {
        case REG_FULL_RESOURCE_DESCRIPTOR:
        case REG_BINARY:
            for (i = 0; i < valueLen; i++, pV++){
                sstream<<std::hex<<(unsigned int)*pV<<" ";
            }
            svalue=sstream.str();
            row->type(1);
            row->value(svalue);
            key->push_back(row);
            break;
        case REG_DWORD:
            svalue=std::to_string((DWORD)*value);
            row->type(3);
            row->value(svalue);
            key->push_back(row);
            break;
        case REG_EXPAND_SZ:
        case REG_MULTI_SZ:
        case REG_SZ:
            qkey=QString::fromWCharArray((LPTSTR)value);
            svalue=qkey.toStdString();
            row->type(2);
            row->value(svalue);
            key->push_back(row);
            break;
        default:
            return FALSE;
        }
        return TRUE;
    }
    BOOL WriteSubKey(LPTSTR keyName, LPTSTR subKeyName);
    BOOL TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags);
};
#endif // QREGISTRY_H
