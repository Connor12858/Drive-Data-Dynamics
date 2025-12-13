#include "homemenu.h"
#include "ui_homemenu.h"
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>

HomeMenu::HomeMenu(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::HomeMenu),
      buildPath(QCoreApplication::applicationDirPath()),
      networkConnection(new PythonProcess(buildPath + "/../libs/connection.py", this)),
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

void HomeMenu::on_startButton_clicked()
{
    ui->startButton->setDisabled(true);
    networkConnection->startProcess();
    ui->stopButton->setDisabled(false);
}

void HomeMenu::on_stopButton_clicked()
{
    ui->startButton->setDisabled(false);
    networkConnection->sendCommand("disconnect");
    networkConnection->stopProcess();
    ui->stopButton->setDisabled(true);
}

void HomeMenu::on_connectionSaveButton_clicked()
{
    SaveSettings();
}

void HomeMenu::on_fileSelectButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "Select a File", QCoreApplication::applicationDirPath(), "Log Files (*.log);;All Files (*)");

    if (!filePath.isEmpty())
    {
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
    qDebug() << ui->clientInputBox->text();
    networkConnection->sendCommand(ui->eventNameInputBox->text());


    QFile file(ui->filePathInputBox->text());
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray fileData = file.readAll();
        networkConnection->sendCommand(QString::number(fileData.size()));
        networkConnection->sendByteArray(fileData);
        file.close();
    }
}

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

void HomeMenu::updateStatus()
{
    QPixmap on(":Images/on.png");
    QPixmap off(":Images/off.png");

    if (networkConnection->isProcessRunning())
    {
        ui->connectionStatusLight->setPixmap(on);
    }
    else
    {
        ui->connectionStatusLight->setPixmap(off);
        ui->stopButton->setDisabled(true);
        ui->startButton->setDisabled(false);
    }
}

void HomeMenu::SettingsSetup()
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
            {
                QString key = parts[0].trimmed();
                QString value = parts[1].trimmed();
                configMap[key] = value;
            }
        }
        configFile.close();
    }

    ui->hostInputBox->setText(configMap.value("HOST", "localhost"));
    ui->portInputBox->setValue(configMap.value("PORT", "5000").toInt());
    ui->clientInputBox->setText(configMap.value("NAME", "User"));
}

void HomeMenu::SaveSettings()
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
                    {
                        line = key + "=" + configMap[key];
                    }
                }
            }
            out << line << "\n";
        }

        configFile.close();
    }
    else
    {
        qDebug() << "Failed to open config file for writing.";
    }
}
