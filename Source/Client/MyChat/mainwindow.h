#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// 窗口和控件
#include <QWidget>
#include <QDebug>
#include <QPushButton>
#include <QToolButton>
#include <QAction>
#include <QMessageBox>
#include <QPixmap>
// 容器
#include <QList>
#include <QMap>
// TCP、多线程
#include <QThread>
#include <QTcpSocket>
// 文件操作
#include <QByteArray>
#include <QFile>
#include <QDir>
// Json操作
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
// 时间和随机数
#include <QTime>
#include <QtGlobal>
// 自定义
#include "chatwindow.h"
#include "friendbtn.h"
#include "search.h"
#include "flag.h"
#include "maketipswin.h"

// 宏 - 服务器地址和端口
#define SERVER  "192.168.130.130"
#define PORT    8000

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:

    MainWindow(QString username,QString password,QWidget *parent = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent *event) override;

    // 建立与服务器连接
    void connectServer();
    // 解析string格式的Json数据
    QJsonObject parserStr(QString str);
    // 读路径是path的Json文件
    QJsonObject readJsonFile(QString path);
    // 写string格式的json数据到path下的文件
    void writeJsonFile(QString path, QJsonObject obj);

private:
    Ui::MainWindow *ui;

    QTcpSocket * socket;

    QString username;
    QString uid;
    QString password;
    // 保存朋友按钮的指针
    QMap<QString, FriendBtn *> myfriend;
    // 搜索窗口
    Search * searchWin;
    // 验证消息数量
    int verityMsgNum = 0;
    int onlinenum = 0;


    // 窗口初始化
    void windowInit();
    // 创建朋友按钮
    FriendBtn * ItemFriend(QString uid, QString name, QString icon = "");
    // 登录请求
    void loginRequest();
    // 从服务器读数据
    void readFromServer();
    // 写QJsonObject数据到server
    void writeToServer(const QJsonObject & obj);
    // 处理服务器的登录返回
    bool dealWithLogin(QJsonObject & obj);
    // 搜索朋友
    void searchwin();
    void searchRequest();
    void searchReponse(const QJsonObject &obj);
    // 添加朋友
    void addRequset();
    void addFriend(const QJsonObject &obj);
    // 好友验证消息添加及处理
    void friendTips(QJsonObject &obj,int flag);
    QWidget * verityProcess(QJsonObject &obj);
    QWidget * dealWithTips(QJsonObject &obj, int flag);

signals:
    void closeBackLog();
    void isconnect();
    void networderr();

};
#endif // MAINWINDOW_H
