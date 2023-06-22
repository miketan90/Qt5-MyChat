#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>

#include "register.h"
#include "mainwindow.h"

namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

    int LoginToServer();

private:
    Ui::Login *ui;

    QString username;
    QString password;

    // 用于播放loading gif图
    QLabel * label = nullptr;

    // 设置等待加载提示
    void loadingMovie();
    // 登录操作
    void LoginChat();
    // 注册操作
    void registerChat();
signals:
    void Tothread(QString,int);
};

#endif // LOGIN_H
