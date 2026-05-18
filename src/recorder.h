#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QTimer>
#include <atomic>

#include "frame.h"

class Recorder: public QObject
{
    Q_OBJECT
public:
    Recorder(QObject *const parent = nullptr);
    QVector<QVector<Frame>> const& getData() const { return _recordings; }

signals:
    void fetch(void);
    void data(QVector<QVector<Frame>> const&);
    void recordProgress(int v, int m);

public slots:
    // From UI
    void setRecordCount(int const count) { _shouldCount = count; }
    void setRecordTimer(int const msec) { _interval = msec; }
    void startRecording();
    void stopRecording();
    void clear() { _recordings.clear(); emit data({}); }

    // From collector
    void newCollection(QVector<Frame> const&);

private:
    int _interval;
    QTimer* _timer = nullptr;
    int _shouldCount;
    std::atomic<int> _isCount;

    QVector<QVector<Frame>> _recordings;
};

#endif // RECORDER_H
