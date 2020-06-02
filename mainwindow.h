#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTcpSocket>
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <mutex>
#include <fstream>
#include <memory>
#include <string>
#include <regex>
#include "ui_mainwindow.h"
#include "full_archive_thr.h"
typedef  std::string string;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void add_to_output(QString txt);
private slots:
    void on_btn_login_clicked();
    void on_btn_registration_clicked();
    void on_full_archive_clicked();
    //read during login process
    void read_log_in();
    //read during registration process
    void read_sign_in();
    //read during send register process
    void read_register_save();
    //read during load register process
    void read_register_load();
    void full_arch_start();
    void full_arch_end();
    void on_tabWidget_2_tabBarClicked(int index);
    void on_Log_out_clicked();
    void on_one_archive_clicked();
private:
    Ui::MainWindow *ui;
    std::shared_ptr<std::mutex> _socket_mtx;
    std::shared_ptr<QTcpSocket> _socket;
    std::unique_ptr<Full_Archive_THR> _thr_full_archive;
    //connector true when data from config file is valid
    bool _connector=false;
    //socket is connecting
    bool _connected=false;
    //someone is login
    bool _login_status=false;
    string _ip;
    string _port;
private:
    string get_time_to_send();
    bool Connect_socket();
};
#endif // MAINWINDOW_H
