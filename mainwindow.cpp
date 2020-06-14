#include "mainwindow.h"
#include "qregistry.h"
using namespace std::chrono_literals;
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
    if(_thr_full_archive.get()!=nullptr)
        if(_thr_full_archive->isRunning())
            _thr_full_archive->wait();
    if(_socket.get()!=nullptr){
        _socket->write("exit");
        _socket->waitForBytesWritten();
    }
    delete ui;
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
        string nick=ui->log_nick->text().toStdString();
        string pass=ui->log_pass->text().toStdString();
        std::regex nick_regex("[\\w]{5,}");
        std::regex pass_regex("[\\w !@#$%&*_]{6,}");
        if(std::regex_search(nick,nick_regex)&&std::regex_search(pass,pass_regex)){
            if(Connect_socket()){
                string fullmess="login|"+nick+"|"+pass;
                _socket_mtx->lock();
                _socket->write(fullmess.c_str());
                _socket->waitForBytesWritten();
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
            string nick=ui->reg_nick->text().toStdString();
            string pass=ui->reg_pass->text().toStdString();
            std::regex nick_regex("[\\w]{5,}");
            std::regex pass_regex("[\\w !@#$%&*_]{6,}");
            if(std::regex_search(nick,nick_regex)&&std::regex_search(pass,pass_regex)){
                if(Connect_socket()){
                    string fullmess="registration|"+nick+"|"+pass;
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
    try {
        QString read=_socket->readLine().trimmed();
        auto list=read.split('|');
        if(list[0]=="login"&&list[1]=="correct"){
            ui->recordslist->clear();
            qDebug()<<"clear";
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
    }
    catch (const char * exc) {
        qDebug()<<exc;
    }
    disconnect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_log_in);
    _socket_mtx->unlock();
}
void MainWindow::read_sign_in(){
    _socket_mtx->lock();
    try {
        QString read=_socket->readLine().trimmed();
        auto list=read.split('|');
        if(list[0]=="registration"&&list[1]=="correct"){
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
    disconnect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_sign_in);
    _socket_mtx->unlock();
}
void MainWindow::read_register_save(){
    _socket_mtx->lock();
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
    disconnect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_register_save);
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
    temp=std::to_string(curr.date().year());
    temp+='-'+std::to_string(curr.date().month());
    temp+='-'+std::to_string(curr.date().day());
    temp+=' '+std::to_string(curr.time().hour());
    temp+=':'+std::to_string(curr.time().minute());
    temp+=':'+std::to_string(curr.time().second());
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
//            QMessageBox msg(this);
//            msg.setIcon(QMessageBox::Information);
//            msg.setText("Udało się połączyć z serwerem");
//            msg.setWindowTitle("Błąd połączenia");
//            msg.setStandardButtons(QMessageBox::Ok);
//            msg.exec();
            _connected=true;
            return true;
        }
        else{
            qDebug()<<"Nie polaczono";
//            QMessageBox msg(this);
//            msg.setIcon(QMessageBox::Information);
//            msg.setText("Nie udało się połączyć z serwerem");
//            msg.setWindowTitle("Błąd połączenia");
//            msg.setStandardButtons(QMessageBox::Ok);
//            msg.exec();
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
        ui->recordslist->clear();
        qDebug()<<"clear";
        _socket->write("exit");
        _socket->waitForBytesWritten();
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
    if(_login_status){
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
            if(stdoriginal[5]=='L')
                hmainkey=HKEY_LOCAL_MACHINE;
            else
                hmainkey=HKEY_USERS;
            std::wstring tempwsubkey(sub_key.begin(),sub_key.end());
            wcscpy(wsubkey,tempwsubkey.c_str());
            std::wstring tempwmainkey(main_key.begin(),main_key.end());
            wcscpy(wmainkey,tempwmainkey.c_str());
            auto registry=reg.get_one_key(hmainkey,wmainkey,wsubkey);
            _socket_mtx->lock();
            string datatime=get_time_to_send();
            for(auto&x:*registry){
                x->key(stdoriginal);
                if(x->is_valid()){
                    //qDebug()<<QString::fromStdString((string)*x);
                    string tempp="registry|"+(string)*x+'|'+datatime+'|';
                    qDebug()<<tempp.c_str();
                    _socket->write(tempp.c_str());
                    _socket->waitForBytesWritten();
                    std::this_thread::sleep_for(100ms);
                }
            }
            string tempp="done";
            _socket->write(tempp.c_str());
            _socket->waitForBytesWritten();
            _socket_mtx->unlock();
            //connect(_socket.get(), &QTcpSocket::readyRead, this, &MainWindow::read_register_save);
        }catch (const char * exc) {
            qDebug()<<"Error "<<exc;
        }
        if(wsubkey!=nullptr) delete []  wsubkey;
        if(wmainkey!=nullptr) delete []  wmainkey;
    }
}
void MainWindow::on_importrecord_clicked(){
    if(_login_status){
        std::shared_ptr<std::queue<std::shared_ptr<RegField>>> import(new std::queue<std::shared_ptr<RegField>>());
        QString temp=ui->recordslist->currentItem()->text();
        //qDebug()<<temp;
        string mess="get|"+temp.toStdString();
        _socket_mtx->lock();
        _socket->write(mess.c_str());
        _socket->waitForBytesWritten();
        _socket->waitForReadyRead();
        while(_socket->canReadLine()){
            QString read=_socket->readLine().trimmed();
            auto list=read.split('|');
            if(list[0]=="registry"&&list.size()==5){
                string _key=list[1].toStdString();
                string _value_name=list[2].toStdString();
                int _type=std::stoi(list[3].toStdString());
                string _value=list[4].toStdString();
                std::shared_ptr<RegField> row(new RegField(_key,_value_name,_value,_type));
                import->push(row);
            }
        }
        _socket_mtx->unlock();
//        //binary 1 string 2 dword 3
//        std::shared_ptr<std::queue<std::shared_ptr<RegField>>> records(new std::queue<std::shared_ptr<RegField>>);
//        records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\test","proba1","hal",2));
//        records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\test","proba2","2 c0 0 0",1));
//        records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\test","proba3","321123",3));
//        records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\proba","proba1","halo",2));
//        records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\proba","proba2","111324",1));
//        records->push(std::make_shared<RegField>("HKEY_USERS\\S-1-5-18\\Software\\proba","proba3","132231",3));
        try {
            QRegistry reg(false);
            //if(reg.Import(records)){
            if(reg.Import(import)){
                qDebug()<<"Udalo się";
    //            QMessageBox msg(this);
    //            msg.setIcon(QMessageBox::Information);
    //            msg.setText("Udało się zaimportować dane");
    //            msg.setWindowTitle("Archiwizacja");
    //            msg.setStandardButtons(QMessageBox::Ok);
    //            msg.exec();
            }
            else{
                qDebug()<<"Nie udalo sie";
    //            QMessageBox msg(this);
    //            msg.setIcon(QMessageBox::Information);
    //            msg.setText("Nie zaimportowano przynajmniej jednego rekordu");
    //            msg.setWindowTitle("Archiwizacja");
    //            msg.setStandardButtons(QMessageBox::Ok);
    //            msg.exec();
            }
        } catch (const char * exc) {
            qDebug()<<exc;
        }
    }
}
void MainWindow::on_tabWidget_currentChanged(int index){
    ui->recordslist->clear();
    qDebug()<<"clear";
    std::this_thread::sleep_for(100ms);
    if(index==2&&_login_status==true){
        string mess="dateget";
        //_socket_mtx->lock();
        _socket->write(mess.c_str());
        _socket->waitForBytesWritten();
        while(_socket->waitForReadyRead(3000)){
            //qDebug()<<"Jestem";
            QString line=_socket->readLine();
            ui->recordslist->addItem(line);
            qDebug()<<line;
        }
    }
}
