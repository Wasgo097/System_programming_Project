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
#include <windows.h>
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
    void full_arch_start();
    void full_arch_end();
    void on_tabWidget_2_tabBarClicked(int index);
    void on_Log_out_clicked();
    void on_one_archive_clicked();
public slots:
    //read during send register process
    void read_register_save();
    //read during load register process
    void read_register_load();
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
    BOOL WriteValue(LPTSTR valueName, DWORD valueType,	LPBYTE value, DWORD valueLen,LPTSTR fullKeyName,std::shared_ptr<std::list<std::shared_ptr<RegField>>>& key){
        LPBYTE pV = value;
        QString qkey=QString::fromWCharArray(fullKeyName);
        string skey=qkey.toStdString();
        unsigned long i;
        int namesize=wcslen(valueName);
        if(namesize==0){
            return false;
        }
        string name="";
        for(int i=0;i<namesize;i++)
            name+=(char)valueName[i];
        std::shared_ptr<RegField> row(new RegField());
        row->key(skey);
        row->value_name(name);
        string svalue;
        std::stringstream sstream;
        switch (valueType) {
        case REG_FULL_RESOURCE_DESCRIPTOR: /* 9: Resource list in the hardware description */
        case REG_BINARY: /*  3: Binary data in any form. */
            for (i = 0; i < valueLen; i++, pV++){
                sstream<<std::hex<<(unsigned int)*pV<<" ";
            }
            svalue=sstream.str();
            row->type(1);
            row->value(svalue);
            key->push_back(row);
            break;
        case REG_DWORD: /* 4: A 32-bit number. */
            svalue=std::to_string((DWORD)*value);
            row->type(3);
            row->value(svalue);
            key->push_back(row);
            break;
        case REG_EXPAND_SZ: /* 2: null-terminated string with unexpanded references to environment variables (for example, “%PATH%”). */
        case REG_MULTI_SZ: /* 7: An array of null-terminated strings, terminated by two null characters. */
        case REG_SZ: /* 1: A null-terminated string. */
            qkey=QString::fromWCharArray((LPTSTR)value);
            svalue=qkey.toStdString();
            row->type(2);
            row->value(svalue);
            key->push_back(row);
            break;
        case REG_DWORD_BIG_ENDIAN: /* 5:  A 32-bit number in big-endian format. */
        case REG_LINK: /* 6: A Unicode symbolic link. */
        case REG_NONE: /* 0: No defined value type. */
        case REG_RESOURCE_LIST: /* 8: A device-driver resource list. */
        default: //_tprintf(_T(" ** Cannot display value of type: %d. Exercise for reader\n"), valueType);
            return false;
        }
        return TRUE;
    }
};
#endif // MAINWINDOW_H
