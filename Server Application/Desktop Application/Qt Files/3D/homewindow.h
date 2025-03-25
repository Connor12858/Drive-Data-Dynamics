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

    void on_kickButton_clicked();

    void SettingsSetup();
    void SaveSettings();

    void on_portInputBox_valueChanged(int value);

    void on_connectionSaveButton_clicked();

    void on_timeoutInputBox_valueChanged(int value);

private:
    Ui::HomeWindow *ui;
    PythonProcess *networkListener;
    QTimer *updateTimer;

    QMap<QString, QString> configMap;
    QStringList configLines;
};
#endif // HOMEWINDOW_H
