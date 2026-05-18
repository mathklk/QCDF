#include "node.h"

#include "crc32.h"

Node::Node(
    QObject *parent,
    QString const& portName,
    QTextEdit* terminal_,
    QLineEdit* command_,
    QProgressBar* progressBar_
    ):
    QObject(parent),
    terminal(terminal_),
    command(command_),
    progressBar(progressBar_),
    _portName(portName)
{
    connect(this, &Node::signalNewFrame, this, &Node::printFrame);
}

void Node::close() {
    if (_port != nullptr && _port->isOpen()) {
        _port->close();
        delete _port;
        _port = nullptr;
    }

    _pendingBuffer.clear();
    _currentFrame.data.clear();
    _currentFrame.nBytes = 0;
    progressBar->reset();
}

bool Node::open() {
    close();

    if (_portName.isEmpty()) {
        return false;
    }

    _port = new QSerialPort(this);
    _port->setPortName(_portName);
    _port->setBaudRate(2e6);
    _port->setParity(QSerialPort::NoParity);
    _port->setStopBits(QSerialPort::OneStop);
    _port->setFlowControl(QSerialPort::NoFlowControl);

    terminal->setReadOnly(true);
    terminal->setLineWrapMode(QTextEdit::NoWrap);

    connect(_port, &QSerialPort::readyRead, this, [this]() {
        processIncomingBytes(_port->readAll());
    });
    connect(_port, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError e) {
        if (e == QSerialPort::SerialPortError::NoError) {
            return;
        }
        terminal->setEnabled(false);
        command->setEnabled(false);
        progressBar->setEnabled(false);
        qDebug() << _portName << "Error :" << _port->errorString();
    });

    command->setPlaceholderText(_portName);
    connect(command, &QLineEdit::returnPressed, this, [this]() {
        QByteArray const bytes = command->text().toLatin1();
        _port->write(bytes);
        command->clear();
    });

    return _port->open(QIODevice::ReadWrite);
}

void Node::clear() {
    terminal->clear();
    progressBar->setValue(0);
    progressBar->setMaximum(0);
}

void Node::processIncomingBytes(QByteArray const& buffer) {
    static const char SOH = '\x01';

    // Append new data to the pending buffer
    _pendingBuffer.append(buffer);

    if (_currentFrame.nBytes == 0) {
        // Currently in Text mode

        int const sohIndex = _pendingBuffer.indexOf(SOH);
        // No SOH in buffer, just normal text
        if (sohIndex < 0) {
            handleText(_pendingBuffer);
            _pendingBuffer.clear();
            return;
        }
        // Handle normal text up until SOH
        handleText(_pendingBuffer.left(sohIndex));
        _pendingBuffer.remove(0, sohIndex);

        // If Header is missing or incomplete, wait for next call
        int const headerSize = 9;
        if (_pendingBuffer.size() < headerSize) {
            return;
        }
        // We know SOH is at index 0
        // ID
        _currentFrame.id = (quint32(quint8(_pendingBuffer.at(1))) << 8) |
                           (quint32(quint8(_pendingBuffer.at(2)))     ) ;
        // Length / Number of Bytes in Data
        _currentFrame.nBytes = (quint32(quint8(_pendingBuffer.at(3))) << 8) |
                               (quint32(quint8(_pendingBuffer.at(4)))     ) ;
        // Checksum
        _currentFrame.crc = (quint32(quint8(_pendingBuffer.at(5))) << 24) |
                            (quint32(quint8(_pendingBuffer.at(6))) << 16) |
                            (quint32(quint8(_pendingBuffer.at(7))) <<  8) |
                            (quint32(quint8(_pendingBuffer.at(8)))      ) ;

        progressBar->setMaximum(_currentFrame.nBytes);
        _pendingBuffer.remove(0, headerSize);
        // now comes data. re-call process to start extraction
        processIncomingBytes(QByteArray());
    } else {
        // Currently in Byte mode, add to pending frame
        int const nToTake = qMin(_pendingBuffer.size(), _currentFrame.nBytes - _currentFrame.data.size());
        _currentFrame.data.append(_pendingBuffer.left(nToTake));
        _pendingBuffer.remove(0, nToTake);
        progressBar->setValue(_currentFrame.data.size());

        // Frame completed, handle it and go back to Text mode
        if (_currentFrame.data.size() == _currentFrame.nBytes) {
            _frameBuffer = _currentFrame;
            emit signalNewFrame(_frameBuffer);
            _currentFrame.data.clear();
            _currentFrame.id = 0;
            _currentFrame.nBytes = 0;
            _currentFrame.crc = 0;
            progressBar->setValue(0);
            processIncomingBytes(QByteArray());
        }
    }
}


void Node::handleText(QByteArray const& str) {
    int const LIMIT = 1000;
    QString cat = terminal->toPlainText() + QString::fromLatin1(str);
    terminal->setText(
        cat.sliced(qMax(0, cat.size() - LIMIT))
        );
    QTextCursor cursor = terminal->textCursor();
    cursor.movePosition(QTextCursor::End);
    terminal->setTextCursor(cursor);
}

void Node::printFrame(Frame const& frame) {
    quint32 const calculatedCrc = crc32((uint8_t*)frame.data.data(), frame.data.size());
    bool const checksumsOk = calculatedCrc != 0 and _currentFrame.crc == calculatedCrc;
    handleText(QString("Frame(id=%1, len=%2%3)\r\n").arg(
        QString::number(frame.id),
        QString::number(frame.data.size()),
        (checksumsOk ? "" : (", BAD CHECKSUMS: " + QString::number(calculatedCrc, 16) + " " + QString::number(_currentFrame.crc, 16)))
    ).toLatin1());
    if (not checksumsOk) {
        qWarning() << "Bad checksum from node" << name() << "frame id" << frame.id;
    }
}