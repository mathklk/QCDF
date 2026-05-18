#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>

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
        QString arrayType;
        float antennaSpacing;
        float lns;
    };

    Settings settings(void);

signals:
    void changed(void);

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
