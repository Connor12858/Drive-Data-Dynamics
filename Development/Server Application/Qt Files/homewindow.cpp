#include "homewindow.h"
#include "ui_homewindow.h"
#include <QFile>
#include <QSqlTableModel>
#include <QFileSystemWatcher>
#include <QDirIterator>
#include <QSet>

HomeWindow::HomeWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::HomeWindow),
      buildPath(QCoreApplication::applicationDirPath()),
      networkListener(new PythonProcess(buildPath + "/../libs/listener.py", this)),
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
    dbManager->openDatabase(QDir::currentPath() + "/../logs/can_logs.db");

    // Watch folders
    QString watchFolderPath = QDir::currentPath() + "/../logs/can_logs";
    QStringList allDirs = getAllSubdirectories(watchFolderPath);
    for (const QString &dir : allDirs)
        if (!fileWatcher->directories().contains(dir))
        {
            fileWatcher->addPath(dir);
            qDebug() << "Watching directory:" << dir;
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
    while (query.next())
        seenFilepaths.insert(query.value(0).toString());
}

HomeWindow::~HomeWindow()
{
    networkListener->stopProcess();
    delete ui;
}

/*
 * Below are the functions for interactivity
 */
// Start button
void HomeWindow::on_startButton_clicked()
{
    ui->startButton->setDisabled(true);
    networkListener->startProcess();
    ui->stopButton->setDisabled(false);
    ui->kickButton->setDisabled(false);
}
// Stop Button
void HomeWindow::on_stopButton_clicked()
{
    ui->startButton->setDisabled(false);
    networkListener->sendCommand("kick all");
    networkListener->stopProcess();
    ui->stopButton->setDisabled(true);
    ui->kickButton->setDisabled(true);
}

void HomeWindow::on_kickButton_clicked()
{
    networkListener->sendCommand("kick all");
}

void HomeWindow::on_connectionSaveButton_clicked()
{
    SaveSettings();
}

void HomeWindow::on_portInputBox_valueChanged(int value)
{
    configMap["PORT"] = QString::number(value);
}

void HomeWindow::on_timeoutInputBox_valueChanged(int value)
{
    configMap["INACTIVITY_TIMEOUT"] = QString::number(value);
}

/*
 *Functions for the timers
 */
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
    QFile inputFile(buildPath + "/../config/connections.ini");

    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString connectionText = in.readLine().remove(QChar('(')).remove(QChar(')')).remove(QChar('\'')).replace(", ", ":");
            ui->connectionsList->addItem(connectionText);
        }
        inputFile.close();
    }
}

void HomeWindow::updateProcess() {
    if (!this->networkListener->isProcessRunning()) {
        ui->startButton->setDisabled(false);
        ui->stopButton->setDisabled(true);
        ui->kickButton->setDisabled(true);
    }
}

/*
 * Configurations functions
 */
void HomeWindow::SettingsSetup()
{
    QFile configFile(buildPath + "/../config/config.ini");
    configMap.clear();
    configLines.clear();

    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&configFile);
        while (!in.atEnd())
        {
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
    QFile configFile(buildPath + "/../config/config.ini");

    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        QTextStream out(&configFile);
        for (QString &line : configLines)
        {
            QString trimmedLine = line.trimmed();
            if (!trimmedLine.isEmpty() && !trimmedLine.startsWith("#") && trimmedLine.contains("="))
            {
                QStringList parts = trimmedLine.split("=", Qt::SkipEmptyParts);
                if (parts.size() == 2)
                {
                    QString key = parts[0].trimmed();
                    if (configMap.contains(key))
                        line = key + "=" + configMap[key];
                }
            }
            out << line << "\n";
        }
        configFile.close();
    }
    else
        qDebug() << "Failed to open config file for writing.";
}

/*
 * Database and file management
 */
void HomeWindow::fetchLogFiles()
{
    dbModel->select();
}

void HomeWindow::onDirectoryChanged(const QString &path)
{
    qDebug() << "Directory changed:" << path;

    QDirIterator dirIt(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (dirIt.hasNext())
    {
        QString subdir = dirIt.next();
        if (!fileWatcher->directories().contains(subdir))
        {
            fileWatcher->addPath(subdir);
            qDebug() << "Added new watch directory:" << subdir;
        }
    }

    QDirIterator fileIt(path, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (fileIt.hasNext())
    {
        QFileInfo fileInfo(fileIt.next());
        QString filepath = fileInfo.absoluteFilePath();
        QString filename = fileInfo.completeBaseName();
        QStringList parts = filename.split("__");

        QString clientName = fileInfo.dir().dirName();
        QString eventName = "NewEvent";

        if (parts.size() >= 2)
        {
            eventName = parts[0];
            filename = parts.mid(1).join("__");
        }

        if (!seenFilepaths.contains(filepath))
        {
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

/*
 * Assist with getting the subdirectories
 */
QStringList HomeWindow::getAllSubdirectories(const QString &path)
{
    QStringList directories;
    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    directories << path;

    while (it.hasNext())
        directories << it.next();

    return directories;
}
