#include "TorClient.h"
#include <QTimer>
#include <QThread>
#include <QCoreApplication>
#include <QSslConfiguration>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QtConcurrent/QtConcurrent>

TorClient::TorClient(QObject *parent)
    : QObject(parent)
    , torSocket(new QSslSocket(this))
    , onionService(new QTcpServer(this))
    , reconnectTimer(new QTimer(this))
    , torProxyHost("127.0.0.1")
    , torProxyPort(9050)
    , connectedToTor(false)
    , encryptionMgr(new EncryptionManager(this))
{
    setupTorProxy();
    configureTorSettings();
    
    connect(reconnectTimer, &QTimer::timeout, this, &TorClient::connectToTor);
    connect(torSocket, &QSslSocket::connected, this, &TorClient::handleTorConnection);
    connect(torSocket, &QSslSocket::disconnected, this, &TorClient::handleTorDisconnection);
    connect(torSocket, QOverload<QAbstractSocket::SocketError>::of(&QSslSocket::error),
            this, &TorClient::handleTorError);
    connect(torSocket, &QSslSocket::readyRead, this, &TorClient::handleIncomingData);
}

TorClient::~TorClient()
{
    disconnectFromTor();
}

bool TorClient::isConnected() const
{
    return connectedToTor;
}

void TorClient::connectToTor()
{
    // Check if Tor is available
    if (torSocket->state() != QAbstractSocket::UnconnectedState) {
        return;
    }
    
    // Attempt to connect to Tor proxy with proper error handling
    connect(torSocket, &QSslSocket::encrypted, this, [this]() {
        qDebug() << "Tor connection encrypted successfully";
        connectedToTor = true;
        
        // Establish onion service for receiving messages
        establishOnionService();
        
        emit connected();
    });
    
    // Configure socket for Tor connection
    torSocket->setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, torProxyHost, torProxyPort));
    
    // Connect to Tor proxy
    torSocket->connectToHostEncrypted("127.0.0.1", torProxyPort);
    
    // Wait for connection (with timeout)
    if (!torSocket->waitForEncrypted(10000)) {
        qDebug() << "Tor connection failed: " << torSocket->errorString();
        // Retry after delay
        QTimer::singleShot(5000, this, &TorClient::connectToTor);
        emit connectionError("Failed to connect to Tor: " + torSocket->errorString());
        return;
    }
}

void TorClient::disconnectFromTor()
{
    connectedToTor = false;
    torSocket->disconnectFromHost();
    if (torSocket->state() != QAbstractSocket::UnconnectedState) {
        torSocket->waitForDisconnected(1000);
    }
    emit disconnected();
}

void TorClient::sendData(const QByteArray &data)
{
    if (!connectedToTor) {
        emit connectionError("Not connected to Tor");
        return;
    }
    
    // Encrypt data before sending using proper end-to-end encryption
    QByteArray encryptedData = encryptionMgr->encryptMessage(data, targetPublicKey);
    
    // Create onion routing packet with destination information
    QByteArray onionPacket = createOnionPacket(encryptedData, destinationAddress);
    
    // Send through Tor connection
    torSocket->write(onionPacket);
    torSocket->waitForBytesWritten(5000); // Wait up to 5 seconds for write to complete
}

QByteArray TorClient::createOnionPacket(const QByteArray &payload, const QString &destination)
{
    // Create onion packet structure for routing through Tor network
    QByteArray packet;
    
    // Add destination address (onion address)
    QByteArray destBytes = destination.toUtf8();
    packet.append(static_cast<char>(destBytes.size()));
    packet.append(destBytes);
    
    // Add payload
    quint32 payloadSize = static_cast<quint32>(payload.size());
    packet.append(reinterpret_cast<const char*>(&payloadSize), sizeof(payloadSize));
    packet.append(payload);
    
    // Add padding to standardize packet size and prevent traffic analysis
    int minPacketSize = 1024; // Minimum packet size in bytes
    if (packet.size() < minPacketSize) {
        int paddingNeeded = minPacketSize - packet.size();
        QByteArray padding = generateRandomPadding(paddingNeeded);
        packet.append(padding);
    }
    
    return packet;
}

QByteArray TorClient::generateRandomPadding(int size)
{
    QByteArray padding(size, 0);
    for (int i = 0; i < size; ++i) {
        padding[i] = static_cast<char>(qrand() % 256);
    }
    return padding;
}

void TorClient::handleTorConnection()
{
    connectedToTor = true;
    
    // Establish onion service for receiving messages
    establishOnionService();
    
    emit connected();
}

void TorClient::handleTorDisconnection()
{
    connectedToTor = false;
    emit disconnected();
    
    // Schedule reconnection
    reconnectTimer->start(10000); // Reconnect after 10 seconds
}

void TorClient::handleTorError()
{
    connectedToTor = false;
    emit connectionError(torSocket->errorString());
    
    // Schedule reconnection
    reconnectTimer->start(10000); // Reconnect after 10 seconds
}

void TorClient::handleIncomingData()
{
    QByteArray data = torSocket->readAll();
    
    // Decrypt data
    QByteArray decryptedData = encryptionMgr->decryptLocal(data, "temporary_key_for_demo");
    
    emit dataReceived(decryptedData);
}

void TorClient::setupTorProxy()
{
    // Configure Tor proxy settings
    // In a real implementation, this would set up the connection to the Tor daemon
}

void TorClient::establishOnionService()
{
    // In a real implementation, this would create an onion service for receiving messages
    // For now, we'll just simulate this functionality
}

void TorClient::configureTorSettings()
{
    // Configure SSL settings for Tor connection
    QSslConfiguration sslConfig = torSocket->sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    torSocket->setSslConfiguration(sslConfig);
    
    // Set up reconnection timer
    reconnectTimer->setSingleShot(true);
}