#ifndef FRIEND_H
#define FRIEND_H
#include"online.h"//这里面包含了协议

#include<QLabel>
#include <QWidget>
#include<QTextEdit>
#include<QListWidget>
#include<QLineEdit>
#include<QPushButton>
#include<QVBoxLayout>
#include<QHBoxLayout>

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllOnlineUsr(PDU *pdu);
    void updateFriendList(PDU *pud);
    void updateGroupMsg(PDU *pud);

    QString m_strSearchName;
    QListWidget *getFriendList();

private:
    QLabel *user;
    QTextEdit *m_pShowMsgTE;
    QListWidget *m_pFriendListWidget;
    QLineEdit *m_pInputMsgLE;

    QPushButton *m_pDelFriendPB;
    QPushButton *m_pFlushFriendPB;
    QPushButton *m_pShowOnlineUsrPB;
    QPushButton *m_pSearchUsrPB;
    QPushButton *m_pMsgSendPB;
    QPushButton *m_pPrivateChatPB;

    Online *m_pOnline;


public slots:
    void showOnline();
    void searchUsr();
    void flushFriend();
    void delFriend();
    void priviteChat();
    void groupChat();




    
signals:

};

#endif // FRIEND_H
