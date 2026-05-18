#include "collector.h"

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
    for (auto& frame : _frameBuffer) {
        frame = Frame();
    }
    _fetchInProgress = true;
    _master->write(QString('x').toLatin1());
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
    emit signalNewCollection();
}
