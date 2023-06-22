#include "mythread.h"
#include <QDebug>
#include <QThread>
#include "flag.h"

QTcpSocket * MyThread::socket = new QTcpSocket;

MyThread::MyThread(QObject *parent) : QObject(parent), QRunnable()
{
    qDebug()<<"Thread构造函数ID:"<<QThread::currentThreadId();
    setAutoDelete(true);
}

MyThread::~MyThread()
{
    delete socket;
}

void MyThread::run()
{
    this->socket->connectToHost(QHostAddress(HOST),PORT);

    // 成功连接
    connect(socket,&QTcpSocket::connected,this,[=](){
        emit this->Isconn();
    });

    // 捕获socket错误
    connect(socket,QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),this,[=](QAbstractSocket::SocketError err){
        if (err == 0)
            emit this->UnableConn(err);
        else
            emit this->LossConn(err);
        socket->close();
    });
}


ReadThread::ReadThread(QObject *parent) : QObject(parent), QRunnable()
{
    setAutoDelete(true);
}

void ReadThread::run()
{
    connect(MyThread::socket,&QTcpSocket::readyRead,this,&ReadThread::read_fun);
}

void ReadThread::read_fun()
{
    if(!MyThread::socket->isValid()){
        qDebug() << "unreadable";
        return;
    }
    // 从socket读取的数据
    qint64 total = MyThread::socket->bytesAvailable();
    qDebug() << "total: " << total;
    char buf[total];
    while(0 < MyThread::socket->readLine(buf,sizeof(buf) + 1)){

        QString line = QString::fromUtf8(buf);

        qDebug() << "Client recv: " << line;

        // 解析Json数据
        QJsonObject obj = this->parserStr(line);

        emit finished(obj);
    }
}

QJsonObject ReadThread::parserStr(QString str){
    //qDebug() << "In parser";
    QJsonParseError e;      // 记录解析错误
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(),&e);   // 字符串转为QJsonDocument，再转json对象
    QJsonObject obj;

    if (e.error != QJsonParseError::NoError && !doc.isNull()) {
        qDebug() << "parser error: " << e.error;
    } else {
        obj = doc.object();
    }

    return obj;
}


WriteThread::WriteThread(QObject *parent) : QObject(parent), QRunnable()
{
    setAutoDelete(true);
}

void WriteThread::run()
{

}

void WriteThread::write_fun(QJsonObject obj) {

    if(obj.isEmpty()){
        qDebug() << "JsonObject is empty";
        return;
    }

    QJsonDocument doc(obj);

    MyThread::socket->write(doc.toJson());

}


