#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
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
            int center;
            int width;
        } calibration;
        struct {
            int center;
            int width;
        } pong;

        float lambda_m() const;
    };

    Settings settings(void);

signals:
    void changed(void);
    void slidersChanged(void);

private slots:
    void updateLambda();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
