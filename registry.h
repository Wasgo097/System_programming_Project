#ifndef REGISTRY_H
#define REGISTRY_H
#include <QMessageBox>
#include <windows.h>
#include <tchar.h>
#include "mainwindow.h"
#define TSIZE sizeof (TCHAR)
class Registry{
    MainWindow * window;
public:
    Registry(MainWindow* window){
        this->window=window;
    }
    BOOL TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags){
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
            if (result == ERROR_SUCCESS && GetLastError() == 0)
                DisplayPair(valueName, valueType, value, valueLen);
            /*  If you wanted to change a value, this would be the place to do it.
                    RegSetValueEx(hSubKey, valueName, 0, valueType, pNewValue, NewValueSize); */
        }
        //Second pass for subkeys
        for (index = 0; index < numSubKeys; index++) {
            subKeyNameLen = maxSubKeyLen + 1;
            result = RegEnumKeyEx(hSubKey, index, subKeyName, &subKeyNameLen, NULL,
                NULL, NULL, &lastWriteTime);
            if (GetLastError() == 0) {
                DisplaySubKey(fullKeyName, subKeyName, &lastWriteTime, flags);
                /*  Display subkey components if -R is specified */
                if (recursive) {
                    //_stprintf(fullSubKeyName, _T("%s\\%s"), fullKeyName, subKeyName);
                    swprintf_s(fullSubKeyName, _T("%s\\%s"), fullKeyName, subKeyName);
                    //
                    TraverseRegistry(hSubKey, fullSubKeyName, subKeyName, flags);
                }
            }
        }
        //_tprintf(_T("\n"));
        free(subKeyName);
        free(valueName);
        free(value);
        RegCloseKey(hSubKey);
        return TRUE;
    }
private:
    BOOL DisplayPair(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen){
        LPBYTE pV = value;
        DWORD i;
        //_tprintf(_T("\n%s = "), valueName);
        switch (valueType) {
        case REG_FULL_RESOURCE_DESCRIPTOR: /* 9: Resource list in the hardware description */
        case REG_BINARY: /*  3: Binary data in any form. */
           // for (i = 0; i < valueLen; i++, pV++)
                window->add_to_output("binary");
                //_tprintf(_T(" %x"), *pV);
            break;
        case REG_DWORD: /* 4: A 32-bit number. */
            //_tprintf(_T("%x"), (DWORD)*value);
            window->add_to_output("dword");
            break;
        case REG_EXPAND_SZ: /* 2: null-terminated string with unexpanded references to environment variables (for example, “%PATH%”). */
        case REG_MULTI_SZ: /* 7: An array of null-terminated strings, terminated by two null characters. */
        case REG_SZ: /* 1: A null-terminated string. */
            //_tprintf(_T("%s"), (LPTSTR)value);
            window->add_to_output("sz");
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
    BOOL DisplaySubKey(LPTSTR keyName, LPTSTR subKeyName, PFILETIME pLastWrite, LPBOOL flags)    {
        BOOL longList = flags[1];
        SYSTEMTIME sysLastWrite;
        QString temp=QString::fromWCharArray(keyName);
        //_tprintf(_T("\n%s"), keyName);
        if (_tcslen(subKeyName) > 0){
            temp+="\\";
            temp+=QString::fromWCharArray(subKeyName);
            //_tprintf(_T("\\%s "), subKeyName);
        }
        //
        window->add_to_output(temp);
        if (longList) {
//            FileTimeToSystemTime(pLastWrite, &sysLastWrite);
//            _tprintf(_T("        %02d/%02d/%04d %02d:%02d:%02d"),
//                sysLastWrite.wMonth, sysLastWrite.wDay,
//                sysLastWrite.wYear, sysLastWrite.wHour,
//                sysLastWrite.wMinute, sysLastWrite.wSecond);
        }
        return TRUE;
    }
};

#endif // REGISTRY_H
