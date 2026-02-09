#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QObject>
#include <QString>
#include <QTimer>

class UserManager;
class TorManager;
class MessageHandler;

/**
 * @brief Main application class that manages the entire AnonymousMessage PC application
 * 
 * This class handles initialization of core components, application lifecycle,
 * and coordination between different modules. It ensures all security measures
 * are properly implemented and maintained throughout the application lifecycle.
 */
class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(int argc, char *argv[]);
    ~Application();

    bool initialize();
    int exec();

private slots:
    void onAboutToQuit();

private:
    void initializeCoreComponents();
    void initializeSecurityProtocols();
    void initializeUI();
    void setupSignalHandlers();
    void schedulePeriodicTasks();

    QApplication *m_app;
    UserManager *m_userManager;
    TorManager *m_torManager;
    MessageHandler *m_messageHandler;
    
    QTimer m_periodicCleanupTimer;
    
    QString m_appDataPath;
    QString m_cachePath;
    QString m_configPath;
};

#endif // APPLICATION_H