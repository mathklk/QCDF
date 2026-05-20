#include <QApplication>
#include <QThread>
#include <QTranslator>
#include <QLibraryInfo>

#include "metaTypes.h"

#include "mainwindow.h"
#include "recorder.h"
#include "node.h"
#include "collector.h"

int main(int argc, char *argv[])
{    
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icon.jpg"));
    registerMetaTypes();

    QVector<QThread*> threads;

    Recorder *const recorder = new Recorder();
    QThread *const recorderThread = new QThread();;
    recorder->moveToThread(recorderThread);
    threads << recorderThread;

    QVector<Node*> nodes;
    for (int i = 0; i < 4; ++i) {
        Node *const node = new Node();
        nodes << node;
        QThread *const nodeThread = new QThread();
        node->moveToThread(nodeThread);
        threads << nodeThread;
    }

    Collector *const collector = new Collector({ nodes[0], nodes[1], nodes[2] }, nodes[3]);
    QThread *const collectorThread = new QThread();
    collector->moveToThread(collectorThread);
    threads << collectorThread;

    QObject::connect(collector, &Collector::signalNewCollection, recorder, &Recorder::newCollection);
    QObject::connect(recorder, &Recorder::fetch, collector, &Collector::fetch);

    for (QThread *const t : threads) {
        t->start();
    }

    MainWindow w(recorder, nodes, collector);
    w.show();

    int const exec = QCoreApplication::exec();
    for (QThread *const t : threads) {
        t->quit();
    }
    return exec;
}
