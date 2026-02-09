#ifndef USER_H
#define USER_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

class User
{
public:
    User();
    User(const QString &userId, const QString &username, const QString &email, 
         const QString &avatarUrl, const QString &publicKey);

    // Getters
    QString getUserId() const;
    QString getUsername() const;
    QString getEmail() const;
    QString getAvatarUrl() const;
    QString getPublicKey() const;
    bool isOnline() const;
    qint64 getCreatedAt() const;

    // Setters
    void setUserId(const QString &userId);
    void setUsername(const QString &username);
    void setEmail(const QString &email);
    void setAvatarUrl(const QString &avatarUrl);
    void setPublicKey(const QString &publicKey);
    void setOnline(bool online);
    void setCreatedAt(qint64 createdAt);

    // Serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

private:
    QString userId;
    QString username;
    QString email;
    QString avatarUrl;
    QString publicKey;  // For encryption
    bool online;
    qint64 createdAt;
};

#endif // USER_H