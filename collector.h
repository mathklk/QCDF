#ifndef COLLECTOR_H
#define COLLECTOR_H

#include "node.h"

#include <QObject>
#include <QVector>

class Collector : public QObject
{
    Q_OBJECT
public:
    explicit Collector(QVector<Node*> const& slaves, Node *const master, QObject* parent = nullptr);

public slots:
    void reset();
    void fetch();
    QVector<Frame> const& collection() { return _frameBuffer; }
    bool isFetchInProgress() const { return _fetchInProgress; }

private slots:
    void checkIfCollected();

signals:
    void signalFetchStarted(void);
    void signalNewCollection();

private:
    Node* _master;
    QVector<Frame> _frameBuffer;
    bool _fetchInProgress = false;
};

#endif // COLLECTOR_H
