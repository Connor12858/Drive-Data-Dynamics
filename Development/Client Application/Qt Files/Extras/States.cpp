#include "States.h"

void States::WindowState(Ui::HomeMenu *ui, bool enabled) {
    ui->startButton->setDisabled(enabled);
    ui->stopButton->setDisabled(!enabled);
}
