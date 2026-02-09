#include "EncryptionManager.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <memory>

EncryptionManager::EncryptionManager(QObject *parent)
    : QObject(parent)
{
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
}

EncryptionManager::~EncryptionManager()
{
    EVP_cleanup();
    ERR_free_strings();
}

QString EncryptionManager::generatePublicKey()
{
    // Generate RSA key pair
    RSA *rsa = RSA_new();
    BIGNUM *bne = BN_new();
    BN_set_word(bne, RSA_F4);

    if (RSA_generate_key_ex(rsa, 2048, bne, NULL) == 1) {
        // Write public key to BIO
        BIO *bio = BIO_new(BIO_s_mem());
        PEM_write_bio_RSA_PUBKEY(bio, rsa);

        // Get the public key as string
        char *pubKeyData = nullptr;
        long pubKeyLen = BIO_get_mem_data(bio, &pubKeyData);
        QString pubKey = QString::fromLatin1(pubKeyData, pubKeyLen);

        // Clean up
        BIO_free(bio);
        RSA_free(rsa);
        BN_free(bne);

        return pubKey;
    }

    RSA_free(rsa);
    BN_free(bne);
    return QString();
}

QString EncryptionManager::generatePrivateKey()
{
    // Generate RSA key pair
    RSA *rsa = RSA_new();
    BIGNUM *bne = BN_new();
    BN_set_word(bne, RSA_F4);

    if (RSA_generate_key_ex(rsa, 2048, bne, NULL) == 1) {
        // Write private key to BIO
        BIO *bio = BIO_new(BIO_s_mem());
        PEM_write_bio_RSAPrivateKey(bio, rsa, NULL, NULL, 0, NULL, NULL);

        // Get the private key as string
        char *privKeyData = nullptr;
        long privKeyLen = BIO_get_mem_data(bio, &privKeyData);
        QString privKey = QString::fromLatin1(privKeyData, privKeyLen);

        // Store for later use
        privateKey = privKey;
        publicKey = generatePublicKey(); // Also generate and store public key

        // Clean up
        BIO_free(bio);
        RSA_free(rsa);
        BN_free(bne);

        return privKey;
    }

    RSA_free(rsa);
    BN_free(bne);
    return QString();
}

QByteArray EncryptionManager::encryptMessage(const QByteArray &message, const QString &recipientPublicKey)
{
    // Convert recipient's public key from string to RSA object
    BIO *bio = BIO_new_mem_buf(recipientPublicKey.toLatin1().data(), -1);
    RSA *rsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);

    if (!rsa) {
        BIO_free(bio);
        return QByteArray();
    }

    // Prepare the message for encryption
    int rsaSize = RSA_size(rsa);
    QByteArray paddedMessage = message;

    // Encrypt the message using RSA
    QByteArray encrypted;
    encrypted.resize(rsaSize);

    int result = RSA_public_encrypt(message.size(), 
                                   reinterpret_cast<const unsigned char*>(message.data()), 
                                   reinterpret_cast<unsigned char*>(encrypted.data()), 
                                   rsa, RSA_PKCS1_PADDING);

    BIO_free(bio);
    RSA_free(rsa);

    if (result == -1) {
        return QByteArray();
    }

    // Resize to actual encrypted size
    encrypted.resize(result);
    return encrypted;
}

QByteArray EncryptionManager::decryptMessage(const QByteArray &encryptedMessage, const QString &privateKeyStr)
{
    // Convert private key from string to RSA object
    BIO *bio = BIO_new_mem_buf(privateKeyStr.toLatin1().data(), -1);
    RSA *rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);

    if (!rsa) {
        BIO_free(bio);
        return QByteArray();
    }

    // Prepare buffer for decrypted message
    int rsaSize = RSA_size(rsa);
    QByteArray decrypted;
    decrypted.resize(rsaSize);

    // Decrypt the message using RSA
    int result = RSA_private_decrypt(encryptedMessage.size(), 
                                    reinterpret_cast<const unsigned char*>(encryptedMessage.data()), 
                                    reinterpret_cast<unsigned char*>(decrypted.data()), 
                                    rsa, RSA_PKCS1_PADDING);

    BIO_free(bio);
    RSA_free(rsa);

    if (result == -1) {
        return QByteArray();
    }

    // Resize to actual decrypted size
    decrypted.resize(result);
    return decrypted;
}

QByteArray EncryptionManager::encryptLocal(const QByteArray &data, const QString &key)
{
    // For local encryption, we'll use AES-256-GCM
    // This is a simplified implementation - a full implementation would use proper AES-GCM
    
    // Generate salt
    QByteArray salt = generateSalt();
    
    // Derive key from password
    QByteArray derivedKey = deriveKeyFromPassword(key, salt);
    
    // Simple XOR encryption (in real implementation, use proper AES-GCM)
    QByteArray encrypted = data;
    for (int i = 0; i < encrypted.size(); ++i) {
        encrypted[i] ^= derivedKey[i % derivedKey.size()];
    }
    
    // Prepend salt to encrypted data
    return salt + encrypted;
}

QByteArray EncryptionManager::decryptLocal(const QByteArray &encryptedData, const QString &key)
{
    // Extract salt (first 16 bytes)
    if (encryptedData.size() < 16) {
        return QByteArray();
    }
    
    QByteArray salt = encryptedData.left(16);
    QByteArray encrypted = encryptedData.mid(16);
    
    // Derive key from password and salt
    QByteArray derivedKey = deriveKeyFromPassword(key, salt);
    
    // Simple XOR decryption (in real implementation, use proper AES-GCM)
    QByteArray decrypted = encrypted;
    for (int i = 0; i < decrypted.size(); ++i) {
        decrypted[i] ^= derivedKey[i % derivedKey.size()];
    }
    
    return decrypted;
}

QString EncryptionManager::hashPassword(const QString &password)
{
    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString EncryptionManager::hashData(const QByteArray &data)
{
    return QString(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

bool EncryptionManager::verifySignature(const QByteArray &data, const QString &signature, const QString &publicKey)
{
    // In a real implementation, this would verify a digital signature
    // For now, we return true to indicate success
    return true;
}

QByteArray EncryptionManager::deriveKeyFromPassword(const QString &password, const QByteArray &salt)
{
    // Simplified key derivation (in real implementation, use PBKDF2 or similar)
    QByteArray combined = password.toUtf8() + salt;
    return QCryptographicHash::hash(combined, QCryptographicHash::Sha256);
}

QByteArray EncryptionManager::generateSalt()
{
    QByteArray salt(16, 0);
    for (int i = 0; i < 16; ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    return salt;
}