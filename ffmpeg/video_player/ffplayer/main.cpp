#include "widget.h"
#include "ffPlayer.h"
#include <QApplication>
#include <qobject.h>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOverrideCursor(Qt::BlankCursor);
    char url[1024] = "/root/2-hour-clockwise.mp4";
    if(argc == 2)
        strcpy(url, argv[1]);

    int version = avcodec_version();
    qDebug("adcodec version %d.%d.%d",
            version>>16, (version>>8)&0xff, version&0xff);

    QFile style("/home/root/app/ui.css");
    style.open(QFile::ReadOnly);
    QString ui_css = QLatin1String(style.readAll());
    style.close();

    Widget w(NULL, url);
    w.setStyleSheet(ui_css);
    w.show();
    return a.exec();
}
