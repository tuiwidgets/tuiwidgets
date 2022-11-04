#include <QCoreApplication>

#include <Tui/ZImage.h>
#include <Tui/ZPainter.h>
#include <Tui/ZPalette.h>
#include <Tui/ZTerminal.h>

#include "button.h"
#include "checkBox.h"
#include "dialog.h"
#include "inputBox.h"
#include "label.h"
#include "listView.h"
#include "menue.h"
#include "radiobutton.h"
#include "textline.h"

#include "demo_gettingstarted.h"

#include <Tui/ZRoot.h>

void export_tpi(Tui::ZTerminal *terminal, QString name, int x, int y, int w, int h) {
    terminal->forceRepaint();

    Tui::ZImage img(terminal,w,h);
    img.painter().drawImage(0,0,terminal->grabCurrentImage(), x, y, w, h);
    img.save(name +".tpi");
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    button();
    checkBox();
    dialog();
    inputBox();
    label();
    listView();
    menue();
    radiobutton();
    textline();

    demo_gettingstarted();

    return 0; // app.exec();
}
