#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include"protocol.h"
namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = nullptr);
    ~PrivateChat();
    static PrivateChat &getInstance();
    //聊天时需要对方的用户名和自身的名字
    void setChatName(QString strName);//保存传过来的名字到成员变量中
    void updateMsg(const PDU *pdu);

private slots:
    void on_sendMsg_pb_clicked();

private:
    Ui::PrivateChat *ui;
    QString m_strChatName;
    QString m_strLoginName;
};

#endif // PRIVATECHAT_H
