#include "homewindow.h"
#include "ui_homewindow.h"
#include <QFile>

HomeWindow::HomeWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HomeWindow)
    , networkListener(new PythonProcess("../../../Network Connection/listener.py", this))
    , updateTimer(new QTimer(this))
{
    ui->setupUi(this);

    // Setup the settings
    SettingsSetup();

    // Connect QTimer to update the UI every second
    connect(updateTimer, &QTimer::timeout, this, &HomeWindow::updateStatus);
    updateTimer->start(3000);  // Check every 3 seconds

    // Connect QTimer to update the UI every second
    connect(updateTimer, &QTimer::timeout, this, &HomeWindow::updateConnections);
    updateTimer->start(1000);  // Check every second
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

void HomeWindow::updateStatus() {
    QPixmap on(":Images/on.png");
    QPixmap off(":Images/off.png");

    // Connection Status
    if (networkListener->isProcessRunning()) {
        ui->connectionStatusLight->setPixmap(on);
    } else {
        ui->connectionStatusLight->setPixmap(off);
    }
}

void HomeWindow::updateConnections() {

    ui->connectionsList->clear();
    QFile inputFile("D:\\Drive-Data-Dynamics\\Server Application\\Network Connection\\connections.ini");
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {

            QString connectionText = in.readLine().remove(QChar('(')).remove(QChar(')')).remove(QChar('\'')).replace(", ", ":");
            ui->connectionsList->addItem(connectionText);
            //qDebug() << in.readLine();
        }
        inputFile.close();
    }
}

void HomeWindow::on_kickButton_clicked()
{
    networkListener->sendCommand("kick all");
}

void HomeWindow::SettingsSetup() {
    //Open the file
    QFile configFile("D:\\Drive-Data-Dynamics\\Server Application\\Network Connection\\config.ini");

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
    ui->timeoutInputBox->setValue(configMap.value("INACTIVITY_TIMEOUT", "3").toInt());
}

// Save the updated settings while preserving the original structure
void HomeWindow::SaveSettings() {
    QFile configFile("D:\\Drive-Data-Dynamics\\Server Application\\Network Connection\\config.ini");

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

