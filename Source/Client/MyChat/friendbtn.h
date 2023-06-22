#ifndef FRIENDBTN_H
#define FRIENDBTN_H

#include <QWidget>
#include <QMouseEvent>
#include "chatwindow.h"

namespace Ui {
class FriendBtn;
}

class FriendBtn : public QWidget
{
    Q_OBJECT

public:
    explicit FriendBtn(QString,QString,QWidget *parent = nullptr);
    ~FriendBtn();

    QIcon getIcon();
    QString getname() { return name; }

    void recvMsg(QJsonObject obj);
    void recvOfflineMsg(QJsonObject obj);
    QString outputMsg(QString myself) { return chatwin->getMsg(myself); }
    void setNameColor(bool);


protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    Ui::FriendBtn *ui;

    QString name;
    QString icon;
    int msgnum = 0;

    ChatWindow *chatwin = nullptr;
    QList<QString> msgList;

    bool isopen = false;

    void delChatWin() { chatwin->deleteLater(); chatwin = nullptr; }

    void addmsgnum() { ++msgnum; }
    void resetmsgnum() { msgnum = 0; }
    void setMsgTips();

signals:
    void doubleclick();
    void dropfriend();
    void sendMsg();
    void closeChatWin();
};

#endif // FRIENDBTN_H
