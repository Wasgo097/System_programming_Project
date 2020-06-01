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
    socket_mtx->lock();
    for(const auto &x:*temp){
        x->reduce_key();
        if(x->is_valid())
            str<<*x<<std::endl;
        //string temp=(string)*x+"\r\n";
        //socket->write(temp.c_str());
    }
    socket_mtx->unlock();
    str.close();
    qDebug()<<"Done";
    this->quit();
}
Full_Archive_THR::Full_Archive_THR(std::shared_ptr<QTcpSocket> socket,std::shared_ptr<std::mutex> socket_mtx, bool hkey_lm){
    this->socket=socket;
    this->hkey_lm=hkey_lm;
    this->socket_mtx=socket_mtx;
}
