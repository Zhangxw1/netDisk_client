#include "tcpclient.h"
#include <QApplication>

#include "sharefile.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    TcpClient w;
//    w.show();

    TcpClient::getInstance().show();

//    ShareFile w;
//    w.test();
//    w.show();


    return a.exec();
}
