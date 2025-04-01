#include "homewindow.h"
#include "ui_homewindow.h"
#include <QFile>
#include <QSqlTableModel>
#include <QFileSystemWatcher>

HomeWindow::HomeWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::HomeWindow), buildPath(QCoreApplication::applicationDirPath()), networkListener(new PythonProcess(buildPath + "/../python_files/listener.py", this)), updateTimer(new QTimer(this)),
    fileWatcher(new QFileSystemWatcher(this))
{
    ui->setupUi(this);

    // Setup the settings
    SettingsSetup();

    // Connect QTimer to update the UI every second
    connect(updateTimer, &QTimer::timeout, this, &HomeWindow::updateStatus);
    updateTimer->start(3000); // Check every 3 seconds

    // Connect QTimer to update the UI every second
    connect(updateTimer, &QTimer::timeout, this, &HomeWindow::updateConnections);
    updateTimer->start(1000); // Check every second

    // Initialize the DatabaseManager and open the database
    dbManager = new DatabaseManager(this);
    QString dbPath = QDir::currentPath() + "/can_logs.db";  // Assuming the db is in the current directory
    dbManager->openDatabase(dbPath);

    // Path to monitor (modify based on your specific setup)
    QString watchFolderPath = QDir::currentPath() + "/../can_logs";  // The folder where your log files are uploaded

    // Set up QFileSystemWatcher
    if (!fileWatcher->directories().contains(watchFolderPath)) {
        fileWatcher->addPath(watchFolderPath);  // Watch this folder for changes
    }

    connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this, &HomeWindow::onDirectoryChanged);

    // Connect QTimer to update the UI every second
    connect(updateTimer, &QTimer::timeout, this, &HomeWindow::fetchLogFiles);
    updateTimer->start(10000); // Check every 10 second

    // Create a QSqlTableModel object
    dbModel = new QSqlTableModel(this);
    // Set up the table view to display the data
    setupTableView();

    fetchLogFiles();
}

HomeWindow::~HomeWindow()
{
    networkListener->stopProcess();
    delete ui;
}

// Only allow one of the buttons pressable, and can only kick when turned on
// This will help prevent issues
void HomeWindow::on_startButton_clicked()
{
    ui->startButton->setDisabled(true);

    networkListener->startProcess();

    ui->stopButton->setDisabled(false);
    ui->kickButton->setDisabled(false);
}
void HomeWindow::on_stopButton_clicked()
{
    ui->startButton->setDisabled(false);

    networkListener->sendCommand("kick all");
    networkListener->stopProcess();

    ui->stopButton->setDisabled(true);
    ui->kickButton->setDisabled(true);
}

// Kick all the users connected to the server
void HomeWindow::on_kickButton_clicked()
{
    networkListener->sendCommand("kick all");
}

// Change the settings
void HomeWindow::on_portInputBox_valueChanged(int value)
{
    configMap["PORT"] = QString::number(value);
}
void HomeWindow::on_timeoutInputBox_valueChanged(int value)
{
    configMap["INACTIVITY_TIMEOUT"] = QString::number(value);
}
void HomeWindow::on_connectionSaveButton_clicked()
{
    SaveSettings();
}

// Intervally update the status of the connection
void HomeWindow::updateStatus()
{
    QPixmap on(":Images/on.png");
    QPixmap off(":Images/off.png");

    // Connection Status
    if (networkListener->isProcessRunning())
    {
        ui->connectionStatusLight->setPixmap(on);
    }
    else
    {
        ui->connectionStatusLight->setPixmap(off);
    }

    // Database status
    if (dbManager->isOpen()) {
        ui->databaseStatusLight->setPixmap(on);
    } else {
        ui->databaseStatusLight->setPixmap(off);
    }
}

// Intervally update the list of the connections
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
            // qDebug() << in.readLine();
        }
        inputFile.close();
    }
}

// Setup the settings
void HomeWindow::SettingsSetup()
{
    // Open the file
    QFile configFile(buildPath + "/../config/config.ini");

    configMap.clear();
    configLines.clear();

    // Map the config
    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&configFile);

        while (!in.atEnd())
        {
            QString line = in.readLine();
            configLines.append(line);

            // Ignore empty lines or comments
            if (line.trimmed().isEmpty() || line.startsWith("#") || !line.contains("="))
                continue;

            // Split into key and value
            QStringList parts = line.split("=", Qt::SkipEmptyParts);
            if (parts.size() == 2)
            {
                QString key = parts[0].trimmed();
                QString value = parts[1].trimmed();

                configMap[key] = value;
            }
        }

        configFile.close();
    }

    // Assign values to variables
    ui->hostInputBox->setText(configMap.value("HOST", "localhost"));
    ui->portInputBox->setValue(configMap.value("PORT", "5000").toInt());
    ui->timeoutInputBox->setValue(configMap.value("INACTIVITY_TIMEOUT", "3").toInt());
}

// Save the updated settings while preserving the original structure
void HomeWindow::SaveSettings()
{
    QFile configFile(buildPath + "/../config/config.ini");

    // Save the updated key value
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        QTextStream out(&configFile);

        for (QString &line : configLines)
        {
            QString trimmedLine = line.trimmed();

            // If it's a key=value pair, update it
            if (!trimmedLine.isEmpty() && !trimmedLine.startsWith("#") && trimmedLine.contains("="))
            {
                QStringList parts = trimmedLine.split("=", Qt::SkipEmptyParts);
                if (parts.size() == 2)
                {
                    QString key = parts[0].trimmed();
                    if (configMap.contains(key))
                    {
                        line = key + "=" + configMap[key]; // Replace with new value
                    }
                }
            }
            out << line << "\n"; // Write the (updated) line
        }

        configFile.close();
    }
    else
    {
        qDebug() << "Failed to open config file for writing.";
    }
}

// Fetch the log files
void HomeWindow::fetchLogFiles()
{
    dbModel->select();
}

void HomeWindow::onDirectoryChanged(const QString &path)
{
    QDir dir(path);

    // Retrieve the list of all files in the folder
    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo &fileInfo : fileList) {
        QString filename = fileInfo.fileName();
        filename = filename.left(filename.lastIndexOf('.'));
        QString filepath = fileInfo.absoluteFilePath();

        QString clientName = "Unknown";
        QString eventName = "NewEvent";

        // Insert the log file metadata into the database
        dbManager->insertLogFile(filename, filepath, clientName, eventName);
    }
}

// In your HomeWindow constructor or initialization function
void HomeWindow::setupTableView() {
    // Set the model to connect to the table in the database
    dbModel->setTable("log_files");
    dbModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    dbModel->select();  // Fetch data from the database and load it into the model

    // Set the model to the table view
    ui->dbTableView->setModel(dbModel);
    ui->dbTableView->setColumnHidden(dbModel->fieldIndex("id"), true);
    ui->dbTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Resize the columns to fit the content
    ui->dbTableView->resizeColumnsToContents();

    // Set a fixed height for the rows (just for appearance)
    ui->dbTableView->setRowHeight(0, 30);
}
