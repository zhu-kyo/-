#include "widget.h"

#include <QApplication>
#include <QFile>
#include <QMediaPlayer>
#include <QMediaContent>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);
    QApplication a(argc, argv);

    QFile styleFile(":/qss/app.qss");
    if(styleFile.open(QFile::ReadOnly))
        a.setStyleSheet(styleFile.readAll());

    styleFile.close();

    Widget w;
    w.show();


    return a.exec();
}
