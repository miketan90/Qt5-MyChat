#include "maketipswin.h"
#include <QPushButton>
#include <QLabel>
#include <QBoxLayout>
#include <QToolButton>
#include "flag.h"

MakeTipsWin::MakeTipsWin(QWidget *parent) : QWidget(parent)
{

}

QWidget * MakeTipsWin::verityWinInit(QString name) {

    QWidget * qwidget = new QWidget(this);

    // 创建窗口主要要素
    QPushButton * btn1 = new QPushButton("同意",qwidget);
    btn1->setFixedWidth(58);
    btn1->setStyleSheet("font-size:18px");
    connect(btn1,&QPushButton::clicked,[=](){
        emit verityWin_agree();
    });

    QPushButton * btn2 = new QPushButton("拒绝",qwidget);
    btn1->setStyleSheet("font-size:18px");
    btn2->setFixedWidth(58);
    connect(btn2,&QPushButton::clicked,[=](){
        emit verityWin_refuse();
    });

    QLabel * label = new QLabel(qwidget);
    label->setText(QString("%1\n请求添加您为好友").arg(name));
    label->setStyleSheet("font-size:18px");

    QToolButton * toolbtn = new QToolButton(qwidget);
    toolbtn->setFixedSize(QSize(70,70));
    toolbtn->setIcon(QIcon(QPixmap(":/img/default0.png")));
    toolbtn->setIconSize(QSize(toolbtn->width(),toolbtn->height()));

    // 设置布局
    QVBoxLayout * vlayout = new QVBoxLayout();
    vlayout->addWidget(btn1);
    vlayout->addWidget(btn2);
    vlayout->setSpacing(3);
    vlayout->setContentsMargins(0, 0, 0, 0); // 页边距设置为0

    QHBoxLayout * hlayout = new QHBoxLayout();
    hlayout->addWidget(toolbtn);
    hlayout->addWidget(label);
    hlayout->addLayout(vlayout);
    hlayout->setSpacing(1);
    hlayout->setAlignment(Qt::AlignTop);
    hlayout->setContentsMargins(0, 0, 0, 0);

    qwidget->setLayout(hlayout);

    return qwidget;
}

QWidget * MakeTipsWin::tipsWinInit(QString name,int flag) {

    QWidget * qwidget = new QWidget(this);

    QPushButton * btn = new QPushButton("确认",qwidget);
    btn->setFixedWidth(58);
    btn->setStyleSheet("font-size:18px");
    connect(btn,&QPushButton::clicked,[=](){
        emit tipsWin_confirm();
    });

    QLabel * label = new QLabel(qwidget);
    switch(flag){
    case REFUSE:
        label->setText(QString("%1\n拒绝您的请求").arg(name));
        label->setStyleSheet("font-size:20px; color:grey");
        break;
    case AGREE:
        label->setText(QString("%1\n同意添加您为好友").arg(name));
        label->setStyleSheet("font-size:20px; color:grey");
        break;
    case DELETE:
        label->setText(QString("%1\n已把你删除").arg(name));
        label->setStyleSheet("font-size:20px; color:grey");
        break;
    }

    QHBoxLayout * hlayout = new QHBoxLayout();
    hlayout->addWidget(label);
    hlayout->addWidget(btn);
    hlayout->setSpacing(1);
    hlayout->setAlignment(Qt::AlignTop);
    hlayout->setContentsMargins(0, 0, 0, 0);

    qwidget->setLayout(hlayout);

    return qwidget;
}
