#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTcpSocket>
#include <QString>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <memory>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include "ui_mainwindow.h"
typedef  std::string string;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_btn_login_clicked();
    void on_btn_registration_clicked();
    void on_full_archive_clicked();
    //read during login process
    void read_log_in(){
        while(socket->canReadLine()){
            ui->output->append(socket->readLine().trimmed());
        }
    }
    //read during registration process
    void read_sign_in(){}
    //read during send register process
    void read_register_save(){}
    //read during load register process
    void read_register_load(){}
private:
    Ui::MainWindow *ui;
    std::mutex socket_mtx;
    std::unique_ptr<QTcpSocket> socket;
    //connected = true when config file is valid, and connection is successful
    bool connected=false;
    //someone is login
    bool login_status=false;
    string ip;
    string port;
private:
    string get_time_to_send();
};
#endif // MAINWINDOW_H
