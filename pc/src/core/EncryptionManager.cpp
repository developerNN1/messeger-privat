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
    // For local encryption, we'll use AES-256-GCM with proper implementation
    // Generate salt
    QByteArray salt = generateSalt();
    
    // Derive key from password using PBKDF2
    QByteArray derivedKey = deriveKeyFromPassword(key, salt);
    
    // Initialize OpenSSL cipher context for AES-256-GCM
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qCritical() << "Failed to create cipher context";
        return QByteArray();
    }
    
    // Initialize encryption operation with generated key
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        qCritical() << "Failed to initialize encryption";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    
    // Generate random IV (Initialization Vector)
    QByteArray iv(16, 0);  // 128-bit IV for AES
    for (int i = 0; i < iv.size(); ++i) {
        iv[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    
    // Set IV length
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL) != 1) {
        qCritical() << "Failed to set IV length";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    
    // Initialize encryption operation with key and IV
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, 
                          reinterpret_cast<const unsigned char*>(derivedKey.constData()), 
                          reinterpret_cast<const unsigned char*>(iv.constData())) != 1) {
        qCritical() << "Failed to initialize encryption with key and IV";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    
    // Allocate space for ciphertext
    QByteArray ciphertext(data.size() + EVP_MAX_BLOCK_LENGTH, 0);
    int len = 0;
    
    // Provide plaintext and obtain ciphertext
    if (EVP_EncryptUpdate(ctx, 
                         reinterpret_cast<unsigned char*>(ciphertext.data()), 
                         &len,
                         reinterpret_cast<const unsigned char*>(data.constData()), 
                         data.size()) != 1) {
        qCritical() << "Failed to encrypt data";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    
    int ciphertext_len = len;
    
    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, 
                           reinterpret_cast<unsigned char*>(ciphertext.data()) + len, 
                           &len) != 1) {
        qCritical() << "Failed to finalize encryption";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    ciphertext_len += len;
    
    // Get authentication tag
    QByteArray tag(16, 0);  // Standard tag size for GCM
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) {
        qCritical() << "Failed to get authentication tag";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    // Format: salt + IV + ciphertext + tag
    QByteArray result;
    result.reserve(salt.size() + iv.size() + ciphertext_len + tag.size());
    result.append(salt);
    result.append(iv);
    result.append(ciphertext.left(ciphertext_len));
    result.append(tag);
    
    return result;
}

QByteArray EncryptionManager::decryptLocal(const QByteArray &encryptedData, const QString &key)
{
    // Extract components: salt (16 bytes) + IV (16 bytes) + ciphertext + tag (16 bytes)
    if (encryptedData.size() < 48) {  // Minimum size: salt + IV + tag
        qCritical() << "Encrypted data too small for proper format";
        return QByteArray();
    }
    
    // Extract salt
    QByteArray salt = encryptedData.left(16);
    
    // Extract IV (next 16 bytes)
    QByteArray iv = encryptedData.mid(16, 16);
    
    // Extract tag (last 16 bytes)
    QByteArray tag = encryptedData.right(16);
    
    // Extract ciphertext (everything in between)
    int ciphertext_start = 32;
    int ciphertext_end = encryptedData.size() - 16;
    QByteArray ciphertext = encryptedData.mid(32, ciphertext_end - ciphertext_start);
    
    // Derive key from password and salt
    QByteArray derivedKey = deriveKeyFromPassword(key, salt);
    
    // Initialize OpenSSL cipher context for AES-256-GCM decryption
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qCritical() << "Failed to create cipher context for decryption";
        return QByteArray();
    }
    
    // Initialize decryption operation
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        qCritical() << "Failed to initialize decryption";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    
    // Set IV length
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv.size(), NULL) != 1) {
        qCritical() << "Failed to set IV length for decryption";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    
    // Initialize decryption operation with key and IV
    if (EVP_DecryptInit_ex(ctx, NULL, NULL,
                          reinterpret_cast<const unsigned char*>(derivedKey.constData()),
                          reinterpret_cast<const unsigned char*>(iv.constData())) != 1) {
        qCritical() << "Failed to initialize decryption with key and IV";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    
    // Allocate space for plaintext
    QByteArray plaintext(ciphertext.size() + EVP_MAX_BLOCK_LENGTH, 0);
    int len = 0;
    
    // Provide ciphertext and obtain plaintext
    if (EVP_DecryptUpdate(ctx,
                         reinterpret_cast<unsigned char*>(plaintext.data()),
                         &len,
                         reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                         ciphertext.size()) != 1) {
        qCritical() << "Failed to decrypt data";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    
    int plaintext_len = len;
    
    // Set expected tag value
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(),
                           reinterpret_cast<void*>(tag.data())) != 1) {
        qCritical() << "Failed to set authentication tag";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    
    // Finalize decryption - this also verifies the tag
    if (EVP_DecryptFinal_ex(ctx,
                           reinterpret_cast<unsigned char*>(plaintext.data()) + len,
                           &len) != 1) {
        qCritical() << "Failed to finalize decryption or authentication failed";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    plaintext_len += len;
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    // Return the plaintext
    return plaintext.left(plaintext_len);
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
    // Convert the public key from string to RSA object
    BIO *bio = BIO_new_mem_buf(publicKey.toLatin1().data(), -1);
    RSA *rsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);

    if (!rsa) {
        qCritical() << "Failed to parse public key for signature verification";
        BIO_free(bio);
        return false;
    }

    // Calculate hash of the data
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash);

    // Decode the signature from base64 string
    QByteArray signatureBytes = QByteArray::fromBase64(signature.toLatin1());
    if (signatureBytes.isEmpty()) {
        qCritical() << "Invalid signature format";
        RSA_free(rsa);
        BIO_free(bio);
        return false;
    }

    // Verify the signature using RSA
    int result = RSA_verify(NID_sha256, 
                           hash, 
                           SHA256_DIGEST_LENGTH,
                           reinterpret_cast<const unsigned char*>(signatureBytes.data()),
                           signatureBytes.size(),
                           rsa);

    // Clean up
    RSA_free(rsa);
    BIO_free(bio);

    // Return true if verification was successful
    return (result == 1);
}

QByteArray EncryptionManager::deriveKeyFromPassword(const QString &password, const QByteArray &salt)
{
    // Proper key derivation using PKCS#5 PBKDF2 with HMAC-SHA256
    // Parameters: password, salt, iteration count, desired key length
    int iterations = 10000;  // Recommended minimum for PBKDF2
    int keyLength = 32;      // 256 bits for AES-256
    
    QByteArray derivedKey(keyLength, 0);
    
    int result = PKCS5_PBKDF2_HMAC(
        password.toUtf8().constData(),
        password.toUtf8().size(),
        reinterpret_cast<const unsigned char*>(salt.constData()),
        salt.size(),
        iterations,
        EVP_sha256(),
        keyLength,
        reinterpret_cast<unsigned char*>(derivedKey.data())
    );
    
    if (result <= 0) {
        qCritical() << "PBKDF2 key derivation failed";
        return QCryptographicHash::hash(password.toUtf8() + salt, QCryptographicHash::Sha256);
    }
    
    return derivedKey;
}

QByteArray EncryptionManager::generateSalt()
{
    QByteArray salt(16, 0);
    for (int i = 0; i < 16; ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    return salt;
}