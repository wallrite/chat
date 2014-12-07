#include "server.h"

#include <QtWidgets>
#include <QtNetwork>

Server::Server(QWidget *parent) :
    QDialog(parent), m_pTCPServer(0), m_pNetworkSession(0)
{
    m_pStatusLabel = new QLabel;
    m_pQuitButton = new QPushButton( tr( "Quit" ) );
    m_pQuitButton->setAutoDefault( false );

    QNetworkConfigurationManager networkManager;
    if( networkManager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired )
    {
        //Get saved network configurations
        QSettings settings( QSettings::UserScope, QLatin1String( "QtProject" ) );
        settings.beginGroup( "QtNetwork" );
        const QString id = settings.value( QLatin1String( "DefaultNetworkConfiguration") ).toString();
        settings.endGroup();

        QNetworkConfiguration networkConfiguration = networkManager.configurationFromIdentifier( id );
        if ( (networkConfiguration.state() & QNetworkConfiguration::Discovered ) !=
             QNetworkConfiguration::Discovered )
        {
            networkConfiguration = networkManager.defaultConfiguration();
        }

        m_pNetworkSession = new QNetworkSession( networkConfiguration, this );
        connect( m_pNetworkSession, SIGNAL( opened() ), this, SLOT( sesionOpened() ) );

        m_pStatusLabel->setText( tr( "Opening network session!" ) );
        m_pNetworkSession->open();
    }
    else
    {
        sesionOpened();
    }

    m_Fortunes      << tr("You've been leading a dog's life. Stay off the furniture.")
                    << tr("You've got to think about tomorrow.")
                    << tr("You will be surprised by a loud noise.")
                    << tr("You will feel hungry again in another hour.")
                    << tr("You might have mail.")
                    << tr("You cannot kill time without injuring eternity.")
                    << tr("Computers are not intelligent. They only think they are.");

    connect( m_pQuitButton, SIGNAL( clicked() ), this, SLOT( close() ) );
    connect( m_pTCPServer, SIGNAL( newConnection() ), this, SLOT( sendFortune() ) );

    QHBoxLayout *pButtonLayout = new QHBoxLayout;
    pButtonLayout->addStretch( 1 );
    pButtonLayout->addWidget( m_pQuitButton );
    pButtonLayout->addStretch( 1 );

    QVBoxLayout *pMainlayout = new QVBoxLayout;
    pMainlayout->addWidget( m_pStatusLabel );
    pMainlayout->addLayout( pButtonLayout );
    setLayout( pMainlayout );

    setWindowTitle( tr( "Fortune Server " ) );


}

void Server::sesionOpened()
{

    //save the used configurations
    if ( m_pNetworkSession )
    {
        QNetworkConfiguration networkConfig = m_pNetworkSession->configuration();
        QString id;

        if ( networkConfig.type() == QNetworkConfiguration::UserChoice )
        {
            id = m_pNetworkSession->sessionProperty( QLatin1String( "UserChoiceConfiguration")).toString();
        }
        else
        {
            id = networkConfig.identifier();
        }

        QSettings settings( QSettings::UserScope, QLatin1String( "QtProject" ) );
        settings.setValue( QLatin1String( "QtNetwork/DefaultNetworkConfiguration" ), id );
    }

    m_pTCPServer = new QTcpServer(this);

    if( !m_pTCPServer->listen() )
    {
        QMessageBox::critical( this, tr( "Fortune Server"),
                               tr( "Unable to start server: %1." )
                               .arg( m_pTCPServer->errorString() ) );
        close();
        return;
    }
    QString ipAddress;
    QList<QHostAddress> ipAddressList = QNetworkInterface::allAddresses();
    //use first non-localhost IPv4 address
    for (int i = 0; i < ipAddressList.size(); ++i)
    {
        if( ipAddressList.at( i ) != QHostAddress::LocalHost &&
                ipAddressList.at( i ).toIPv4Address() )
        {
            ipAddress = ipAddressList.at( i ).toString();
            break;
        }
    }
    //If we did not find one, use IPv4 localgost
    if ( ipAddress.isEmpty() )
    {
        ipAddress = QHostAddress( QHostAddress::LocalHost ).toString();
    }

    m_pStatusLabel->setText( tr( "The server is running on\n\nIP: %1\nPort: %2\n\n")
                             .arg( ipAddress ).arg( m_pTCPServer->serverPort() ) );

}


void Server::sendFortune()
{
    QByteArray block;
    QDataStream out( &block, QIODevice::WriteOnly );
    out.setVersion( QDataStream::Qt_4_0 );
    out << (quint16)0;
    out << m_Fortunes.at( qrand() % m_Fortunes.size() );
    out.device()->seek( 0 );
    out << (quint16)( block.size() - sizeof( quint16 ) );

    QTcpSocket *pClientConnection = m_pTCPServer->nextPendingConnection();
    connect( pClientConnection, SIGNAL( disconnected() ), pClientConnection, SLOT( deleteLater() ) );

    pClientConnection->write( block );
    pClientConnection->disconnectFromHost();
}
