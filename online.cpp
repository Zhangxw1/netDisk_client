#include "online.h"
#include "ui_online.h"
#include<QDebug>
#include"tcpclient.h"
#include<QMessageBox>

Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUsr(PDU *pdu)
{
    if(NULL==pdu){
        return;
    }
    ui->online_lw->clear();
    uint uiSize=pdu->uiMsgLen/32; //个数=消息部分/32
    char caTmp[32];//caTmp 是一个局部数组，它在每次循环迭代开始时都会自动“清除”或说“重置”，因为它每次都会重新进入作用域
    for(uint i=0;i<uiSize;i++){
        memcpy(caTmp,(char*)(pdu->caMsg)+i*32,32);
        ui->online_lw->addItem(caTmp);
    }
}


void Online::on_addFriend_pb_clicked()
{
    if(nullptr==ui->online_lw->currentItem()){
        QMessageBox::critical(this, "添加好友", "请选择你要添加的好友");
        return;
    }
    QListWidgetItem *pItem=ui->online_lw->currentItem();
    QString strPerUsrName=pItem->text();//获取选中的用户名
    QString strLoginName=TcpClient::getInstance().loginName();//获得登入的用户名
//应该没用
//    if( NULL == strPerUsrName || NULL == strLoginName)
//    {
//        QMessageBox::critical(this, "添加好友", "添加好友失败：当前用户或选择不能为空");
//        return ;
//    }
    if(strPerUsrName == strLoginName)
    {
        QMessageBox::critical(this, "添加好友", "添加好友失败：不能添加自己");
        return ;
    }
    PDU *pdu=mkPDU(0);//名字32字节，小于64字节，放得下，实际消息传0就行
    pdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData,strPerUsrName.toStdString().c_str(),strPerUsrName.size());//前32字节放选中的用户名
    memcpy(pdu->caData+32,strLoginName.toStdString().c_str(),strLoginName.size());//32后字节放登入的用户名

    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//利用socket把数据发送给服务器
    free(pdu);
    pdu=NULL;
}
