#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QFile>
#include <QDir>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->comboBoxArrayType->addItems({
        "ULA",
        "UCA"
    });

    connect(ui->pushButtonApply, &QPushButton::clicked, this, [this](){
        emit changed();
    });
    connect(ui->pushButtonOk, &QPushButton::clicked, this, [this](){
        emit changed();
        close();
    });

    QFile comFile("COM.txt");
    QStringList comPorts;
    if (comFile.exists()) {
        if (comFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            while (!comFile.atEnd()) {
                QString line = comFile.readLine().trimmed();
                if (!line.isEmpty()) {
                    comPorts.append(line);
                }
            }
            comFile.close();
        } else {
            qCritical() << "Failed to open COM.txt file";
        }
        if (comPorts.size() == 4) {
            ui->lineEdit_1->setText(comPorts[0]);
            ui->lineEdit_2->setText(comPorts[1]);
            ui->lineEdit_3->setText(comPorts[2]);
            ui->lineEdit_4->setText(comPorts[3]);
        } else {
            qCritical() << "COM.txt contains wrong number of entries:" << comPorts.size() << " (expected" << 4 << ")";
        }
    } else {
        qWarning() << "COM.txt file not found in directory" << QDir::currentPath();
        show();
    }

    ui->caliSliders->setRange(0, 7000);
    ui->pongSliders->setRange(0, 7000);
    ui->caliSliders->setValues(380, 280);
    ui->pongSliders->setValues(5448, 246);
    connect(ui->caliSliders, &DoubleSlider::changed, this, &SettingsDialog::slidersChanged);
    connect(ui->pongSliders, &DoubleSlider::changed, this, &SettingsDialog::slidersChanged);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

SettingsDialog::Settings SettingsDialog::settings() {
    Settings s;
    for (QLineEdit *const le : {
        ui->lineEdit_1,
        ui->lineEdit_2,
        ui->lineEdit_3,
        ui->lineEdit_4
    }) {
        if (not le->text().isEmpty()) s.serialPorts << le->text();
    }
    s.carrierFrequency_MHz = ui->doubleSpinBoxFrequency->value();
    s.arrayType            = ui->comboBoxArrayType->currentText();
    s.antennaSpacing       = ui->doubleSpinBoxDLambda->value();
    s.lns                  = ui->doubleSpinBoxLns->value();
    s.calibration.center   = ui->caliSliders->first();
    s.calibration.width    = ui->caliSliders->second();
    s.pong.center          = ui->pongSliders->first();
    s.pong.width           = ui->pongSliders->second();
    return s;
}