#include <QApplication>
#include "watchdial.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    watchdial dial;
    dial.resize(200, 200);
    dial.show();

    return app.exec();
}
