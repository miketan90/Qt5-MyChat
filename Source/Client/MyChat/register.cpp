#include "register.h"
#include "ui_register.h"
#include <QMessageBox>
#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonDocument>

Register::Register(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(QPixmap(":/img/ChatIcon.jpg")));

    this->addr = "192.168.130.130";
    this->port = 8000;

    QTcpSocket * socket = new QTcpSocket;

    // 返回按钮
    connect(ui->backBtn,&QPushButton::clicked,[=](){
       emit this->backlogin();
    });

    // 注册按钮
    connect(ui->rgBtn, &QPushButton::clicked,[=](){

        ui->rgBtn->setDisabled(true);

        if (ui->userEd->text().isEmpty()) {
            QMessageBox::critical(this,"error","用户名不能为空");
            return;
        }

        if (ui->pwEd->text().isEmpty()) {
            QMessageBox::critical(this,"error","密码不能为空");
            return;
        }

        if (ui->conpwEd->text().isEmpty()) {
            QMessageBox::critical(this,"error","确认密码不能为空");
            return;
        }

        if (ui->pwEd->text() != ui->conpwEd->text()) {
            QMessageBox::critical(this,"error","两次输入的密码不同");
            return;
        }


        socket->connectToHost(QHostAddress(this->addr),this->port);

    });

    connect(socket,&QTcpSocket::connected,[=](){

        QJsonObject obj;

        obj.insert("cmd",REGISTER);
        obj.insert("user",this->ui->userEd->text());
        obj.insert("password",this->ui->pwEd->text());

        QJsonDocument doc(obj);

        socket->write(doc.toJson(QJsonDocument::Compact));

    });

    // 读事件
    connect(socket,&QTcpSocket::readyRead,[=](){
        if(!socket->isValid()){
            qDebug() << "unreadable";
            return;  // 判断不可读返回
        }
        QString recv = QString::fromUtf8(socket->readAll());

        QJsonParseError e;      // 记录解析错误
        QJsonDocument doc = QJsonDocument::fromJson(recv.toUtf8(),&e);   // 字符串转为QJsonDocument，再转json对象

        if (e.error != QJsonParseError::NoError && !doc.isNull()) {
            qDebug() << "parser error: " << e.error;
        }

        QJsonObject obj = doc.object();
        int res = obj.value("result").toInt();

        switch(res){
        case REGISTER_SUCCESS:
            //qDebug() << "register success";
            QMessageBox::information(this,"提示","注册成功");

            socket->close();
            socket->deleteLater();
            emit this->backlogin(); // 发送返回信号
            break;

        case REGISTER_USER_EXIST:
            //qDebug() << "user exist";
            QMessageBox::critical(this,"error",QString("用户已存在"));
            ui->rgBtn->setEnabled(true);
            break;

        case MYSQL_ERROR:
            //qDebug() << "mysql error";
            QMessageBox::critical(this,"error",QString("mysql错误"));
            ui->rgBtn->setEnabled(true);
            break;
        default:
            break;
        }

    });

    // 拦截错误
    connect(socket,QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),[=](QAbstractSocket::SocketError err){
        QMessageBox::critical(this,"error",QString("ServerError: %1, 网络错误，注册失败").arg(err));
        ui->rgBtn->setEnabled(true);
        socket->close();
    });

}

Register::~Register()
{
    delete ui;
}
