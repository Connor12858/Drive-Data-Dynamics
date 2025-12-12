#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <QTimer>
#include <QSqlTableModel>
#include <QFileSystemWatcher>

#include "pythonprocess.h"
#include "DatabaseManager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class HomeWindow;
}
QT_END_NAMESPACE

class HomeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HomeWindow(QWidget *parent = nullptr);
    ~HomeWindow();

private slots:
    // Button Actions
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_kickButton_clicked();
    void on_connectionSaveButton_clicked();

    // Input Value Change
    void on_portInputBox_valueChanged(int value);
    void on_timeoutInputBox_valueChanged(int value);

    // Settings
    void SettingsSetup();
    void SaveSettings();

    // Periodic Updates
    void updateStatus();
    void updateConnections();
    void updateProcess();

    // Directory + File Handling
    void fetchLogFiles();
    void onDirectoryChanged(const QString &path);
    QStringList getAllSubdirectories(const QString &path);

    // Table UI
    void setupTableView();

private:
    void setupTimers();
    void setupFileWatcher(const QString &watchFolderPath);

    Ui::HomeWindow *ui;
    QString buildPath;
    PythonProcess *networkListener;
    DatabaseManager *dbManager;
    QSqlTableModel *dbModel;
    QFileSystemWatcher *fileWatcher;

    QMap<QString, QString> configMap;
    QStringList configLines;
    QSet<QString> seenFilepaths;
};

#endif // HOMEWINDOW_H
