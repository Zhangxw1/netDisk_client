#include "book.h"
#include"protocol.h"
#include"tcpclient.h"
#include<QInputDialog>
#include<QMessageBox>
#include<QFileDialog>
#include "opewidget.h"
#include"sharefile.h"

Book::Book(QWidget *parent) : QWidget(parent),m_bDownload(false)
{
    m_strEnterDir.clear();//将m_strEnterDir变为""
    m_pTimer=new QTimer;

    m_pBookListW=new QListWidget;
    m_pReruenPB=new QPushButton("返回上一级目录");
    m_pCreateDirPB=new QPushButton("创建文件夹");
    m_pDelDirPB=new QPushButton("删除文件夹");
    m_pRenamePB=new QPushButton("重命名文件");
    m_pFlushFilePB=new QPushButton("刷新文件");

    m_pUploadPB=new QPushButton("上传文件");
    m_pDownLoadPB=new QPushButton("下载文件");
    m_pDelFilePB=new QPushButton("删除文件");
    m_pShareFilePB=new QPushButton("共享文件");
    m_pMoveFilePB=new QPushButton("移动文件");
    m_pSelectDirPB=new QPushButton("目标目录文件");
    m_pSelectDirPB->setEnabled(false);

    QVBoxLayout *pDirVBL=new QVBoxLayout;
    pDirVBL->addWidget(m_pReruenPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    QVBoxLayout *pFileVBL=new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout *pMain=new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);//pMain设置为窗口总的布局(因为这个水平布局把所有内容都攘括进去了)



    connect(m_pReruenPB,&QPushButton::clicked,this,&Book::returnPre);
    connect(m_pCreateDirPB,&QPushButton::clicked,this,&Book::createDir);
    connect(m_pDelDirPB,&QPushButton::clicked,this,&Book::delDir);
    connect(m_pRenamePB,&QPushButton::clicked,this,&Book::renameFile);
    connect(m_pFlushFilePB,&QPushButton::clicked,this,&Book::flushFile);

    connect(m_pBookListW,&QListWidget::doubleClicked,this,&Book::enterDir);//void doubleClicked(const QModelIndex &index); 槽函数的形参要和信号参数一致

    connect(m_pUploadPB,&QPushButton::clicked,this,&Book::uploadFile);
    connect(m_pTimer,&QTimer::timeout,this,&Book::uploadFileData);
    connect(m_pDelFilePB,&QPushButton::clicked,this,&Book::delRegFile);
    connect(m_pDownLoadPB,&QPushButton::clicked,this,&Book::downloadFile);
    connect(m_pShareFilePB,&QPushButton::clicked,this,&Book::shareFile);
    connect(m_pMoveFilePB,&QPushButton::clicked,this,&Book::moveFile);
    connect(m_pSelectDirPB,&QPushButton::clicked,this,&Book::selectDesDir);

}

void Book::updateFileList(PDU *pdu)
{
    if(nullptr==pdu){
        return;
    }
    m_pBookListW->clear();
    FileInfo *pFileInfo=nullptr;
    int iCount=pdu->uiMsgLen/sizeof(FileInfo);
    for(int i=0;i<iCount;i++){
        pFileInfo=(FileInfo*)(pdu->caMsg)+i;
        qDebug()<<pFileInfo->caFileName<<pFileInfo->iFileType;
        QListWidgetItem *pItem=new QListWidgetItem;
        if(0==pFileInfo->iFileType){//目录
            pItem->setIcon(QIcon(QPixmap(":/dir.png")));
        }else if(1==pFileInfo->iFileType){//文件
            pItem->setIcon(QIcon(QPixmap(":/file.png")));
        }
        pItem->setText(pFileInfo->caFileName);
        m_pBookListW->addItem(pItem);
    }
}

QString Book::getEnterDir()
{
      return m_strEnterDir;
}

void Book::createDir()
{
    QString strNewDir=QInputDialog::getText(this,"新建文件夹","新建文件夹名字");//getText为静态成员函数,返回新建文件夹名字
    if(!strNewDir.isEmpty())
    {
        if(strNewDir.size()>32){
            QMessageBox::warning(this,"新建文件夹","新文件夹名长度不能超过32");
        }else{
            //传1、用户名 2、新的目录名   放在caData里,各占32字节
            //3、当前所在目录   放在caMsg
            QString strName=TcpClient::getInstance().loginName();
            QString strCurPath=TcpClient::getInstance().curPath();
            PDU *pdu=mkPDU(strCurPath.size()+1);
            pdu->uiMsgType=ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData,strName.toStdString().c_str(),strName.size());//拷贝名字
            strncpy(pdu->caData+32,strNewDir.toStdString().c_str(),strNewDir.size());//拷贝新的目录名
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());//拷贝当前所在目录
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu=nullptr;
        }
    }else{
        QMessageBox::warning(this,"新建文件夹","新文件夹名不能为空");
    }
}

void Book::flushFile()
{
    QString strCurPath=TcpClient::getInstance().curPath();
    //免得子目录很多，路径长度超过64，直接放在caMsg里面
    PDU *pdu=mkPDU(strCurPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());//char *strncpy(char *Dest,const char *Source,size_t);
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=nullptr;

}

void Book::delDir()
{
    QString strCurPath=TcpClient::getInstance().curPath();
    QListWidgetItem *pItem=m_pBookListW->currentItem();
    if(nullptr==pItem){
        QMessageBox::warning(this,"删除文件","请选择您要删除的文件");
    }else{
        QString strDelName=pItem->text();
        PDU *pdu=mkPDU(strCurPath.size()+1);//传caMsg大小
        pdu->uiMsgType=ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;
    }
}

void Book::renameFile()
{
    QString strCurPath=TcpClient::getInstance().curPath();
    QListWidgetItem *pItem=m_pBookListW->currentItem();
    if(nullptr==pItem){
        QMessageBox::warning(this,"重命名文件","请选择您要重命名的文件");
    }else{
        QString strOldName=pItem->text();
        QString strNewName=QInputDialog::getText(this,"重命名文件","请输入新的文件名");
        if(!strNewName.isEmpty()){
            PDU *pdu=mkPDU(strCurPath.size()+1);//传caMsg大小
            pdu->uiMsgType=ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData,strOldName.toStdString().c_str(),strOldName.size());
            strncpy(pdu->caData+32,strNewName.toStdString().c_str(),strNewName.size());
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu=nullptr;
        }else{
            QMessageBox::warning(this,"重命名文件","输入的文件名不能为空");
        }
    }
}

void Book::enterDir(const QModelIndex &index)//index获得双击选项内容
{
    QString strDirName=index.data().toString();//QVariant转换成字符串
//    qDebug()<<strDirName;
    m_strEnterDir=strDirName;//保存要进入的目录
    QString strCurPath=TcpClient::getInstance().curPath();
    PDU *pdu=mkPDU(strCurPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData,strDirName.toStdString().c_str(),strDirName.size());
    memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=nullptr;
}

void Book::returnPre()
{
    QString strCurPath=TcpClient::getInstance().curPath();
    QString strRootPath="./"+TcpClient::getInstance().loginName();//获取顶层目录，若为顶层目录则不然返回
    if(strCurPath==strRootPath){
        QMessageBox::warning(this,"返回上一级","返回失败:已经在最开始的的目录中");
    }
    else
    {//  "./aa/bb/cc" -> "./aa/bb"     这里index为7
        int index=strCurPath.lastIndexOf('/');//从后往前找/的位置索引
        strCurPath.remove(index,strCurPath.size()-index);//从index位置开始，移除10-7=3个字节大小
        TcpClient::getInstance().setCurPath(strCurPath);//设置新的当前路径
        clearEnterDir();//刷新前要清空，因为刷新回复有一个拼接路径的逻辑，不清除则会重复拼接
        flushFile();//调用刷新文件函数(刷新为当前路径--strCurPath的内容)
    }
}

void Book::uploadFile()
{
    m_strUploadFilePath=QFileDialog::getOpenFileName();//弹出打开文件的窗口(返回打开文件的绝对路径)  eg."D:/1/2/选取的文件"

    if(!m_strUploadFilePath.isEmpty()){
        int index=m_strUploadFilePath.lastIndexOf('/'); //  "D:/1/2/选取的文件"  index=6   总长12
        QString strFileName=m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);//截取文件名 从右往左截取12-6-1=5个字节大小          //与206行效果差不多

        QFile file(m_strUploadFilePath);
        qint64 fileSize=file.size();//文件字节大小
        QString strCurPath=TcpClient::getInstance().curPath();
        PDU *pdu=mkPDU(strCurPath.size()+1);//传入当前路径长度+1
        pdu->uiMsgType=ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        sprintf(pdu->caData,"%s %lld",strFileName.toStdString().c_str(),fileSize);//文件名和文件的字节大小caData里

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;

        //启动定时器
        m_pTimer->start(1000);//1s后在上传数据文件，直接传会粘包

    }else{
        QMessageBox::warning(this,"上传文件","上传失败:文件名不能为空");
    }
}
//读数据给服务器，服务器再写入对应客户端的文件里面
void Book::uploadFileData()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"上传文件","打开文件失败");
        return;
    }

    char *pBuffer=new char[4096];
    qint64 ret=0;
    //循环读取文件内容发送给服务器
    while(1){
        ret=file.read(pBuffer,4096);//ret存储实际读到的字节数
        if(ret>0&&ret<=4096){
            TcpClient::getInstance().getTcpSocket().write(pBuffer,ret);//独到的数据发送给服务器
        }
        else if(0==ret)//读到文件末尾 没有读到数据
        {
            break;
        }else{
            QMessageBox::warning(this,"上传文件","上传文件失败:读取文件失败");
            break;
        }
    }
    file.close();
    delete []pBuffer;
    pBuffer=nullptr;
}

void Book::delRegFile()
{
    QString strCurPath=TcpClient::getInstance().curPath();
    QListWidgetItem *pItem=m_pBookListW->currentItem();
    if(nullptr==pItem){
        QMessageBox::warning(this,"删除文件","请选择您要删除的文件");
    }else{
        QString strDelName=pItem->text();
        PDU *pdu=mkPDU(strCurPath.size()+1);//传caMsg大小
        pdu->uiMsgType=ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;
    }
}

void Book::downloadFile()
{
    QListWidgetItem *pItem=m_pBookListW->currentItem();
    QString strCurPath=TcpClient::getInstance().curPath();
    if(nullptr==pItem){
        QMessageBox::warning(this,"下载文件","请选择您要下载的文件");
    }
    else
    { 
        //指定文件存放的位置
        QString strSaveFilePath=QFileDialog::getSaveFileName();//getSaveFileName()会弹出窗口让你选择保存的文件路径(默认打开当前工作目录）)
        if(!strSaveFilePath.isEmpty()){
            m_strSaveFilePath=strSaveFilePath;
        }else{
            QMessageBox::warning(this,"下载文件","请选择您要保存的位置");
            m_strSaveFilePath.clear();
            return;
        }
        
        PDU *pdu=mkPDU(strCurPath.size()+1);
        pdu->uiMsgType=ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        QString strFileName=pItem->text();
        strncpy(pdu->caData,strFileName.toStdString().c_str(),strFileName.size());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);

        free(pdu);
        pdu=nullptr;
    }
}

void Book::shareFile()
{
    QListWidgetItem *pItem=m_pBookListW->currentItem();
    QString strCurPath=TcpClient::getInstance().curPath();
    if(nullptr==pItem){
        QMessageBox::warning(this,"分享文件","请选择您要分享的文件");
        return;
    }
    else
    { 
        m_strShareFileName=pItem->text();
    }
    
    Friend *pFriend=OpeWidget::getInstance().getFriend();
    QListWidget *pFriendList=pFriend->getFriendList();
//    for(int i=0;i<pFriendList->count();i++){
//        qDebug()<<pFriendList->item(i)->text();
//    }
    ShareFile::getInstance().updateFriend(pFriendList);
    if(ShareFile::getInstance().isHidden()){
        ShareFile::getInstance().show();
    }
}


void Book::moveFile()
{
    QListWidgetItem *pCurItem=m_pBookListW->currentItem();
    if(NULL!=pCurItem){
        m_strMoveFileName=pCurItem->text();
        QString strCurPath=TcpClient::getInstance().curPath();
        m_strMoveFilePath=strCurPath+'/'+m_strMoveFileName;//要移动文件的当前路径

        m_pSelectDirPB->setEnabled(true);
        QMessageBox::information(this, "移动文件", "请选择要移动的目的文件夹");
        return;
    }else{
        QMessageBox::warning(this,"移动文件","请选择您要移动的文件");
        return;
    }
}

void Book::selectDesDir()
{
    QListWidgetItem *pCurItem=m_pBookListW->currentItem();
    if(NULL!=pCurItem){
        QString strDestDir=pCurItem->text();
        QString strCurPath=TcpClient::getInstance().curPath();
        m_strDestDir=strCurPath+'/'+strDestDir;//移动到的目的目录

        //原路径(带文件名)和目的路径放在caMsg  文件名和两个路径大小放在caData里
        int srcLen=m_strMoveFilePath.size();
        int destLen=m_strDestDir.size();
        PDU *pdu=mkPDU(srcLen+destLen+2);
        pdu->uiMsgType=ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData,"%d %d %s",srcLen,destLen,m_strMoveFileName.toStdString().c_str());
        memcpy(pdu->caMsg,m_strMoveFilePath.toStdString().c_str(),srcLen);
        memcpy((char*)(pdu->caMsg)+(srcLen+1),m_strDestDir.toStdString().c_str(),destLen);
        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=nullptr;
    }else{
        QMessageBox::warning(this,"移动文件","请选择您要移动的目的文件夹");
        return;
    }
    m_pSelectDirPB->setEnabled(false);

}




void Book::clearEnterDir()
{
    m_strEnterDir.clear();
}

void Book::setDownloadStatus(bool status)
{
    m_bDownload=status;
}

bool Book::getDownloadStatus()
{
    return m_bDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}





