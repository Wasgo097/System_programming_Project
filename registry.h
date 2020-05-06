#ifndef REGISTRY_H
#define REGISTRY_H
#include <QMessageBox>
#include <windows.h>
#include <tchar.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "mainwindow.h"
#define TSIZE sizeof (TCHAR)
class Registry{
    MainWindow * window;
    std::fstream str;
    std::vector<const wchar_t *> exception;
public:
    Registry(MainWindow* window){
        this->window=window;
        str.open("logs.txt",std::fstream::in | std::fstream::out | std::fstream::app);
        if(str.good())
            qDebug()<<"good";
        //exception.push_back(L"HKEY_USERS\\S-1-5-21-2315024851-3994538975-640974175-1001\\Software\\Classes\\VirtualStore");
    }
    ~Registry(){
        str.close();
    }
    BOOL TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags){
        //if(fullKeyName==L"HKEY_USERS\\S-1-5-21-2315024851-3994538975-640974175-1001\\Software\\Classes\\VirtualStore"){
        //int size=wcslen(L"HKEY_USERS\\S-1-5-21-2315024851-3994538975-640974175-1001\\Software\\Classes\\VirtualStore");
        if(is_exception(fullKeyName)){
            qDebug()<<"Jestem tutaj";
            return true;
        }
        HKEY hSubKey;
        BOOL recursive = flags[0];
        LONG result;
        DWORD valueType, index;
        DWORD numSubKeys, maxSubKeyLen, numValues, maxValueNameLen, maxValueLen;
        DWORD subKeyNameLen, valueNameLen, valueLen;
        FILETIME lastWriteTime;
        LPTSTR subKeyName, valueName;
        LPBYTE value;
        // Having a large array such as fullSubKeyName on the stack is a bad idea or, if you prefer, an extemely poor programming practice, even a crime (I plead no contest)
         /* 1) It can consume lots of space as you traverse the directory
          * 2) You risk a stack overflow, which is a security risk
          * 3) You cannot deal with long paths (> MAX_PATH), using the \\?\ prefix
          *    SUGGESTION: See lsW (Chapter 3) for a better implementation and fix this program accordingly.
          */
        TCHAR fullSubKeyName[MAX_PATH + 1];
        /* Open up the key handle. */
        if (RegOpenKeyEx(hKey, subKey, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS){
//            QMessageBox msg(window);
//            msg.setIcon(QMessageBox::Warning);
//            msg.setText("Nie można otworzyć klucza");
//            //msg.setDetailedText("Skontaktuj się z twórcą oprogramowania");
//            msg.setWindowTitle("Błąd dostępu do rejestru");
//            msg.setStandardButtons(QMessageBox::Ok);
//            msg.exec();
            return 0;
            //ReportError(_T("\nCannot open subkey"), 2, TRUE);
        }
        /*  Find max size info regarding the key and allocate storage */
        if (RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &numSubKeys,&maxSubKeyLen,NULL,&numValues,&maxValueNameLen,&maxValueLen,NULL,&lastWriteTime) != ERROR_SUCCESS){
//            QMessageBox msg(window);
//            msg.setIcon(QMessageBox::Warning);
//            msg.setText("Nie można wyszukać informacji o podkluczu");
//            //msg.setDetailedText("Skontaktuj się z twórcą oprogramowania");
//            msg.setWindowTitle("Błąd dostępu do rejestru");
//            msg.setStandardButtons(QMessageBox::Ok);
//            msg.exec();
            return 0;
        }
        subKeyName =(LPTSTR) malloc(TSIZE * (maxSubKeyLen + 1));   /* size in bytes */
        valueName =(LPTSTR) malloc(TSIZE * (maxValueNameLen + 1));
        value =(LPBYTE) malloc(maxValueLen);      /* size in bytes */
        /*First pass for key-value pairs.
        Important assumption: No one edits the registry under this subkey
        during this loop. Doing so could change add new values */
        for (index = 0; index < numValues; index++) {
            valueNameLen = maxValueNameLen + 1; /* A very common bug is to forget to set */
            valueLen = maxValueLen + 1;     /* these values; both are in/out params  */
            result = RegEnumValue(hSubKey, index, valueName, &valueNameLen, NULL, &valueType, value, &valueLen);
            //if (result == ERROR_SUCCESS && GetLastError() == 0)
            WriteValue(valueName, valueType, value, valueLen);
            /*  If you wanted to change a value, this would be the place to do it.
                RegSetValueEx(hSubKey, valueName, 0, valueType, pNewValue, NewValueSize); */
        }
        //Second pass for subkeys
        for (index = 0; index < numSubKeys; index++) {
            subKeyNameLen = maxSubKeyLen + 1;
            result = RegEnumKeyEx(hSubKey, index, subKeyName, &subKeyNameLen, NULL,NULL, NULL, &lastWriteTime);
            //if (GetLastError() == 0) {
            WriteSubKey(fullKeyName, subKeyName, &lastWriteTime, flags);
            /*  Display subkey components if -R is specified */
            if (recursive) {
                //_stprintf(fullSubKeyName, _T("%s\\%s"), fullKeyName, subKeyName);
                swprintf_s(fullSubKeyName, _T("%s\\%s"), fullKeyName, subKeyName);
                TraverseRegistry(hSubKey, fullSubKeyName, subKeyName, flags);
            }
            //}
        }
        //_tprintf(_T("\n"));
        free(subKeyName);
        free(valueName);
        free(value);
        RegCloseKey(hSubKey);
        return TRUE;
    }
private:
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen){
        LPBYTE pV = value;
        //QString temp=QString::fromWCharArray(key);
        unsigned long i;
        int size=wcslen(valueName);
        string name="";
        for(int i=0;i<size;i++)
            name+=(char)valueName[i];
        if(name==""){
            str<<" exc "<<std::endl;
            return false;
        }
        //str<<temp.toStdString()<<std::endl;
        str<<name<<":"<<std::endl;
        string valuee="val : ";
        std::stringstream sstream;
        switch (valueType) {
        case REG_FULL_RESOURCE_DESCRIPTOR: /* 9: Resource list in the hardware description */
        case REG_BINARY: /*  3: Binary data in any form. */
            //window->add_to_output("binary");
            //qDebug()<<"binary";
            str<<"bin "<<std::endl;
            for (i = 0; i < valueLen; i++, pV++){
//                sprintf(temp,"%x",*pV);
//                size=strlen(temp);
//                for(j=0;j<size;j++)
//                    valuee+=temp[i];
                //str<<std::hex<<(unsigned int)*pV<<" ";
                sstream<<std::hex<<(unsigned int)*pV<<" ";
            }
            sstream<<std::endl;
            valuee+=sstream.str();
            str<<valuee;
            //str<<valuee<<std::endl;
            //_tprintf(_T(" %x"), *pV);
            //qDebug()<<*pV;
            break;
        case REG_DWORD: /* 4: A 32-bit number. */
            //_tprintf(_T("%x"), (DWORD)*value);
            //window->add_to_output("dword");
            //qDebug()<<"dword";
            str<<"dword "<<std::endl;
            break;
        case REG_EXPAND_SZ: /* 2: null-terminated string with unexpanded references to environment variables (for example, “%PATH%”). */
        case REG_MULTI_SZ: /* 7: An array of null-terminated strings, terminated by two null characters. */
        case REG_SZ: /* 1: A null-terminated string. */
            //_tprintf(_T("%s"), (LPTSTR)value);
            //window->add_to_output("sz");
            //qDebug()<<"str";
            str<<"str "<<std::endl;
            break;
        case REG_DWORD_BIG_ENDIAN: /* 5:  A 32-bit number in big-endian format. */
        case REG_LINK: /* 6: A Unicode symbolic link. */
        case REG_NONE: /* 0: No defined value type. */
        case REG_RESOURCE_LIST: /* 8: A device-driver resource list. */
        default: //_tprintf(_T(" ** Cannot display value of type: %d. Exercise for reader\n"), valueType);
            break;
        }
        return TRUE;
    }
    BOOL WriteSubKey(LPTSTR keyName, LPTSTR subKeyName, PFILETIME pLastWrite, LPBOOL flags)    {
        BOOL longList = flags[1];
        //SYSTEMTIME sysLastWrite;
        QString temp=QString::fromWCharArray(keyName);
        //_tprintf(_T("\n%s"), keyName);
        if (_tcslen(subKeyName) > 0){
            temp+="\\";
            temp+=QString::fromWCharArray(subKeyName);
            //_tprintf(_T("\\%s "), subKeyName);
        }
//        if(temp=="HKEY_USERS\\S-1-5-21-2315024851-3994538975-640974175-1001\\Software\\Classes\\VirtualStore\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\009")
//        {
//            //
//            qDebug()<<"proba "+ temp;
//        }
        //qDebug()<<temp;
        str<<temp.toStdString()<<std::endl;
        //window->add_to_output(temp);
        if (longList) {
//            FileTimeToSystemTime(pLastWrite, &sysLastWrite);
//            _tprintf(_T("        %02d/%02d/%04d %02d:%02d:%02d"),
//                sysLastWrite.wMonth, sysLastWrite.wDay,
//                sysLastWrite.wYear, sysLastWrite.wHour,
//                sysLastWrite.wMinute, sysLastWrite.wSecond);
        }
        return TRUE;
    }
    bool is_exception(const wchar_t * str){
        for(const auto &x:exception){
            int sizex=wcslen(x);
            int sizestr=wcslen(str);
            if(sizex==sizestr){
                bool flag=true;
                for(int i=0;i<sizex;i++){
                    if(x[i]!=str[i]){
                        flag=false;
                        break;
                    }
                }
                if(flag==true)
                    return true;
            }
        }
        return false;
    }
};

#endif // REGISTRY_H
