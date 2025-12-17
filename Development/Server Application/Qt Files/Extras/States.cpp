#include "States.h"

void States::WindowState(Ui::HomeWindow *ui, bool enabled) {
    ui->startButton->setDisabled(enabled);
    ui->stopButton->setDisabled(!enabled);
    ui->kickButton->setDisabled(!enabled);
}
