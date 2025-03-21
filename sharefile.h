#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include<QLabel>
#include<QPushButton>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QButtonGroup>
#include<QScrollArea>
#include<QListWidget>

#include"protocol.h"


class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);
    static ShareFile &getInstance();
    void updateFriend(QListWidget *pFriendList);
    void test();

private:
    QPushButton *m_pSelectAllPB;
    QPushButton *m_pCancelSelectPB;

    QPushButton *m_pOKPB;
    QPushButton *m_pCancelPB;

    QScrollArea *m_pSA;
    QWidget *m_pFriendW;
    QButtonGroup *m_pButtonGroup;
    QVBoxLayout *m_pFriendWVBL;


public slots:
    void selectAll();
    void cancelSelect();
    
    void okShare();
    void cancelShare();
    
signals:

};

#endif // SHAREFILE_H
