#include "search.h"
#include "ui_search.h"
#include <QLabel>
#include <QDebug>
#include <QMenu>

Search * Search::search = nullptr;
QMutex Search::mutex;

Search::Search(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Search)
{
    ui->setupUi(this);


    connect(ui->backBtn, &QPushButton::clicked, [=](){
        emit this->back();
    });

    connect(ui->clearBtn, &QPushButton::clicked, [=](){
        ui->resListWid->clear();
    });

    connect(ui->searchBtn, &QPushButton::clicked, [=](){

        ui->resListWid->clear();
        emit this->startfind();
    });

    connect(ui->addBtn, &QPushButton::clicked, [=](){
        emit this->startadd();
    });

}

Search::~Search()
{
    delete ui;
}

Search * Search::getInstance() {

    //  这里使用了两个 if 判断语句的技术称为双检锁；好处是，只有判断指针为空的时候才加锁，
    //  避免每次调用 getInstance 的方法都加锁，锁的开销毕竟还是有点大的。
    if (search == nullptr) {
        QMutexLocker locker(&mutex);
        if (search == nullptr){
            search = new Search;
        }
    }

    return search;
}

void Search::deleteInstance() {

    QMutexLocker locker(&mutex);
    if (search){
        delete search;
        search = nullptr;
    }

}

QString Search::getTarget() const {
    return ui->lineEdit->text();
}

QString Search::getItem() const {

    QListWidgetItem *curItem = ui->resListWid->currentItem();
    if (curItem == NULL){
        //qDebug() << "none ";
        return NULL;
    }

    QString curText = curItem->text();
    qDebug() << "select " << curText;

    return curText;
}

void Search::push_result(QString * res, int n){
    QList<QString> tmp;
    if (res != nullptr){
        for(int i = 0; i < n; ++i){
            tmp.push_back(res[i]);
        }
    }

    this->result.push_back(tmp);
}

void Search::show_result() {

    for(auto it = this->result.begin(); it != this->result.end(); ++it) {
        QListWidgetItem * item = new QListWidgetItem;
        //初始化空的子项,比如设置颜色,高度,内容等等
        //设置大小
        item->setSizeHint(QSize(ui->resListWid->width(),50));
        //设置内容
        item->setText(QString((*it)[1] + "  UID:"+ (*it)[0]));

        ui->resListWid->addItem(item);
    }
    ui->resListWid->setSortingEnabled(true);
    ui->resListWid->sortItems();

}
