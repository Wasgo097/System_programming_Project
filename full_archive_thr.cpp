#include "full_archive_thr.h"
void Full_Archive_THR::run() {
    QRegistry reg(false);
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
    for(const auto &x:*temp){
        x->reduce_key();
        str<<*x<<std::endl;
    }
    str.close();
    qDebug()<<"zrobione";
    this->quit();
}
Full_Archive_THR::Full_Archive_THR(std::shared_ptr<QTcpSocket> socket, bool hkey_lm){
    this->socket=socket;
    this->hkey_lm=hkey_lm;
}
