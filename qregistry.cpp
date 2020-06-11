#include "mainwindow.h"
#include "qregistry.h"
QRegistry::QRegistry(bool log){
    log_on=log;
    if(log_on){
        str.open("logs.txt",std::fstream::in | std::fstream::out | std::fstream::app);
        if(str.good())
            qDebug()<<"good";
    }
}
QRegistry::~QRegistry(){
    str.close();
}
std::shared_ptr<std::list<std::shared_ptr<RegField> > > QRegistry::get_full_registry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags){
    //full_registry=std::make_shared<std::list<std::shared_ptr<RegField>>>();
    full_registry.reset(new std::list<std::shared_ptr<RegField>>());
    TraverseRegistry(hKey,fullKeyName,subKey,flags);
    return full_registry;
}
std::shared_ptr<std::list<std::shared_ptr<RegField>>> QRegistry::get_one_key(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey){
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
        ReportError(_T("\nCannot open subkey"), 2, TRUE);
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
BOOL QRegistry::TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags){
    HKEY hSubKey;
    BOOL recursive = flags[0];
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
    for (index = 0; index < numValues; index++) {
        valueNameLen = maxValueNameLen + 1; /* A very common bug is to forget to set */
        valueLen = maxValueLen + 1;     /* these values; both are in/out params  */
        RegEnumValue(hSubKey, index, valueName, &valueNameLen, NULL, &valueType, value, &valueLen);
        WriteValue(valueName, valueType, value, valueLen,fullSubKeyName);
        /*  If you wanted to change a value, this would be the place to do it.
                RegSetValueEx(hSubKey, valueName, 0, valueType, pNewValue, NewValueSize);*/
    }
    //Second pass for subkeys
    for (index = 0; index < numSubKeys; index++) {
        subKeyNameLen = maxSubKeyLen + 1;
        RegEnumKeyEx(hSubKey, index, subKeyName, &subKeyNameLen, NULL,NULL, NULL, &lastWriteTime);
        //WriteSubKey(fullKeyName, subKeyName);
        /*  Display subkey components if -R is specified */
        if (recursive) {
            swprintf_s(fullSubKeyName, _T("%s\\%s"), fullKeyName, subKeyName);
            TraverseRegistry(hSubKey, fullSubKeyName, subKeyName, flags);
        }
    }
    free(subKeyName);
    free(valueName);
    free(value);
    RegCloseKey(hSubKey);
    return TRUE;
}
BOOL QRegistry::WriteValue(LPTSTR valueName, DWORD valueType, LPBYTE value, DWORD valueLen, LPTSTR fullKeyName){
    LPBYTE pV = value;
    QString qkey=QString::fromWCharArray(fullKeyName);
    string skey=qkey.toStdString();
    unsigned long i;
    int namesize=wcslen(valueName);
    if(namesize==0){
        if(log_on)
            str<<"exc1 value without name"<<std::endl;
        return false;
    }
    string name="";
    for(int i=0;i<namesize;i++)
        name+=(char)valueName[i];
    std::shared_ptr<RegField> row(new RegField());
    row->key(skey);
    if(log_on)
        str<<skey<<std::endl;
    row->value_name(name);
    if(log_on)
        str<<name<<":"<<std::endl;
    string svalue;
    std::stringstream sstream;
    switch (valueType) {
    case REG_FULL_RESOURCE_DESCRIPTOR: /* 9: Resource list in the hardware description */
    case REG_BINARY: /*  3: Binary data in any form. */
        if(log_on)
            str<<"bin "<<std::endl;
        for (i = 0; i < valueLen; i++, pV++){
            sstream<<std::hex<<(unsigned int)*pV<<" ";
        }
        svalue=sstream.str();
        if(log_on)
            str<<"val : "<<svalue<<std::endl;
        row->type(1);
        row->value(svalue);
        full_registry->push_back(row);
        break;
    case REG_DWORD: /* 4: A 32-bit number. */
        if(log_on)
            str<<"dword "<<std::endl;
        svalue=std::to_string((DWORD)*value);
        if(log_on)
            str<<"val : "<<svalue<<std::endl;
        row->type(3);
        row->value(svalue);
        full_registry->push_back(row);
        break;
    case REG_EXPAND_SZ: /* 2: null-terminated string with unexpanded references to environment variables (for example, “%PATH%”). */
    case REG_MULTI_SZ: /* 7: An array of null-terminated strings, terminated by two null characters. */
    case REG_SZ: /* 1: A null-terminated string. */
        if(log_on)
            str<<"str "<<std::endl;
        qkey=QString::fromWCharArray((LPTSTR)value);
        svalue=qkey.toStdString();
        if(log_on)
            str<<"val : "<<svalue<<std::endl;
        row->type(2);
        row->value(svalue);
        full_registry->push_back(row);
        break;
    case REG_DWORD_BIG_ENDIAN: /* 5:  A 32-bit number in big-endian format. */
    case REG_LINK: /* 6: A Unicode symbolic link. */
    case REG_NONE: /* 0: No defined value type. */
    case REG_RESOURCE_LIST: /* 8: A device-driver resource list. */
    default: //_tprintf(_T(" ** Cannot display value of type: %d. Exercise for reader\n"), valueType);
        if(log_on)
            str<<"exc2 unknow type of value"<<std::endl;
        return false;
    }
    return TRUE;
}
WINBOOL QRegistry::WriteValue(LPTSTR valueName, DWORD valueType, LPBYTE value, DWORD valueLen, LPTSTR fullKeyName, std::shared_ptr<std::list<std::shared_ptr<RegField>>> &key){
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
WINBOOL QRegistry::WriteSubKey(LPTSTR keyName, LPTSTR subKeyName){
    QString temp=QString::fromWCharArray(keyName);
    if (_tcslen(subKeyName) > 0){
        temp+="\\";
        temp+=QString::fromWCharArray(subKeyName);
    }
    //qDebug()<<temp;
    //str<<temp.toStdString()<<std::endl;
    //window->add_to_output(temp);
    return TRUE;
}
