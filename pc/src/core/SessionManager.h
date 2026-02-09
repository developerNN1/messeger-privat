#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QTcpSocket>
#include "User.h"

class SessionManager : public QObject
{
    Q_OBJECT

public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager();

    bool isLoggedIn() const;
    User getCurrentUser() const;

signals:
    void loginSuccess();
    void loginFailure(const QString &error);
    void registrationSuccess();
    void registrationFailure(const QString &error);
    void userLoggedOut();

public slots:
    void login(const QString &email, const QString &password);
    void registerUser(const QString &username, const QString &email, const QString &password);
    void logout();

private slots:
    void handleServerResponse();
    void handleConnectionError();

private:
    void sendRequest(const QJsonObject &request);
    void saveSession(const QString &token, const User &user);
    void loadSession();
    void clearSession();

    QTcpSocket *socket;
    bool loggedIn;
    User currentUser;
    QString authToken;
    QString serverAddress;
    quint16 serverPort;
};

#endif // SESSIONMANAGER_H