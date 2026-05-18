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
    explicit Node(
        QObject *parent,
        QString const& portName,
        QTextEdit* terminal,
        QLineEdit* command,
        QProgressBar* progressBar
    );

    QString const& name() { return _portName; }
    QTextEdit* terminal;
    QLineEdit* command;
    QProgressBar* progressBar;
    Frame const& frame() { return _frameBuffer; }

public slots:
    void close();
    bool open();
    void clear();
    void write(QByteArray const& b) { _port->write(b); }

signals:
    void signalNewFrame(Frame);

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
