#pragma once
#include <QApplication>
#include <QFontDatabase>
#include <QDirIterator>
#include <QIcon>
#include <QDebug>

void tianli_init(int argc, char *argv[])
{
    // 高分辨率屏幕支持
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        // break :/qt-project.org/*
        auto path = it.next();
        if (path.contains(":/qt-project.org/"))
            break;

        qDebug() << path;
    }

    // 加载字体
    try
    {
        // auto id = QFontDatabase::addApplicationFont(":/font/resource/font/UIDFont.ttf");
        // if (id == -1)
        //     qDebug() << "Failed to load font.";
    }
    catch (const std::exception &e)
    {
        qDebug() << e.what();
    }
}