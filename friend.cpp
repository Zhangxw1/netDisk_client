#include "friend.h"
#include"protocol.h"
#include"tcpclient.h"
#include<QInputDialog>
#include<QDebug>
#include"privatechat.h"
#include<QMessageBox>


Friend::Friend(QWidget *parent) : QWidget(parent)
{
    QString strLoginName="当前用户:"+TcpClient::getInstance().loginName();//获得登入的用户名
    user=new QLabel(strLoginName);
    m_pShowMsgTE=new QTextEdit;
    m_pFriendListWidget=new QListWidget;

    m_pInputMsgLE=new QLineEdit;
    m_pMsgSendPB=new QPushButton("发送");

    m_pDelFriendPB=new QPushButton("删除好友");
    m_pFlushFriendPB=new QPushButton("刷新好友");
    m_pShowOnlineUsrPB=new QPushButton("显示在线用户");
    m_pSearchUsrPB=new QPushButton("查找用户");
    m_pPrivateChatPB=new QPushButton("私聊");

    QVBoxLayout *pRightPBVBL=new QVBoxLayout;
    pRightPBVBL->addWidget(user);
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);

    QHBoxLayout *pTopHBL=new QHBoxLayout;
    pTopHBL->addWidget(m_pShowMsgTE);
    pTopHBL->addWidget(m_pFriendListWidget);
    pTopHBL->addLayout(pRightPBVBL);           //三个水平摆

    QHBoxLayout *pMsgHBL=new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline=new Online;

    QVBoxLayout *pMain=new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();

    setLayout(pMain);//pMain这个垂直布局把所有内容都攘括进去了，就可以setLayout铺满

    connect(m_pShowOnlineUsrPB,&QPushButton::clicked,this,&Friend::showOnline);
    connect(m_pSearchUsrPB,&QPushButton::clicked,this,&Friend::searchUsr);
    connect(m_pFlushFriendPB,&QPushButton::clicked,this,&Friend::flushFriend);
    connect(m_pDelFriendPB,&QPushButton::clicked,this,&Friend::delFriend);
    connect(m_pPrivateChatPB,&QPushButton::clicked,this,&Friend::priviteChat);
    connect(m_pMsgSendPB,&QPushButton::clicked,this,&Friend::groupChat);

}

void Friend::showAllOnlineUsr(PDU *pdu)
{
    if(nullptr==pdu){
        return;
    }
    m_pOnline->showUsr(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    if(nullptr==pdu){
        return;
    }
    m_pFriendListWidget->clear();//清空列表
    uint uiSize=pdu->uiMsgLen/32; //人数
    char caName[32]={'\0'};
    for(uint i=0;i<uiSize;i++){
        memcpy(caName,(char*)(pdu->caMsg)+i*32,32);
        m_pFriendListWidget->addItem(caName);
    }
}

void Friend::updateGroupMsg(PDU *pdu)
{
    QString strMsg=QString("%1 says:%2").arg(pdu->caData).arg((char*)pdu->caMsg);
    m_pShowMsgTE->append(strMsg);

}

QListWidget *Friend::getFriendList()
{
    return m_pFriendListWidget;
}

void Friend::showOnline()
{
    if(m_pOnline->isHidden()){
        m_pOnline->show();
        //请求所有用户的信息
        PDU *pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//利用socket把数据发送给服务器
        free(pdu);
        pdu=NULL;
    }else{
        m_pOnline->hide();
    }
}

void Friend::searchUsr()
{
    m_strSearchName=QInputDialog::getText(this,"搜索","用户名:");
    if(!m_strSearchName.isEmpty()){
        qDebug()<<"您搜索的是"<<m_strSearchName;
        PDU *pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        strcpy(pdu->caData,m_strSearchName.toStdString().c_str()); //char caData[64]; name由QString转char*类型
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//利用socket把数据发送给服务器
        free(pdu);
        pdu=NULL;
    }
}

void Friend::flushFriend()
{

    QString strName=TcpClient::getInstance().loginName();
    PDU *pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//利用socket把数据发送给服务器
    free(pdu);
    pdu=NULL;
}

//被删人在线要提示被删用户  不在线也不好删...
void Friend::delFriend()
{
    //没选择则不操作(也可以给提示)
    if(nullptr!=m_pFriendListWidget->currentItem()){
        QString strFriendName=m_pFriendListWidget->currentItem()->text();
        PDU *pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_TYPE_DEL_FRIEND_REQUEST;
        QString strSelfName=TcpClient::getInstance().loginName();
        memcpy(pdu->caData,strSelfName.toStdString().c_str(),strSelfName.size());
        memcpy(pdu->caData+32,strFriendName.toStdString().c_str(),strFriendName.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//利用socket把数据发送给服务器
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::warning(this,"删除好友","请选择您要删除的好友");
    }
}

void Friend::priviteChat()
{
    if(nullptr!=m_pFriendListWidget->currentItem()){
        QString strFriendName=m_pFriendListWidget->currentItem()->text();
        PrivateChat::getInstance().setChatName(strFriendName);//设置对方的名字
        if(PrivateChat::getInstance().isHidden()){
            PrivateChat::getInstance().show();
        }
    }else{
        QMessageBox::warning(this,"私聊","请选择您要私聊的对象");
    }
}

void Friend::groupChat()
{
    QString strMsg=m_pInputMsgLE->text();
    m_pInputMsgLE->clear();
    if(!strMsg.isEmpty()){
        PDU *pdu=mkPDU(strMsg.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        QString strName=TcpClient::getInstance().loginName();
        memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());
        memcpy((char*)(pdu->caMsg),strMsg.toStdString().c_str(),strMsg.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//群聊请求发送给服务器
    }else{
        QMessageBox::warning(this,"群聊","发送的消息不能为空");
    }
}




