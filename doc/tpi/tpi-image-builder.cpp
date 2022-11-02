#include <QCoreApplication>

#include <Tui/ZButton.h>
#include <Tui/ZCheckBox.h>
#include <Tui/ZColor.h>
#include <Tui/ZCommandNotifier.h>
#include <Tui/ZHBoxLayout.h>
#include <Tui/ZInputBox.h>
#include <Tui/ZImage.h>
#include <Tui/ZLabel.h>
#include <Tui/ZMenu.h>
#include <Tui/ZMenuItem.h>
#include <Tui/ZMenubar.h>
#include <Tui/ZPainter.h>
#include <Tui/ZPalette.h>
#include <Tui/ZRadioButton.h>
#include <Tui/ZShortcut.h>
#include <Tui/ZSymbol.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTextLayout.h>
#include <Tui/ZTextMetrics.h>
#include <Tui/ZVBoxLayout.h>
#include <Tui/ZWindow.h>

#include "button.h"
#include "checkBox.h"
#include "inputBox.h"
#include "label.h"
//#include "tpi-image-builder.h"

#include <Tui/ZRoot.h>

void export_tpi(Tui::ZTerminal *terminal, QString name, int x, int y, int w, int h) {
    terminal->forceRepaint();
    Tui::ZImage img_with_terminal = terminal->grabCurrentImage();
    img_with_terminal.painter().setForeground(0,0,{0xff,0xff,0xff});

    Tui::ZImage img(terminal,w,h);
    img.painter().drawImage(0,0,img_with_terminal, x, y, w, h);
    img.painter().setForeground(0,0,{0xff,0xff,0xff});

    img.save(name +".tpi");
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    button();
    checkBox();
    inputBox();
    label();

    return 0; // app.exec();
}
