#ifndef SEARCH_H
#define SEARCH_H

#include <QWidget>
#include <QListWidget>
#include <QMutex>
#include <QMutexLocker>

namespace Ui {
class Search;
}

class Search : public QWidget
{
    Q_OBJECT

public:

    static Search * getInstance();
    static void deleteInstance();

    QString getTarget() const;
    QString getItem() const;
    void push_result(QString * res, int len = 0);
    void clear_result() { if(!this->result.isEmpty()) this->result.clear(); }
    void show_result();

private:
    Ui::Search *ui;

    // 单例模式
    explicit Search(QWidget *parent = nullptr);
    ~Search();
    Search(const Search&);
    Search& operator=(const Search&);

    static Search * search;
    static QMutex mutex;

    QList<QList<QString>> result;

signals:
    void back();
    void startfind();
    void startadd();
};

#endif // SEARCH_H
