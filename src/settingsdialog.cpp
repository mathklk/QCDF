#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "physics.h"

#include <QFile>
#include <QDir>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->comboBoxArrayType->addItem("ULA", QVariant::fromValue(AntennaArrayType::ULA));
    ui->comboBoxArrayType->addItem("UCA", QVariant::fromValue(AntennaArrayType::UCA));
    connect(ui->doubleSpinBoxFrequency, &QDoubleSpinBox::valueChanged, this, &SettingsDialog::updateLambda);
    connect(ui->doubleSpinBoxDLambda,   &QDoubleSpinBox::valueChanged, this, &SettingsDialog::updateLambda);
    updateLambda();

    connect(ui->pushButtonApply, &QPushButton::clicked, this, &SettingsDialog::apply);
    connect(ui->pushButtonOk, &QPushButton::clicked, this, [this](){
        apply();
        close();
    });
    connect(ui->checkBoxManualCalibration, &QCheckBox::clicked, ui->doubleSpinBoxOffset01, &QDoubleSpinBox::setEnabled);
    connect(ui->checkBoxManualCalibration, &QCheckBox::clicked, ui->doubleSpinBoxOffset02, &QDoubleSpinBox::setEnabled);

    // This will store settings in ~/.config/Uni-Oldenburg/QCDF.ini
    QCoreApplication::setOrganizationName("Uni-Oldenburg");
    QCoreApplication::setApplicationName("QCDF");
    _qSettings = new QSettings(
        QSettings::IniFormat,
        QSettings::UserScope,
        QCoreApplication::organizationName(),
        QCoreApplication::applicationName()
    );

    // Load Settings
    if (_qSettings->contains("serialPorts")) {
        QStringList const ports = _qSettings->value("serialPorts").toStringList();
        if (ports.size() == 4) {
            ui->lineEdit_1->setText(ports[0]);
            ui->lineEdit_2->setText(ports[1]);
            ui->lineEdit_3->setText(ports[2]);
            ui->lineEdit_4->setText(ports[3]);
        } else {
            qCritical() << "QSettings contains wrong number of serial ports:" << ports.size() << " (expected" << 4 << ")";
        }
    }
    if (_qSettings->contains("calibration01")) {
        ui->doubleSpinBoxOffset01->setValue(_qSettings->value("calibration01").toDouble());
    }
    if (_qSettings->contains("calibration02")) {
        ui->doubleSpinBoxOffset02->setValue(_qSettings->value("calibration02").toDouble());
    }

    ui->caliSliders->setRange(0, 7000);
    ui->pongSliders->setRange(0, 7000);
    ui->caliSliders->setValues(380, 280);
    ui->pongSliders->setValues(5448, 246);
    //connect(ui->caliSliders, &DoubleSlider::changed, this, &SettingsDialog::slidersChanged);
    //connect(ui->pongSliders, &DoubleSlider::changed, this, &SettingsDialog::slidersChanged);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::apply()
{
    _qSettings->setValue("serialPorts", QStringList{
        ui->lineEdit_1->text(),
        ui->lineEdit_2->text(),
        ui->lineEdit_3->text(),
        ui->lineEdit_4->text()
    });
    _qSettings->setValue("calibration01", ui->doubleSpinBoxOffset01->value());
    _qSettings->setValue("calibration02", ui->doubleSpinBoxOffset02->value());
    emit changed();
}

void SettingsDialog::updateLambda() {
    double const lambda_cm = 100 * physics::speedOfLightInAir_mps / (ui->doubleSpinBoxFrequency->value() * 1e6);
    ui->doubleSpinBoxFrequency->setToolTip(QString::number(lambda_cm, 'f', 2) + " cm");
    ui->doubleSpinBoxDLambda  ->setToolTip(QString::number(lambda_cm * ui->doubleSpinBoxDLambda->value(), 'f', 2) + " cm");
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
    s.carrierFrequency_MHz       = ui->doubleSpinBoxFrequency->value();
    s.arrayType                  = AntennaArrayType(ui->comboBoxArrayType->currentData().value<int>());
    s.antennaSpacing             = ui->doubleSpinBoxDLambda->value();
    s.lns                        = ui->doubleSpinBoxLns->value();
    s.onboardCalibration.enabled = ui->checkBoxOnboardCalibration->isChecked();
    s.manualCalibration.enabled  = ui->checkBoxManualCalibration->isChecked();
    s.manualCalibration.offset01 = ui->doubleSpinBoxOffset01->value();
    s.manualCalibration.offset02 = ui->doubleSpinBoxOffset02->value();
    s.calibrationRange.center    = ui->caliSliders->first();
    s.calibrationRange.width     = ui->caliSliders->second();
    s.pongRange.center           = ui->pongSliders->first();
    s.pongRange.width            = ui->pongSliders->second();
    return s;
}

float SettingsDialog::Settings::lambda_m() const {
    return physics::speedOfLightInAir_mps / (carrierFrequency_MHz * 1e6);
}
