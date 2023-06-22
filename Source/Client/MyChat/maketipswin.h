#ifndef MAKETIPSWIN_H
#define MAKETIPSWIN_H

#include <QWidget>

class MakeTipsWin : public QWidget
{
    Q_OBJECT
public:
    explicit MakeTipsWin(QWidget *parent = nullptr);

    QWidget * verityWinInit(QString );
    QWidget * tipsWinInit(QString,int);

signals:
    void verityWin_agree();
    void verityWin_refuse();

    void tipsWin_confirm();
};

#endif // MAKETIPSWIN_H
