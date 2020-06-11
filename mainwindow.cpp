#include "mainwindow.h"
#include "qregistry.h"
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    for(int i=0;i<10;i++)
        ui->recordslist->addItem(QString::number(i)+" iteam");
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
                string fullmess="login|"+nick+"|"+pass+"\r\n";
                _socket_mtx->lock();
                _socket->write(fullmess.c_str());
                _socket_mtx->unlock();
                connect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_log_in);
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
                //qDebug()<<"Poprawme passy, zarejestrowano "<<nick.c_str()<<" "<<pass.c_str();
                if(Connect_socket()){
                    string fullmess="reg|"+nick+"|"+pass+"\r\n";
                    _socket_mtx->lock();
                    _socket->write(fullmess.c_str());
                    _socket_mtx->unlock();
                    connect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_sign_in);
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
                _thr_full_archive=std::make_unique<Full_Archive_THR>(_socket,_socket_mtx,hkey_lm,this);
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
                    _thr_full_archive=std::make_unique<Full_Archive_THR>(_socket,_socket_mtx,hkey_lm,this);
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
    _socket_mtx->lock();
    if(_socket->canReadLine()){
        try {
            QString read=_socket->readLine().trimmed();
            auto list=read.split('|');
            if(list[0]=="login"&&list[1]=="correct"){
                _login_status=true;
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Information);
                msg.setText("Udało się zalogować");
                msg.setWindowTitle("Logowanie");
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
            }
            else{
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Information);
                msg.setText("Nie udało się zalogować");
                msg.setWindowTitle("Logowanie");
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
            }
        } catch (const char * exc) {
            qDebug()<<exc;
        }
    }
    disconnect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_log_in);
    _socket_mtx->unlock();
}
void MainWindow::read_sign_in(){
    _socket_mtx->lock();
    if(_socket->canReadLine()){
        try {
            QString read=_socket->readLine().trimmed();
            auto list=read.split('|');
            if(list[0]=="reg"&&list[1]=="correct"){
                _login_status=true;
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Information);
                msg.setText("Udało się zarejestrować");
                msg.setWindowTitle("Rejestracja");
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
            }
            else{
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Information);
                msg.setText("Nie udało się zarejestrować");
                msg.setWindowTitle("Rejestracja");
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
            }
        } catch (const char * exc) {
            qDebug()<<exc;
        }
    }
    disconnect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_sign_in);
    _socket_mtx->unlock();
}
void MainWindow::read_register_save(){
    _socket_mtx->lock();
    if(_socket->canReadLine()){
        try {
            QString read=_socket->readLine().trimmed();
            auto list=read.split('|');
            if(list[0]=="registry"&&list[1]=="correct"){
                _login_status=true;
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Information);
                msg.setText("Udało się zarchiwizować dane");
                msg.setWindowTitle("Archiwizacja");
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
            }
            else{
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Information);
                msg.setText("Nie udało się zarchiwizować danych");
                msg.setWindowTitle("Rejestracja");
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
            }
        } catch (const char * exc) {
            qDebug()<<exc;
        }
    }
    disconnect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_register_save);
    _socket_mtx->unlock();
}
void MainWindow::read_register_load(){
    _socket_mtx->lock();
    try {
        std::shared_ptr<std::list<std::shared_ptr<RegField>>> temp(new std::list<std::shared_ptr<RegField>>());
        while(_socket->canReadLine()){
            QString read=_socket->readLine().trimmed();
            auto list=read.split('|');
            if(list[0]=="registry"&&list.size()==5){
                string _key=list[1].toStdString();
                string _value_name=list[2].toStdString();
                int _type=std::stoi(list[3].toStdString());
                string _value=list[4].toStdString();
                std::shared_ptr<RegField> row(new RegField(_key,_value_name,_value,_type));
                temp->push_back(row);
            }
        }
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Information);
        msg.setText("Udało się zaimportować dane");
        msg.setWindowTitle("Archiwizacja");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
    } catch (const char * exc) {
        qDebug()<<exc;
    }
    disconnect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_register_load);
    _socket_mtx->unlock();
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
    wchar_t* wsubkey=nullptr;
    wchar_t* wmainkey=nullptr;
    try {
        QString original=ui->key->text();
        string stdoriginal=original.toStdString();
        size_t slash=stdoriginal.find("\\");
        string main_key=stdoriginal.substr(0,slash);
        string sub_key=stdoriginal.substr(slash+1);
        QRegistry reg(true);
        qDebug()<<original<<" "<<main_key.c_str()<<" "<<sub_key.c_str();
        HKEY hmainkey;
        wsubkey=new wchar_t[sub_key.length()+1];
        wmainkey=new wchar_t[main_key.length()+1];
        if(stdoriginal[5]=='L'){
            hmainkey=HKEY_LOCAL_MACHINE;
        }
        else{
            hmainkey=HKEY_USERS;
        }
        //QString::fromStdString(main_key).toStdWString().c_str();
        std::wstring tempwsubkey(sub_key.begin(),sub_key.end());
        wcscpy(wsubkey,tempwsubkey.c_str());
        //wsubkey[sub_key.length()]='\0';
        std::wstring tempwmainkey(main_key.begin(),main_key.end());
        wcscpy(wmainkey,tempwmainkey.c_str());
        //wsubkey[tempwmainkey.length()]='\0';
        auto registry=reg.get_one_key(hmainkey,wmainkey,wsubkey);
        //auto registry=reg.get_one_key(HKEY_LOCAL_MACHINE,L"HKEY_LOCAL_MACHINE",L"SOFTWARE\\test");
        for(auto&x:*registry){
            qDebug()<<QString::fromStdString((string)*x);
        }
//        HKEY key;
//        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"SOFTWARE\\test",0,KEY_ALL_ACCESS,&key)==ERROR_SUCCESS){
//            qDebug()<<"Otworzono";
//        }
    }catch (const char * exc) {
        qDebug()<<"Error "<<exc;
    }
    if(wsubkey!=nullptr) delete []  wsubkey;
    if(wmainkey!=nullptr) delete []  wmainkey;
}
void MainWindow::on_importrecord_clicked(){
    int index=ui->recordslist->currentIndex().row();
    qDebug()<<index;
    std::shared_ptr<std::queue<std::shared_ptr<RegField>>> records(new std::queue<std::shared_ptr<RegField>>);
    records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\test","proba1","XDDD",2));
    records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\test","proba2","1113213",1));
    records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\test","proba2","123",3));
    records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\proba","proba1","XDDD",2));
    records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\proba","proba2","111324",1));
    records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\proba","proba2","123",3));
    QRegistry reg(false);
    if(reg.Import(records)){
        qDebug()<<"UDało się";
    }
    else{
        qDebug()<<"XD";
    }
}
