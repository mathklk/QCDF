#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "plot.h"
#include "polar.h"
#include "ivector.h"

#include <QFontDatabase>
#include <QFileDialog>
#include <QMessageBox>
#include <QPolarChart>
#include <QClipboard>

namespace brush {
    QBrush gray    (QColor::fromRgb(0x7f, 0x7f, 0x7f));
    QBrush black   (QColor::fromRgb(0x00, 0x00, 0x01));

    QBrush red     (QColor::fromRgb(191, 89, 62));
    QBrush green   (QColor::fromRgb(0x77, 0xdd, 0x77));
    QBrush blue    (QColor::fromRgb(23, 159, 223));

    QBrush orange  (QColor::fromRgb(255, 150, 0));
    QBrush cyan    (QColor::fromRgb(0, 255, 247));
    QBrush magenta (QColor::fromRgb(255, 0, 247));
};

constexpr int FRAME_SIZE = 7000;

MainWindow::MainWindow(Recorder *const core, QVector<Node*> const& nodes, Collector *const collector):
    QMainWindow(nullptr),
    ui(new Ui::MainWindow),
    _recorder(core),
    _nodes(nodes),
    _collector(collector)
{
    ui->setupUi(this);
    setWindowTitle(_windowTitle);

    for (Node *const node : nodes) {
        connect(this, &MainWindow::reconnectNodes, node, &Node::reconnect);
    }
    connect(this, &MainWindow::bytesToNode0, nodes[0], &Node::write);
    connect(this, &MainWindow::bytesToNode1, nodes[1], &Node::write);
    connect(this, &MainWindow::bytesToNode2, nodes[2], &Node::write);
    connect(this, &MainWindow::bytesToNode3, nodes[3], &Node::write);

    // Settings
    _settingsDialog = new SettingsDialog();
    connect(ui->actionSettings, &QAction::triggered, _settingsDialog, &QWidget::show);
    connect(_settingsDialog, &SettingsDialog::changed, this, &MainWindow::settingsChanged);
    connect(_settingsDialog, &SettingsDialog::slidersChanged, this, &MainWindow::calc);

    // Terminals
    _nodeInit = {
        {ui->textEdit_1, ui->progressBar_1, ui->lineEdit_1},
        {ui->textEdit_2, ui->progressBar_2, ui->lineEdit_2},
        {ui->textEdit_3, ui->progressBar_3, ui->lineEdit_3},
        {ui->textEdit_4, ui->progressBar_4, ui->lineEdit_4}
    };
    QFont monospaceFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monospaceFont.setPointSize(12);
    for (NodeInit const& ni : _nodeInit) {
        ni.terminal->setFont(monospaceFont);
        ni.terminal->setReadOnly(true);
        ni.terminal->setLineWrapMode(QTextEdit::NoWrap);
        ni.lineEdit->setFont(monospaceFont);
    }
    for (int i = 0; i < 4; ++i) {
        connect(_nodes[i], &Node::progress, this, [this, i](int v, int m){
            _nodeInit[i].progressBar->setMaximum(m);
            _nodeInit[i].progressBar->setValue(v);
        });
        connect(_nodes[i], &Node::signalNewChars, this, [this, i](QString const& str){
            int const LIMIT = 1000;
            QString cat = _nodeInit[i].terminal->toPlainText() + str;
            _nodeInit[i].terminal->setText(
                cat.sliced(qMax(0, cat.size() - LIMIT))
            );
            QTextCursor cursor = _nodeInit[i].terminal->textCursor();
            cursor.movePosition(QTextCursor::End);
            _nodeInit[i].terminal->setTextCursor(cursor);
        });
        connect(_nodes[i], &Node::active, this, [this, i](bool active){
            _nodeInit[i].lineEdit->setEnabled(active);
            _nodeInit[i].progressBar->setEnabled(active);
            _nodeInit[i].terminal->setEnabled(active);
        });
        connect(ui->pushButtonClearConsole, &QPushButton::clicked, _nodeInit[i].terminal, &QTextEdit::clear);
    }
    connect(_nodeInit[0].lineEdit, &QLineEdit::returnPressed, this, [this]() {
        QByteArray const bytes = _nodeInit[0].lineEdit->text().toLatin1();
        emit bytesToNode0(bytes);
        _nodeInit[0].lineEdit->clear();
    });
    connect(_nodeInit[1].lineEdit, &QLineEdit::returnPressed, this, [this]() {
        QByteArray const bytes = _nodeInit[1].lineEdit->text().toLatin1();
        emit bytesToNode1(bytes);
        _nodeInit[1].lineEdit->clear();
    });
    connect(_nodeInit[2].lineEdit, &QLineEdit::returnPressed, this, [this]() {
        QByteArray const bytes = _nodeInit[2].lineEdit->text().toLatin1();
        emit bytesToNode2(bytes);
        _nodeInit[2].lineEdit->clear();
    });
    connect(_nodeInit[3].lineEdit, &QLineEdit::returnPressed, this, [this]() {
        QByteArray const bytes = _nodeInit[3].lineEdit->text().toLatin1();
        emit bytesToNode3(bytes);
        _nodeInit[3].lineEdit->clear();
    });


    // Broadcast Line
    connect(ui->lineEditBroadcast, &QLineEdit::returnPressed, this, [this](){
        QByteArray const bytes = ui->lineEditBroadcast->text().toLatin1();
        emit bytesToNode0(bytes);
        emit bytesToNode1(bytes);
        emit bytesToNode2(bytes);
        emit bytesToNode3(bytes);
        ui->lineEditBroadcast->clear();
    });

    // Recorder
    connect(_recorder, &Recorder::recordProgress, this, [this](int v, int m){
        ui->progressBarRecord->setMaximum(m);
        ui->progressBarRecord->setValue(v);
        if (v >= m) {
            // Recorder is done and stopped by itself.
            ui->pushButtonStart->setEnabled(true);
            ui->pushButtonStop->setEnabled(false);
        }
    });
    connect(_recorder, &Recorder::data, this, [this](QVector<QVector<Frame>> data){
        ui->pushButtonSave->setEnabled(not data.empty());
        ui->pushButtonClearRecord->setEnabled(not data.empty());
        ui->listWidgetRecord->clear();
        for (auto const& collection : data) {
            QVector<ComplexList> cl;
            cl << collection[0].asComplex() << collection[1].asComplex() << collection[2].asComplex();
            auto item = new ListItem(
                collection[0].id,
                "Record " + QString::number(collection[0].id),
                cl,
                QString("[%1, %2, %3] %4").arg(
                    QString::number(collection[0].id),
                    QString::number(collection[1].id),
                    QString::number(collection[2].id),
                    (collection[0].crcIsOk and collection[1].crcIsOk and collection[2].crcIsOk) ? "" : "BAD CHECKSUM"
                ),
                ui->listWidgetRecord
            );
            item->setSelected(true);
        }
        ui->listWidgetRecord->scrollToBottom();
    });

    // Recording
    connect(ui->spinBoxRecordCount, &QSpinBox::valueChanged, ui->progressBarRecord, &QProgressBar::setMaximum);
    connect(ui->spinBoxRecordCount, &QSpinBox::valueChanged, _recorder, &Recorder::setRecordCount);
    connect(ui->spinBoxRecordTimer, &QSpinBox::valueChanged, _recorder, &Recorder::setRecordTimer);
    _recorder->setRecordCount(ui->spinBoxRecordCount->value());
    _recorder->setRecordTimer(ui->spinBoxRecordTimer->value());
    connect(ui->pushButtonStart, &QPushButton::clicked, this, [this](){
        ui->pushButtonStart->setEnabled(false);
        ui->pushButtonStop->setEnabled(true);
    });
    connect(ui->pushButtonStart, &QPushButton::clicked, _recorder, &Recorder::startRecording);
    connect(ui->pushButtonStop, &QPushButton::clicked, this, [this](){
        ui->pushButtonStop->setEnabled(false);
        ui->pushButtonStart->setEnabled(true);
        _collector->reset();
    });
    connect(ui->pushButtonStop, &QPushButton::clicked, this, &MainWindow::recorderStopRecording);
    connect(this, &MainWindow::recorderStopRecording, _recorder, &Recorder::stopRecording);
    connect(ui->pushButtonSave, &QPushButton::clicked, this, &MainWindow::saveClicked);
    connect(ui->listWidgetRecord, &QListWidget::itemSelectionChanged, this, [this](){
        ui->pushButtonSave->setEnabled(not ui->listWidgetRecord->selectedItems().isEmpty());
    });
    connect(ui->pushButtonClearRecord, &QPushButton::clicked, _recorder, &Recorder::clear);

    // Loading
    connect(ui->pushButtonLoadFiles, &QPushButton::clicked, this, [this]{
        QStringList const files = QFileDialog::getOpenFileNames(nullptr, "Select recordings to load");
        if (not files.isEmpty()) {
            putFilesIntoListWidget(files);
            setWindowTitle(_windowTitle);
        }
    });
    connect(ui->pushButtonLoadDir, &QPushButton::clicked, this, [this]{
        QString const directory = QFileDialog::getExistingDirectory(nullptr, "Select directory to load all files from");
        if (directory.isEmpty()) {
            return;
        }
        // List all json files in directory
        QStringList const files = QDir(directory).entryList({"*.json"}, QDir::Files);
        if (files.isEmpty()) {
            QMessageBox::warning(this, "Directory is empty", "Directory '" + directory + "' contains no JSON files.");
        } else {
            QStringList absoluteFiles;
            for (QString const& file : files) {
                absoluteFiles << QDir(directory).absoluteFilePath(file);
            }
            putFilesIntoListWidget(absoluteFiles);
            setWindowTitle(_windowTitle + " " + directory);
        }
    });
    connect(ui->pushButtonClearLoad, &QPushButton::clicked, this, [this](){
        ui->listWidgetLoad->clear();
        setWindowTitle(_windowTitle);
    });

    // Calculation
    _comboBoxBatchChartType = new QComboBox(this);
    _comboBoxBatchChartType->addItems({
        "MUSIC Cartesian Seperate",
        "MUSIC Cartesian Sum",
        "MUSIC Polar Seperate",
        "MUSIC Polar Sum",
        "PDOA Polar",
        "Amplitude",
        "Complex IQ",
        "Reference Phases",
        "MUSIC Peaks over time",
        "PDOA over time",
        "MUSIC Spectrogram"
    });
    ui->chartWidget->addToolWidget(_comboBoxBatchChartType);
    connect(_comboBoxBatchChartType, &QComboBox::currentTextChanged, this, &MainWindow::calc);
    connect(_settingsDialog, &SettingsDialog::changed, this, &MainWindow::calc);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::calc);
    connect(ui->doubleSpinBoxTrueAlpha, &QDoubleSpinBox::valueChanged, this, &MainWindow::calc);
    connect(ui->pushButtonAuto, &QPushButton::clicked, this, &MainWindow::autoCalc);

    // Actions
    QList<QAction*> reCalcActions = {
        ui->actionShow_Pong_Only,
        ui->actionShow_MUSIC_Peaks,
        ui->actionLog_Amplitude,
        ui->actionShow_Ranges
    };
    for (QAction *const action : reCalcActions) {
        connect(action, &QAction::toggled, this, &MainWindow::calc);
    }
    connect(ui->actionClear_Cache, &QAction::triggered, this, [this](){
        _cache.clear();
        calc();
    });

    // I'll never remember doing this in the .ui
    ui->tabWidget->setCurrentIndex(0);

    settingsChanged();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::externalSignal() {
    emit recorderStopRecording();
    ui->pushButtonStop->setEnabled(false);
    ui->pushButtonStart->setEnabled(true);
    _collector->reset();
}

void MainWindow::settingsChanged() {
    QStringList const serialPorts = _settingsDialog->settings().serialPorts;
    if (serialPorts.size() != 4) {
        return;
    }
    for (int i = 0; i < 4; ++i) {
        _nodeInit[i].progressBar->setValue(0);
        _nodeInit[i].progressBar->setMaximum(1);
        _nodeInit[i].lineEdit->setPlaceholderText(serialPorts[i]);
        _nodes[i]->setPortName(serialPorts[i]);
    }
    emit reconnectNodes();
}

bool MainWindow::currentTabIsAnalyisTab() const {
    return ui->tabWidget->widget(ui->tabWidget->currentIndex())->objectName() == "Analyse";
}

QVector<ComplexList> MainWindow::collectionFromFile(QString const& filePath) const {
    QFile file(filePath);
    if (not file.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open file" << file.fileName() << file.errorString();
        return {};
    }
    QByteArray const bytes = file.readAll();
    file.close();
    QJsonDocument const doc = QJsonDocument::fromJson(bytes);
    if (not doc.isArray()) {
        qWarning() << "File" << file.errorString() << "does not contain an array";
        return {};
    }
    QJsonArray const& array = doc.array();
    if (array.size() != 3) {
        qWarning() << "File" << file.errorString() << "does not contain three subarrays" << array.size();
        return {};
    }
    return {
        ComplexList::fromJson(array[0].toArray()),
        ComplexList::fromJson(array[1].toArray()),
        ComplexList::fromJson(array[2].toArray()),
    };
}

void MainWindow::saveClicked() {
    QString dir = QFileDialog::getExistingDirectory(nullptr, "Select Save Location");
    if (dir.isEmpty()) {
        return;
    }
    bool const dirIsEmpty = QDir(dir).entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
    if (not dir.isEmpty() and not dirIsEmpty) {
        // Ask user if they want to save in non-empty directory, this may overwrite files
        bool const overwrite = QMessageBox::question(
           this,
           "Warning",
           "The Selected directory is not empty.\nDo you still want to save recordings here?\nThis may overwrite existing files.\n\n" +dir
        ) == QMessageBox::Yes;
        if (not overwrite) {
            return;
        }
    }
    for (QListWidgetItem *const qitem : ui->listWidgetRecord->selectedItems()) {
        ListItem *const item = dynamic_cast<ListItem *const>(qitem);
        if (item == nullptr) {
            continue;
        }
        QJsonArray json;
        for (ComplexList const& cl : item->data) {
            json << cl.toJson();
        }
        int const id = item->id;
        QFile file(dir + "/" + QString("%1").arg(id, 5, 10, QChar('0')) + ".json");
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Couldn't open save file." << file.errorString();
            continue;
        }
        file.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
    }
}

void MainWindow::putFilesIntoListWidget(QStringList const& files) {
    ui->listWidgetLoad->clear();
    ui->pushButtonClearLoad->setEnabled(not files.isEmpty());
    ui->progressBarLoad->setMaximum(files.size());
    int i = 0;
    for (QString const& fileName : files) {
        ui->progressBarLoad->setValue(++i);
        auto const collection = collectionFromFile(fileName);
        // id from filename "00001.json"
        int const id = QFileInfo(fileName).baseName().toInt();
        auto item = new ListItem(id, fileName, collection, QFileInfo(fileName).fileName(), ui->listWidgetLoad);
        item->setToolTip(fileName);
        item->setSelected(true);
    }
}

/**
 * evaluate(), calc(), loadCached() and autoCalc() should MOST DEFINITELY not be in this GUI class, let alone
 * even run in the GUI thread. It freezes the UI for a solid few seconds every time they run.
 * However, this makes testing and interfacing with the UI easier and I dont have time to refactor it right now.
 */
Evaluation MainWindow::evaluate(QVector<CacheEntry> const& batch, double const& trueAngle, QPair<double, double> const& range) const {
    Evaluation eval;

    // General info
    eval.N = batch.size();
    eval.n = 0;
    for (auto const& entry : batch) {
        eval.n += entry.hasPong ? 1 : 0;
    }

    if (batch.empty()) {
        QMessageBox::critical(nullptr, "Leeres Batch", "Leeres Batch");
        return eval;
    }

    // Individual peaks
    for (auto const& entry : batch) {
        if (entry.hasPong) {
            eval.musicSeparate.peaks << double(entry.music.peakAngle);
        }
    }
    eval.musicSeparate.msr.mean   = polar::circularMeanDeg(eval.musicSeparate.peaks);
    eval.musicSeparate.msr.median = polar::circularMedianDeg(eval.musicSeparate.peaks);
    eval.musicSeparate.msr.std    = polar::circularStdDevDeg(eval.musicSeparate.peaks);
    eval.musicSeparate.msr.rmse   = polar::rmse(eval.musicSeparate.peaks, trueAngle, &range);

    eval.musicSeparate.minY = INFINITY;
    eval.musicSeparate.maxY = -INFINITY;
    for (auto const& entry : batch) {
        eval.musicSeparate.minY = qMin(entry.music.min, eval.musicSeparate.minY);
        eval.musicSeparate.maxY = qMax(entry.music.max, eval.musicSeparate.maxY);
    }

    // Sum up all spectra
    // Init sumspectrum with angles and 0
    for (auto const& [angle, amp] : batch[0].music.spectrum) {
        eval.musicSum.spectrum.append({angle, 0});
    }
    // Add all spectra (that have a pong)
    for (auto const& entry : batch) {
        // Ignore records without pong
        if (not entry.hasPong) {
            continue;
        }
        for (int i = 0; i < entry.music.spectrum.size(); ++i) {
            eval.musicSum.spectrum[i].second += entry.music.spectrum[i].second;
        }
    }
    // Sum Peak
    eval.musicSum.maxY = -INFINITY;
    eval.musicSum.minY = INFINITY;
    for (auto const& [angle, amp]: eval.musicSum.spectrum) {
        if (amp > eval.musicSum.maxY) {
            eval.musicSum.maxY = amp;
            eval.musicSum.peak = angle;
        }
        eval.musicSum.minY = qMin(eval.musicSum.minY, amp);
    }
    eval.musicSum.quality = eval.musicSum.spectrum.atAngle(trueAngle) / eval.musicSum.minY;
    
    // PDOA
    NumList<double> pdoa01;
    NumList<double> pdoa12;
    NumList<double> pdoa02;
    NumList<double> pdoaMean;
    for (auto const& entry : batch) {
        if (entry.hasPong) {
            pdoa01 << entry.pdoa.alpha01;
            pdoa12 << entry.pdoa.alpha12;
            pdoa02 << entry.pdoa.alpha02;
            pdoaMean << entry.pdoa.alphaMean;
        }
    }
    eval.pdoa.msr01.mean   = polar::circularMeanDeg(pdoa01);
    eval.pdoa.msr01.median = polar::circularMedianDeg(pdoa01);
    eval.pdoa.msr01.std    = polar::circularStdDevDeg(pdoa01);
    eval.pdoa.msr01.rmse   = polar::rmse(pdoa01, trueAngle, &range);
    eval.pdoa.msr12.mean   = polar::circularMeanDeg(pdoa12);
    eval.pdoa.msr12.median = polar::circularMedianDeg(pdoa12);
    eval.pdoa.msr12.std    = polar::circularStdDevDeg(pdoa12);
    eval.pdoa.msr12.rmse   = polar::rmse(pdoa12, trueAngle, &range);
    eval.pdoa.msr02.mean   = polar::circularMeanDeg(pdoa02);
    eval.pdoa.msr02.median = polar::circularMedianDeg(pdoa02);
    eval.pdoa.msr02.std    = polar::circularStdDevDeg(pdoa02);
    eval.pdoa.msr02.rmse   = polar::rmse(pdoa02, trueAngle, &range);
    eval.pdoa.msr.mean     = polar::circularMeanDeg(pdoaMean);
    eval.pdoa.msr.median   = polar::circularMedianDeg(pdoaMean);
    eval.pdoa.msr.std      = polar::circularStdDevDeg(pdoaMean);
    eval.pdoa.msr.rmse     = polar::rmse(pdoaMean, trueAngle, &range);

    return eval;
}

QVector<MainWindow::CacheEntry> MainWindow::loadCached(QVector<QPair<QString, QVector<ComplexList>>> const& records) const {
    SettingsDialog::Settings const settings = _settingsDialog->settings();

    // Calibration and Pong ranges
    int const caliCenter = settings.calibrationRange.center;
    int const caliWidth  = settings.calibrationRange.width;
    int const pongCenter = settings.pongRange.center;
    int const pongWidth  = settings.pongRange.width;
    int const caliStart = qMax(0,          caliCenter - caliWidth);
    int const caliEnd   = qMin(FRAME_SIZE, caliCenter + caliWidth);
    int const pongStart = qMax(0,          pongCenter - pongWidth);
    int const pongEnd   = qMin(FRAME_SIZE, pongCenter + pongWidth);

    // Load all json files in dir as ComplexLists
    // QVector<ComplexList> are the three modules I/Q samples
    QVector<CacheEntry> batch;
    batch.reserve(records.size());
    ui->progressBarChart->setMaximum(records.size());
    for (auto const& [name, _collection] : records) {
        ui->progressBarChart->setValue(batch.size());

        // First, check if file is already cached
        // The cache key includes the full file path and settings, but CAN be ambiguous for new recordings
        QString const cacheKey = name + "_" + QString::number(qHashBits(&settings, sizeof(settings)), 32);
        if (_cache.contains(cacheKey)) {
            batch << _cache[cacheKey];
            continue;
        }
        // If not cached, make calculations and add to cache
        CacheEntry entry;
        entry.name = name;
        entry.collection = _collection;

        /* Check if 'pong' answer is present
         * This is done by looking at the amplitude, taking the logarithm and then the standard deviation over the pong duration
         * Noise will be, well *noisy* and have a large standard deviation
         * A clean signal will have a consistent amplitude and thus a low standard deviation
         *
         * (this is kind of a simplified workaround for the Signal-to-noise ratio)
         *
         * Since we only look at the amplitude, it does not matter if we use the calbirated or uncalibrated collection
         */
        float const absLnStdDev = entry.collection[0].sliced(pongStart, 2*pongWidth).abs().ln().stdDev();
        entry.hasPong = absLnStdDev < settings.lns;

        // Apply onboard calibration by using the masters' reference signal
        if (settings.onboardCalibration.enabled) {
            entry.pdoa.cali01 = gr_doa::phaseDifference(entry.collection[0], entry.collection[1], caliStart, caliEnd);
            entry.pdoa.cali02 = gr_doa::phaseDifference(entry.collection[0], entry.collection[2], caliStart, caliEnd);
            entry.collection[1].shift(-entry.pdoa.cali01);
            entry.collection[2].shift(-entry.pdoa.cali02);
        } else {
            entry.pdoa.cali01 = 0;
            entry.pdoa.cali02 = 0;
        }

        // Apply manual/hardware calibration from settings dialog
        //qDebug() << "Calibration offsets:" << settings.calibration.enabled << settings.calibration.offset01 << settings.calibration.offset02;
        if (settings.manualCalibration.enabled) {
            entry.collection[1].shift(-settings.manualCalibration.offset01);
            entry.collection[2].shift(-settings.manualCalibration.offset02);
        }
        // Note that for the purpose of AOA estimation, it wouldn't be necessary to shift the entire recording,
        // the calculation is only based on the short timeframe where the pong signal is present
        // However for the sake of simplicity and debugging its easier to just work with a single copy

        // ===============================
        //              PDOA
        // ===============================

        // Calculate PDOA angles by taking the phase difference during the pong timeframe
        entry.pdoa.phase01 = gr_doa::phaseDifference(entry.collection[0], entry.collection[1], pongStart, pongEnd);
        entry.pdoa.phase12 = gr_doa::phaseDifference(entry.collection[1], entry.collection[2], pongStart, pongEnd);
        entry.pdoa.phase02 = gr_doa::phaseDifference(entry.collection[0], entry.collection[2], pongStart, pongEnd);

        /* For the angle calculation from phase difference, we need to account for the array structure (ULA / UCA)
         * "v" indicates the zero bearing
         *
         * The ULA angles already match the desired reference
         * Note that the distance between 0 and 2 is double!
         * IF THIS DISTANCE IS LARGER THAN HALF A WAVELENGTH, THE ANGLE WILL BE AMBIGUOUS
         *
         * For UCA, we have to correct for different baseline orientations
         * +-60 degrees for sides
         * +180° for the base
         *
         * ULA:
         *
         *     v
         * 0 - 1 - 2
         *
         * UCA:
         *
         *    v
         *    0
         *   / \
         *  2 - 1
         */

        double const lambda_m = settings.lambda_m();
        int const spacing02 = (settings.arrayType == AntennaArrayType::ULA) ? 2 : 1;
        entry.pdoa.alpha01 = gr_doa::angle(entry.pdoa.phase01, lambda_m, settings.antennaSpacing * lambda_m            );
        entry.pdoa.alpha12 = gr_doa::angle(entry.pdoa.phase12, lambda_m, settings.antennaSpacing * lambda_m            );
        entry.pdoa.alpha02 = gr_doa::angle(entry.pdoa.phase02, lambda_m, settings.antennaSpacing * lambda_m * spacing02);
        entry.pdoa.alphaMean = polar::circularMeanDeg(QList<double>{entry.pdoa.alpha01, entry.pdoa.alpha12, entry.pdoa.alpha02});

        if (settings.arrayType == AntennaArrayType::UCA) {
            entry.pdoa.alpha01 = polar::wrap180(entry.pdoa.alpha01 + 60 );
            entry.pdoa.alpha02 = polar::wrap180(entry.pdoa.alpha02 - 60 );
            entry.pdoa.alpha12 = polar::wrap180(entry.pdoa.alpha12 + 180);
        }

        // ===============================
        //              MUSIC
        // ===============================

        QVector<ComplexList> const pongCollection = {
            entry.collection[0].sliced(pongStart, 2*pongWidth),
            entry.collection[1].sliced(pongStart, 2*pongWidth),
            entry.collection[2].sliced(pongStart, 2*pongWidth)
        };

        // Calculate MUSIC spectrum. The onboard-calibration by reference signal is done within this method, too.
        entry.music.spectrum = music(
            settings.arrayType,
            settings.antennaSpacing,
            pongCollection,
            entry.music.peakAngle,
            entry.music.min,
            entry.music.max
        );

        _cache[cacheKey] = entry;
        batch << entry;
    }
    ui->progressBarChart->setValue(batch.size());
    return batch;
}

void setLabelMeanAndStdDev(QLabel *const label, evaluation::MeanMedianStdRmse const& msr) {
    label->setText(
        QString("%1° ± %2° | %3° | %4°").arg(
            QString::number(msr.mean,   'f', 2),
            QString::number(msr.std ,   'f', 2),
            QString::number(msr.median, 'f', 2),
            QString::number(msr.rmse,   'f', 2)
        )
    );
};

void MainWindow::calc() {
    if (not currentTabIsAnalyisTab()) {
        return;
    }
    // Items selected in the UI are the ones to be evaluated
    QVector<QPair<QString, QVector<ComplexList>>> records;
    for (QListWidget *const listWidget : { ui->listWidgetLoad, ui->listWidgetRecord }) {
        for (QListWidgetItem *const qitem : listWidget->selectedItems()) {
            ListItem *const item = dynamic_cast<ListItem *const>(qitem);
            if (item == nullptr) {
                continue;
            }
            records.append({item->name, item->data});
        }
    }
    if (records.isEmpty()) {
        return;
    }

    IVector<CacheEntry> const batch = loadCached(records);

    // Evaluate the batch
    int const truth = ui->doubleSpinBoxTrueAlpha->value();
    QPair<double, double> const range = _settingsDialog->settings().arrayType == AntennaArrayType::ULA ? QPair<double, double>(-90, 90) : QPair<double, double>(-180, 180);
    Evaluation const eval = evaluate(batch, truth, range);

    // Write evaluation to UI
    ui->labelEvalSamples->setText(QString::number(eval.n) + " / " + QString::number(eval.N));
    ui->labelEvalSumPeak->setText(QString::number(eval.musicSum.peak) + "°");
    ui->labelEvalSumQuality->setText(QString::number(eval.musicSum.quality));
    ui->labelEvalSumQuality->setToolTip(QString::number(eval.musicSum.minY));
    setLabelMeanAndStdDev(ui->labelEvalSepPeak , eval.musicSeparate.msr );
    setLabelMeanAndStdDev(ui->labelEvalPdoa01  , eval.pdoa.msr01        );
    setLabelMeanAndStdDev(ui->labelEvalPdoa12  , eval.pdoa.msr12        );
    setLabelMeanAndStdDev(ui->labelEvalPdoa02  , eval.pdoa.msr02        );
    setLabelMeanAndStdDev(ui->labelEvalPdoaMean, eval.pdoa.msr          );


    // Calc X Units (ms)
    constexpr float resolution = 1.0f / 1000;
    QList<float> X;
    X.reserve(FRAME_SIZE);
    for (int i = 0; i < FRAME_SIZE; ++i) {
        X.append(i * resolution);
    }
    // Calibration and Pong ranges
    int const caliCenter = _settingsDialog->settings().calibrationRange.center;
    int const caliWidth  = _settingsDialog->settings().calibrationRange.width;
    int const pongCenter = _settingsDialog->settings().pongRange.center;
    int const pongWidth  = _settingsDialog->settings().pongRange.width;
    int const caliStart = qMax(0,          caliCenter - caliWidth);
    int const caliEnd   = qMin(FRAME_SIZE, caliCenter + caliWidth);
    int const pongStart = qMax(0,          pongCenter - pongWidth);
    int const pongEnd   = qMin(FRAME_SIZE, pongCenter + pongWidth);

    double const lim    = _settingsDialog->settings().arrayType == AntennaArrayType::ULA ? 90.0 : 180.0;
    bool const pongOnly = ui->actionShow_Pong_Only->isChecked();

    // Plot Batch
    QChart *chart = nullptr;
    QTransform transform;
    QString const chartType = _comboBoxBatchChartType->currentText();
    if (chartType == "MUSIC Cartesian Seperate") {
        chart = new QChart();
        for (int i = 0; i < batch.size(); ++i) {
            // Spectrum
            auto const entry = batch[i];
            if (entry.hasPong or not pongOnly) {
                QBrush const& brush = entry.hasPong ? brush::blue : brush::red;
                QLineSeries *const series = plot::line(chart, entry.music.spectrum, "", brush);
                connect(series, &QLineSeries::clicked, this, [entry](){
                    qInfo() << "Spectrum" << entry.name;
                });
            }
            // Peak
            if (entry.hasPong and ui->actionShow_MUSIC_Peaks->isChecked()) {
                QLineSeries *const peakLine = plot::box(chart, entry.music.peakAngle, entry.music.peakAngle, float(eval.musicSeparate.minY), 0.0f, QString::number(i), brush::green);
                connect(peakLine, &QLineSeries::clicked, this, [entry](){
                    qInfo() << "Peak" << entry.name;
                });
            }

        }
        plot::makeAxes(
            chart,
            "Winkel / °",
            "MUSIC Pseudo-Spektrum / dB"
        );
        chart->legend()->hide();
    } else if (chartType == "MUSIC Cartesian Sum") {
        chart = new QChart();
        plot::line(chart, eval.musicSum.spectrum);
        if (ui->actionShow_MUSIC_Peaks->isChecked()) {
            plot::box(chart, eval.musicSum.peak, eval.musicSum.peak, eval.musicSum.minY, eval.musicSum.maxY, "", brush::green);
        }
        plot::makeAxes(
            chart,
            "Winkel / °",
            "MUSIC Pseudo-Spektrum / dB"
        );
        chart->legend()->hide();
    } else if (chartType == "MUSIC Polar Seperate") {
        QPolarChart* polarChart = new QPolarChart();
        auto angularAxis = new QValueAxis();
        angularAxis->setTickCount(9);
        angularAxis->setLabelFormat("%.1f");
        angularAxis->setRange(-180, 180);
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        QVector<QAbstractSeries*> serieses;
        for (int i = 0; i < batch.size(); ++i) {
            // Spectrum
            auto const entry = batch[i];
            if (entry.hasPong or not pongOnly) {
                QBrush const& brush = entry.hasPong ? brush::blue : brush::red;
                QVector<QPair<int,float>> spectrum = entry.music.spectrum;
                /*
                if (_checkBoxBatchChartNormalization->isChecked()) {
                    float localMax = -INFINITY;
                    for (auto const& [angle, amp] : spectrum) {
                        localMax = qMax(localMax, amp);
                    }
                    if (localMax != 0) {
                        for (auto& [angle, amp] : spectrum) {
                            amp /= localMax;
                        }
                    }
                }
                */
                for (auto const& [angle, amp] : spectrum) {
                    minY = qMin(minY, amp);
                    maxY = qMax(maxY, amp);
                }
                serieses << plot::line(polarChart, spectrum, "", brush);
            }
            // Peak
            if (entry.hasPong and ui->actionShow_MUSIC_Peaks->isChecked()) {
                QLineSeries *const peakLine = plot::box(polarChart, entry.music.peakAngle, entry.music.peakAngle, float(eval.musicSeparate.minY), 0.0f, QString::number(i), brush::green);
                connect(peakLine, &QLineSeries::clicked, this, [entry](){
                    qInfo() << "Peak" << entry.name;
                });
                serieses << peakLine;
            }
        }
        auto radialAxis = new QValueAxis();
        radialAxis->setTickCount(2);
        radialAxis->setRange(minY, maxY);
        polarChart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);
        polarChart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);
        polarChart->legend()->hide();
        polarChart->setMargins(QMargins(3, 3, 3, 3));
        for (auto const& series : serieses) {
            series->attachAxis(angularAxis);
            series->attachAxis(radialAxis);
        }
        transform.rotate(180);
        chart = polarChart;
    } else if (chartType == "MUSIC Polar Sum") {
        QPolarChart* polarChart = new QPolarChart();
        auto angularAxis = new QValueAxis();
        angularAxis->setTickCount(9);
        angularAxis->setLabelFormat("%.1f");
        angularAxis->setRange(-180, 180);
        QVector<QPair<int,float>> sumSpectrum;
        for (auto const& [angle, amp] : batch[0].music.spectrum) {
            sumSpectrum.append({angle, 0});
        }
        for (auto const& entry : batch) {
            if (not entry.hasPong) {
                continue;
            }
            for (int i = 0; i < entry.music.spectrum.size(); ++i) {
                sumSpectrum[i].second += entry.music.spectrum[i].second;
            }
        }
        /*
        if (_checkBoxBatchChartNormalization->isChecked()) {
            float localMax = std::numeric_limits<float>::lowest();
            for (auto const& [angle, amp] : sumSpectrum) {
                localMax = qMax(localMax, amp);
            }
            if (localMax != 0) {
                for (auto& [angle, amp] : sumSpectrum) {
                    amp /= localMax;
                }
            }
        }
        */
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        for (auto const& [angle, amp] : sumSpectrum) {
            minY = qMin(minY, amp);
            maxY = qMax(maxY, amp);
        }
        QVector<QAbstractSeries*> serieses;
        serieses << plot::line(polarChart, sumSpectrum);
        int peakAngle = 0;
        float peakVal = std::numeric_limits<float>::lowest();
        for (auto const& [angle, amp] : sumSpectrum) {
            if (amp > peakVal) {
                peakVal = amp;
                peakAngle = angle;
            }
        }
        if (ui->actionShow_MUSIC_Peaks->isChecked()) {
            serieses << plot::box(polarChart, peakAngle, peakAngle, minY, maxY);
        }
        auto radialAxis = new QValueAxis();
        radialAxis->setTickCount(2);
        radialAxis->setRange(minY, maxY);
        polarChart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);
        polarChart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);
        polarChart->legend()->hide();
        polarChart->setMargins(QMargins(3, 3, 3, 3));
        for (auto const& series : serieses) {
            series->attachAxis(angularAxis);
            series->attachAxis(radialAxis);
        }
        transform.rotate(180);
        chart = polarChart;
    } else if (chartType == "PDOA Polar") {
        QPolarChart* polarChart = new QPolarChart();
        auto angularAxis = new QValueAxis();
        angularAxis->setTickCount(9);
        angularAxis->setLabelFormat("%.1f");
        angularAxis->setRange(-180, 180);
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        QVector<QAbstractSeries*> serieses;
        for (int i = 0; i < batch.size(); ++i) {
            auto const entry = batch[i];
            if (entry.hasPong or not pongOnly) {
                QLineSeries *const line01 = plot::box(polarChart, entry.pdoa.alpha01, entry.pdoa.alpha01, 0, 1, entry.name, entry.hasPong ? brush::orange  : brush::red);
                QLineSeries *const line12 = plot::box(polarChart, entry.pdoa.alpha12, entry.pdoa.alpha12, 0, 1, entry.name, entry.hasPong ? brush::magenta : brush::red);
                QLineSeries *const line02 = plot::box(polarChart, entry.pdoa.alpha02, entry.pdoa.alpha02, 0, 1, entry.name, entry.hasPong ? brush::blue    : brush::red);
                connect(line01, &QLineSeries::clicked, this, [entry](){
                    qInfo() << entry.name;
                });
                connect(line12, &QLineSeries::clicked, this, [entry](){
                    qInfo() << entry.name;
                });
                connect(line02, &QLineSeries::clicked, this, [entry](){
                    qInfo() << entry.name;
                });
                serieses << line01 << line12 << line02;
            }
        }
        auto radialAxis = new QValueAxis();
        radialAxis->setTickCount(2);
        radialAxis->setRange(minY, maxY);
        polarChart->addAxis(angularAxis, QPolarChart::PolarOrientationAngular);
        polarChart->addAxis(radialAxis, QPolarChart::PolarOrientationRadial);
        polarChart->legend()->hide();
        polarChart->setMargins(QMargins(3, 3, 3, 3));
        for (auto const& series : serieses) {
            series->attachAxis(angularAxis);
            series->attachAxis(radialAxis);
        }
        transform.rotate(180);
        chart = polarChart;
    } else if (chartType == "Amplitude") {
        chart = new QChart();

        if (batch.size() == 1) {
            NumList const Y = ui->actionLog_Amplitude->isChecked() ? batch[0].collection[0].abs().ln() : batch[0].collection[0].abs();
            plot::line<float,float>(chart, X, Y, "Modul 0");
            //plot::line<float,float>(chart, X, batch[0].collection[1].abs(), "Modul 1");
            //plot::line<float,float>(chart, X, batch[0].collection[2].abs(), "Modul 2");
        } else {
            for (auto const& entry : batch) {
                if (entry.hasPong or not pongOnly) {
                    QBrush const& brush = entry.hasPong ? brush::blue : brush::red;
                    for (auto const& complexList : entry.collection) {
                        NumList const Y = ui->actionLog_Amplitude->isChecked() ? complexList.abs().ln() : complexList.abs();
                        plot::line<float,float>(chart, X, Y, "", brush);
                    }
                }
            }
        }

        if (ui->actionShow_Ranges->isChecked()) {
            float maxY = M_SQRT2 * INT16_MAX;
            if (ui->actionLog_Amplitude->isChecked()) {
                maxY = qLn(maxY);
            }
            plot::box<float,int>(chart, resolution * caliStart, resolution * caliEnd, 0, maxY, "Calibration", brush::gray);
            plot::box<float,int>(chart, resolution * pongStart, resolution * pongEnd, 0, maxY, "Pong", brush::black);
        }
        chart->legend()->hide();
        plot::makeAxes(
            chart,
            "Zeit / ms"
            );
    } else if (chartType == "Complex IQ") {
        chart = new QChart();
        ui->progressBarChart->setMaximum(batch.size());
        int i = -1;
        for (auto const& entry : batch) {
            ui->progressBarChart->setValue(++i);
            if (pongOnly and not entry.hasPong) {
                continue;
            }
            auto const& collection = entry.collection;
            QVector<float> phaseOffsets;
            phaseOffsets << 0; // 0 always reference
            /*if (_comboBoxIqPhaseShift->currentText() == "Cali") {
                phaseOffsets << gr_doa::phaseDifference(collection[0], collection[1], caliStart, caliEnd);
                phaseOffsets << gr_doa::phaseDifference(collection[0], collection[2], caliStart, caliEnd);
            } else if (_comboBoxIqPhaseShift->currentText() == "Pong") {
                phaseOffsets << gr_doa::phaseDifference(collection[0], collection[1], pongStart, pongEnd);
                phaseOffsets << gr_doa::phaseDifference(collection[0], collection[2], pongStart, pongEnd);
            } else */ {
                phaseOffsets << 0;
                phaseOffsets << 0;
            }

            for (int ant = 0; ant < collection.size(); ++ant) {
                QVector<float> I, Q;
                int n = -1;
                for (auto const& x : collection[ant]) {
                    n++;
                    float const pongFactor = 0.2f;
                    if (not ((n > caliStart and n < caliEnd) or (n > (pongStart + pongWidth * (1-pongFactor)) and n < pongEnd))) {
                        continue;
                    }
                    gr_complex shifted = x * std::polar(1.0f, -phaseOffsets[ant]);
                    if (ui->actionLog_Amplitude->isChecked()) {
                        shifted = polar::cLn(shifted);
                    }

                    I.append(shifted.real());
                    Q.append(shifted.imag());
                }
                plot::scatter(chart, I, Q, "", Qt::NoBrush, 2);
            }
        }
        float const max = ui->actionLog_Amplitude->isChecked() ? qLn(INT16_MAX) : INT16_MAX;
        plot::box(chart, -max, max, -max, max, "", brush::gray);
        plot::makeAxes(chart, "real", "imag");
    } else if (chartType == "Reference Phases") {
        chart = new QChart();      
        //plot::scatter(chart, batch.map<double>([=](auto e){ return (pongOnly and not e.hasPong) ? NAN : e.pdoa.cali01; }), "Cali01");
        //plot::scatter(chart, batch.map<double>([=](auto e){ return (pongOnly and not e.hasPong) ? NAN : e.pdoa.cali02; }), "Cali02");
        //plot::scatter(chart, batch.map<double>([=](auto e){ return (pongOnly and not e.hasPong) ? NAN : e.pdoa.pong01; }), "Pong01");
        //plot::scatter(chart, batch.map<double>([=](auto e){ return (pongOnly and not e.hasPong) ? NAN : e.pdoa.pong12; }), "Pong12");
        //plot::scatter(chart, batch.map<double>([=](auto e){ return (pongOnly and not e.hasPong) ? NAN : e.pdoa.pong02; }), "Pong02");

        NumList<double> const phase01 = batch.map<double>([=](auto e){ return (pongOnly and not e.hasPong) ? NAN : e.pdoa.phase01; });
        NumList<double> const phase12 = batch.map<double>([=](auto e){ return (pongOnly and not e.hasPong) ? NAN : e.pdoa.phase12; });
        NumList<double> const phase02 = batch.map<double>([=](auto e){ return (pongOnly and not e.hasPong) ? NAN : e.pdoa.phase02; });

        qInfo() << "Cali01: " << phase01.mean() << "+-" << phase01.stdDev() << "(" << 100*phase01.mean() / (2*M_PI) << "%";
        qInfo() << "Cali12: " << phase12.mean() << "+-" << phase12.stdDev() << "(" << 100*phase12.mean() / (2*M_PI) << "%";
        qInfo() << "Cali02: " << phase02.mean() << "+-" << phase02.stdDev() << "(" << 100*phase02.mean() / (2*M_PI) << "%";

        // copy to clipboard
        QString clipboard;
        clipboard += phase01.toQString() + ",\n";
        clipboard += phase12.toQString() + ",\n";
        clipboard += phase02.toQString() + "\n";
        QApplication::clipboard()->setText(clipboard);

        plot::line(chart, phase01, "0-1");
        plot::line(chart, phase12, "1-2");
        plot::line(chart, phase02, "0-2");
        plot::box(chart, 0ll, batch.size()-1, -M_PI, M_PI, "", brush::gray);
        plot::makeAxes(
            chart,
            "Peilung #",
            "Phasendifferenz / rad"
        );
    } else if (chartType == "MUSIC Peaks over time") {
        chart = new QChart();
        QList<float> pong;
        QList<float> noPong;
        for (auto const& entry : batch) {
            if (entry.hasPong) {
                pong << entry.music.peakAngle;
                noPong << NAN;
            } else {
                noPong << entry.music.peakAngle;
                pong << NAN;
            }
        }
        plot::scatter(chart, pong, "", brush::blue);
        plot::scatter(chart, noPong, "", brush::red);
        plot::box(chart, 0ll, batch.size(), -lim, lim, "", brush::gray);
        chart->legend()->hide();
        plot::makeAxes(
            chart,
            "Peilung #",
            "Winkel / °"
        );
    } else if (chartType == "PDOA over time") {
        chart = new QChart();
        plot::line(chart, batch.map<double>([=](CacheEntry const& e){ return (pongOnly and not e.hasPong) ? NAN : e.pdoa.alpha01; }), "Alpha01");
        plot::line(chart, batch.map<double>([=](CacheEntry const& e){ return (pongOnly and not e.hasPong) ? NAN : e.pdoa.alpha12; }), "Alpha12");
        plot::line(chart, batch.map<double>([=](CacheEntry const& e){ return (pongOnly and not e.hasPong) ? NAN : e.pdoa.alpha02; }), "Alpha02");
        plot::box(chart, 0ll, batch.size(), -lim, lim, "", brush::gray);
        plot::makeAxes(
            chart,
            "Peilung #",
            "Winkel / °"
        );
    } else if (chartType == "MUSIC Spectrogram") {
        chart = new QChart();
        int const nEntries = batch.size();
        int const nAngles =  batch[0].music.spectrum.size();
        int const angleMin = batch[0].music.spectrum.first().first;
        int const angleMax = batch[0].music.spectrum.last().first;

        QList<QList<float>> Z;
        Z.reserve(nEntries);
        for (int x = 0; x < nEntries; ++x) {
            auto const& entry = batch[x];
            QList<float> col;
            col.reserve(nAngles);
            for (int y = 0; y < nAngles; ++y) {
                float amp = entry.music.spectrum[y].second;
                if (pongOnly and not entry.hasPong) {
                    amp = eval.musicSeparate.minY;
                }
                col.append(amp);
            }
            Z.append(col);
        }
        plot::heatmap(
            chart,
            Z,
            {0, nEntries - 1},
            {angleMin, angleMax},
            "Peilung #",
            "Winkel / °"
            );
    }
    ui->chartWidget->chartView()->setTransform(transform);
    ui->chartWidget->chartView()->setChart(chart);
}

QVector<QStringList> transpose(QVector<QStringList> const& L) {
    if (L.isEmpty() || L[0].isEmpty()) return {};

    int rows = L.size();
    int cols = L[0].size();

    QVector<QStringList> result(cols, QStringList(rows, QString()));

    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            result[j][i] = L[i][j];

    return result;
}

void MainWindow::autoCalc() {
    QString parentDir = QFileDialog::getExistingDirectory(nullptr, "Select Parent Directory");
    if (parentDir.isEmpty()) {
        return;
    }
    QStringList const subDirs = QDir(parentDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (subDirs.empty()) {
        QMessageBox::critical(this, "No Subdirectories", QString("The selected parent directory does not contain any subdirectories\n\n%1").arg(parentDir));
        return;
    }

    ui->progressBarAuto->setMaximum(subDirs.size());
    ui->progressBarAuto->setValue(0);

    // Iterate through angles and perform evaluation
    QVector<QPair<double, Evaluation>> evaluations;
    int i = 0;
    for (QString const& dir : subDirs) {
        bool validName;
        double const angle = QString(dir).replace("+-", "").toDouble(&validName);
        if (!validName) {
            QMessageBox::critical(this, "Ungültiger Verzeichnisname", QString("Der Name '%1' kann nicht als Winkel interpretiert werden").arg(dir));
            return;
        }
        ui->progressBarAuto->setValue(i++);
        QVector<QPair<QString, QVector<ComplexList>>> records;
        QStringList const files = QDir(parentDir + "/" + dir).entryList({"*.json"}, QDir::Files);
        for (QString const& fileName : files) {
            QString const filePath = parentDir + "/" + dir + "/" + fileName;
            auto const collection = collectionFromFile(filePath);
            records.append({filePath, collection});
        }
        auto const batch = loadCached(records);
        double const trueAngle = dir.toDouble();
        QPair<double, double> range;
        if (_settingsDialog->settings().arrayType == AntennaArrayType::ULA) {
            range = QPair<double, double>(-90, 90);
        } else if (_settingsDialog->settings().arrayType == AntennaArrayType::UCA) {
            range = QPair<double, double>(-180, 180);
        } else {
            QMessageBox::critical(this, "Error", QString("Array Type %1 not supported for autoCalc").arg(int(_settingsDialog->settings().arrayType)));
            return;
        }
        evaluations.append({angle, evaluate(batch, trueAngle, range)});
    }
    ui->progressBarAuto->setValue(i);
    // sort by angle (-90, -60 .. 90)
    std::sort(evaluations.begin(), evaluations.end(), [](auto const& a, auto const& b){
        return a.first < b.first;
    });

    QString clipboard;
    if (!ui->actionDebug->isChecked()) {
        QStringList angleLables;
        QVector<QStringList> evalData;
        for (auto const& [angle, eval] : evaluations) {
            angleLables.append(QString::number(angle));
            evalData.append(eval.toColumn());
        }
        QStringList str;
        for (QStringList const& row : transpose(evalData)) {
            str << row.join("\t");
        }
        clipboard = str.join("\n");
    } else {
        clipboard += "alpha\t";
        QStringList angleLables;
        for (auto const& [angle, value] : evaluations.first().second.musicSum.spectrum) {
            angleLables.append(QString::number(angle));
        }
        clipboard += angleLables.join('\t') + "\n";
        for (auto const& [alpha, eval] : evaluations) {
            clipboard += QString::number(alpha) + "\t";
            QStringList values;
            for (auto const& [angle, value] : eval.musicSum.spectrum) {
                values.append(QString::number(value));
            }
            clipboard += values.join('\t') + "\n";
        }
    }


    QGuiApplication::clipboard()->setText(clipboard);
    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    QString const name = parentDir.split("/").last();
    box.setWindowTitle("Resultat - " + name);
    box.setText(name + "\n\n" + _settingsDialog->settings().arrayType + "\n\n" + QString::number(evaluations.size()) + " Spalten");
    box.addButton("Kopieren", QMessageBox::ActionRole);
    box.exec();
    QGuiApplication::clipboard()->setText(clipboard);
}

