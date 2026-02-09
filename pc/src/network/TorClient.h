#ifndef TORCLIENT_H
#define TORCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QSslSocket>
#include <QTcpServer>
#include <QNetworkProxy>
#include "core/EncryptionManager.h"

class TorClient : public QObject
{
    Q_OBJECT

public:
    explicit TorClient(QObject *parent = nullptr);
    ~TorClient();

    bool isConnected() const;
    void connectToTor();
    void disconnectFromTor();

signals:
    void connected();
    void disconnected();
    void connectionError(const QString &error);
    void dataReceived(const QByteArray &data);

public slots:
    void sendData(const QByteArray &data);

private slots:
    void handleTorConnection();
    void handleTorDisconnection();
    void handleTorError();
    void handleIncomingData();

private:
    void setupTorProxy();
    void establishOnionService();
    void configureTorSettings();
    QByteArray createOnionPacket(const QByteArray &payload, const QString &destination);
    QByteArray generateRandomPadding(int size);
    
    QSslSocket *torSocket;
    QTcpServer *onionService;
    QTimer *reconnectTimer;
    QString torProxyHost;
    quint16 torProxyPort;
    bool connectedToTor;
    EncryptionManager *encryptionMgr;
    QString targetPublicKey;      // Public key of the recipient for end-to-end encryption
    QString destinationAddress;   // Onion address of the destination
};

#endif // TORCLIENT_H