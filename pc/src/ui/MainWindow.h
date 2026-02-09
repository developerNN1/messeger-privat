#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QTcpSocket>
#include <QTimer>
#include "core/SessionManager.h"
#include "network/TorClient.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginSuccess();
    void onLoginFailure(const QString &error);
    void onTorConnected();
    void onTorDisconnected();
    void onNewMessageReceived(const QString &sender, const QString &message);
    void onCallIncoming(const QString &caller);

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupTrayIcon();
    void connectSignals();
    
    // UI Elements
    QWidget *centralWidget;
    QStackedWidget *mainStack;
    
    // Login/Register Screens
    QWidget *loginScreen;
    QWidget *registerScreen;
    QWidget *mainChatScreen;
    
    // Login Form Elements
    QLineEdit *loginEmailInput;
    QLineEdit *loginPasswordInput;
    QPushButton *loginButton;
    QPushButton *switchToRegisterButton;
    
    // Register Form Elements
    QLineEdit *registerUsernameInput;
    QLineEdit *registerEmailInput;
    QLineEdit *registerPasswordInput;
    QLineEdit *registerConfirmPasswordInput;
    QPushButton *registerButton;
    QPushButton *switchToLoginButton;
    
    // Main Chat Elements
    QListWidget *chatList;
    QWidget *chatArea;
    QTextEdit *messageInput;
    QPushButton *sendButton;
    
    // Session and Network
    SessionManager *sessionManager;
    TorClient *torClient;
    
    // System Tray
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;
};

#endif // MAINWINDOW_H