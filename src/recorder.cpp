#include "recorder.h"

#include <QFile>

Recorder::Recorder(QObject *const parent):
    QObject(parent)
{

}

void Recorder::startRecording() {
    if (_timer == nullptr) {
        _timer = new QTimer(this);
        connect(_timer, &QTimer::timeout, this, &Recorder::fetch);
    }

    if (_timer->isActive()) {
        qCritical() << "Recorder::startRecording called while already recording.";
        return;
    }
    _timer->setInterval(_interval);
    _isCount = 0;
    _timer->start();
}

void Recorder::stopRecording() {
    if (!_timer->isActive()) {
        qCritical() << "Recorder::stopRecording called while not recording.";
        return;
    }
    _timer->stop();
}

void Recorder::newCollection(QVector<Frame> const& frames) {
    _recordings << frames;
    _isCount++;
    emit recordProgress(_isCount, _shouldCount);
    emit data(_recordings);
    if (_isCount >= _shouldCount) {
        stopRecording();
    }

    // Save recording as JSON
    /*
    if (not _recordDir.isEmpty()) {
        QJsonArray json;
        for (Frame const& frame : frames) {
            json << frame.asComplex().toJson();
        }
        int const id = frames.first().id;
        QFile file(_recordDir + "/" + QString("%1").arg(id, 5, 10, QChar('0')) + ".json");
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(json).toJson(QJsonDocument::Compact));
        } else {
            qWarning() << "Couldn't open save file" << file.errorString();
        }
    }
    */
}