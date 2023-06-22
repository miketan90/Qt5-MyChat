#include "friendbtn.h"
#include "ui_friendbtn.h"
#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QMenu>
#include "mainwindow.h"

FriendBtn::FriendBtn(QString n, QString ic, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendBtn)
{
    ui->setupUi(this);

    this->name = n;
    this->icon = ic;

    setWindowFlag(Qt::FramelessWindowHint);
    ui->iconToolBtn->setIcon(QIcon(QPixmap(this->icon)));
    ui->iconToolBtn->setIconSize(QSize(ui->iconToolBtn->width(),ui->iconToolBtn->height()));
    ui->namelab->setText(this->name);
    ui->tipslab->setText("");


    // 设置右键菜单
    ui->iconToolBtn->setContextMenuPolicy(Qt::CustomContextMenu);

    // 设置右键弹出菜单信号和槽
    connect(ui->iconToolBtn, &QListWidget::customContextMenuRequested, [=](const QPoint &){
        //定义菜单类,listWidget控件为父类，作用就是把菜单和控件进行绑定
        QMenu *m_menu = new QMenu(ui->iconToolBtn);

        //new一个Action功能类,菜单的子项
        QAction *act = new QAction(this);
        act->setText("删除好友");

        //当触发该子项时，执行对应的槽函数
        connect(act,&QAction::triggered,this,[=](){
            qDebug() << "delete somebody";
            emit this->dropfriend();
            this->close();
        });

        //将该子项插入菜单中
        m_menu->addAction(act);

        //执行菜单,菜单的位置在当前光标位置上
        m_menu->exec(QCursor::pos());
        delete m_menu;//执行完毕删除菜单

    });

}

FriendBtn::~FriendBtn()
{
    if (this->chatwin != nullptr)
        this->chatwin->deleteLater();

    delete ui;
}

// 双击显示窗口
void FriendBtn::mouseDoubleClickEvent(QMouseEvent *){
    //qDebug() << "双击事件";

    if (this->isopen) {
        //防止重复打开
        QMessageBox::warning(this,"警告","此窗口已打开");
        return;
    }

    if (this->chatwin == nullptr) {
        this->chatwin = new ChatWindow;
        this->chatwin->setWindowIcon(ui->iconToolBtn->icon());
        this->chatwin->setWindowTitle(this->name);
        this->chatwin->show();

        // 发送
        connect(this->chatwin,&ChatWindow::ChatSendMsg,[=](){
            emit this->sendMsg();
        });

        // 关闭窗口设置flag
        connect(this->chatwin,&ChatWindow::closeWidget,[=](){
            this->chatwin->hide();
            this->isopen = false;
        });
    } else {
        this->chatwin->show();
    }

    // 有历史消息先载入历史消息
    if (!this->msgList.isEmpty()){
        this->chatwin->setTxtColor(QColor(Qt::blue));
        for(auto it = this->msgList.begin(); it != this->msgList.end(); ++it){
            this->chatwin->recvMsg(*it,false);
        }
        this->msgList.clear();
    }

    resetmsgnum();
    setMsgTips();

    this->isopen = true;
}

QIcon FriendBtn::getIcon() {
    return ui->iconToolBtn->icon();
}

void FriendBtn::setMsgTips() {
    if (msgnum > 0) {
        QString tips = QString("%1").arg(this->msgnum);
        ui->tipslab->setText(tips);
        ui->tipslab->setStyleSheet("color:red");
    } else {
        ui->tipslab->setText("");
    }
}

void FriendBtn::recvMsg(QJsonObject obj) {

    QString msg = obj.value("msg").toString();

    // 没有双击打开窗口，消息存入消息列表，更新历史消息数量
    if (this->chatwin == nullptr){

        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        this->msgList.push_back("["+this->name+"] "+time);
        this->msgList.push_back(msg);
        addmsgnum();
        setMsgTips();

    // 窗口打开，直接接收消息
    } else {
        if(obj.value("isonline").toBool()) {
            if (this->chatwin->isHidden()) {
                addmsgnum();
                setMsgTips();
            }
            this->chatwin->setTxtColor(QColor(Qt::blue));
            this->chatwin->recvMsg(msg);
        } else {
            this->chatwin->setTxtColor(QColor(Qt::gray));
            this->chatwin->recvMsg("您发送的是离线消息", false);
        }
    }
}

void FriendBtn::recvOfflineMsg(QJsonObject obj) {

    QString msg = obj.value("msg").toString();
    QString time = obj.value("time").toString();


    // 没有双击打开窗口，消息存入消息列表，更新历史消息数量
    if (this->chatwin == nullptr){

        this->msgList.push_back("["+this->name+"] "+time);
        this->msgList.push_back(msg);
        addmsgnum();
        setMsgTips();

    // 窗口打开，直接接收消息
    } else {
        this->chatwin->setTxtColor(QColor(Qt::blue));
        this->chatwin->recvMsg("["+this->name+"] "+time,false);
        this->chatwin->recvMsg(msg,false);
    }

}

void FriendBtn::setNameColor(bool f)
{
    if (f)
        ui->namelab->setStyleSheet("color:blue");
    else
        ui->namelab->setStyleSheet("color:black");
}


