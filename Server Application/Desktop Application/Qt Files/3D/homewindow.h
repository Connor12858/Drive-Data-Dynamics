#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <pythonprocess.h>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class HomeWindow;
}
QT_END_NAMESPACE

class HomeWindow : public QMainWindow
{
    Q_OBJECT

public:
    HomeWindow(QWidget *parent = nullptr);
    ~HomeWindow();

private slots:
    void on_startButton_clicked();

    void on_stopButton_clicked();

    void updateStatus();
    void updateConnections();

private:
    Ui::HomeWindow *ui;
    PythonProcess *networkListener;
    QTimer *updateTimer;
};
#endif // HOMEWINDOW_H
