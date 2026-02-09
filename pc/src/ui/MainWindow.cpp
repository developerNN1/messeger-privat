#include "MainWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QIcon>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupTrayIcon();
    connectSignals();
    
    // Initialize session manager and Tor client
    sessionManager = new SessionManager(this);
    torClient = new TorClient(this);
    
    // Connect to Tor network
    torClient->connectToTor();
    
    // Initially show login screen
    mainStack->setCurrentWidget(loginScreen);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainStack = new QStackedWidget(this);
    
    // Create login screen
    loginScreen = new QWidget();
    auto *loginLayout = new QVBoxLayout(loginScreen);
    
    auto *titleLabel = new QLabel("AnonymousMessage");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");
    
    loginEmailInput = new QLineEdit();
    loginEmailInput->setPlaceholderText("Email");
    loginEmailInput->setMinimumWidth(300);
    
    loginPasswordInput = new QLineEdit();
    loginPasswordInput->setPlaceholderText("Password");
    loginPasswordInput->setEchoMode(QLineEdit::Password);
    
    loginButton = new QPushButton("Sign In");
    switchToRegisterButton = new QPushButton("Create New Account");
    
    loginLayout->addWidget(titleLabel);
    loginLayout->addWidget(loginEmailInput);
    loginLayout->addWidget(loginPasswordInput);
    loginLayout->addWidget(loginButton);
    loginLayout->addWidget(switchToRegisterButton);
    loginLayout->addStretch();
    
    // Create register screen
    registerScreen = new QWidget();
    auto *registerLayout = new QVBoxLayout(registerScreen);
    
    auto *regTitleLabel = new QLabel("Create Account");
    regTitleLabel->setAlignment(Qt::AlignCenter);
    regTitleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");
    
    registerUsernameInput = new QLineEdit();
    registerUsernameInput->setPlaceholderText("Username");
    
    registerEmailInput = new QLineEdit();
    registerEmailInput->setPlaceholderText("Email");
    
    registerPasswordInput = new QLineEdit();
    registerPasswordInput->setPlaceholderText("Password");
    registerPasswordInput->setEchoMode(QLineEdit::Password);
    
    registerConfirmPasswordInput = new QLineEdit();
    registerConfirmPasswordInput->setPlaceholderText("Confirm Password");
    registerConfirmPasswordInput->setEchoMode(QLineEdit::Password);
    
    registerButton = new QPushButton("Register");
    switchToLoginButton = new QPushButton("Already have an account? Sign In");
    
    registerLayout->addWidget(regTitleLabel);
    registerLayout->addWidget(registerUsernameInput);
    registerLayout->addWidget(registerEmailInput);
    registerLayout->addWidget(registerPasswordInput);
    registerLayout->addWidget(registerConfirmPasswordInput);
    registerLayout->addWidget(registerButton);
    registerLayout->addWidget(switchToLoginButton);
    registerLayout->addStretch();
    
    // Create main chat screen
    mainChatScreen = new QWidget();
    auto *chatLayout = new QHBoxLayout(mainChatScreen);
    
    chatList = new QListWidget();
    chatList->setMaximumWidth(250);
    
    chatArea = new QWidget();
    auto *chatAreaLayout = new QVBoxLayout(chatArea);
    
    messageInput = new QTextEdit();
    messageInput->setMaximumHeight(100);
    
    sendButton = new QPushButton("Send");
    
    chatAreaLayout->addWidget(messageInput);
    chatAreaLayout->addWidget(sendButton);
    
    chatLayout->addWidget(chatList);
    chatLayout->addWidget(chatArea);
    
    // Add screens to stack
    mainStack->addWidget(loginScreen);
    mainStack->addWidget(registerScreen);
    mainStack->addWidget(mainChatScreen);
    
    auto *layout = new QVBoxLayout(centralWidget);
    layout->addWidget(mainStack);
    
    setWindowTitle("AnonymousMessage");
    resize(900, 600);
    
    // Connect buttons
    connect(loginButton, &QPushButton::clicked, [this]() {
        // Attempt login
        QString email = loginEmailInput->text();
        QString password = loginPasswordInput->text();
        sessionManager->login(email, password);
    });
    
    connect(switchToRegisterButton, &QPushButton::clicked, [this]() {
        mainStack->setCurrentWidget(registerScreen);
    });
    
    connect(registerButton, &QPushButton::clicked, [this]() {
        // Attempt registration
        QString username = registerUsernameInput->text();
        QString email = registerEmailInput->text();
        QString password = registerPasswordInput->text();
        QString confirmPassword = registerConfirmPasswordInput->text();
        
        if (password != confirmPassword) {
            QMessageBox::warning(this, "Registration Error", "Passwords do not match!");
            return;
        }
        
        sessionManager->registerUser(username, email, password);
    });
    
    connect(switchToLoginButton, &QPushButton::clicked, [this]() {
        mainStack->setCurrentWidget(loginScreen);
    });
    
    connect(sendButton, &QPushButton::clicked, [this]() {
        // Send message logic here
        QString message = messageInput->toPlainText();
        if (!message.trimmed().isEmpty()) {
            // Send message through Tor
            // For now, just clear the input
            messageInput->clear();
        }
    });
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu("File");
    
    QAction *preferencesAction = fileMenu->addAction("Preferences");
    preferencesAction->setShortcut(QKeySequence("Ctrl+,"));
    
    fileMenu->addSeparator();
    
    QAction *quitAction = fileMenu->addAction("Quit");
    quitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quitAction, &QAction::triggered, this, &QMainWindow::close);
    
    // Edit menu
    QMenu *editMenu = menuBar->addMenu("Edit");
    
    QAction *copyAction = editMenu->addAction("Copy");
    copyAction->setShortcut(QKeySequence::Copy);
    
    QAction *pasteAction = editMenu->addAction("Paste");
    pasteAction->setShortcut(QKeySequence::Paste);
    
    // Help menu
    QMenu *helpMenu = menuBar->addMenu("Help");
    
    QAction *aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About AnonymousMessage", 
                          "AnonymousMessage v1.0\n\n"
                          "A secure, anonymous messaging application that works through Tor without requiring hosting.\n\n"
                          "All communications are end-to-end encrypted and routed through the Tor network.");
    });
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready");
}

void MainWindow::setupTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }
    
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icon.png")); // Placeholder icon
    trayIcon->setVisible(true);
    
    trayMenu = new QMenu(this);
    minimizeAction = trayMenu->addAction("Minimize");
    maximizeAction = trayMenu->addAction("Maximize");
    restoreAction = trayMenu->addAction("Restore");
    quitAction = trayMenu->addAction("Quit");
    
    trayIcon->setContextMenu(trayMenu);
    
    connect(minimizeAction, &QAction::triggered, this, &QMainWindow::showMinimized);
    connect(maximizeAction, &QAction::triggered, this, &QMainWindow::showMaximized);
    connect(restoreAction, &QAction::triggered, this, &QMainWindow::showNormal);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    
    connect(trayIcon, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            showNormal();
            raise();
            activateWindow();
        }
    });
}

void MainWindow::connectSignals()
{
    connect(sessionManager, &SessionManager::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(sessionManager, &SessionManager::loginFailure, this, &MainWindow::onLoginFailure);
    connect(torClient, &TorClient::connected, this, &MainWindow::onTorConnected);
    connect(torClient, &TorClient::disconnected, this, &MainWindow::onTorDisconnected);
}

void MainWindow::onLoginSuccess()
{
    mainStack->setCurrentWidget(mainChatScreen);
    statusBar()->showMessage("Logged in successfully");
}

void MainWindow::onLoginFailure(const QString &error)
{
    QMessageBox::critical(this, "Login Failed", error);
    statusBar()->showMessage("Login failed: " + error, 3000);
}

void MainWindow::onTorConnected()
{
    statusBar()->showMessage("Connected to Tor network", 2000);
}

void MainWindow::onTorDisconnected()
{
    statusBar()->showMessage("Disconnected from Tor network", 2000);
}

void MainWindow::onNewMessageReceived(const QString &sender, const QString &message)
{
    // Handle incoming message
    // For now just update status bar
    statusBar()->showMessage(QString("New message from %1").arg(sender), 3000);
}

void MainWindow::onCallIncoming(const QString &caller)
{
    // Handle incoming call
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Incoming Call", 
                                 QString("Call from %1. Accept?").arg(caller),
                                 QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Accept call logic
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon && trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}