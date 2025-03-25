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

