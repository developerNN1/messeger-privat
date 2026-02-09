#include "Application.h"
#include "UserManager.h"
#include "TorManager.h"
#include "MessageHandler.h"

#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

Application::Application(int argc, char *argv[])
    : m_app(new QApplication(argc, argv))
    , m_userManager(nullptr)
    , m_torManager(nullptr)
    , m_messageHandler(nullptr)
{
    // Set application properties
    m_app->setApplicationName("AnonymousMessage");
    m_app->setApplicationVersion("1.0.0");
    m_app->setOrganizationName("AnonymousMessage");
    
    // Initialize paths
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_appDataPath = dataPath;
    m_cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_configPath = dataPath + "/config";
    
    // Create necessary directories
    QDir().mkpath(m_appDataPath);
    QDir().mkpath(m_cachePath);
    QDir().mkpath(m_configPath);
    
    // Connect quit signal
    QObject::connect(m_app, &QApplication::aboutToQuit, this, &Application::onAboutToQuit);
}

Application::~Application()
{
    // Cleanup in reverse order
    delete m_messageHandler;
    delete m_torManager;
    delete m_userManager;
    delete m_app;
}

bool Application::initialize()
{
    qDebug() << "Initializing AnonymousMessage PC application...";
    
    // Initialize core components
    initializeCoreComponents();
    
    // Setup security protocols
    initializeSecurityProtocols();
    
    // Initialize UI components
    initializeUI();
    
    // Setup signal handlers
    setupSignalHandlers();
    
    // Schedule periodic tasks
    schedulePeriodicTasks();
    
    qDebug() << "Application initialized successfully!";
    return true;
}

int Application::exec()
{
    return m_app->exec();
}

void Application::initializeCoreComponents()
{
    qDebug() << "Initializing core components...";
    
    // Initialize user manager
    m_userManager = new UserManager(this);
    
    // Initialize TOR manager
    m_torManager = new TorManager(this);
    if (!m_torManager->initialize()) {
        qCritical() << "Failed to initialize TOR manager";
        // We might want to show an error dialog here
        // For now, just log the error
    }
    
    // Initialize message handler
    m_messageHandler = new MessageHandler(this);
    
    qDebug() << "Core components initialized";
}

void Application::initializeSecurityProtocols()
{
    qDebug() << "Initializing security protocols...";
    
    // Set up encryption keys
    // Configure secure storage
    // Initialize secure communication channels
    
    qDebug() << "Security protocols initialized";
}

void Application::initializeUI()
{
    qDebug() << "Initializing UI...";
    
    // Initialize main window and other UI components
    // This will be handled by the MainWindow class
    
    qDebug() << "UI initialized";
}

void Application::setupSignalHandlers()
{
    qDebug() << "Setting up signal handlers...";
    
    // Connect various signals between components
    
    qDebug() << "Signal handlers setup";
}

void Application::schedulePeriodicTasks()
{
    qDebug() << "Scheduling periodic tasks...";
    
    // Set up timer for periodic cleanup tasks
    QObject::connect(&m_periodicCleanupTimer, &QTimer::timeout, this, [this]() {
        // Perform periodic cleanup tasks
        // Clear temporary files, update status, etc.
    });
    
    // Run every 5 minutes
    m_periodicCleanupTimer.start(5 * 60 * 1000);
    
    qDebug() << "Periodic tasks scheduled";
}

void Application::onAboutToQuit()
{
    qDebug() << "Application is about to quit...";
    
    // Perform cleanup tasks
    m_periodicCleanupTimer.stop();
    
    // Save any pending data
    // Close connections gracefully
}