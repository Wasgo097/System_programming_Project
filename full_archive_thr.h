#ifndef FULL_ARCHIVE_THR_H
#define FULL_ARCHIVE_THR_H
#include <QThread>
#include <QTcpSocket>
#include <fstream>
#include "qregistry.h"
class Full_Archive_THR : public QThread{
    Q_OBJECT
    void run() override;
    std::shared_ptr<QTcpSocket> socket;
    bool hkey_lm;
public:
    Full_Archive_THR(std::shared_ptr<QTcpSocket> socket,bool hkey_lm);
};

#endif // FULL_ARCHIVE_THR_H
