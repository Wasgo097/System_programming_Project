#include "mainwindow.h"
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
void MainWindow::on_btn_login_clicked(){
    if(connected){

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
    QString path;
    bool valid=false;
    if(ui->hkey_lm->isChecked()){
        qDebug()<<"Hkey_lm is checked";
        valid=true;
        path="HKEY_LOCAL_MACHINE";
    }
    if(ui->hkey_u->isChecked()){
        qDebug()<<"Hkey_lm is checked";
        valid=true;
        path="HKEY_USERS";
    }
    if(valid){
        qDebug()<<"Lecimy z "<<path;
        ui->output->clear();
        QSettings s("HKEY_USERS",QSettings::NativeFormat);
        for(auto&var:s.allKeys())
            ui->output->append(var);
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
