#include "tpi-image-builder.h"

#include <QCoreApplication>

#include <Tui/ZImage.h>
#include <Tui/ZPainter.h>
#include <Tui/ZTerminal.h>

void export_tpi(Tui::ZTerminal *terminal, QString name, int x, int y, int w, int h) {
    terminal->forceRepaint();

    Tui::ZImage img(terminal,w,h);
    img.painter().drawImage(0, 0, terminal->grabCurrentImage(), x, y, w, h);
    img.save(name +".tpi");
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    boxlayout();
    button();
    checkBox();
    dialog();
    inputBox();
    label();
    listView();
    menu();
    radiobutton();
    root();
    textedit();
    textline();
    window();

    demo_gettingstarted();

    return 0;
}
