#include "collector.h"

#include <QDateTime>

Collector::Collector(
    QVector<Node*> const& slaves,
    Node *const master,
    QObject* parent
    ):
    QObject{parent},
    _master(master)
{
    _frameBuffer.reserve(3);
    int i = 0;
    for (Node* slave : slaves) {
        _frameBuffer << Frame();
        connect(slave, &Node::signalNewFrame, this, [this, i](Frame frame){
            _frameBuffer[i] = frame;
            checkIfCollected();
        });
        ++i;
    }
}

void Collector::reset() {
    _fetchInProgress = false;
    for (auto& frame : _frameBuffer) {
        frame = Frame();
    }
}

void Collector::fetch() {
    if (_fetchInProgress) {
        qCritical() << "Collector::fetch called but fetch is already in progress.";
        return;
    }
    _fetchInProgress = true;
    _fetchStarted = QDateTime::currentDateTime();
    for (auto& frame : _frameBuffer) {
        frame = Frame();
    }
    QMetaObject::invokeMethod(_master, "write", Q_ARG(QByteArray, QString('x').toLatin1()));
    //_master->write(QString('x').toLatin1());
    emit signalFetchStarted();
}

void Collector::checkIfCollected() {
    bool collectedAll = true;
    for (auto& frame : _frameBuffer) {
        collectedAll &= frame.isValid();
    }
    if (not collectedAll or not _fetchInProgress) {
        return;
    }
    _fetchInProgress = false;
    emit signalNewCollection(collection());
    qInfo() << "Collector: Total Fetch Time: " << _fetchStarted.msecsTo(QDateTime::currentDateTime()) << "ms";
}
