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
    if(thr_full_archive!=nullptr){
        if(thr_full_archive->joinable())
            thr_full_archive->join();
        delete thr_full_archive;
    }
}
void MainWindow::add_to_output(QString txt){
    ui->output->append(txt);
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
    if(ui->hkey_u->isChecked()||ui->hkey_lm->isChecked()){
        bool hkey_lm=false;
        if(ui->hkey_lm->isChecked())
            hkey_lm=true;
        if(thr_full_archive==nullptr){
            thr_full_archive=new std::thread(&MainWindow::full_archive,this,hkey_lm);
            QMessageBox msg(this);
            msg.setIcon(QMessageBox::Information);
            msg.setText("Rozpoczeto pelna archwizacje rejestru.");
            msg.setWindowTitle("Pelna archiwizacja rejestru");
            msg.setStandardButtons(QMessageBox::Ok);
            msg.exec();
        }
        else{
            if(thr_full_archive->joinable()){
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Information);
                msg.setText("Trwa archiwizacja rejestru.");
                msg.setWindowTitle("Pelna archiwizacja rejestru");
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
                thr_full_archive->join();
            }
            else{
                QMessageBox msg(this);
                msg.setIcon(QMessageBox::Information);
                msg.setText("Rozpoczeto pelna archwizacje rejestru.");
                msg.setWindowTitle("Pelna archiwizacja rejestru");
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
                delete thr_full_archive;
                thr_full_archive=new std::thread(&MainWindow::full_archive,this,hkey_lm);
            }
        }
    }
}
void MainWindow::full_archive(bool hkey_lm){
    //ui->output->clear();
    QRegistry reg(this,false);
    BOOL flags[2];
    flags[0]=1;
    flags[1]=0;
    std::fstream str;
    str.open("logs2.txt",std::fstream::in | std::fstream::out | std::fstream::app);
    std::shared_ptr<std::list<std::shared_ptr<RegField>>> temp;
    if(hkey_lm){
        qDebug()<<"Hkey_lm is run";
        temp=reg.get_full_registry(HKEY_LOCAL_MACHINE,L"HKEY_LOCAL_MACHINE",NULL,flags);
    }
    else{
        qDebug()<<"Hkey_users is run";
        temp=reg.get_full_registry(HKEY_USERS,L"HKEY_USERS",NULL,flags);
    }
    //qDebug()<<"Print";
    for(const auto &x:*temp){
        //str<<*x<<std::endl;
        x->reduce_key();
        str<<*x<<std::endl;
    }
    str.close();
    qDebug()<<"Zrobione";
//    QMessageBox msg;
//    msg.setIcon(QMessageBox::Information);
//    msg.setText("Ukonczono pelna archwizacje rejestru.");
//    msg.setWindowTitle("Pelna archiwizacja rejestru");
//    msg.setStandardButtons(QMessageBox::Ok);
//    msg.exec();
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
