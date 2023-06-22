#include "veritywin.h"
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QBoxLayout>

VerityWin::VerityWin(QWidget *parent) : QWidget(parent)
{
    // 创建窗口主要要素
    QPushButton * btn1 = new QPushButton("同意",qwidget);
    btn1->setFixedWidth(58);
    btn1->setStyleSheet("font-size:18px");
    connect(btn1,&QPushButton::clicked,[=](){

        this->verityMsgNum--;
        if (this->verityMsgNum == 0){
            this->ui->toolBox->setItemText(1, "好友验证");
        } else {
            this->ui->toolBox->setItemText(1, QString("好友验证 - %1").arg(this->verityMsgNum));
        }

        // 回复请求


        qwidget->close();
        qwidget->deleteLater();
    });

    QPushButton * btn2 = new QPushButton("拒绝",qwidget);
    btn1->setStyleSheet("font-size:18px");
    btn2->setFixedWidth(58);
    connect(btn2,&QPushButton::clicked,[=](){

        this->verityMsgNum--;
        if (this->verityMsgNum == 0){
            this->ui->toolBox->setItemText(1, "好友验证");
        } else {
            this->ui->toolBox->setItemText(1, QString("好友验证 - %1").arg(this->verityMsgNum));
        }

        // 回复请求

    });

    QLabel * label = new QLabel(qwidget);
    label->setText(QString("%1\n请求添加您为好友").arg(obj["sendername"].toString()));
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

}
