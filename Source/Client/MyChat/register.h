#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>
#include "flag.h"

namespace Ui {
class Register;
}

class Register : public QWidget
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();

private:
    Ui::Register *ui;

    QString addr;
    unsigned short port;

signals:
    void backlogin();
};

#endif // REGISTER_H
