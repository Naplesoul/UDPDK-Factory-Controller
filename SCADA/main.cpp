#include "Master.h"
#include "widgets.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow m;

    return a.exec();
}
