#ifndef HOMEMENU_H
#define HOMEMENU_H

#include <QMainWindow>
#include <QProcess>
#include <pythonprocess.h>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class HomeMenu;
}
QT_END_NAMESPACE

class HomeMenu : public QMainWindow
{
    Q_OBJECT

public:
    HomeMenu(QWidget *parent = nullptr);
    ~HomeMenu();

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();

    void on_connectionSaveButton_clicked();

    void updateStatus();

    void SettingsSetup();
    void SaveSettings();

    void on_hostInputBox_textChanged(const QString &value);
    void on_portInputBox_textChanged(const QString &value);
    void on_clientInputBox_textChanged(const QString &value);

    void on_fileSelectButton_clicked();
    void on_fileSendButton_clicked();

private:
    Ui::HomeMenu *ui;
    QString buildPath;
    PythonProcess *networkConnection;
    QTimer *updateTimer;

    QMap<QString, QString> configMap;
    QStringList configLines;

    QString client_name;
};
#endif // HOMEMENU_H
