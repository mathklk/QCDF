#ifndef NODE_H
#define NODE_H

#include "frame.h"

#include <QObject>
#include <QSerialPort>
#include <QTextEdit>
#include <QLineEdit>
#include <QProgressBar>

class Node : public QObject
{
    Q_OBJECT
public:
    explicit Node();
    ~Node();

    void setPortName(QString const& name) { _portName = name; }
    QString const& name() { return _portName; }
    Frame const& frame() { return _frameBuffer; }

public slots:
    void close();
    bool open();
    void reconnect();
    void write(QByteArray const& b);

signals:
    void signalNewFrame(Frame);
    void signalNewChars(QString);

    void active(bool);
    void progress(int, int);

private slots:
    void processIncomingBytes(QByteArray const&);
    void handleText(QByteArray const& str);
    void printFrame(Frame const& frame);

private:
    QString _portName;
    QSerialPort* _port = nullptr;

    // Streaming state
    QByteArray _pendingBuffer;
    Frame _currentFrame;
    Frame _frameBuffer;
};

#endif // NODE_H
