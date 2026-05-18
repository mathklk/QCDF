#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _settingsDialog = new SettingsDialog();
    connect(ui->actionSettings, &QAction::triggered, _settingsDialog, &QWidget::show);

    connect(_settingsDialog, &SettingsDialog::changed, this, &MainWindow::reconnectNodes);

    reconnectNodes();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::reconnectNodes()
{
    for (Node *const node : _nodes) {
        delete node;
    }
    _nodes.clear();
    if (_collector != nullptr) {
        delete _collector;
    }

    struct NodeInit {
        QTextEdit* terminal;
        QProgressBar* progressBar;
        QLineEdit* lineEdit;
    };

    QVector<NodeInit> const nodeInit = {
        {ui->textEdit_1, ui->progressBar_1, ui->lineEdit_1},
        {ui->textEdit_2, ui->progressBar_2, ui->lineEdit_2},
        {ui->textEdit_3, ui->progressBar_3, ui->lineEdit_3},
        {ui->textEdit_4, ui->progressBar_4, ui->lineEdit_4}
    };

    QStringList const serialPorts = _settingsDialog->settings().serialPorts;
    if (serialPorts.size() != 4) {
        return;
    }
    for (int i = 0; i < 4; ++i) {
        _nodes << new Node(
            this,
            serialPorts[i],
            nodeInit[i].terminal,
            nodeInit[i].lineEdit,
            nodeInit[i].progressBar
        );
        if (not _nodes.last()->open()) {
            qWarning() << "Failed to open" << serialPorts[i];
        }
    }
    _collector = new Collector(
        { _nodes[0], _nodes[1], _nodes[2] },
        _nodes[3]
    );
}