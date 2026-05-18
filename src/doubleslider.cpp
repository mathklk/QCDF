#include "doubleslider.h"
#include "ui_doubleslider.h"

DoubleSlider::DoubleSlider(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DoubleSlider)
{
    ui->setupUi(this);
    connect(ui->topSlider, &QSlider::valueChanged, this, &DoubleSlider::onChange);
    connect(ui->botSlider, &QSlider::valueChanged, this, &DoubleSlider::onChange);
    connect(ui->topSlider, &QSlider::valueChanged, this, [this](int x){ ui->topLabel->setText(QString::number(x)); });
    connect(ui->botSlider, &QSlider::valueChanged, this, [this](int x){ ui->botLabel->setText(QString::number(x)); });
    ui->topLabel->setText(QString::number(ui->topSlider->value()));
    ui->botLabel->setText(QString::number(ui->botSlider->value()));
}

DoubleSlider::~DoubleSlider()
{
    delete ui;
}

int DoubleSlider::minimum() {
    return ui->topSlider->minimum();
}

int DoubleSlider::maximum() {
    return ui->topSlider->maximum();
}

void DoubleSlider::setRange(int min, int max) {
    ui->topSlider->setRange(min, max);
    ui->botSlider->setRange(min, max);
}

void DoubleSlider::setValues(int first, int second){
    ui->topSlider->setValue(first);
    ui->botSlider->setValue(second);
}

int DoubleSlider::first() const {
    return ui->topSlider->value();
}

int DoubleSlider::second() const {
    return ui->botSlider->value();
}

void DoubleSlider::onChange() {
    emit changed(first(), second());
}
