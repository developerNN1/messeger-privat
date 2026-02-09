#ifndef TOR_MANAGER_H
#define TOR_MANAGER_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QMutex>
#include <QTcpSocket>

/**
 * @brief Manages the TOR daemon and related network operations
 * 
 * This class handles starting, stopping, and monitoring the TOR process,
 * configures SOCKS proxy settings, and manages anonymous communication
 * channels. It ensures all traffic goes through TOR for anonymity.
 */
class TorManager : public QObject
{
    Q_OBJECT

public:
    explicit TorManager(QObject *parent = nullptr);
    ~TorManager();

    bool initialize();
    void shutdown();

    bool isTorRunning() const;
    int getSocksPort() const;
    int getControlPort() const;

    QString getTorDataDir() const;
    QString getTorExecutablePath() const;

signals:
    void torStatusChanged(bool isRunning);
    void torConnectionEstablished();
    void torConnectionLost();

public slots:
    void startTor();
    void stopTor();
    void restartTor();

private slots:
    void onTorProcessStarted();
    void onTorProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onTorProcessErrorOccurred(QProcess::ProcessError error);
    void onTorOutputReady();
    void onTorErrorReady();

private:
    void setupTorConfiguration();
    void setupTorDirectories();
    void findTorExecutable();
    void setupSocksProxy();
    void monitorTorHealth();
    void handleTorOutput(const QByteArray &output);
    void handleTorError(const QByteArray &error);

    QProcess *m_torProcess;
    QTimer m_healthCheckTimer;
    QTcpSocket m_controlSocket;

    bool m_isTorRunning;
    int m_socksPort;
    int m_controlPort;
    QString m_torDataDir;
    QString m_torExecutablePath;
    QString m_geoIpFilePath;

    mutable QMutex m_mutex;
    
    // Configuration parameters
    int m_maxRetries;
    int m_retryDelayMs;
    int m_healthCheckIntervalMs;
};

#endif // TOR_MANAGER_H