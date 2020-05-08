#include "qregistry.h"
#include "mainwindow.h"
QRegistry::QRegistry(MainWindow *window,bool log){
    log_on=log;
    this->window=window;
    if(log_on){
        str.open("logs.txt",std::fstream::in | std::fstream::out | std::fstream::app);
        if(str.good())
            qDebug()<<"good";
    }
    //exception.push_back(L"HKEY_USERS\\.DEFAULT\\Control Panel\\Desktop\\");
}
QRegistry::~QRegistry(){
    str.close();
}
BOOL QRegistry::TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags){
//  if(is_exception(fullKeyName)){
//      qDebug()<<"Jestem tutaj";
//      return true;
//  }
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
//      QMessageBox msg(window);
//      msg.setIcon(QMessageBox::Warning);
//      msg.setText("Nie można otworzyć klucza");
//      //msg.setDetailedText("Skontaktuj się z twórcą oprogramowania");
//      msg.setWindowTitle("Błąd dostępu do rejestru");
//      msg.setStandardButtons(QMessageBox::Ok);
//      msg.exec();
        if(log_on)
        str<<"exc3 cannot open key"<<std::endl;
        return false;
    }
    /*  Find max size info regarding the key and allocate storage */
    if (RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &numSubKeys,&maxSubKeyLen,NULL,&numValues,&maxValueNameLen,&maxValueLen,NULL,&lastWriteTime) != ERROR_SUCCESS){
//      QMessageBox msg(window);
//      msg.setIcon(QMessageBox::Warning);
//      msg.setText("Nie można wyszukać informacji o podkluczu");
//      //msg.setDetailedText("Skontaktuj się z twórcą oprogramowania");
//      msg.setWindowTitle("Błąd dostępu do rejestru");
//      msg.setStandardButtons(QMessageBox::Ok);
//      msg.exec();
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
                RegSetValueEx(hSubKey, valueName, 0, valueType, pNewValue, NewValueSize); */
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
    QString qtemp=QString::fromWCharArray(fullKeyName);
    string stemp=qtemp.toStdString();
//        auto remove_useless_part_from_string=[](string src)->string{
//            string tmp=src;
//            int last_slash=0;
//            for(int i=0;i<tmp.length();i++){
//                if(tmp[i]=='\\'){
//                     last_slash=i;
//                     continue;
//                }
//                if((int)tmp[i]<0||(int)tmp[i]>255)
//                    break;
//            }
//            tmp.erase(tmp.begin()+last_slash-1,tmp.end());
//            return tmp;
//        };
    //stemp=remove_useless_part_from_string(stemp);
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
    //str<<temp.toStdString()<<std::endl;
    std::shared_ptr<RegField> row(new RegField);
    row->key(stemp);
    if(log_on)
        str<<stemp<<std::endl;
    row->value_name(name);
    if(log_on)
        str<<name<<":"<<std::endl;
    string svalue="val : ";
    std::stringstream sstream;
    switch (valueType) {
    case REG_FULL_RESOURCE_DESCRIPTOR: /* 9: Resource list in the hardware description */
    case REG_BINARY: /*  3: Binary data in any form. */
        if(log_on)
            str<<"bin "<<std::endl;
        for (i = 0; i < valueLen; i++, pV++){
            sstream<<std::hex<<(unsigned int)*pV<<" ";
        }
        svalue+=sstream.str();
        if(log_on)
            str<<svalue<<std::endl;
        row->type(1);
        row->value(svalue.erase(0,6));
        full_registry->push_back(row);
        break;
    case REG_DWORD: /* 4: A 32-bit number. */
        //_tprintf(_T("%x"), (DWORD)*value);
        //window->add_to_output("dword");
        //qDebug()<<"dword";
        if(log_on)
            str<<"dword "<<std::endl;
        stemp=std::to_string((DWORD)*value);
        if(log_on)
            str<<stemp<<std::endl<<std::endl;
        row->type(3);
        row->value(svalue.erase(0,6));
        full_registry->push_back(row);
        break;
    case REG_EXPAND_SZ: /* 2: null-terminated string with unexpanded references to environment variables (for example, “%PATH%”). */
    case REG_MULTI_SZ: /* 7: An array of null-terminated strings, terminated by two null characters. */
    case REG_SZ: /* 1: A null-terminated string. */
        //_tprintf(_T("%s"), (LPTSTR)value);
        //window->add_to_output("sz");
        //qDebug()<<"str";
        if(log_on)
            str<<"str "<<std::endl;
        qtemp=QString::fromWCharArray((LPTSTR)value);
        stemp=qtemp.toStdString();
        if(log_on)
            str<<stemp<<std::endl;
        row->type(2);
        row->value(svalue.erase(0,6));
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
//bool QRegistry::is_exception(const wchar_t *str){
//    int sizestr=wcslen(str);
//    for(const auto &x:exception){
//        int sizex=wcslen(x);
//        if(sizex==sizestr){
//            bool flag=true;
//            for(int i=0;i<sizex;i++){
//                if(x[i]!=str[i]){
//                    flag=false;
//                    break;
//                }
//            }
//            if(flag==true)
//                return true;
//        }
//    }
//    return false;
//}
