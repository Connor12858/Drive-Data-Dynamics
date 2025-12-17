#include "homewindow.h"
#include "ui_homewindow.h"
#include <QFile>
#include <QSqlTableModel>
#include <QFileSystemWatcher>
#include <QDirIterator>
#include <QSet>

// Constants
const QString listenerpy = "/../libs/listener.py";
const QString logsdb = "/../logs/can_logs.db";
const QString canlogsfolder = "/../logs/can_logs";
const QString configini = "/../config/config.ini";
const QString connectionsini = "/../config/connections.ini";

HomeWindow::HomeWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::HomeWindow),
      buildPath(QCoreApplication::applicationDirPath()),
      networkListener(new PythonProcess(buildPath + listenerpy, this)),
      fileWatcher(new QFileSystemWatcher(this))
{
    ui->setupUi(this);
    SettingsSetup();

    // Timers
    QTimer *logsTimer = new QTimer(this);
    QTimer *updateTimer = new QTimer(this);

    // Connect the timers and start them
    connect(updateTimer, &QTimer::timeout, this, &HomeWindow::updateStatus);
    connect(updateTimer, &QTimer::timeout, this, &HomeWindow::updateConnections);
    connect(updateTimer, &QTimer::timeout, this, &HomeWindow::updateProcess);
    updateTimer->start(1000);

    // Create a connection to the data base
    dbManager = new DatabaseManager(this);
    dbManager->openDatabase(QDir::currentPath() + logsdb);

    // Watch folders
    QString watchFolderPath = QDir::currentPath() + canlogsfolder;
    QStringList allDirs = getAllSubdirectories(watchFolderPath);
    for (QString &dir : allDirs) {
        if (!fileWatcher->directories().contains(dir)) {
            fileWatcher->addPath(dir);
            qDebug() << "Watching directory:" << dir;
        }
    }

    connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this, &HomeWindow::onDirectoryChanged);

    // Logs fetcher
    connect(logsTimer, &QTimer::timeout, this, &HomeWindow::fetchLogFiles);
    logsTimer->start(10000);

    // Lets display the database
    dbModel = new QSqlTableModel(this);
    setupTableView();
    fetchLogFiles();

    QSqlQuery query("SELECT filepath FROM log_files");
    while (query.next()) {
        seenFilepaths.insert(query.value(0).toString());
    }
}

HomeWindow::~HomeWindow()
{
    networkListener->stopProcess();
    delete ui;
}

// BUTTON FUNCTIONS
void HomeWindow::on_startButton_clicked()
{
    networkListener->startProcess();
    States::WindowState(ui, true);
}
void HomeWindow::on_stopButton_clicked()
{
    networkListener->sendCommand("kick all");
    networkListener->stopProcess();
    States::WindowState(ui, false);
}
void HomeWindow::on_kickButton_clicked()
{
    networkListener->sendCommand("kick all");
}
void HomeWindow::on_connectionSaveButton_clicked()
{
    SaveSettings();
}

// VALUE CHANGES
void HomeWindow::on_portInputBox_valueChanged(int value)
{
    configMap["PORT"] = QString::number(value);
}

void HomeWindow::on_timeoutInputBox_valueChanged(int value)
{
    configMap["INACTIVITY_TIMEOUT"] = QString::number(value);
}

// FUNCTIONS FOR TIMERS
void HomeWindow::updateStatus()
{
    QPixmap on(":Images/on.png");
    QPixmap off(":Images/off.png");

    ui->connectionStatusLight->setPixmap(networkListener->isProcessRunning() ? on : off);
    ui->databaseStatusLight->setPixmap(dbManager->isOpen() ? on : off);
}
void HomeWindow::updateConnections()
{
    ui->connectionsList->clear();
    QFile inputFile(buildPath + connectionsini);

    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        while (!in.atEnd()) {
            QString connectionText = in.readLine().remove(QChar('(')).remove(QChar(')')).remove(QChar('\'')).replace(", ", ":");
            ui->connectionsList->addItem(connectionText);
        }
        inputFile.close();
    }
}
void HomeWindow::updateProcess() {
    if (!this->networkListener->isProcessRunning()) {
        States::WindowState(ui, false);
    }
}

// CONFIG FUNCTIONS
void HomeWindow::SettingsSetup()
{
    QFile configFile(buildPath + configini);
    configMap.clear();
    configLines.clear();

    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&configFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            configLines.append(line);

            if (line.trimmed().isEmpty() || line.startsWith("#") || !line.contains("="))
                continue;

            QStringList parts = line.split("=", Qt::SkipEmptyParts);
            if (parts.size() == 2)
                configMap[parts[0].trimmed()] = parts[1].trimmed();
        }
        configFile.close();
    }

    ui->hostInputBox->setText(configMap.value("HOST", "localhost"));
    ui->portInputBox->setValue(configMap.value("PORT", "5000").toInt());
    ui->timeoutInputBox->setValue(configMap.value("INACTIVITY_TIMEOUT", "3").toInt());
}
void HomeWindow::SaveSettings()
{
    QFile configFile(buildPath + configini);

    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&configFile);
        for (QString &line : configLines) {
            QString trimmedLine = line.trimmed();
            if (!trimmedLine.isEmpty() && !trimmedLine.startsWith("#") && trimmedLine.contains("=")) {
                QStringList parts = trimmedLine.split("=", Qt::SkipEmptyParts);
                if (parts.size() == 2) {
                    QString key = parts[0].trimmed();
                    if (configMap.contains(key))
                        line = key + "=" + configMap[key];
                }
            }
            out << line << "\n";
        }
        configFile.close();
    } else {
        qDebug() << "Failed to open config file for writing.";
    }
}

// DATABASE AND FILE FUNCTIONS
void HomeWindow::fetchLogFiles()
{
    dbModel->select();
}

void HomeWindow::onDirectoryChanged(const QString &path)
{
    qDebug() << "Directory changed:" << path;

    QDirIterator dirIt(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (dirIt.hasNext()) {
        QString subdir = dirIt.next();
        if (!fileWatcher->directories().contains(subdir)) {
            fileWatcher->addPath(subdir);
            qDebug() << "Added new watch directory:" << subdir;
        }
    }

    QDirIterator fileIt(path, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (fileIt.hasNext()) {
        QFileInfo fileInfo(fileIt.next());
        QString filepath = fileInfo.absoluteFilePath();
        QString filename = fileInfo.completeBaseName();
        QStringList parts = filename.split("__");

        QString clientName   = fileInfo.dir().dirName();
        QString eventName = "NewEvent";

        if (parts.size() >= 2) {
            eventName = parts[0];
            filename = parts.mid(1).join("__");
        }

        if (!seenFilepaths.contains(filepath)) {
            seenFilepaths.insert(filepath);
            dbManager->insertLogFile(filename, filepath, clientName, eventName);
        }
    }
}
void HomeWindow::setupTableView()
{
    dbModel->setTable("log_files");
    dbModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    dbModel->select();

    ui->dbTableView->setModel(dbModel);
    ui->dbTableView->setColumnHidden(dbModel->fieldIndex("id"), true);
    ui->dbTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->dbTableView->resizeColumnsToContents();
    ui->dbTableView->setRowHeight(0, 30);
}

// HELPERS
QStringList HomeWindow::getAllSubdirectories(const QString &path)
{
    QStringList directories;
    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    directories << path;

    while (it.hasNext())
        directories << it.next();

    return directories;
}
