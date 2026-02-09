#ifndef ENCRYPTIONMANAGER_H
#define ENCRYPTIONMANAGER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QtCrypto>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

class EncryptionManager : public QObject
{
    Q_OBJECT

public:
    explicit EncryptionManager(QObject *parent = nullptr);
    ~EncryptionManager();

    // Generate key pair for user
    QString generatePublicKey();
    QString generatePrivateKey();

    // Encrypt/decrypt functions
    QByteArray encryptMessage(const QByteArray &message, const QString &recipientPublicKey);
    QByteArray decryptMessage(const QByteArray &encryptedMessage, const QString &privateKey);

    // Symmetric encryption for local storage
    QByteArray encryptLocal(const QByteArray &data, const QString &key);
    QByteArray decryptLocal(const QByteArray &encryptedData, const QString &key);

    // Hash functions
    QString hashPassword(const QString &password);
    QString hashData(const QByteArray &data);

    // Verify signatures
    bool verifySignature(const QByteArray &data, const QString &signature, const QString &publicKey);

private:
    // Helper methods
    QByteArray deriveKeyFromPassword(const QString &password, const QByteArray &salt);
    QByteArray generateSalt();
    
    // Store our keys
    QString privateKey;
    QString publicKey;
};

#endif // ENCRYPTIONMANAGER_H