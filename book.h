#ifndef BOOK_H
#define BOOK_H

#include <QWidget>

#include<QLabel>
#include<QListWidget>
#include<QPushButton>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include"protocol.h"
#include<QTimer>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(PDU *pdu);
    QString getEnterDir();
    void clearEnterDir();
    void setDownloadStatus(bool status);
    bool getDownloadStatus();
    QString getSaveFilePath();
    qint64 m_iTotal;//总下载文件大小
    qint64 m_iRecved;//已收到的下载文件大小
    QString getShareFileName();
    
private:
    QListWidget *m_pBookListW;
    QPushButton *m_pReruenPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pUploadPB;
    QPushButton *m_pDownLoadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pShareFilePB;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pSelectDirPB;

    QString m_strEnterDir;
    QString m_strUploadFilePath;

    QTimer *m_pTimer;//发送上传文件请求后 隔一会儿再上传文件  否则会出现粘包的现象

    QString m_strSaveFilePath;
    bool m_bDownload;
    QString m_strShareFileName;

    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strDestDir;
public slots:
    void createDir();
    void flushFile();
    void delDir();
    void renameFile();
    void enterDir(const QModelIndex &index);
    void returnPre();
    void uploadFile();
    void uploadFileData();
    void delRegFile();
    void downloadFile();
    void shareFile();
    void moveFile();
    void selectDesDir();


signals:

};

#endif // BOOK_H
