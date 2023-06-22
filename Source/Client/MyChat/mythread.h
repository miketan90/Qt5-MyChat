#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include <QRunnable>    // 线程池
#include "mainwindow.h"

#define HOST "192.168.130.130"
#define PORT 8000

class ReadThread;
class WriteThread;

class MyThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit MyThread(QObject *parent = nullptr);
    ~MyThread();

    void run() override;

    friend class ReadThread;
    friend class WriteThread;

private:
    static QTcpSocket * socket;


signals:
    void Isconn();
    void UnableConn(QAbstractSocket::SocketError);
    void LossConn(QAbstractSocket::SocketError);

};

class ReadThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ReadThread(QObject *parent = nullptr);

    void run() override;

    void read_fun();
    QJsonObject parserStr(QString str);

signals:
    void finished(QJsonObject);

};

class WriteThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit WriteThread(QObject *parent = nullptr);

    void run() override;

    void write_fun(QJsonObject);

signals:
    void finished();

};

#endif
