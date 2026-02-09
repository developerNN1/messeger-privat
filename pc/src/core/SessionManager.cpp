#include "SessionManager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , socket(new QTcpSocket(this))
    , loggedIn(false)
    , serverAddress("127.0.0.1")
    , serverPort(8080)
{
    connect(socket, &QTcpSocket::readyRead, this, &SessionManager::handleServerResponse);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &SessionManager::handleConnectionError);
    
    // Load existing session if available
    loadSession();
}

SessionManager::~SessionManager()
{
    if (loggedIn) {
        logout();
    }
}

bool SessionManager::isLoggedIn() const
{
    return loggedIn;
}

User SessionManager::getCurrentUser() const
{
    return currentUser;
}

void SessionManager::login(const QString &email, const QString &password)
{
    // Validate inputs
    if (email.isEmpty() || password.isEmpty()) {
        emit loginFailure("Email and password are required");
        return;
    }

    // Create login request
    QJsonObject request;
    request["type"] = "login";
    request["email"] = email;
    request["password"] = password;  // In real implementation, this should be hashed

    // Send request asynchronously
    QtConcurrent::run([this, request]() {
        sendRequest(request);
    });
}

void SessionManager::registerUser(const QString &username, const QString &email, const QString &password)
{
    // Validate inputs
    if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
        emit registrationFailure("All fields are required");
        return;
    }

    // Validate username format
    if (username.length() < 3 || !username.contains(QRegExp("^[a-zA-Z0-9_]+$"))) {
        emit registrationFailure("Invalid username format");
        return;
    }

    // Validate email format
    if (!email.contains("@") || !email.contains(".")) {
        emit registrationFailure("Invalid email format");
        return;
    }

    // Validate password strength
    if (password.length() < 6) {
        emit registrationFailure("Password must be at least 6 characters");
        return;
    }

    // Create registration request
    QJsonObject request;
    request["type"] = "register";
    request["username"] = username;
    request["email"] = email;
    request["password"] = password;  // In real implementation, this should be hashed

    // Send request asynchronously
    QtConcurrent::run([this, request]() {
        sendRequest(request);
    });
}

void SessionManager::logout()
{
    if (!loggedIn) {
        return;
    }

    // Create logout request
    QJsonObject request;
    request["type"] = "logout";
    request["auth_token"] = authToken;

    // Send request
    sendRequest(request);

    // Clear session data
    clearSession();

    emit userLoggedOut();
}

void SessionManager::handleServerResponse()
{
    QByteArray responseData = socket->readAll();
    QJsonDocument document = QJsonDocument::fromJson(responseData);
    QJsonObject response = document.object();

    QString responseType = response["type"].toString();

    if (responseType == "login_response") {
        if (response["success"].toBool()) {
            // Parse user data
            QJsonObject userData = response["user"].toObject();
            currentUser.fromJson(userData);
            
            // Save session
            saveSession(response["auth_token"].toString(), currentUser);
            
            loggedIn = true;
            emit loginSuccess();
        } else {
            emit loginFailure(response["error"].toString("Unknown error"));
        }
    } else if (responseType == "register_response") {
        if (response["success"].toBool()) {
            emit registrationSuccess();
        } else {
            emit registrationFailure(response["error"].toString("Registration failed"));
        }
    } else if (responseType == "logout_response") {
        if (response["success"].toBool()) {
            clearSession();
            emit userLoggedOut();
        }
    }
}

void SessionManager::handleConnectionError()
{
    emit loginFailure("Connection error: " + socket->errorString());
}

void SessionManager::sendRequest(const QJsonObject &request)
{
    // Convert JSON object to bytes
    QJsonDocument doc(request);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);

    // Connect to server if not already connected
    if (socket->state() != QAbstractSocket::ConnectedState) {
        socket->connectToHost(serverAddress, serverPort);
        
        // Wait for connection
        if (!socket->waitForConnected(5000)) {
            emit loginFailure("Cannot connect to server: " + socket->errorString());
            return;
        }
    }

    // Send request
    socket->write(requestData);
    socket->flush();
}

void SessionManager::saveSession(const QString &token, const User &user)
{
    // Create session data
    QJsonObject sessionData;
    sessionData["auth_token"] = token;
    sessionData["user"] = user.toJson();
    sessionData["timestamp"] = QDateTime::currentSecsSinceEpoch();

    // Convert to JSON
    QJsonDocument doc(sessionData);
    QByteArray sessionBytes = doc.toJson(QJsonDocument::Compact);

    // Determine config directory
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir dir(configPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Write session to file
    QFile sessionFile(dir.filePath("anonymous_message_session.json"));
    if (sessionFile.open(QIODevice::WriteOnly)) {
        sessionFile.write(sessionBytes);
        sessionFile.close();
    }
}

void SessionManager::loadSession()
{
    // Determine config directory
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir dir(configPath);

    // Read session from file
    QFile sessionFile(dir.filePath("anonymous_message_session.json"));
    if (!sessionFile.exists()) {
        return;
    }

    if (sessionFile.open(QIODevice::ReadOnly)) {
        QByteArray sessionData = sessionFile.readAll();
        sessionFile.close();

        QJsonDocument doc = QJsonDocument::fromJson(sessionData);
        QJsonObject sessionObj = doc.object();

        // Check if session is still valid (not expired)
        qint64 timestamp = sessionObj["timestamp"].toVariant().toLongLong();
        if (QDateTime::currentSecsSinceEpoch() - timestamp < 86400) {  // 24 hours
            authToken = sessionObj["auth_token"].toString();
            currentUser.fromJson(sessionObj["user"].toObject());
            loggedIn = true;
        }
    }
}

void SessionManager::clearSession()
{
    loggedIn = false;
    authToken.clear();
    currentUser = User();
    
    // Remove session file
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir dir(configPath);
    QFile::remove(dir.filePath("anonymous_message_session.json"));
}