#include "homewindow.h"
#include "ui_homewindow.h"


HomeWindow::HomeWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::HomeWindow)
{
    ui->setupUi(this);
}

HomeWindow::~HomeWindow()
{
    delete ui;
}

void HomeWindow::on_pushButton_9_clicked()
{

}

// Only allow one of the buttons pressable, and can only kick when turned on
// This will help prevent issues
void HomeWindow::on_startButton_clicked()
{
    ui->startButton->setDisabled(true);
    ui->stopButton->setDisabled(false);
    ui->kickButton->setDisabled(false);
}
void HomeWindow::on_stopButton_clicked()
{
    ui->startButton->setDisabled(false);
    ui->stopButton->setDisabled(true);
    ui->kickButton->setDisabled(true);
}

