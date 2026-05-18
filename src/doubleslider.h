#ifndef DOUBLESLIDER_H
#define DOUBLESLIDER_H

#include <QWidget>

namespace Ui {
class DoubleSlider;
}

class DoubleSlider : public QWidget
{
    Q_OBJECT

public:
    explicit DoubleSlider(QWidget *parent = nullptr);
    ~DoubleSlider();

    int minimum();
    int maximum();
    void setRange(int, int);
    void setValues(int, int);
    int first() const;
    int second() const;

signals:
    void changed(int, int);
    void firstChanged(int);
    void secondChanged(int);

private slots:
    void onChange();

private:
    Ui::DoubleSlider *ui;
};

#endif // DOUBLESLIDER_H
