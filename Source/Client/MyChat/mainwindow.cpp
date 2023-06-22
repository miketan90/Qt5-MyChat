#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QAction>

MainWindow::MainWindow(QString username,QString password,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qDebug()<<"mainwindow构造函数ID:"<<QThread::currentThreadId();

    QString path1 = QDir::cleanPath(QCoreApplication::applicationDirPath() + \
                                    QDir::separator() + QString("/data"));
    QDir dir1(path1);
    if (!dir1.exists()){
        dir1.mkdir(path1);
    }

    this->setWindowIcon(QIcon(QPixmap(":/img/ChatIcon.jpg")));

    this->username = username;
    this->password = password;
    this->socket = nullptr;
    this->searchWin = nullptr; 

    ui->toolBox->setItemText(0,"我的好友(0/0)");
    ui->toolBox->setItemText(1,"好友验证");

    // 添加朋友窗口信号和槽
    connect(ui->addfriendbtn, &QPushButton::clicked, this, [=](){
        this->searchwin();
    });


}

MainWindow::~MainWindow()
{
    Search::deleteInstance();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *) {
    //qDebug() << "closeEvent";
    socket->close();
    socket->deleteLater();
    emit this->closeBackLog();
}

void MainWindow::windowInit(){

    this->setWindowTitle("MyChat2023");

    // 初始化用户信息
    QPixmap pix;
    pix.load(":/img/default0.png");
    this->ui->headBtn->setIcon(QIcon(pix));
    this->ui->headBtn->setIconSize(QSize(ui->headBtn->width(),ui->headBtn->height()));
    this->ui->userlabel->setText(this->username);
    this->ui->userlabel->setFont(QFont("黑体",14,QFont::Bold));

    // 读Json文件，初始化服务器保存的数据
    QString fname = QString("/data/%1_data.json").arg(this->username);
    QString path = QDir::cleanPath(QCoreApplication::applicationDirPath() + \
                                   QDir::separator() + fname);
    //qDebug() << path;
    QJsonArray arr;
    QJsonObject ob = readJsonFile(path);
    if(ob.empty()) {
        qDebug() << "init failure";
        return;
    } else {
        //qDebug() << "read ok";
        this->uid = ob.value("uid").toString();
        arr = ob.value("friend").toArray();
    }

    // 创建朋友对象
    //QGroupBox *groupbox = new QGroupBox(ui->page_0);          // 分组
    QVBoxLayout * playout = new QVBoxLayout(ui->page_0);          // 布局
    for (int i = 0; i < arr.size(); ++i){
        qsrand(QTime::currentTime().msec());          //设置随机数种子
        int rand = qrand() % 4 + 1;

        QString imgpath = QString(":/img/default%1.png").arg(rand);
        //qDebug() << "rand:" << rand << "imgpath: " << imgpath;
        QJsonObject item = arr[i].toObject();

        FriendBtn * fBtn = ItemFriend(item.value("uid").toString(), item.value("username").toString(), imgpath);
        playout->addWidget(fBtn);

        QThread::msleep(100);
    }
    //playout->addStretch();
    playout->setSpacing(1);
    playout->setAlignment(Qt::AlignTop);

    // 取toolBox第一个窗口，设置布局进去
    ui->toolBox->widget(0)->setLayout(playout);
    ui->toolBox->setItemText(0,QString("我的好友(%1/%2)").arg(this->onlinenum).arg(this->myfriend.size()));
}

void MainWindow::connectServer(){

    this->socket = new QTcpSocket(this);

    this->socket->connectToHost(QHostAddress(SERVER),PORT);

    // 成功连接
    connect(socket,&QTcpSocket::connected,this,[=](){
        //qDebug() << "prepare login";
        this->loginRequest();
    });

    // 捕获socket错误
    connect(socket,QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),this,[=](QAbstractSocket::SocketError err){
        if (err == 0)
            QMessageBox::critical(this,"error",QString("ServerError: %1, 无法连接服务器").arg(err));
        else
            QMessageBox::critical(this,"error",QString("ServerError: %1, 网络连接中断").arg(err));
        //emit this->networderr();
        this->close();
    });

    // 读事件
    connect(this->socket,&QTcpSocket::readyRead,this,[=](){
        this->readFromServer();
    });
}

void MainWindow::loginRequest() {
    QJsonObject obj;
    obj.insert("cmd",LOGIN);
    obj.insert("user",this->username);
    obj.insert("password",this->password);

    this->writeToServer(obj);
}

void MainWindow::readFromServer() {
    //qDebug() << "read";
    // 判断是否可读
    if(!this->socket->isValid()){
        qDebug() << "unreadable";
        return;
    }
    // 从socket读取的数据
    qint64 total = this->socket->bytesAvailable();
    qDebug() << "total: " << total;
    char buf[total];
    while(0 < this->socket->readLine(buf,sizeof(buf) + 1)){

        QString line = QString::fromUtf8(buf);

        qDebug() << "Client recv: " << line;

        // 解析Json数据
        QJsonObject obj = this->parserStr(line);

        // 登录响应处理
        int cmd = obj.value("cmd").toInt();
        if (cmd == LOGIN){
            //qDebug() << "deal login reply";

            if(!dealWithLogin(obj)){
                this->close();
                return;
            }

            QString fname = QString("/data/%1_data.json").arg(obj["user"].toString());
            QString path = QDir::cleanPath(QCoreApplication::applicationDirPath() + \
                                           QDir::separator() + fname);

            writeJsonFile(path, obj);

            // 登录成功并且请求到数据，初始化主窗口
            this->windowInit();

            // 初始化成功，显示主窗口
            emit this->isconnect();

            qDebug() << "Init success";

        // 聊天响应处理
        } else if (cmd == CHAT){

            if (obj.value("isonline").toBool() == false) {
                // 对方不在线，消息在自己客户端的好友窗口显示
                QString id = obj.value("receiver").toString();
                myfriend[id]->recvMsg(obj);

            } else {
                // 对方在线，消息在对方客户端的好友窗口显示
                QString id = obj.value("sender").toString();
                if (obj.find("time") != obj.end()) {
                    myfriend[id]->recvOfflineMsg(obj);
                } else {
                    myfriend[id]->recvMsg(obj);
                }
            }

       // 好友操作响应处理
        } else if (cmd == FRIEND){

            int op = obj.value("operate").toInt();
            if ( op == SEARCH) {
                // 搜索结果
                this->searchReponse(obj);

            } else if (op == ADD) {
                // 处理好友已验证回发的消息
                if (obj.value("verity").toBool()){
                    bool IsAgree = obj.value("IsAgree").toBool();
                    if (IsAgree) {
                        if (obj.find("time") == obj.end())
                            this->addFriend(obj);
                        // 提示同意
                        this->friendTips(obj,AGREE);
                        ui->toolBox->setItemText(0,QString("我的好友(%1/%2)").arg(this->onlinenum).arg(this->myfriend.size()));

                    } else {
                        // 在好友验证列表提示拒绝
                        this->friendTips(obj,REFUSE);
                    }

                // 处理好友未验证的消息
                } else {
                    // 加入好友验证列表，点击确认修改verify回发给服务器
                    this->friendTips(obj,VERITY);
                }

            } else if (op == DEL) {

                // 对方客户端窗口删除好友
                QString id = obj.value("sender").toString();
                this->myfriend[id]->close();
                this->myfriend[id]->deleteLater();
                this->myfriend.remove(id);
                // 提示被删除
                this->friendTips(obj,DELETE);
                ui->toolBox->setItemText(0,QString("我的好友(%1/%2)").arg(this->onlinenum).arg(this->myfriend.size()));

                QJsonObject rep;
                rep.insert("cmd",FRIEND);
                rep.insert("operate",DEL);
                rep.insert("target",id);
                rep.insert("clean",true);
                this->writeToServer(rep);

            }  else {}

        } else if (cmd == ONLINE){


            bool on = obj.value("iscome").toBool();
            if (on)
                this->onlinenum++;
            else {
                if (onlinenum > 0)
                    this->onlinenum--;
            }

            if (obj.find("friend") == obj.end()){
                // 好友设置我在线
                QString id = obj.value("sender").toString();
                if (myfriend.contains(id))
                    this->myfriend[id]->setNameColor(on);
            } else {
                // 我设置好友在线
                QString id = obj.value("friend").toString();
                if (myfriend.contains(id))
                    this->myfriend[id]->setNameColor(on);
            }
            ui->toolBox->setItemText(0,QString("我的好友(%1/%2)").arg(this->onlinenum).arg(this->myfriend.size()));

        } else {}

    }

}

bool MainWindow::dealWithLogin(QJsonObject & obj) {

    int res = obj.value("result").toInt();

    switch(res){

    case LOGIN_SUCCESS:
        qDebug() << "login success";
        return true;

    case LOGIN_ALREADY_DO:
        //qDebug() << "already login";
        QMessageBox::critical(this,"error",QString("用户已被登录"));
        return false;

    case LOGIN_PASSWORD_ERROR:
        //qDebug() << "password error";
        QMessageBox::critical(this,"error",QString("密码错误"));
        return false;

    case LOGIN_USER_NONEXISTENCE:
        //qDebug() << "user nonexistence";
        QMessageBox::critical(this,"error",QString("用户不存在"));
        return false;

    default:
        //qDebug() << "other";
        QMessageBox::critical(this,"error",QString("其他错误"));
        return false;
    }

}

QJsonObject MainWindow::parserStr(QString str){
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

void MainWindow::writeToServer(const QJsonObject & obj) {

    if(obj.isEmpty()){
        qDebug() << "JsonObject is empty";
        return;
    }

    QJsonDocument doc(obj);

    this->socket->write(doc.toJson());

}

QJsonObject MainWindow::readJsonFile(QString path){

    QFile file(path);
    if(!file.exists()){
        qDebug() << "file not exists";
        return QJsonObject();
    }

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "open failure";
        return QJsonObject();
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString str;
    while(!stream.atEnd()){
        str.append(stream.readLine());
    }

    file.close();

    QJsonObject obj = parserStr(str);

    return obj;
}

void MainWindow::writeJsonFile(QString path, QJsonObject obj){

    QFile file(path);
    if(file.exists()) {
        if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
            qDebug() << "open failure";
            return;
        }
    } else {
        qDebug() << "file not exists";
        if(!file.open(QIODevice::Append)){
            qDebug() << "open failure";
            return;
        }
    }

    // 写string方法写入，不能调格式
    //QTextStream stream(&file);
    //stream << text.toUtf8();

    // 用QJsonDocument可以调格式
    QJsonDocument doc(obj);
    file.write(doc.toJson());

    file.close();
}

FriendBtn * MainWindow::ItemFriend(QString fuid, QString fname, QString icon){

    FriendBtn * friendBtn = new FriendBtn(fname, icon);
    friendBtn->setParent(ui->page_0);
    this->myfriend[fuid] = friendBtn;

    // 右键删除好友
    connect(friendBtn,&FriendBtn::dropfriend,this,[=](){
        QJsonObject obj;
        obj.insert("cmd",FRIEND);
        obj.insert("operate",DEL);
        obj.insert("sender",this->uid);
        obj.insert("sendername",this->username);
        obj.insert("receiver",fuid);
        //obj.insert("stillfriend",true);

        this->writeToServer(obj);

        this->myfriend[fuid]->close();
        this->myfriend[fuid]->deleteLater();
        this->myfriend.remove(fuid);
    });

    // TCP发送消息通过服务器转发给对方客户端
    connect(friendBtn,&FriendBtn::sendMsg,this,[=](){
        QJsonObject obj;
        obj.insert("cmd",CHAT);
        obj.insert("sender",this->uid);
        obj.insert("receiver",fuid);
        obj.insert("msg",friendBtn->outputMsg(this->username));

        this->writeToServer(obj);
    });


    return friendBtn;
}

void MainWindow::searchwin() {
    this->searchWin = Search::getInstance();
    this->searchWin->show();

    // 返回
    connect(this->searchWin,&Search::back,this,[=](){
        this->searchWin->hide();
        Search::deleteInstance();
        this->searchWin = nullptr;
    });

    // 查找，向服务器发起查找请求
    connect(this->searchWin,&Search::startfind,this,[=](){
        this->searchRequest();
    });

    // 添加，向对方发起添加好友请求
    connect(this->searchWin,&Search::startadd,this,[=](){
        this->addRequset();
    });
}

void MainWindow::searchRequest(){
    QString target = this->searchWin->getTarget();
    this->searchWin->clear_result();
    QString sql = QString("select uid,username from chatuser where username like '%%1%';").arg(target);

    // 去服务器查找相关信息
    QJsonObject obj;
    obj.insert("cmd",FRIEND);
    obj.insert("operate",SEARCH);
    obj.insert("sql", sql);

    this->writeToServer(obj);

}

void MainWindow::searchReponse(const QJsonObject &obj) {
    //qDebug() << "recvSearchRes";

    // 无数据
    if (obj["result"].isNull()){
        QMessageBox::information(this->searchWin,"提示","没有查询结果");
        return;
    }

    // 有数据
    QJsonArray arr = obj["result"].toArray();
    if (arr[0].toArray().isEmpty()){
        QString row[arr.size()];
        for(int i = 0; i < arr.size(); ++i){
            row[i] = arr[i].toString();
        }
        this->searchWin->push_result(row,arr.size());

    } else {
        for(int i = 0; i < arr.size(); ++i){
            QJsonArray tmp = arr[i].toArray();

            QString row[tmp.size()];
            for(int j = 0; j < tmp.size(); ++j){
                row[j] = tmp[j].toString();
                //qDebug() << row[j];
            }
            this->searchWin->push_result(row,tmp.size());
        }
    }


    this->searchWin->show_result();

}

void MainWindow::addRequset() {

    QString item = this->searchWin->getItem();

    if (item.isNull()) {
        QMessageBox::warning(this->searchWin,"警告","未选中数据");
        qDebug() << "no data";

    } else {
        QString tmp[2];
        int uidpos = item.indexOf("UID:");
        tmp[0] = item.mid(uidpos+4);
        tmp[1] = item.mid(0,uidpos-2);

        // 防止重复添加
        if(this->myfriend.constFind(tmp[0]) != this->myfriend.end()){
            QMessageBox::warning(this->searchWin,"警告","已添加过此人");
            return;
        }
        if(this->username == tmp[1]){
            QMessageBox::warning(this->searchWin,"警告","不能添加自己");
            return;
        }

        // 去服务器执行添加操作
        QJsonObject obj;
        obj.insert("cmd",FRIEND);
        obj.insert("operate",ADD);
        obj.insert("sender",this->uid);
        obj.insert("sendername",this->username);
        obj.insert("receiver",tmp[0]);
        obj.insert("receivername",tmp[1]);
        obj.insert("verity",false);

        this->writeToServer(obj);

        QMessageBox::information(this->searchWin,"提示","发送完成");
    }

}

void MainWindow::addFriend(const QJsonObject &obj){

        QString fuid;   QString fname;
        // 判断添加对方，防止自己添加自己
        if (this->uid == obj["receiver"].toString()) {
            fuid = obj["sender"].toString();
            fname = obj["sendername"].toString();
        } else {
            fuid = obj["receiver"].toString();
            fname = obj["receivername"].toString();
        }

        FriendBtn * fBtn = ItemFriend(fuid, fname, ":/img/default1.png");
        fBtn->setParent(ui->toolBox);

        // 添加到toolbox
        QLayout * layout = ui->toolBox->widget(0)->layout();
        if (layout == nullptr) {
            //QGroupBox *groupbox = new QGroupBox(ui->page_0);       // 分组
            QVBoxLayout * vlayout = new QVBoxLayout(ui->page_0);      // 布局
            vlayout->addWidget(fBtn);
            //playout->addStretch();
            vlayout->setSpacing(1);
            vlayout->setAlignment(Qt::AlignTop);
            ui->toolBox->widget(0)->setLayout(vlayout);

        } else {
            layout->addWidget(fBtn);
            layout->setAlignment(Qt::AlignTop);
        }

        this->myfriend[fuid] = fBtn;
        qDebug() << "add success";

}

void MainWindow::friendTips(QJsonObject &obj, int flag){

    this->verityMsgNum++;
    this->ui->toolBox->setItemText(1, QString("好友验证(%1)").arg(this->verityMsgNum));

    QWidget * tipswin;
    switch(flag){
    case VERITY:
        tipswin = verityProcess(obj);
        break;
    case AGREE:
        tipswin = dealWithTips(obj, AGREE);
        break;
    case REFUSE:
        tipswin = dealWithTips(obj, REFUSE);
        break;
    case DELETE:
        tipswin = dealWithTips(obj, DELETE);
        break;
    default:
        tipswin = nullptr;
        break;
    }


    // 添加布局
    QLayout * mainlayout = ui->toolBox->widget(1)->layout();
    if (mainlayout == nullptr) {
        //QGroupBox *groupbox = new QGroupBox(ui->page_1);
        QVBoxLayout * layout = new QVBoxLayout(ui->page_1);
        layout->addWidget(tipswin);
        layout->setSpacing(5);
        layout->setAlignment(Qt::AlignTop);
        ui->toolBox->widget(1)->setLayout(layout);
    } else {
        mainlayout->addWidget(tipswin);
    }

}

QWidget * MainWindow::verityProcess(QJsonObject &obj){

    MakeTipsWin * make = new MakeTipsWin(ui->page_1);
    QWidget * pwidget = make->verityWinInit(obj["sendername"].toString());

    connect(make,&MakeTipsWin::verityWin_agree,this,[=](){

        this->verityMsgNum--;
        if (this->verityMsgNum == 0){
            this->ui->toolBox->setItemText(1, "好友验证");
        } else {
            this->ui->toolBox->setItemText(1, QString("好友验证(%1)").arg(this->verityMsgNum));
        }

        // 回复请求
        // 检查有没有操作添加好友，防止相同的验证消息多次重复验证
        if(this->myfriend.constFind(obj["sender"].toString()) == this->myfriend.end()) {
            QJsonObject rep;
            rep.insert("cmd",obj["cmd"]);
            rep.insert("operate",obj["operate"]);
            rep.insert("sender",obj["receiver"]);
            rep.insert("sendername",obj["receivername"]);
            rep.insert("receiver",obj["sender"]);
            rep.insert("receivername",obj["sendername"]);
            rep.insert("verity",true);
            rep.insert("IsAgree",true);
            this->writeToServer(rep);

            addFriend(obj);
        }

        pwidget->close();
        delete make;
    });

    connect(make,&MakeTipsWin::verityWin_refuse,this,[=](){

        this->verityMsgNum--;
        if (this->verityMsgNum == 0){
            this->ui->toolBox->setItemText(1, "好友验证");
        } else {
            this->ui->toolBox->setItemText(1, QString("好友验证(%1)").arg(this->verityMsgNum));
        }

        // 回复请求
        if(this->myfriend.constFind(obj["sender"].toString()) == this->myfriend.end()) {
            QJsonObject rep;
            rep.insert("cmd",obj["cmd"]);
            rep.insert("operate",obj["operate"]);
            rep.insert("sender",obj["receiver"]);
            rep.insert("sendername",obj["receivername"]);
            rep.insert("receiver",obj["sender"]);
            rep.insert("receivername",obj["sendername"]);
            rep.insert("verity",true);
            rep.insert("IsAgree",false);
            this->writeToServer(rep);
        }

        pwidget->close();
        delete make;
    });

    return pwidget;
}

QWidget * MainWindow::dealWithTips(QJsonObject &obj, int flag) {

    MakeTipsWin * make = new MakeTipsWin(ui->page_1);
    QWidget * pwidget = make->tipsWinInit(obj["sendername"].toString(), flag);

    connect(make,&MakeTipsWin::tipsWin_confirm,this,[=](){

        this->verityMsgNum--;
        if (this->verityMsgNum == 0){
            this->ui->toolBox->setItemText(1, "好友验证");
        } else {
            this->ui->toolBox->setItemText(1, QString("好友验证(%1)").arg(this->verityMsgNum));
        }

        pwidget->close();
        delete make;
    });

    return pwidget;

}
