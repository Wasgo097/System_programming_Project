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
            try {
                RegField actual=*keys->front();
                QString temp=QString::fromStdString((string)actual);
                QStringList field=temp.split("|");
                qDebug()<<field;
                keys->pop();
                //
                size_t slash=field[0].toStdString().find("\\");
                string key=field[0].toStdString().substr(0,slash),subkey=field[0].toStdString().substr(slash+1);
                std::wstring wsubkey(subkey.begin(),subkey.end());
                qDebug()<<key.c_str()<<" "<<subkey.c_str();
                HKEY hmainkey,hkey;
                DWORD type,dwDisp;
                if(key[5]=='L')
                    hmainkey=HKEY_LOCAL_MACHINE;
                else
                    hmainkey=HKEY_USERS;
                if(field[2]=="1"){
                    type=REG_BINARY;
                    wchar_t buf[255];
                    lstrcpy(buf,field[3].toStdWString().c_str());
                    qDebug()<<lstrlen(buf)+1;
                    if(RegCreateKeyEx(hmainkey,wsubkey.c_str(),0,NULL, REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,&dwDisp)==ERROR_SUCCESS){
                        if(RegSetValueEx(hkey,field[1].toStdWString().c_str(),0,type,(LPBYTE)buf,field[3].size()*sizeof (wchar_t))==ERROR_SUCCESS)
                            qDebug()<<"Pikobello";
                        else
                            throw "Zapis";
                        RegCloseKey(hkey);
                    }
                    else
                        throw "Otwarcie";
                }
                else if(field[2]=="2"){
                    type=REG_SZ;
                    wchar_t buf[255];
                    lstrcpy(buf,field[3].toStdWString().c_str());
                    qDebug()<<lstrlen(buf)+1;
                    if(RegCreateKeyEx(hmainkey,wsubkey.c_str(),0,NULL, REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,&dwDisp)==ERROR_SUCCESS){
                        if(RegSetValueEx(hkey,field[1].toStdWString().c_str(),0,type,(LPBYTE)buf,lstrlen(buf)*sizeof (wchar_t))==ERROR_SUCCESS)
                            qDebug()<<"Pikobello";
                        else
                            throw "Zapis";
                        RegCloseKey(hkey);
                    }
                    else
                        throw "Otwarcie";
                }
                else{
                    type=REG_DWORD;
                    DWORD data=field[3].toULong();
                    if(RegCreateKeyEx(hmainkey,wsubkey.c_str(),0,NULL, REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,&dwDisp)==ERROR_SUCCESS){
                        if(RegSetValueEx(hkey,field[1].toStdWString().c_str(),0,type,(LPBYTE)&data,sizeof(DWORD))==ERROR_SUCCESS)
                            qDebug()<<"Pikobello";
                        else
                            throw "Zapis";
                        RegCloseKey(hkey);
                    }
                    else
                        throw "Otwarcie";
                }
                //qDebug()<<QString::fromWCharArray(wsubkey.c_str());
                //if(RegOpenKeyEx(hmainkey,wsubkey.c_str(),0,KEY_ALL_ACCESS,&hkey)!=ERROR_SUCCESS){
//                if(RegCreateKeyEx(hmainkey,wsubkey.c_str(),0,NULL, REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,&dwDisp)==ERROR_SUCCESS){
//                    if(RegSetValueEx(hkey,field[1].toStdWString().c_str(),0,type,data,field[3].size())==ERROR_SUCCESS)
//                        qDebug()<<"Pikobello";
//                    else
//                        throw "Zapis";
//                    RegCloseKey(hkey);
//                }
//                else
//                    throw "Otwarcie";
            } catch (const char * exc) {
                qDebug()<<exc;
                flag=false;
            }
        }
        return flag;
    }
private:
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName);
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName,std::shared_ptr<std::list<std::shared_ptr<RegField>>>& key);
    BOOL WriteSubKey(LPTSTR keyName, LPTSTR subKeyName);
    BOOL TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags);
    VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printErrorMessage);
};
#endif // QREGISTRY_H
