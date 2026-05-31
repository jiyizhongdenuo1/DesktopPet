#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDateTime>
#include "CMainNoteListViewModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    CMainNoteListViewModel noteModel;

    const QUrl url(u"qrc:/qt/qml/Widgets/MainWidget.qml"_qs);

    noteModel.AddNote("欢迎使用 DesktopPet!", QDateTime::currentDateTime());
    noteModel.AddNote("点击我可以改变状态哦~", QDateTime::currentDateTime())  ;

    engine.rootContext()->setContextProperty("noteModel", &noteModel);
    // 连接信号，防止加载失败静默退出
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}