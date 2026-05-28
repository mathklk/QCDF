#ifndef PLOT_H
#define PLOT_H

#include <QChart>
#include <QList>
#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QImage>
#include <QPair>

namespace plot {

// Line Plot

template <typename XT, typename YT>
static QLineSeries* line(
    QChart *const chart,
    QList<XT> const& X,
    QList<YT> const& Y,
    QString const& label = "",
    QBrush const& brush = Qt::NoBrush
    ){
    auto const series = new QLineSeries();
    if (not label.isEmpty()) {
        series->setName(label);
    }
    if (brush != Qt::NoBrush) {
        series->setBrush(brush);
        series->setColor(brush.color());
    }
    size_t i = 0;
    for (XT const& x : X) {
        series->append(x, Y[i++]);
    }
    chart->addSeries(series);
    return series;
}

template <typename XT, typename YT>
static QLineSeries* line(
    QChart *const chart,
    QList<QPair<XT,YT>> const& XY,
    QString const& label = "",
    QBrush const& brush = Qt::NoBrush
    ) {
    QList<XT> X;
    QList<YT> Y;
    for (auto const& xy : XY) {
        X.append(xy.first);
        Y.append(xy.second);
    }
    return line(chart, X, Y, label, brush);
}

template <typename YT>
static QLineSeries* line(
    QChart *const chart,
    QList<YT> const& Y,
    QString const& label = "",
    QBrush const& brush = Qt::NoBrush
    ) {
    QList<int> X;
    for (int i = 0; i < Y.size(); ++i) {
        X.append(i);
    }
    return line(chart, X, Y, label, brush);
}

template <typename XT, typename YT>
static QLineSeries* box(
    QChart *const chart,
    XT x0,
    XT x1,
    YT y0,
    YT y1,
    QString const& label = "",
    QBrush const& brush = Qt::NoBrush
    ) {
    return line<XT, YT>(chart, {x0, x0, x1, x1, x0}, {y0, y1, y1, y0, y0}, label, brush);
}

// Scatter Plot

template <typename XT, typename YT>
static QScatterSeries* scatter(
    QChart *const chart,
    QList<XT> const& X,
    QList<YT> const& Y,
    QString const& label = "",
    QBrush const& brush = Qt::NoBrush,
    int width = -1
    ){
    auto const series = new QScatterSeries();
    if (not label.isEmpty()) {
        series->setName(label);
    }
    if (brush != Qt::NoBrush) {
        series->setBrush(brush);
        series->setColor(brush.color());
    }
    if (width > 0) {
        QPen pen = series->pen();
        pen.setWidth(1);
        series->setPen(pen);
        series->setBorderColor(series->color());
    }
    size_t i = 0;
    for (XT const& x : X) {
        series->append(x, Y[i++]);
    }
    chart->addSeries(series);
    return series;
}

template <typename YT>
static QScatterSeries* scatter(
    QChart *const chart,
    QList<YT> const& Y,
    QString const& label = "",
    QBrush const& brush = Qt::NoBrush,
    int width = -1
    ) {
    QList<int> X;
    for (int i = 0; i < Y.size(); ++i) {
        X.append(i);
    }
    return scatter(chart, X, Y, label, brush, width);
}

// Jet colormap (t in [0,1])
inline QColor jetColor(float t) {
    t = qBound(0.0f, t, 1.0f);
    float r, g, b;
    if (t < 0.125f) {
        r = 0; g = 0; b = 0.5f + 4.0f * t;
    } else if (t < 0.375f) {
        r = 0; g = 4.0f * (t - 0.125f); b = 1.0f;
    } else if (t < 0.625f) {
        r = 4.0f * (t - 0.375f); g = 1.0f; b = 1.0f - 4.0f * (t - 0.375f);
    } else if (t < 0.875f) {
        r = 1.0f; g = 1.0f - 4.0f * (t - 0.625f); b = 0;
    } else {
        r = 1.0f - 4.0f * (t - 0.875f); g = 0; b = 0;
    }
    return QColor::fromRgbF(qBound(0.0f, r, 1.0f), qBound(0.0f, g, 1.0f), qBound(0.0f, b, 1.0f));
}

// Heatmap
// Z is [columns][rows] with values in [0,1]. Rendered with Jet colormap.
static void heatmap(
    QChart *const chart,
    QList<QList<float>> const& Z,
    QPair<float,float> xRange = {0, 1},
    QPair<float,float> yRange = {0, 1},
    QString const& xTitle = "",
    QString const& yTitle = ""
    ) {
    if (Z.isEmpty() || Z[0].isEmpty()) return;
    int const nCols = Z.size();
    int const nRows = Z[0].size();
    float minVal = INFINITY;
    float maxVal = -INFINITY;
    for (auto const& col : Z) {
        for (auto const& v : col) {
            minVal = qMin(minVal, v);
            maxVal = qMax(maxVal, v);
        }
    }
    float const range = maxVal - minVal;

    QImage image(nCols, nRows, QImage::Format_RGB32);
    for (int x = 0; x < nCols; ++x) {
        for (int y = 0; y < nRows; ++y) {
            float const t = (range != 0.0f) ? (Z[x][y] - minVal) / range : 0.0f;
            image.setPixel(x, nRows - 1 - y, jetColor(t).rgb());
        }
    }

    auto *stretcher = new QLineSeries();
    stretcher->append(xRange.first, yRange.first);
    stretcher->append(xRange.second, yRange.second);
    chart->addSeries(stretcher);
    stretcher->setVisible(false);

    auto *axisX = new QValueAxis();
    axisX->setRange(xRange.first, xRange.second);
    axisX->setTitleText(xTitle);
    axisX->setGridLineVisible(false);
    auto *axisY = new QValueAxis();
    axisY->setRange(yRange.first, yRange.second);
    axisY->setTitleText(yTitle);
    axisY->setGridLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    stretcher->attachAxis(axisX);
    stretcher->attachAxis(axisY);

    chart->legend()->hide();
    chart->setPlotAreaBackgroundVisible(true);
    QObject::connect(chart, &QChart::plotAreaChanged, chart, [chart, image](const QRectF &plotArea) {
        if (plotArea.isEmpty()) return;
        QPixmap scaled = QPixmap::fromImage(image).scaled(
            plotArea.size().toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        chart->setPlotAreaBackgroundBrush(QBrush(scaled));
    });
}

// Text

template <typename XT, typename YT>
static void text(XT const x, YT const y, QString const& t) {

}

// Axes

static void makeAxes(
    QChart *const chart,
    QString const& xTitle,
    QString const& yTitle = ""
    ) {
    chart->createDefaultAxes();
    auto const& xAxes = chart->axes(Qt::Horizontal);
    auto const& yAxes = chart->axes(Qt::Vertical);
    if (not xAxes.isEmpty()) {
        xAxes.first()->setTitleText(xTitle);
    }
    if (not yAxes.isEmpty()) {
        yAxes.first()->setTitleText(yTitle);
    }
}

}; // namespace plot


#endif // PLOT_H
