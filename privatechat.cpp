#include "privatechat.h"
#include "ui_privatechat.h"
#include"protocol.h"
#include"tcpclient.h"
#include<QMessageBox>
PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

//设置 与你聊天人的名字
void PrivateChat::setChatName(QString strName)
{
    m_strChatName=strName;
    m_strLoginName=TcpClient::getInstance().loginName();
}

void PrivateChat::updateMsg(const PDU *pdu)
{//tcpclient.cpp传来的pdu的caData,前32位是发送给你信息的人,后32位存的此客户端的人的名字
    if(nullptr==pdu){
        return;
    }
    char caSendName[32]={'\0'};
    memcpy(caSendName,pdu->caData,32);
    QString strMsg=QString("%1 says:%2").arg(caSendName).arg((char*)pdu->caMsg);//pdu->caMsg为聊天的信息(从最开始的pdu拷贝了一次一直没用过，直到这里显示)
    ui->showMsg_te->append(strMsg);
}


//
void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg=ui->inputMsg_le->text();
    ui->inputMsg_le->clear();
    if(!strMsg.isEmpty()){
        PDU *pdu=mkPDU(strMsg.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData,m_strLoginName.toStdString().c_str(),m_strLoginName.size());//拷贝我方的名字
        memcpy(pdu->caData+32,m_strChatName.toStdString().c_str(),m_strChatName.size());//拷贝对方的名字
        //拷贝聊天信息
        strcpy((char*)pdu->caMsg,strMsg.toStdString().c_str());//caMsg为int类型,不能用于strcpy接受char*型    strcpy(char *,const char *);

        //在发送者自己的聊天框显示
        QString strSelfMsg=QString("%1 says:%2").arg(m_strLoginName).arg(strMsg);
        ui->showMsg_te->append(strSelfMsg);

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//获取socket发送pdu
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::warning(this,"私聊","发送的聊天信息不能为空");
    }
}
