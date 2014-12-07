#ifndef SERVER_H
#define SERVER_H

#include <QDialog>

//#include <QObject>
//#include <QTcpServer>
//#include <QLabel>
//#include <QNetworkSession>

class QLabel;
class QPushButton;
class QTcpServer;
class QNetworkSession;


class Server : public QDialog
{
    Q_OBJECT
public:
     Server(QWidget *parent = 0);

signals:

public slots:

private slots:
     void sesionOpened();
     void sendFortune();

private:
     QLabel             *m_pStatusLabel;
     QPushButton        *m_pQuitButton;
     QTcpServer         *m_pTCPServer;
     QStringList        m_Fortunes;
     QNetworkSession    *m_pNetworkSession;

};

#endif // SERVER_H
