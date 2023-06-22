#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QDataStream>
#include <QMessageBox>
#include <QDateTime>
#include <QColorDialog>
#include <QFileDialog>

ChatWindow::ChatWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
/*
    this->port = 9999;
    this->udpSocket = new QUdpSocket(this);

*/

    //聊天窗口功能区
    //加粗
    connect(ui->fontCbx,&QComboBox::currentTextChanged,[=](const QFont &font){
        ui->msgTxtEdit->setCurrentFont(font);
        ui->msgTxtEdit->setFocus();
    });
    //字体大小
    connect(ui->sizeCbx,&QComboBox::currentTextChanged,[=](const QString &text){
        ui->msgTxtEdit->setFontPointSize(text.toDouble());
        ui->msgTxtEdit->setFocus();
    });
    //加粗
    connect(ui->boldBtn,&QPushButton::clicked,[=](bool checked){
        if(checked)
            ui->msgTxtEdit->setFontWeight(QFont::Bold);
        else
            ui->msgTxtEdit->setFontWeight(QFont::Normal);
    });
    //倾斜
    connect(ui->italicBtn,&QPushButton::clicked,[=](bool checked){
        ui->msgTxtEdit->setFontItalic(checked);
        ui->msgTxtEdit->setFocus();
    });
    //下划线
    connect(ui->underlineBtn,&QPushButton::clicked,[=](bool checked){
        ui->msgTxtEdit->setFontUnderline(checked);
        ui->msgTxtEdit->setFocus();
    });
    //清除文本
    connect(ui->clearBtn,&QPushButton::clicked,[=](){
        ui->msgTxtEdit->clear();
    });
    //文本颜色
    connect(ui->colorBtn,&QPushButton::clicked,[=](){
        //颜色对话框
        QColor color = QColorDialog::getColor(color,this);
        ui->msgTxtEdit->setTextColor(color);
    });
    //保存聊天记录
    connect(ui->saveBtn,&QPushButton::clicked,[=](){
        if(ui->recvTxtEdit->document()->isEmpty()){
            QMessageBox::warning(this,"警告","暂无聊天内容");
        } else {
            QString filename = QFileDialog::getSaveFileName(this,"保存","聊天记录","*.txt");
            if(!filename.isEmpty()){
                QFile file(filename);
                file.open(QIODevice::WriteOnly | QFile::Text);
                QTextStream stream(&file);
                stream << ui->recvTxtEdit->toPlainText();
                file.close();
            }
        }
    });

/*
    // upd群聊
    //数据传输功能
    udpSocket->bind(this->port,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    //用udp接收信号，接收消息
    connect(udpSocket,&QUdpSocket::readyRead,this,&ChatWindow::RecvMsg);
    //发送消息按钮
    connect(ui->sendBtn,&QPushButton::clicked,[=](){
        sendMsg(Msg);
    });
    //新用户进入发言
    sendMsg(UserEnter);
*/


    // 发送消息按钮
    connect(ui->sendBtn,&QPushButton::clicked,[=](){
        emit this->ChatSendMsg();
    });


}


ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::closeEvent(QCloseEvent * ) {
    /*sendMsg(UserLeave);
    udpSocket->close();
    udpSocket->destroyed();*/

    emit this->closeWidget();
}

QString ChatWindow::getMsg(QString myself){

    QString msg = ui->msgTxtEdit->toPlainText();
    ui->msgTxtEdit->clear();
    ui->msgTxtEdit->setFocus();

    // 在自己的窗口上显示自己的内容
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->recvTxtEdit->setTextColor(QColor(Qt::darkGreen));
    ui->recvTxtEdit->append("["+myself+"] "+time);
    ui->recvTxtEdit->append(msg);

    return msg;
}

void ChatWindow::recvMsg(QString msg, bool flag){

    if (flag) {
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        ui->recvTxtEdit->append("["+this->windowTitle()+"] "+time);
        ui->recvTxtEdit->append(msg);
    } else {
        ui->recvTxtEdit->append(msg);
    }
}

void ChatWindow::setTxtColor(QColor color) {
    ui->recvTxtEdit->setTextColor(color);
}

/*
void ChatWindow::sendMsg(ChatWindow::Msgtype sendtype){
    QByteArray array;
    QDataStream stream(&array,QIODevice::WriteOnly);

    stream << sendtype << getName();

    switch(sendtype){
        case Msg:
            if(ui->msgTxtEdit->toPlainText()==""){
                QMessageBox::warning(this,"警告","发送内容不能为空");
                return;
            }
            stream << this->getMsg();
            break;
        case UserEnter:
            break;
        case UserLeave:
            break;
    }
    //书写报文，广播
    udpSocket->writeDatagram(array.data(),array.size(),QHostAddress::Broadcast,this->port);
}

void ChatWindow::RecvMsg(){
    //获取udp包大小
    qint64 udpSize = udpSocket->pendingDatagramSize();
    int size = static_cast<int>(udpSize);

    //堆区创建mysize大小的数组，并初始化为0
    QByteArray *array = new QByteArray(size,0);

    //读报文
    udpSocket->readDatagram(array->data(),udpSize);
    QDataStream stream(array,QIODevice::ReadOnly);

    int recvtype;
    QString name,msg;
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    stream >> recvtype >> name;

    switch(recvtype){
    case Msg:
        stream >> msg;
        ui->recvTxtEdit->setTextColor(QColor(Qt::blue));
        ui->recvTxtEdit->append("["+name+"] "+time);
        ui->recvTxtEdit->append(msg);
        break;
    case UserEnter:
        userEnter(name);
        break;
    case UserLeave:
        userLeave(name,time);
        break;
    }
}

void ChatWindow::userEnter(QString username){
    bool IsEmpty = ui->usrTbWidget->findItems(username,Qt::MatchExactly).isEmpty();

    if(IsEmpty){
        QTableWidgetItem *table = new QTableWidgetItem(username);
        //设置上线人员列表
        ui->usrTbWidget->insertRow(0);
        ui->usrTbWidget->setItem(0,0,table);
        //设置上线提示
        ui->recvTxtEdit->setTextColor(QColor(Qt::gray));
        ui->recvTxtEdit->append(username+"已上线");
        //更新在线人数
        ui->userNumlabel->setText(QString("在线人数：%1").arg(ui->usrTbWidget->rowCount()));

        sendMsg(UserEnter);
    }
}

void ChatWindow::userLeave(QString username,QString time){
    bool IsEmpty = ui->usrTbWidget->findItems(username,Qt::MatchExactly).isEmpty();

    if(!IsEmpty){
        //找到行
        int row = ui->usrTbWidget->findItems(username,Qt::MatchExactly).first()->row();
        //删除
        ui->usrTbWidget->removeRow(row);

        //设置上线提示
        ui->recvTxtEdit->setTextColor(QColor(Qt::gray));
        ui->recvTxtEdit->append("用户"+username+"于"+time+"下线");
        //更新在线人数
        ui->userNumlabel->setText(QString("在线人数：%1").arg(ui->usrTbWidget->rowCount()));
    }
}
*/
