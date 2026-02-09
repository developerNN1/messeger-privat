#include "User.h"

User::User()
    : online(false)
    , createdAt(0)
{
}

User::User(const QString &userId, const QString &username, const QString &email,
           const QString &avatarUrl, const QString &publicKey)
    : userId(userId)
    , username(username)
    , email(email)
    , avatarUrl(avatarUrl)
    , publicKey(publicKey)
    , online(false)
    , createdAt(QDateTime::currentMSecsSinceEpoch())
{
}

// Getters
QString User::getUserId() const
{
    return userId;
}

QString User::getUsername() const
{
    return username;
}

QString User::getEmail() const
{
    return email;
}

QString User::getAvatarUrl() const
{
    return avatarUrl;
}

QString User::getPublicKey() const
{
    return publicKey;
}

bool User::isOnline() const
{
    return online;
}

qint64 User::getCreatedAt() const
{
    return createdAt;
}

// Setters
void User::setUserId(const QString &userId)
{
    this->userId = userId;
}

void User::setUsername(const QString &username)
{
    this->username = username;
}

void User::setEmail(const QString &email)
{
    this->email = email;
}

void User::setAvatarUrl(const QString &avatarUrl)
{
    this->avatarUrl = avatarUrl;
}

void User::setPublicKey(const QString &publicKey)
{
    this->publicKey = publicKey;
}

void User::setOnline(bool online)
{
    this->online = online;
}

void User::setCreatedAt(qint64 createdAt)
{
    this->createdAt = createdAt;
}

// Serialization
QJsonObject User::toJson() const
{
    QJsonObject obj;
    obj["user_id"] = userId;
    obj["username"] = username;
    obj["email"] = email;
    obj["avatar_url"] = avatarUrl;
    obj["public_key"] = publicKey;
    obj["online"] = online;
    obj["created_at"] = QString::number(createdAt);
    return obj;
}

void User::fromJson(const QJsonObject &json)
{
    userId = json["user_id"].toString();
    username = json["username"].toString();
    email = json["email"].toString();
    avatarUrl = json["avatar_url"].toString();
    publicKey = json["public_key"].toString();
    online = json["online"].toBool();
    createdAt = json["created_at"].toString().toLongLong();
}