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
    _thr_full_archive->wait();
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
    if(_socket.get()==nullptr){
        _socket=std::make_shared<QTcpSocket>(this);
        QString ip_temp=QString::fromStdString(_ip);
        quint16 port_temp=std::stoi(_port);
        _socket->connectToHost(ip_temp,port_temp);
        if(_socket->waitForConnected()){
            qDebug()<<"Polaczono";
            return true;
        }
        else{
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
