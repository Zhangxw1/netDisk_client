#include "opewidget.h"

OpeWidget::OpeWidget(QWidget* parent):QWidget(parent)
{
    m_pListW=new QListWidget(this);
    m_pListW->addItem("好友");
    m_pListW->addItem("图书");

    m_pFriend=new Friend;
    m_pBook=new Book;
    m_pSW=new QStackedWidget;
    m_pSW->addWidget(m_pFriend);//默认显示第一个添加的窗口
    m_pSW->addWidget(m_pBook);

    QHBoxLayout *pMain=new QHBoxLayout;
    pMain->addWidget(m_pListW);
    pMain->addWidget(m_pSW);

    setLayout(pMain);
    //currentRowChanged 信号本身会传递当前行(点击的那一行)的索引值，直接作为参数传递给 setCurrentIndex
    connect(m_pListW,&QListWidget::currentRowChanged,m_pSW,&QStackedWidget::setCurrentIndex);//void currentRowChanged(int currentRow);
                                                                                             //void setCurrentIndex(int index);


}

OpeWidget &OpeWidget::getInstance()
{
    static OpeWidget instance;
    return instance;
}

Friend *OpeWidget::getFriend()
{
    return m_pFriend;
}

Book *OpeWidget::getBook()
{
    return m_pBook;
}
