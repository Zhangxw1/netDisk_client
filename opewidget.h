#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QWidget>
#include<QListWidget>
#include"friend.h"
#include"book.h"
#include<QStackedWidget>//堆栈控件

class OpeWidget:public QWidget
{
public:
    OpeWidget(QWidget* parent = nullptr);
    static OpeWidget &getInstance();
    Friend *getFriend();
    Book *getBook();

private:
    QListWidget *m_pListW;
    Friend *m_pFriend;
    Book *m_pBook;

    QStackedWidget *m_pSW;
};

#endif // OPEWIDGET_H
