#include "tcpclient.h"
#include "ui_tcpclient.h"
#include<QByteArray>
#include<QDebug>
#include<QMessageBox>
#include<QHostAddress>
#include"privatechat.h"
TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    resize(400,220);
    loadConfig();


    connect(&m_tcpSocket,&QTcpSocket::connected,this,&TcpClient::showConnect);
    //    connect(&m_tcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));  //Qt4
    connect(&m_tcpSocket,&QTcpSocket::readyRead,this,&TcpClient::recvMsg);//服务端有数据发来(有readyRead信号) 则接收数据做出回复


    //连接服务器(连接成功则会发出connected信号，上面做了提示的connect连接)
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);//virtual void connectToHost(const QHostAddress &address服务器ip地址, quint16 port服务器端口,默认读写打开);
}

TcpClient::~TcpClient()
{
    delete ui;
}

//加载配置文件(服务器的地址结构)
void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray baData=file.readAll();//读取所有数据  字节数组返回QByteArray readAll();
        QString strData=baData.toStdString().c_str();//字节类型转换成字符串再转成char*
        file.close();
        strData.replace("\r\n"," ");//使用空格替换\r\n
        //    qDebug()<<strData;
        QStringList strList=strData.split(" ");//按空格切分
        //    for(int i=0;i<strList.size();i++){
        //        qDebug()<<strList[i];
        //        }
        m_strIP=strList[0];
        m_usPort=strList[1].toUShort();//(端口号)字符串转成无符号短整型
        qDebug()<<"ip:"<<m_strIP<<" port:"<<m_usPort;
    }
    else
    {
        QMessageBox::critical(this,"open config","open congif failed");
    }

}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TcpClient::loginName()
{
    return m_strLoginName;
}

QString TcpClient::curPath()
{
    return m_strCurPath;

}

void TcpClient::setCurPath(QString strCurPath)
{
    m_strCurPath=strCurPath;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功！");
}

void TcpClient::recvMsg()
{
    //为下载状态,则不接受数据
    if(!OpeWidget::getInstance().getBook()->getDownloadStatus()){
        qDebug()<<m_tcpSocket.bytesAvailable();
        uint uiPDULen=0;
        m_tcpSocket.read((char*)&uiPDULen,sizeof(uint));
        uint uiMsgLen=uiPDULen-sizeof(PDU);
        PDU *pdu=mkPDU(uiMsgLen);
        m_tcpSocket.read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));

        switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_RESPOND:
        {
            if(0==strcmp(pdu->caData,REGIST_OK))//按ASCII码比较，pdu->caData小于REGIST_OK则返回负值，相同返回0，否则正值
            {
                QMessageBox::information(this,"注册",REGIST_OK);
            }else if(0==strcmp(pdu->caData,REGIST_FAILED)){
                QMessageBox::warning(this,"注册",REGIST_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND:
        {
            if(0==strcmp(pdu->caData,LOGIN_OK))
            {
                m_strCurPath=QString("./%1").arg(m_strLoginName);
                QMessageBox::information(this,"登录",LOGIN_OK);
                //登入成功显示操作界面
                OpeWidget::getInstance().show();
                this->hide();
            }else if(0==strcmp(pdu->caData,LOGIN_FAILED)){
                QMessageBox::warning(this,"登录",LOGIN_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
        {
            //循环将名字提取出来 放在好友列表上面
            OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
        {
            if(0==strcmp(SEARCH_USR_NO,pdu->caData))
            {
                QMessageBox::information(this,"搜索",QString("%1 ").arg(OpeWidget::getInstance().getFriend()->m_strSearchName)+SEARCH_USR_NO);
            }
            else if(0==strcmp(SEARCH_USR_ONLINE,pdu->caData))
            {
                QMessageBox::information(this,"搜索",QString("%1 ").arg(OpeWidget::getInstance().getFriend()->m_strSearchName)+SEARCH_USR_ONLINE);
            }
            else if(0==strcmp(SEARCH_USR_OFFLINE,pdu->caData))
            {
                QMessageBox::information(this,"搜索",QString("%1 ").arg(OpeWidget::getInstance().getFriend()->m_strSearchName)+SEARCH_USR_OFFLINE);
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST://服务器那边原样转发过来的
        {
            char caName[32]={'\0'};
            strncpy(caName,pdu->caData+32,32);//后32字节 存的想要添加你的人的名字(原服务器里面自己的名字)
            int ret=QMessageBox::information(this,"添加好友",QString("%1 wanna to add you as friend").arg(caName), QMessageBox::Yes,QMessageBox::No);
            PDU *respdu=mkPDU(0);

            //todo (服务器转发给对方--需要对方的名字--后面32个字节)对方同意后设置friendInfo表他两的关系  并给发送添加请求方回馈
            memcpy(respdu->caData,pdu->caData,64);//两个名字又放进respdu(前32字节为被添加者(即当前客户端)  后32字节为添加者)  --这里为64，两个名字都需要的
            if(ret==QMessageBox::Yes)//同意
            {
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;
            }
            else//不同意
            {
                respdu->uiMsgType=ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            }
            m_tcpSocket.write((char*)respdu,respdu->uiPDULen);//respdu发送给服务器  然后释放
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND://服务器那边的回复
        {
            QMessageBox::information(this,"添加好友",pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE://转发过来的同意添加
        {
            char friendName[32] = {'\0'};//好友的名称(前32位)
            strncpy(friendName, pdu->caData, 32);
            QMessageBox::information(this, "添加好友", QString("添加好友成功！%1 同意了您的好友添加请求").arg(friendName));
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE://转发过来的拒绝添加
        {
            char friendName[32] = {'\0'};//好友的名称
            strncpy(friendName, pdu->caData, 32);
            QMessageBox::information(this, "添加好友", QString("添加好友失败！%1 拒绝了您的好友添加请求").arg(friendName));
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND://刷新好友回复
        {
            OpeWidget::getInstance().getFriend()->updateFriendList(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DEL_FRIEND_RESPOND://删除好友回复
        {
            QMessageBox::information(this, "删除好友", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DEL_FRIEND_REQUEST://转发过来的被删除
        {
            char caName[32] = {'\0'};//删除你的人的名字
            memcpy(caName,pdu->caData,32);
            QMessageBox::information(this, "删除好友", QString("%1删除你作为他的好友").arg(caName));
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST://接受转发过来的私聊请求
        {
            if(PrivateChat::getInstance().isHidden()){
                PrivateChat::getInstance().show();
            }
            char caSendName[32]={'\0'};
            memcpy(caSendName,pdu->caData,32);//这是发送私聊的人的名字(目前客户端是接受私聊的人)
            QString strSendName=caSendName;
            //设置当前客户端的聊天对象为发送信息的人，即strSendName
            PrivateChat::getInstance().setChatName(strSendName);
            PrivateChat::getInstance().updateMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST://接受转发的群聊请求
        {
            OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);//friend界面上显示群聊消息
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND://创建目录回复
        {
            QMessageBox::information(this, "创建文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND://刷新文件回复
        {
            OpeWidget::getInstance().getBook()->updateFileList(pdu);
            QString strEnterDir=OpeWidget::getInstance().getBook()->getEnterDir();
            if(!strEnterDir.isEmpty()){
                m_strCurPath=m_strCurPath+"/"+strEnterDir;//返回的时候给m_strCurPath清空
                qDebug()<<"enter dir:"<<m_strCurPath;
            }
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_RESPOND://删除目录回复
        {
            QMessageBox::information(this, "删除文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND://重命名文件回复
        {
            QMessageBox::information(this, "重命名文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND://进入目录回复
        {
            OpeWidget::getInstance().getBook()->clearEnterDir();//进入一个错的目录(常规文件)，把原来记录的 进入目录清空
            QMessageBox::information(this, "进入目录", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND://上传文件回复
        {
            QMessageBox::information(this, "上传文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_RESPOND://删除常规文件回复
        {
            QMessageBox::information(this, "删除文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND://下载文件回复
        {
            qDebug()<<pdu->caData;//打印要下载的文件名和文件大小

            char caFileName[32]={'\0'};
            sscanf(pdu->caData,"%s %lld",caFileName,&(OpeWidget::getInstance().getBook()->m_iTotal));//取出要下载的文件和文件大小
            if(strlen(caFileName)>0&&OpeWidget::getInstance().getBook()->m_iTotal>0)
            {
                OpeWidget::getInstance().getBook()->setDownloadStatus(true);//设置为下载状态
                m_file.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
                if(!m_file.open(QIODevice::WriteOnly))//只读的方式打开这个文件(没有则会创建)
                {
                    QMessageBox::warning(this, "下载文件", "获得保存文件路径失败");
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND://共享文件回复
        {
            QMessageBox::information(this, "共享文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE://共享文件通知(是否接受对方共享的文件)
        {
            char *pPath=new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);//分享文件的路径拷贝到pPath里
            char *pos=strrchr(pPath,'/');//从右往左找'/'的指针(C语言版)
            if(NULL!=pos)
            {
                pos++;
                //(char* <=>String)
                QString strNote=QString("%1 shared file %2 \nDo you receive?").arg(pdu->caData).arg(pos);
                int ret=QMessageBox::question(this,"共享文件",strNote);
                //接受 则给服务器回馈 让服务器拷贝文件到接收者的目录里
                //不接受则不做处理
                if(QMessageBox::Yes==ret){
                    PDU *respdu=mkPDU(pdu->uiMsgLen);//传共享文件路径大小
                    respdu->uiMsgType=ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                    //拷贝分享文件的路径 和 接受者的名字
                    memcpy(respdu->caMsg,pdu->caMsg,pdu->uiMsgLen);
                    QString strName=TcpClient::getInstance().loginName();
                    strcpy(respdu->caData,strName.toStdString().c_str());
                    m_tcpSocket.write((char*)respdu,respdu->uiPDULen);//pdu发送出去然后释放
                    free(respdu);
                    respdu=NULL;
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND://移动文件回复
        {
            QMessageBox::information(this, "移动文件", pdu->caData);
            break;
        }

        default:
            break;
        }
        free(pdu);
        pdu=NULL;
    }
    else//为真则接受(要下载的文件)数据
    {
        QByteArray buffer= m_tcpSocket.readAll();
        m_file.write(buffer);//把接受到的文件写到要保存的目录下
        Book *pBook=OpeWidget::getInstance().getBook();
        pBook->m_iRecved+=buffer.size();
        if(pBook->m_iTotal==pBook->m_iRecved)//接受成功
        {
            m_file.close();
            pBook->m_iTotal=0;
            pBook->m_iRecved=0;
            pBook->setDownloadStatus(false);
            QMessageBox::information(this,"下载文件","下载文件成功");
        }
        else if(pBook->m_iTotal<pBook->m_iRecved)
        {
            m_file.close();
            pBook->m_iTotal=0;
            pBook->m_iRecved=0;
            pBook->setDownloadStatus(false);
            QMessageBox::critical(this, "下载文件", "下载文件失败");
        }
    }

}


#if 0
//测试
void TcpClient::on_send_pd_clicked()
{
    QString strMsg=ui->lineEdit->text();
    if(!strMsg.isEmpty()){
        PDU *pdu=mkPDU(strMsg.size());//传入实际信息大小
        pdu->uiMsgType=8888;
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());//strcpy也可
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);//发数据
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::warning(this,"信息发送","发送信息不能为空");
    }
}
#endif





//注册
void TcpClient::on_regist_pb_clicked()
{
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();
    if(!strName.isEmpty()&&!strPwd.isEmpty()){
        //将获得的用户名和密码填充到pdu(协议)里面
        PDU *pdu=mkPDU(0);//消息队列不放信息  传0
        pdu->uiMsgType=ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData,strName.toStdString().c_str(),32);//前32字节放用户名
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);//后32字节放密码
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);//pdu发送出去然后释放
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::critical(this,"注册","注册失败:用户名或密码不能为空");
    }
}

//注销
void TcpClient::on_logout_pb_clicked()
{

}

//登录
void TcpClient::on_login_pb_clicked()
{
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();
    if(!strName.isEmpty()&&!strPwd.isEmpty()){
        m_strLoginName=strName;                                //不为空就记录？应该确定登录成功了再记录这个用户名
        //将获得的用户名和密码填充到pdu(协议)里面
        PDU *pdu=mkPDU(0);//消息队列不放信息  传0
        pdu->uiMsgType=ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData,strName.toStdString().c_str(),32);//前32字节放用户名
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);//后32字节放密码
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);//pdu发送出去然后释放
        free(pdu);
        pdu=NULL;
    }else{
        QMessageBox::critical(this,"登录","登录失败:用户名或密码不能为空");
    }
}
