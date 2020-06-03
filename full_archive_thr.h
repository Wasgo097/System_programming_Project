#ifndef FULL_ARCHIVE_THR_H
#define FULL_ARCHIVE_THR_H
#include <QThread>
#include <QTcpSocket>
#include <fstream>
#include "qregistry.h"
class MainWindow;
class Full_Archive_THR : public QThread{
    Q_OBJECT
    void run() override;
    std::shared_ptr<QTcpSocket> socket;
    std::shared_ptr<std::mutex> socket_mtx;
    bool hkey_lm;
    MainWindow * window;
public:
    Full_Archive_THR(std::shared_ptr<QTcpSocket> socket,std::shared_ptr<std::mutex> socket_mtx,bool hkey_lm,MainWindow * window);
};

#endif // FULL_ARCHIVE_THR_H
