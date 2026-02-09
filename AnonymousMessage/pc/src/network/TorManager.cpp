#include "TorManager.h"
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <QRegularExpression>
#include <QThread>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>

TorManager::TorManager(QObject *parent)
    : QObject(parent)
    , m_torProcess(nullptr)
    , m_isTorRunning(false)
    , m_socksPort(9050)
    , m_controlPort(9051)
    , m_maxRetries(3)
    , m_retryDelayMs(5000)
    , m_healthCheckIntervalMs(30000) // 30 seconds
{
    setupTorConfiguration();
    setupTorDirectories();
    findTorExecutable();
    
    // Connect health check timer
    connect(&m_healthCheckTimer, &QTimer::timeout, this, &TorManager::monitorTorHealth);
    m_healthCheckTimer.start(m_healthCheckIntervalMs);
}

TorManager::~TorManager()
{
    shutdown();
}

bool TorManager::initialize()
{
    qDebug() << "Initializing TOR manager...";
    
    // Try to start TOR
    startTor();
    
    // Wait a bit for TOR to potentially start
    QThread::msleep(2000);
    
    return m_isTorRunning;
}

void TorManager::shutdown()
{
    qDebug() << "Shutting down TOR manager...";
    
    m_healthCheckTimer.stop();
    
    if (m_torProcess && m_torProcess->state() == QProcess::Running) {
        stopTor();
    }
    
    if (m_controlSocket.state() == QTcpSocket::ConnectedState) {
        m_controlSocket.close();
    }
}

void TorManager::startTor()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isTorRunning) {
        qDebug() << "TOR is already running";
        return;
    }
    
    qDebug() << "Starting TOR...";
    
    if (!QFileInfo::exists(m_torExecutablePath)) {
        qWarning() << "TOR executable not found:" << m_torExecutablePath;
        return;
    }
    
    if (m_torProcess) {
        delete m_torProcess;
    }
    
    m_torProcess = new QProcess(this);
    
    // Connect signals
    connect(m_torProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TorManager::onTorProcessFinished);
    connect(m_torProcess, QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
            this, &TorManager::onTorProcessErrorOccurred);
    connect(m_torProcess, &QProcess::started,
            this, &TorManager::onTorProcessStarted);
    
    // Prepare arguments for TOR
    QStringList arguments;
    arguments << "--DataDirectory" << m_torDataDir
              << "--SocksPort" << QString::number(m_socksPort)
              << "--ControlPort" << QString::number(m_controlPort)
              << "--GeoIPFile" << m_geoIpFilePath
              << "--Log" << "notice stdout"
              << "--AvoidDiskWrites" << "1";
    
    qDebug() << "Starting TOR with arguments:" << arguments.join(" ");
    
    // Start TOR process
    m_torProcess->start(m_torExecutablePath, arguments);
    
    // Wait for process to start (non-blocking)
    if (!m_torProcess->waitForStarted(10000)) { // 10 second timeout
        qWarning() << "Failed to start TOR process:" << m_torProcess->errorString();
        emit torStatusChanged(false);
        return;
    }
    
    qDebug() << "TOR process started successfully";
}

void TorManager::stopTor()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isTorRunning || !m_torProcess) {
        qDebug() << "TOR is not running or process not initialized";
        return;
    }
    
    qDebug() << "Stopping TOR...";
    
    // Try graceful shutdown first
    m_torProcess->write("SHUTDOWN\r\n");
    m_torProcess->waitForBytesWritten(1000);
    
    // Wait for process to finish gracefully
    if (!m_torProcess->waitForFinished(5000)) { // 5 second timeout
        qDebug() << "TOR didn't shut down gracefully, killing process...";
        m_torProcess->kill();
        m_torProcess->waitForFinished(2000);
    }
    
    m_isTorRunning = false;
    emit torStatusChanged(false);
    emit torConnectionLost();
    
    qDebug() << "TOR stopped";
}

void TorManager::restartTor()
{
    qDebug() << "Restarting TOR...";
    stopTor();
    QThread::msleep(1000); // Wait a second before restarting
    startTor();
}

bool TorManager::isTorRunning() const
{
    QMutexLocker locker(&m_mutex);
    return m_isTorRunning;
}

int TorManager::getSocksPort() const
{
    return m_socksPort;
}

int TorManager::getControlPort() const
{
    return m_controlPort;
}

QString TorManager::getTorDataDir() const
{
    return m_torDataDir;
}

QString TorManager::getTorExecutablePath() const
{
    return m_torExecutablePath;
}

void TorManager::onTorProcessStarted()
{
    qDebug() << "TOR process started";
    
    QMutexLocker locker(&m_mutex);
    m_isTorRunning = true;
    emit torStatusChanged(true);
    emit torConnectionEstablished();
}

void TorManager::onTorProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    
    qDebug() << "TOR process finished with exit code:" << exitCode;
    
    QMutexLocker locker(&m_mutex);
    m_isTorRunning = false;
    emit torStatusChanged(false);
    emit torConnectionLost();
    
    // Attempt to restart if this wasn't intentional shutdown
    // In a real implementation, we'd track if shutdown was intentional
    static int restartAttempts = 0;
    if (restartAttempts < m_maxRetries) {
        qDebug() << "Attempting to restart TOR (attempt" << (restartAttempts + 1) << ")";
        restartAttempts++;
        QMetaObject::invokeMethod(this, &TorManager::startTor, Qt::QueuedConnection);
    } else {
        qWarning() << "Max restart attempts reached, not restarting TOR";
    }
}

void TorManager::onTorProcessErrorOccurred(QProcess::ProcessError error)
{
    qWarning() << "TOR process error occurred:" << error;
    
    switch (error) {
    case QProcess::FailedToStart:
        qWarning() << "TOR failed to start - check if executable exists and has proper permissions";
        break;
    case QProcess::Crashed:
        qWarning() << "TOR crashed during execution";
        break;
    case QProcess::Timedout:
        qWarning() << "TOR process timed out";
        break;
    case QProcess::WriteError:
        qWarning() << "Error writing to TOR process";
        break;
    case QProcess::ReadError:
        qWarning() << "Error reading from TOR process";
        break;
    case QProcess::UnknownError:
        qWarning() << "Unknown error with TOR process";
        break;
    }
    
    QMutexLocker locker(&m_mutex);
    m_isTorRunning = false;
    emit torStatusChanged(false);
    emit torConnectionLost();
}

void TorManager::onTorOutputReady()
{
    if (!m_torProcess) return;
    
    QByteArray output = m_torProcess->readAllStandardOutput();
    handleTorOutput(output);
}

void TorManager::onTorErrorReady()
{
    if (!m_torProcess) return;
    
    QByteArray error = m_torProcess->readAllStandardError();
    handleTorError(error);
}

void TorManager::setupTorConfiguration()
{
    // Set default configuration values
    m_socksPort = 9050;
    m_controlPort = 9051;
    m_maxRetries = 3;
    m_retryDelayMs = 5000;
    m_healthCheckIntervalMs = 30000;
}

void TorManager::setupTorDirectories()
{
    // Set up data directory for TOR
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_torDataDir = appDataPath + "/tor_data";
    
    QDir dir;
    if (!dir.mkpath(m_torDataDir)) {
        qWarning() << "Failed to create TOR data directory:" << m_torDataDir;
    }
    
    // Set up GeoIP file path
    m_geoIpFilePath = appDataPath + "/geoip";
    
    // Create a dummy geoip file if it doesn't exist
    QFile geoIpFile(m_geoIpFilePath);
    if (!geoIpFile.exists()) {
        geoIpFile.open(QIODevice::WriteOnly);
        geoIpFile.write("127.0.0.1/8 US\r\n"); // Dummy entry
        geoIpFile.close();
    }
}

void TorManager::findTorExecutable()
{
    // Look for TOR executable in common locations
    QStringList possiblePaths = {
        QCoreApplication::applicationDirPath() + "/tor",
        "/usr/bin/tor",
        "/usr/local/bin/tor",
        "/opt/homebrew/bin/tor",  // macOS Homebrew
        "C:/Program Files/Tor Browser/Browser/TorBrowser/Tor/tor.exe",  // Windows
        "/Applications/Tor Browser.app/Contents/MacOS/Tor/tor.real"     // macOS Tor Browser
    };
    
    for (const QString &path : possiblePaths) {
        if (QFileInfo::exists(path) && QFileInfo(path).isExecutable()) {
            m_torExecutablePath = path;
            qDebug() << "Found TOR executable at:" << path;
            return;
        }
    }
    
    // If not found, log warning
    qWarning() << "TOR executable not found in standard locations";
    qWarning() << "Please install TOR or place the executable in the application directory";
}

void TorManager::setupSocksProxy()
{
    // Configure Qt to use SOCKS proxy for network requests
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::Socks5Proxy);
    proxy.setHostName("127.0.0.1");
    proxy.setPort(static_cast<quint16>(m_socksPort));
    
    QNetworkProxy::setApplicationProxy(proxy);
    qDebug() << "SOCKS proxy configured on port" << m_socksPort;
}

void TorManager::monitorTorHealth()
{
    // Check if TOR process is still running
    if (m_torProcess && m_torProcess->state() == QProcess::Running) {
        // Optionally send a simple command to TOR to verify it's responsive
        // For now, just check if process is running
        if (!m_isTorRunning) {
            // TOR is running but our status says it's not - update status
            QMutexLocker locker(&m_mutex);
            m_isTorRunning = true;
            emit torStatusChanged(true);
        }
    } else if (m_isTorRunning) {
        // TOR should be running but it's not - update status
        QMutexLocker locker(&m_mutex);
        m_isTorRunning = false;
        emit torStatusChanged(false);
        emit torConnectionLost();
    }
}

void TorManager::handleTorOutput(const QByteArray &output)
{
    if (output.isEmpty()) return;
    
    // Parse TOR output for important events
    QString outputStr = QString::fromUtf8(output);
    QStringList lines = outputStr.split('\n', Qt::SkipEmptyParts);
    
    for (const QString &line : lines) {
        qDebug() << "[TOR OUTPUT]" << line;
        
        // Look for indicators of successful startup
        if (line.contains("Bootstrapped 100%")) {
            qDebug() << "TOR bootstrap complete";
            // Could emit a signal here if needed
        }
    }
}

void TorManager::handleTorError(const QByteArray &error)
{
    if (error.isEmpty()) return;
    
    QString errorStr = QString::fromUtf8(error);
    QStringList lines = errorStr.split('\n', Qt::SkipEmptyParts);
    
    for (const QString &line : lines) {
        qWarning() << "[TOR ERROR]" << line;
    }
}