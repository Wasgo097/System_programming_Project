#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTcpSocket>
#include <QString>
#include <QSettings>
#include <memory>
#include <fstream>
#include <string>
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
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void read();
private:
    Ui::MainWindow *ui;
    std::unique_ptr<QTcpSocket> socket;
    bool config_file=false;
    bool login_status=false;
    string ip;
    string port;
};
#endif // MAINWINDOW_H
