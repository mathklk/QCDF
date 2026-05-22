#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QListWidgetItem>
#include <QComboBox>

#include "recorder.h"
#include "node.h"
#include "collector.h"
#include "settingsdialog.h"

#include "music.h"
#include "gr_doa.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Recorder *const, QVector<Node*> const&, Collector* const);
    ~MainWindow() override;

signals:
    void reconnectNodes();
    void bytesToNode0(QByteArray);
    void bytesToNode1(QByteArray);
    void bytesToNode2(QByteArray);
    void bytesToNode3(QByteArray);

private:
    void settingsChanged();
    void loadFiles(QStringList const&);
    void calc();

private:
    class ListItem: public QListWidgetItem {
    public:
        ListItem(
            int id_,
            QString name_,
            QVector<ComplexList> data_,
            QString const& text,
            QListWidget* parent = nullptr
        ):
            QListWidgetItem(text, parent),
            id(id_),
            name(name_),
            data(data_)
        {}
        int id = -1;
        QString name;
        QVector<ComplexList> data;
    };

private:
    Ui::MainWindow *ui;

    Recorder* _recorder;
    QVector<Node*> _nodes;
    Collector* _collector;

    struct NodeInit {
        QTextEdit* terminal;
        QProgressBar* progressBar;
        QLineEdit* lineEdit;
    };
    QVector<NodeInit> _nodeInit;

    SettingsDialog* _settingsDialog;
    QComboBox* _comboBoxBatchChartType;

    struct CacheEntry {
        QString name;
        QVector<ComplexList> collection;
        bool hasPong;
        struct {
            Spectrum spectrum;
            int peakAngle;
            double min;
            double max;
        } music;
        struct {
            double alpha01;
            double alpha12;
            double alpha02;
            double alphaMean;
        } pdoa;
    };
    QMap<QString, CacheEntry> _cache;
};
#endif // MAINWINDOW_H
