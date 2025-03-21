#include "sharefile.h"
#include<QCheckBox>
#include<QDebug>
#include"tcpclient.h"
#include"opewidget.h"


ShareFile::ShareFile(QWidget *parent) : QWidget(parent)
{
    this->resize(400,220);

    m_pSelectAllPB=new QPushButton("全选");
    m_pCancelSelectPB=new QPushButton("取消全选");

    m_pOKPB=new QPushButton("确定");
    m_pCancelPB=new QPushButton("取消");

    m_pSA=new QScrollArea;
    m_pFriendW=new QWidget;
    m_pFriendWVBL=new QVBoxLayout(m_pFriendW);//一个窗口不能设置多个布局
    m_pButtonGroup=new QButtonGroup(m_pFriendW);//m_pButtonGroup在m_pFriendW里面
    m_pButtonGroup->setExclusive(false);//设置ButtonGroup可以多选

    QHBoxLayout *pTopHBL=new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch();//在水平布局右边加一个弹簧

    QHBoxLayout *pDownHBL=new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCancelPB);

    QVBoxLayout *pMain=new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addWidget(m_pSA);
    pMain->addLayout(pDownHBL);

    setLayout(pMain);//pMain这个垂直布局把所有内容都攘括进去了

//    test();

    connect(m_pSelectAllPB,&QPushButton::clicked,this,&ShareFile::selectAll);
    connect(m_pCancelSelectPB,&QPushButton::clicked,this,&ShareFile::cancelSelect);
    
    connect(m_pOKPB,&QPushButton::clicked,this,&ShareFile::okShare);
    connect(m_pCancelPB,&QPushButton::clicked,this,&ShareFile::cancelShare);
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;//不管调用多少次返回的都是这个静态对象的引用
}


//QScrollArea里面是一个Widget组件(满铺)，组件里有一个垂直布局和按钮组(用来管理按钮)，垂直布局里放入所有按钮，按钮组也放入所有按钮
void ShareFile::test()
{
    QVBoxLayout *p=new QVBoxLayout(m_pFriendW);
    QCheckBox *pCB=nullptr;
    for(int i=0;i<10;i++){
        pCB=new QCheckBox(QString("jack%1").arg(i+1));
        p->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);//需要通过 QButtonGroup 只是管理这些按钮 或利用其功能(QCheckBox需要单独放到布局里)
    }
    m_pSA->setWidget(m_pFriendW);
}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList= m_pButtonGroup->buttons();
    for(int i=0;i<cbList.size();i++){
        if(!cbList[i]->isChecked()){
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList= m_pButtonGroup->buttons();
    for(int i=0;i<cbList.size();i++){
        if(cbList[i]->isChecked()){
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::okShare()
{
    QString strName=TcpClient::getInstance().loginName();
    QString strCurPath=TcpClient::getInstance().curPath();
    QString strShareFileName=OpeWidget::getInstance().getBook()->getShareFileName();
    QString strPath=strCurPath+"/"+strShareFileName;//组合成 文件路径
    //计算要分享的人数
    QList<QAbstractButton*> cbList= m_pButtonGroup->buttons();
    int num=0;
    for(int i=0;i<cbList.size();i++){
        if(cbList[i]->isChecked()){
            num++;
        }
    }
    PDU *pdu=mkPDU(32*num+strPath.size()+1);//提前指定caMsg大小，好友名字和文件路径放在caMsg里面
    pdu->uiMsgType=ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData,"%s %d",strName.toStdString().c_str(),num);                                                       //分享者和要分享的人数 放caData
    int j=0;
    for(int i=0;i<cbList.size();i++){
        if(cbList[i]->isChecked()){
            memcpy((char*)(pdu->caMsg)+j*32,cbList[i]->text().toStdString().c_str(),cbList[i]->text().size());           //拷贝要分享的好友名字 到caMsg
            j++;
        }
    }
    memcpy((char*)(pdu->caMsg)+num*32,strPath.toStdString().c_str(),strPath.size());                                     //先偏移，再拷贝要分享文件的路径 到caMsg
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=nullptr;
}

void ShareFile::cancelShare()
{
    this->hide();
}


//先删除再添加以达到刷新的效果
void ShareFile::updateFriend(QListWidget *pFriendList)
{
    if(NULL==pFriendList){
        return;
    }
    //移除之前分享过的好友
    QAbstractButton* tmp=NULL;
    QList<QAbstractButton*> preFriendList= m_pButtonGroup->buttons();//获取之前所有可选择的好友
    //布局、按钮组和preFriendList均删除
    for(int i=0;i<preFriendList.size();i++){
        tmp=preFriendList[i];
        m_pFriendWVBL->removeWidget(tmp);
        m_pButtonGroup->removeButton(tmp);
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp=nullptr;
    }
    //向框中加内容
    QCheckBox *pCB=nullptr;
    for(int i=0;i<pFriendList->count();i++){
        qDebug() << "好友的名称："<< pFriendList->item(i)->text();
        pCB=new QCheckBox(pFriendList->item(i)->text());
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);//需要通过 QButtonGroup 管理这些按钮或利用其功能
    }
    m_pSA->setWidget(m_pFriendW);
}









