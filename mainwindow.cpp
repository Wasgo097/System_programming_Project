#include "mainwindow.h"
#include "qregistry.h"
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    std::fstream str;
    str.open("config");
    if(str.good()){
        try {
            str>>_ip>>_port;
            str.close();
            _socket_mtx=std::make_shared<std::mutex>();
            _connector=true;
            _login_status=true;//temp
        }
        catch (...) {
            QMessageBox msg(this);
            msg.setIcon(QMessageBox::Warning);
            msg.setText("Błąd zawartości pliku konfiguracyjnego");
            msg.setDetailedText("Skontaktuj się z twórcą oprogramowania");
            msg.setWindowTitle("Błąd pliku konfiguracyjnego");
            msg.setStandardButtons(QMessageBox::Ok);
            msg.exec();
        }
    }
    else{
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Warning);
        msg.setText("Nie udało się otworzyć pliku konfiguracyjnego");
        msg.setDetailedText("Skontaktuj się z twórcą oprogramowania");
        msg.setWindowTitle("Błąd pliku konfiguracyjnego");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
    }
}
MainWindow::~MainWindow(){
    if(_thr_full_archive->isRunning())_thr_full_archive->wait();
    delete ui;
}
void MainWindow::add_to_output(QString txt){
    ui->output->append(txt);
}
void MainWindow::on_btn_login_clicked(){
    if(_connector){
        if(_login_status){
            QMessageBox msg(this);
            msg.setIcon(QMessageBox::Information);
            msg.setText("Jakiś użykownik jest już zalogowany");
            msg.setWindowTitle("Błąd logowania");
            msg.setDetailedText("Wyloguj się");
            msg.setStandardButtons(QMessageBox::Ok);
            msg.exec();
            return;
        }
        //login_process
        string nick=ui->log_nick->text().toStdString();
        string pass=ui->log_pass->text().toStdString();
        std::regex nick_regex("[\\w]{5,}");
        std::regex pass_regex("[\\w !@#$%&*_]{6,}");
        if(std::regex_search(nick,nick_regex)&&std::regex_search(pass,pass_regex)){
            qDebug()<<"Poprawne passy, zalogowano "<<nick.c_str()<<" "<<pass.c_str();
            if(Connect_socket()){
                connect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_log_in);
                _login_status=true;
                disconnect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_log_in);
            }
            else{
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Information);
                msg.setText("Nie udało się połączyć z serwerem");
                msg.setWindowTitle("Błąd połączenia");
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
            }
        }
        else{
            QMessageBox msg(this);
            msg.setIcon(QMessageBox::Information);
            msg.setText("Podana wartość nicku lub hasła jest niepoprawna.");
            msg.setWindowTitle("Niepoprawny nick i/lub haslo!");
            msg.setDetailedText("Hasło musi zawierać min 6 znaków, może zawierać znaki specjalne !@#$%&*. Nick musi zawierać 5 znaków");
            msg.setStandardButtons(QMessageBox::Ok);
            msg.exec();
            return;
        }
    }
    else{
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Warning);
        msg.setText("Nie udało się otworzyć pliku konfiguracyjnego i pobrać z niego danych");
        msg.setDetailedText("Skontaktuj się z twórcą oprogramowania");
        msg.setWindowTitle("Błąd pliku konfiguracyjnego");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
    }
}
void MainWindow::on_btn_registration_clicked(){
    if(_connector){
        if(!_login_status){
            //registration process
            string nick=ui->reg_nick->text().toStdString();
            string pass=ui->reg_pass->text().toStdString();
            std::regex nick_regex("[\\w]{5,}");
            std::regex pass_regex("[\\w !@#$%&*_]{6,}");
            if(std::regex_search(nick,nick_regex)&&std::regex_search(pass,pass_regex)){
                qDebug()<<"Poprawme passy, zarejestrowano "<<nick.c_str()<<" "<<pass.c_str();
                if(Connect_socket()){
                    connect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_sign_in);
                    disconnect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_sign_in);
                }
                else{
                    QMessageBox msg(this);
                    msg.setIcon(QMessageBox::Information);
                    msg.setText("Nie udało się połączyć z serwerem");
                    msg.setWindowTitle("Błąd połączenia");
                    msg.setStandardButtons(QMessageBox::Ok);
                    msg.exec();
                }
            }
            else{
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Information);
                msg.setText("Podana wartość nicku lub hasła jest niepoprawna.");
                msg.setWindowTitle("Niepoprawny nick i/lub haslo!");
                msg.setDetailedText("Hasło musi zawierać min 6 znaków, może zawierać znaki specjalne !@#$%&*. Nick musi zawierać 5 znaków");
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
                return;
            }
        }
        else{
            QMessageBox msg(this);
            msg.setIcon(QMessageBox::Information);
            msg.setText("Nie można zarejstrować nowego użytkownika, wyloguj się z aktualnego konta.");
            msg.setWindowTitle("Błąd rejestracji użytkownika!");
            msg.setStandardButtons(QMessageBox::Ok);
            msg.exec();
        }
    }
    else{
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Warning);
        msg.setText("Nie udało się otworzyć pliku konfiguracyjnego i pobrać z niego danych");
        msg.setDetailedText("Skontaktuj się z twórcą oprogramowania");
        msg.setWindowTitle("Błąd pliku konfiguracyjnego");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
    }
}
void MainWindow::on_full_archive_clicked(){
    if(_login_status){
        if(ui->hkey_u->isChecked()||ui->hkey_lm->isChecked()){
            bool hkey_lm=false;
            if(ui->hkey_lm->isChecked())
                hkey_lm=true;
            if(_thr_full_archive.get()==nullptr){
                _thr_full_archive=std::make_unique<Full_Archive_THR>(_socket,_socket_mtx,hkey_lm);
                connect(_thr_full_archive.get(),&Full_Archive_THR::started,this,&MainWindow::full_arch_start);
                connect(_thr_full_archive.get(),&Full_Archive_THR::finished,this,&MainWindow::full_arch_end);
                _thr_full_archive->start();
            }
            else{
                if(_thr_full_archive->isRunning()){
                    QMessageBox msg(this);
                    msg.setIcon(QMessageBox::Information);
                    msg.setText("Trwa archiwizacja rejestru.");
                    msg.setWindowTitle("Pelna archiwizacja rejestru");
                    msg.setStandardButtons(QMessageBox::Ok);
                    msg.exec();
                }
                else{
                    disconnect(_thr_full_archive.get(),&Full_Archive_THR::started,this,&MainWindow::full_arch_start);
                    disconnect(_thr_full_archive.get(),&Full_Archive_THR::finished,this,&MainWindow::full_arch_end);
                    _thr_full_archive=std::make_unique<Full_Archive_THR>(_socket,_socket_mtx,hkey_lm);
                    connect(_thr_full_archive.get(),&Full_Archive_THR::started,this,&MainWindow::full_arch_start);
                    connect(_thr_full_archive.get(),&Full_Archive_THR::finished,this,&MainWindow::full_arch_end);
                    _thr_full_archive->start();
                }
            }
        }
    }
    else{
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Warning);
        msg.setText("Nie można archiwizować rejestru gdy użytkownik nie jest zalogowany.");
        msg.setDetailedText("Zaloguj się");
        msg.setWindowTitle("Użytkownik niezalogowany");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
    }
}
void MainWindow::read_log_in(){
    while(_socket->canReadLine()){
        ui->output->append(_socket->readLine().trimmed());
    }
}
void MainWindow::read_sign_in(){
    while(_socket->canReadLine()){
        ui->output->append(_socket->readLine().trimmed());
    }
}
void MainWindow::read_register_save(){
    while(_socket->canReadLine()){
        ui->output->append(_socket->readLine().trimmed());
    }
}
void MainWindow::read_register_load(){
    while(_socket->canReadLine()){
        ui->output->append(_socket->readLine().trimmed());
    }
}
void MainWindow::full_arch_start(){
    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Information);
    msg.setText("Rozpoczeto pelna archwizacje rejestru.");
    msg.setWindowTitle("Pelna archiwizacja rejestru");
    msg.setStandardButtons(QMessageBox::Ok);
    msg.exec();
}
void MainWindow::full_arch_end(){
    QMessageBox msg;
    msg.setIcon(QMessageBox::Information);
    msg.setText("Ukonczono archwizacje rejestru.");
    msg.setWindowTitle("Pelna archiwizacja rejestru");
    msg.setStandardButtons(QMessageBox::Ok);
    msg.exec();
}
string MainWindow::get_time_to_send(){
    string temp("");
    QDateTime curr=QDateTime::currentDateTime();
    temp+=std::to_string(curr.date().month());
    temp+='_'+std::to_string(curr.date().day());
    temp+='_'+std::to_string(curr.date().year());
    temp+='_'+std::to_string(curr.time().hour());
    temp+='_'+std::to_string(curr.time().minute());
    return temp;
}
bool MainWindow::Connect_socket(){
    if(_connector==false)return false;
    if(_socket.get()==nullptr||!_connected){
        _socket=std::make_shared<QTcpSocket>(this);
        QString ip_temp=QString::fromStdString(_ip);
        quint16 port_temp=std::stoi(_port);
        _socket->connectToHost(ip_temp,port_temp);
        if(_socket->waitForConnected()){
            qDebug()<<"Polaczono";
            _connected=true;
            return true;
        }
        else{
            _connected=true;//temp
            return true;    //temp
            qDebug()<<"Nie polaczono";
            return false;
        }
    }
    return true;
}
void MainWindow::on_tabWidget_2_tabBarClicked(int index){
    if(index==0){
        ui->reg_nick->clear();
        ui->reg_pass->clear();
    }
    else{
        ui->log_nick->clear();
        ui->log_pass->clear();
    }
}
void MainWindow::on_Log_out_clicked(){
    if(_login_status){
        _socket->close();
        _socket.reset();
        _login_status=false;
        QMessageBox msg;
        msg.setIcon(QMessageBox::Information);
        msg.setText("Użytkownik został wylogowany.");
        msg.setWindowTitle("Wylogowano");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
    }
}
void MainWindow::on_one_archive_clicked(){
    try {
        QString original=ui->key->text();
        string stdoriginal=original.toStdString();
        size_t slash=stdoriginal.find("\\");
        string main_key=stdoriginal.substr(0,slash);
        string sub_key=stdoriginal.substr(slash+1);
        std::shared_ptr<std::list<std::shared_ptr<RegField>>> registry(new std::list<std::shared_ptr<RegField>>());
        //qDebug()<<original<<" "<<main_key.c_str()<<" "<<sub_key.c_str();
        HKEY hmainkey;
        HKEY hsubkey;
        DWORD valueType, index;
        DWORD numSubKeys, maxSubKeyLen, numValues, maxValueNameLen, maxValueLen;
        DWORD subKeyNameLen, valueNameLen, valueLen;
        FILETIME lastWriteTime;
        LPTSTR subKeyName, valueName;
        LPBYTE value;
        TCHAR fullSubKeyName[MAX_PATH + 1];
        wchar_t* wsubkey=new wchar_t[sub_key.length()+1];
        if(stdoriginal[5]=='L')hmainkey=HKEY_LOCAL_MACHINE;
        else hmainkey=HKEY_USERS;
        std::wstring tempwstr(sub_key.begin(),sub_key.end());
        wcscpy(wsubkey,tempwstr.c_str());
        wsubkey[sub_key.length()]='\0';
        //qDebug()<<wsubkey<<" "<<tempwstr.c_str();
        if(RegOpenKeyEx(hmainkey,wsubkey,0,KEY_ALL_ACCESS,&hsubkey)!=ERROR_SUCCESS){
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
        subKeyName =(LPTSTR) malloc(TSIZE * (maxSubKeyLen + 1));   /* size in bytes */
        valueName =(LPTSTR) malloc(TSIZE * (maxValueNameLen + 1));
        value =(LPBYTE) malloc(maxValueLen);      /* size in bytes */
        /*First pass for key-value pairs.
            Important assumption: No one edits the registry under this subkey
            during this loop. Doing so could change add new values */
        swprintf_s(fullSubKeyName, _T("%s\\%s"), original.toStdWString().c_str(), subKeyName);
        for (index = 0; index < numValues; index++) {
            valueNameLen = maxValueNameLen + 1; /* A very common bug is to forget to set */
            valueLen = maxValueLen + 1;     /* these values; both are in/out params  */
            RegEnumValue(hsubkey, index, valueName, &valueNameLen, NULL, &valueType, value, &valueLen);
            WriteValue(valueName, valueType, value, valueLen,fullSubKeyName,registry);
            /*  If you wanted to change a value, this would be the place to do it.
                    RegSetValueEx(hSubKey, valueName, 0, valueType, pNewValue, NewValueSize); */
        }
        free(subKeyName);
        free(valueName);
        free(value);
        RegCloseKey(hsubkey);
        for(auto&x:*registry){
            qDebug()<<QString::fromStdString((string)*x);
        }
//        tempwstr.copy(wsubkey,tempwstr.length());
//        wsubkey[tempwstr.length()]='\0';
//        int x=5+2;
//        qDebug()<<x;

//        std::queue<string>subkeys;
//        while(sub_key.find("\\")!=string::npos){
//            slash=sub_key.find("\\");
//            string temp=sub_key.substr(0,slash);
//            subkeys.push(temp);
//            sub_key.erase(0,slash+1);
//        }
//        subkeys.push(sub_key);
//        while(!subkeys.empty()){
//            qDebug()<<subkeys.front().c_str();
//            subkeys.pop();
//        }


//        HKEY key;
//        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"SOFTWARE\\test",0,KEY_ALL_ACCESS,&key)==ERROR_SUCCESS){
//            qDebug()<<"Otworzono";
//        }



        //QRegistry reg(true);
        //auto key=reg.get_one_key()
        /////////////////////////////////
    //    HKEY hkSoftware;
    //    LONG result;
    //    result = RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"SOFTWARE", 0, KEY_ALL_ACCESS, & hkSoftware );
    //    if( result == ERROR_SUCCESS ){
    //        qDebug()<<"Otworzono klucz";
    //        HKEY hkTest;
    //        DWORD dwDisp;
    //        result = RegCreateKeyEx( hkSoftware, L"test", 0, NULL, REG_OPTION_NON_VOLATILE,
    //        KEY_ALL_ACCESS, NULL, & hkTest, & dwDisp );
    //        if( result == ERROR_SUCCESS )
    //        if( dwDisp == REG_CREATED_NEW_KEY )
    //            qDebug()<<"Stworzono";
    //        else if( dwDisp == REG_OPENED_EXISTING_KEY )
    //            qDebug()<<"Otworzono";
    //        wchar_t buf[ 20 ];
    //        lstrcpy( buf, L"Jakiś tam tekst" );
    //        result = RegSetValueEx( hkTest, L"MojaWartość", 0, REG_SZ,( LPBYTE ) buf, lstrlen( buf ) + 1 );
    //        if( result == ERROR_SUCCESS )
    //            qDebug()<<"Ustawiono";
    //        char buf2[ 21 ];
    //        DWORD dwBufSize = 20;
    //        DWORD dwRegsz = REG_SZ;
    //        result = RegQueryValueEx( hkTest, L"MojaWartość", NULL, & dwRegsz,( LPBYTE ) buf, & dwBufSize );
    //        if( result == ERROR_SUCCESS ) {
    //            buf2[ 20 ] = '\0';
    //            qDebug()<<"zapis"<<buf2;
    //        }
            ///////////////////////////////////////////
    //        result = RegDeleteValue( hkTest, L"MojaWartość" );
    //        if( result == ERROR_SUCCESS )
    //             qDebug()<<"Usunieto wartosc";
    //        result = RegDeleteKey( hkSoftware, L"test" ); // usuń klucz "test"
    //        if( result == ERROR_SUCCESS )
    //             qDebug()<<"Usunieto klucz\n";
    //    }
    }catch (const char * exc) {
        qDebug()<<"Error "<<exc;
    }
}
