#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include<QFile>
#include<QTcpSocket>
#include"protocol.h"
#include"opewidget.h"


QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();

    void loadConfig();
    static TcpClient &getInstance();
    QTcpSocket &getTcpSocket();
    QString loginName();
    QString curPath();
    void setCurPath(QString strCurPath);


private:
    Ui::TcpClient *ui;
    QString m_strIP;//存放ip
    quint16 m_usPort;//无符号短整型 来存放端口号

    QTcpSocket m_tcpSocket;//通过这个socket对象来连接服务器，和服务器数据交互

    QString m_strLoginName;
    QString m_strCurPath;
    QFile m_file;


public slots:
    void showConnect();
    void recvMsg();


private slots:
//    void on_send_pd_clicked();
    void on_regist_pb_clicked();
    void on_logout_pb_clicked();
    void on_login_pb_clicked();
};
#endif // TCPCLIENT_H
