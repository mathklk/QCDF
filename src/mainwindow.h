#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "node.h"
#include "collector.h"
#include "settingsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void reconnectNodes();

private:
    Ui::MainWindow *ui;

    QVector<Node*> _nodes;
    Collector* _collector = nullptr;

    SettingsDialog* _settingsDialog;
};
#endif // MAINWINDOW_H
