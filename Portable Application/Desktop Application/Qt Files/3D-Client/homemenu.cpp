#include "homemenu.h"
#include <QFile>
#include "ui_homemenu.h"

HomeMenu::HomeMenu(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HomeMenu)
    , buildPath(QCoreApplication::applicationDirPath())
    , networkConnection(new PythonProcess(buildPath + "/../python_files/connection.py", this))
    , updateTimer(new QTimer(this))
{
    ui->setupUi(this);

    // Setup the settings
    SettingsSetup();

    // Connect QTimer to update the UI every second
    connect(updateTimer, &QTimer::timeout, this, &HomeMenu::updateStatus);
    updateTimer->start(3000);  // Check every 3 seconds
}

HomeMenu::~HomeMenu()
{
    networkConnection->stopProcess();
    delete ui;
}

// Only allow one of the buttons pressable, and can only kick when turned on
// This will help prevent issues
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

void HomeMenu::updateStatus() {
    QPixmap on(":Images/on.png");
    QPixmap off(":Images/off.png");

    // Connection Status
    if (networkConnection->isProcessRunning()) {
        ui->connectionStatusLight->setPixmap(on);
    } else {
        ui->connectionStatusLight->setPixmap(off);
    }
}


void HomeMenu::SettingsSetup() {
    //Open the file
    QFile configFile(buildPath + "/../config/config.ini");

    configMap.clear();
    configLines.clear();

    // Map the config
    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&configFile);

        while (!in.atEnd()) {
            QString line = in.readLine();
            configLines.append(line);

            // Ignore empty lines or comments
            if (line.trimmed().isEmpty() || line.startsWith("#") || !line.contains("="))
                continue;

            // Split into key and value
            QStringList parts = line.split("=", Qt::SkipEmptyParts);
            if (parts.size() == 2) {
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
}

// Save the updated settings while preserving the original structure
void HomeMenu::SaveSettings() {
    QFile configFile(buildPath + "/../config/config.ini");

    // Save the updated key value
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&configFile);

        for (QString &line : configLines) {
            QString trimmedLine = line.trimmed();

            // If it's a key=value pair, update it
            if (!trimmedLine.isEmpty() && !trimmedLine.startsWith("#") && trimmedLine.contains("=")) {
                QStringList parts = trimmedLine.split("=", Qt::SkipEmptyParts);
                if (parts.size() == 2) {
                    QString key = parts[0].trimmed();
                    if (configMap.contains(key)) {
                        line = key + "=" + configMap[key];  // Replace with new value
                    }
                }
            }
            out << line << "\n";  // Write the (updated) line
        }

        configFile.close();
    } else {
        qDebug() << "Failed to open config file for writing.";
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

