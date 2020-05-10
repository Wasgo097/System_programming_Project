#include "mainwindow.h"
#include "qregistry.h"
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    std::fstream str;
    str.open("config");
    if(str.good()){
        try {
            str>>ip>>port;
            //qDebug()<<ip.c_str()<<" "<<port.c_str();
            str.close();
            socket=std::make_unique<QTcpSocket>(this);
            QString ip_temp=QString::fromStdString(ip);
            quint16 port_temp=std::stoi(port);
            socket->connectToHost(ip_temp,port_temp);
            if(socket->waitForConnected()){
                //ui->output->append("Polaczono");
                //connect(socket.get(),&QTcpSocket::readyRead,this,&MainWindow::read);
                connected=true;
            }
            else{
                //ui->output->append("Nie polaczono");
            }
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
    delete ui;
}
void MainWindow::add_to_output(QString txt){
    ui->output->append(txt);
}
void MainWindow::on_btn_login_clicked(){
    if(connected){
        //login_process
    }
    else{
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Warning);
        msg.setText("Nie udało się otworzyć pliku konfiguracyjnego i pobrać z niego danych lub połączyć do serwera");
        msg.setDetailedText("Skontaktuj się z twórcą oprogramowania");
        msg.setWindowTitle("Błąd pliku konfiguracyjnego");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
    }
}
void MainWindow::on_btn_registration_clicked(){
    if(connected){
        if(!login_status){
            //registration process
        }
        else{
            QMessageBox msg(this);
            msg.setIcon(QMessageBox::Information);
            msg.setText("Aktualnie nie można zarejstrować nowego użytkownika, wyloguj się z aktualnego konta.");
            msg.setWindowTitle("Błąd rejestracji użytkownika!");
            msg.setStandardButtons(QMessageBox::Ok);
            msg.exec();
        }
    }
    else{
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Warning);
        msg.setText("Nie udało się otworzyć pliku konfiguracyjnego i pobrać z niego danych lub połączyć do serwera");
        msg.setDetailedText("Skontaktuj się z twórcą oprogramowania");
        msg.setWindowTitle("Błąd pliku konfiguracyjnego");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
    }
}
void MainWindow::on_full_archive_clicked(){
    if(ui->hkey_u->isChecked()||ui->hkey_lm->isChecked()){
        bool hkey_lm=false;
        if(ui->hkey_lm->isChecked())
            hkey_lm=true;
        if(thr_full_archive.get()==nullptr){
            thr_full_archive=std::make_unique<Full_Archive_THR>(socket,hkey_lm);
            connect(thr_full_archive.get(),&Full_Archive_THR::started,this,&MainWindow::full_arch_start);
            connect(thr_full_archive.get(),&Full_Archive_THR::finished,this,&MainWindow::full_arch_end);
            thr_full_archive->start();
        }
        else{
            if(thr_full_archive->isRunning()){
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Information);
                msg.setText("Trwa archiwizacja rejestru.");
                msg.setWindowTitle("Pelna archiwizacja rejestru");
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
            }
            else{
                disconnect(thr_full_archive.get(),&Full_Archive_THR::started,this,&MainWindow::full_arch_start);
                disconnect(thr_full_archive.get(),&Full_Archive_THR::finished,this,&MainWindow::full_arch_end);
                thr_full_archive=std::make_unique<Full_Archive_THR>(socket,hkey_lm);
                connect(thr_full_archive.get(),&Full_Archive_THR::started,this,&MainWindow::full_arch_start);
                connect(thr_full_archive.get(),&Full_Archive_THR::finished,this,&MainWindow::full_arch_end);
                thr_full_archive->start();
            }
        }
    }
}
void MainWindow::read_log_in(){
    while(socket->canReadLine()){
        ui->output->append(socket->readLine().trimmed());
    }
}
void MainWindow::read_sign_in(){
    while(socket->canReadLine()){
        ui->output->append(socket->readLine().trimmed());
    }
}
void MainWindow::read_register_save(){
    while(socket->canReadLine()){
        ui->output->append(socket->readLine().trimmed());
    }
}
void MainWindow::read_register_load(){
    while(socket->canReadLine()){
        ui->output->append(socket->readLine().trimmed());
    }
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
