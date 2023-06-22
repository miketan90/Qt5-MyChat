#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QUdpSocket>
#include <QJsonObject>

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    enum Msgtype{Msg,UserEnter,UserLeave};//状态机

    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

    void closeEvent(QCloseEvent * e);

    // UDP发消息
    //void sendMsg(Msgtype type);
    // UDP收消息
    //void RecvMsg();
    //用户进入
    //void userEnter(QString username);
    //用户退出
    //void userLeave(QString username,QString time);

    QString getMsg(QString myself);                     // 获取自己输入的内容
    void recvMsg(QString msg, bool flag = true);        // 接收其他人的聊天消息
    void setTxtColor(QColor color);

private:
    Ui::ChatWindow *ui;

    //QUdpSocket *udpSocket;
    //quint16 port;

signals:
    void closeWidget();

    void ChatSendMsg();

};

#endif // CHATWINDOW_H
