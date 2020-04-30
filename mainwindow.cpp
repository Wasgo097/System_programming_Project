#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    std::fstream str;
    str.open("config");
    if(str.good()){
        try {
            //qDebug()<<"Działa";
            str>>ip>>port;
            config_file=true;
            //qDebug()<<ip.c_str()<<" "<<port.c_str();
            str.close();
            socket=std::make_unique<QTcpSocket>(this);
            QString ip_temp=QString::fromStdString(ip);
            quint16 port_temp=std::stoi(port);
            socket->connectToHost(ip_temp,port_temp);
            if(socket->waitForConnected()){
                //ui->output->append("Polaczono");
                connect(socket.get(),&QTcpSocket::readyRead,this,&MainWindow::read);
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
void MainWindow::on_btn_login_clicked(){
    if(config_file){

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
    if(config_file){
        if(!login_status){

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
        msg.setText("Nie udało się otworzyć pliku konfiguracyjnego i pobrać z niego danych");
        msg.setDetailedText("Skontaktuj się z twórcą oprogramowania");
        msg.setWindowTitle("Błąd pliku konfiguracyjnego");
        msg.setStandardButtons(QMessageBox::Ok);
        msg.exec();
    }
}
void MainWindow::read(){
    while(socket->canReadLine()){
        ui->output->append(socket->readLine().trimmed());
    }
}
//lm
void MainWindow::on_pushButton_clicked(){
    ui->output->clear();
    QSettings s("HKEY_LOCAL_MACHINE",QSettings::NativeFormat);
    for(auto&var:s.allKeys())
        ui->output->append(var);
}
//u
void MainWindow::on_pushButton_2_clicked(){
    ui->output->clear();
    QSettings s("HKEY_USERS",QSettings::NativeFormat);
    for(auto&var:s.allKeys())
        ui->output->append(var);
}
