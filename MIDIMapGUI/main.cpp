#include "MIDIMap.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);



    MIDIMap w;
    w.show();
    return a.exec();
}
