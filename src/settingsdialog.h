#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QSettings>
#include "antenna_array_type.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    struct Settings {
        QStringList serialPorts;
        float carrierFrequency_MHz;
        AntennaArrayType arrayType;
        float antennaSpacing;
        float lns;
        struct {
            bool enabled;
        } onboardCalibration;
        struct {
            bool enabled;
            float offset01;
            float offset02;
        } manualCalibration;
        struct {
            int center;
            int width;
        } calibrationRange;
        struct {
            int center;
            int width;
        } pongRange;

        float lambda_m() const;
    };

    Settings settings(void);

signals:
    void changed(void);
    void slidersChanged(void);

private slots:
    void apply(void);
    void updateLambda();

private:
    Ui::SettingsDialog *ui;
    QSettings* _qSettings;
};

#endif // SETTINGSDIALOG_H
