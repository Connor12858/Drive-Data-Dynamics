#include "homemenu.h"
#include "ui_homemenu.h"
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>

// Constants
const QString CONNECTIONPY = "/../libs/connection.py";
const QString CONFIGINI = "/../config/config.ini";

// Constructor
HomeMenu::HomeMenu(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::HomeMenu),
      buildPath(QCoreApplication::applicationDirPath()),
      networkConnection(new PythonProcess(buildPath + CONNECTIONPY, this)),
      updateTimer(new QTimer(this))
{
    ui->setupUi(this);

    SettingsSetup();

    connect(updateTimer, &QTimer::timeout, this, &HomeMenu::updateStatus);
    updateTimer->start(1000);
}

HomeMenu::~HomeMenu()
{
    networkConnection->stopProcess();
    delete ui;
}

// Setup the settings to match the config file
void HomeMenu::SettingsSetup()
{
    QFile configFile(buildPath + CONFIGINI);
    configMap.clear();
    configLines.clear();

    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&configFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            configLines.append(line);

            // Only check 'valid' lines
            if (line.trimmed().isEmpty() || line.startsWith("#") || !line.contains("="))
                continue;

            QStringList parts = line.split("=", Qt::SkipEmptyParts);
            if (parts.size() == 2) {
                QString key = parts[0].trimmed();
                QString value = parts[1].trimmed();
                configMap[key] = value;
            }
        }
        configFile.close();
    }

    // What we do need set it to match, or use default value
    ui->hostInputBox->setText(configMap.value("HOST", "localhost"));
    ui->portInputBox->setValue(configMap.value("PORT", "5000").toInt());
    ui->clientInputBox->setText(configMap.value("NAME", "User"));
}

// BUTTON FUNCTIONS
void HomeMenu::on_startButton_clicked()
{
    networkConnection->startProcess();
    States::WindowState(ui, true);
}
void HomeMenu::on_stopButton_clicked()
{
    networkConnection->sendCommand("disconnect");
    networkConnection->stopProcess();
    States::WindowState(ui, false);
}
void HomeMenu::on_connectionSaveButton_clicked()
{
    SaveSettings();
}
void HomeMenu::on_fileSelectButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "Select a File", QCoreApplication::applicationDirPath(), "Log Files (*.log);;All Files (*)");

    if (!filePath.isEmpty()) {
        ui->filePathInputBox->setText(filePath);
        ui->fileNameInputBox->setText(QFileInfo(filePath).fileName());
    }
}
void HomeMenu::on_fileSendButton_clicked()
{
    networkConnection->sendCommand("file");
    qDebug() << "Sending file data...";

    networkConnection->sendCommand(ui->fileNameInputBox->text());
    networkConnection->sendCommand(ui->clientInputBox->text());
    networkConnection->sendCommand(ui->eventNameInputBox->text());

    QFile file(ui->filePathInputBox->text());
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray fileData = file.readAll();
        networkConnection->sendCommand(QString::number(fileData.size()));
        networkConnection->sendByteArray(fileData);
        file.close();
    }
}

// TEXT CHANGED
void HomeMenu::on_hostInputBox_textChanged(const QString &value)
{
    configMap["HOST"] = value;
}
void HomeMenu::on_portInputBox_textChanged(const QString &value)
{
    configMap["PORT"] = value;
}
void HomeMenu::on_clientInputBox_textChanged(const QString &value)
{
    configMap["NAME"] = value;
}

// Timer function that happens every second
void HomeMenu::updateStatus()
{
    QPixmap on(":Images/on.png");
    QPixmap off(":Images/off.png");
    QPixmap waiting(":Images/waiting.png");

    if (networkConnection->isProcessRunning()) {
        if (networkConnection -> isGood()) {
            ui->connectionStatusLight->setPixmap(on);
        } else {
            ui->connectionStatusLight->setPixmap(waiting);
        }
    } else {
        ui->connectionStatusLight->setPixmap(off);
        networkConnection->setToGood(false);
        States::WindowState(ui, false);
    }
}

// Save Settings, not in button for use later on
void HomeMenu::SaveSettings()
{
    QFile configFile(buildPath + CONFIGINI);

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
