#include "node.h"

#include "crc32.h"

Node::Node()
{
    connect(this, &Node::signalNewFrame, this, &Node::printFrame);
}

Node::~Node() {
    if (_port != nullptr) delete _port;
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
    //progressBar->reset();
    emit active(false);
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

    connect(_port, &QSerialPort::readyRead, this, [this]() {
        processIncomingBytes(_port->readAll());
    });
    connect(_port, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError e) {
        if (e == QSerialPort::SerialPortError::NoError) {
            return;
        }
        emit active(false);
        qDebug() << _portName << "Error :" << _port->errorString();
    });

    bool const ok = _port->open(QIODevice::ReadWrite);
    if (not ok) {
        qCritical() << "Couldn't open COM port" << _portName << ":" << _port->errorString();
    }
    emit active(ok);
    return ok;
}

void Node::reconnect(void) {
    close();
    open();
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

        emit progress(0, _currentFrame.nBytes);
        _pendingBuffer.remove(0, headerSize);
        // now comes data. re-call process to start extraction
        processIncomingBytes(QByteArray());
    } else {
        // Currently in Byte mode, add to pending frame
        int const nToTake = qMin(_pendingBuffer.size(), _currentFrame.nBytes - _currentFrame.data.size());
        _currentFrame.data.append(_pendingBuffer.left(nToTake));
        _pendingBuffer.remove(0, nToTake);
        emit progress(_currentFrame.data.size(), _currentFrame.nBytes);

        // Frame completed, handle it and go back to Text mode
        if (_currentFrame.data.size() == _currentFrame.nBytes) {
            _frameBuffer = _currentFrame;
            // Verify CRC
            _frameBuffer.calculatedCrc = crc32((uint8_t*)_frameBuffer.data.data(), _frameBuffer.data.size());
            _frameBuffer.crcIsOk = _frameBuffer.calculatedCrc != 0 and _frameBuffer.crc == _frameBuffer.calculatedCrc;

            emit signalNewFrame(_frameBuffer);
            _currentFrame.data.clear();
            _currentFrame.id = 0;
            _currentFrame.nBytes = 0;
            _currentFrame.crc = 0;
            emit progress(0, 1);
            processIncomingBytes(QByteArray());
        }
    }
}


void Node::handleText(QByteArray const& bytes) {
    emit signalNewChars(QString::fromLatin1(bytes));
}

void Node::printFrame(Frame const& frame) {
    handleText(QString("Frame(id=%1, len=%2%3)\r\n").arg(
        QString::number(frame.id),
        QString::number(frame.data.size()),
        (frame.crcIsOk ? "" : (", BAD CHECKSUMS: " + QString::number(frame.calculatedCrc, 16) + " " + QString::number(_currentFrame.crc, 16)))
    ).toLatin1());
    if (not frame.crcIsOk) {
        qWarning() << "Bad checksum from node" << name() << "frame id" << frame.id;
    }
}

void Node::write(QByteArray const& bytes) {
    if (_port == nullptr) {
        qCritical() << "Node::write called but port is not open for node" << name();
        return;
    }
    _port->write(bytes);
}

