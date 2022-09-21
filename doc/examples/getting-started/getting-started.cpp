// snippet-header-start
#include "getting-started.h"

#include <QCoreApplication>
#include <QRect>

#include <Tui/ZButton.h>
#include <Tui/ZShortcut.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWindow.h>
// snippet-header-end

// snippet-root-start
void Root::terminalChanged() {
    // (1)
    Tui::ZShortcut *shortcut = new Tui::ZShortcut(Tui::ZKeySequence::forKey(Tui::Key_Escape),
                                                  this, Tui::ApplicationShortcut);
    QObject::connect(shortcut,
                     &Tui::ZShortcut::activated,
                     this, &Root::quit);

    // (2)
    Tui::ZWindow *win = new Tui::ZWindow("Hello World", this);
    win->setGeometry({5, 3, 20, 10});

    // (3)
    Tui::ZButton *button = new Tui::ZButton(Tui::withMarkup, "<m>Q</m>uit", win);
    QObject::connect(button, &Tui::ZButton::clicked, this, &Root::quit);
    button->setGeometry({6, 7, 10, 1});
    button->setFocus();
}

// (4)
void Root::quit() {
    QCoreApplication::instance()->quit();
}
// snippet-root-end


// snippet-main-start
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    Tui::ZTerminal terminal;

    Root root;

    terminal.setMainWidget(&root);

    return app.exec();
}
// snippet-main-end
