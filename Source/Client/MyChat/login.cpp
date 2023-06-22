#include "login.h"
#include "ui_login.h"
#include <QDebug>
#include <QLabel>
#include <QDialog>
#include <QMovie>
#include <QThreadPool>

Login::Login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    qDebug()<<"Login窗口线程ID:"<<QThread::currentThreadId();

    this->setWindowIcon(QIcon(QPixmap(":/img/ChatIcon.jpg")));

    //注册按钮
    connect(ui->registerBtn,&QPushButton::clicked,this,&Login::registerChat);

    //登录按钮
    connect(ui->loginBtn,&QPushButton::clicked, this, &Login::LoginChat);


}

Login::~Login()
{
    delete ui;
}

void Login::loadingMovie(){

    if (this->label == nullptr) {

        this->label = new QLabel(this);
        this->label->setAlignment(Qt::AlignCenter);
        this->label->setStyleSheet("background-color:rgb(255,255,255);");
        this->label->setFixedSize(this->width(),this->height());
        // 载入gif图片
        QMovie *movie = new QMovie(":/img/loading.gif");
        movie->setParent(this->label);
        movie->setScaledSize(QSize(100,100)*1.2);
        this->label->setMovie(movie);

        this->label->show();
        movie->start();
    }
}

void Login::LoginChat() {

    if (ui->userLEdit->text().isEmpty()){
        QMessageBox::critical(this,"error","用户名不能为空");
        return;
    }

    if (ui->pwLEdit->text().isEmpty()){
        QMessageBox::critical(this,"error","密码不能为空");
        return;
    }

    // 创建主窗口线程
    MainWindow * mWin = new MainWindow(ui->userLEdit->text(),ui->pwLEdit->text());

    // 使登录键不可用
    ui->loginBtn->setDisabled(true);
    ui->registerBtn->setDisabled(true);

    this->loadingMovie();

    // 关闭主窗口返回登录界面
    connect(mWin, &MainWindow::closeBackLog, this,[=](){

        mWin->hide();
        mWin->deleteLater();
        //this->pthread->deleteLater();
        if (this->label) {
            delete this->label;
            this->label = nullptr;
        }

        this->show();
        ui->loginBtn->setEnabled(true); // 恢复登录按键
        ui->registerBtn->setEnabled(true);

    });

    // 连接成功隐藏登录窗口
    connect(mWin, &MainWindow::isconnect, this, [=](){
        mWin->show();
        this->hide();

        delete this->label;
        this->label = nullptr;
    });

    mWin->connectServer();  // 连接服务器

}

void Login::registerChat() {

    Register *rg = new Register;

    rg->setGeometry(this->geometry());

    this->hide();
    rg->show();

    connect(rg,&Register::backlogin,this, [=](){
        this->setGeometry(rg->geometry());
        rg->hide();
        this->show();
        delete rg;
    });
}
