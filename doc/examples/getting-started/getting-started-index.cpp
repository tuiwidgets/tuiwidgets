#include <QCoreApplication>
#include <QRect>

#include <Tui/ZButton.h>
#include <Tui/ZRoot.h>
#include <Tui/ZShortcut.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZWindow.h>

class Root : public Tui::ZRoot {
public:
    void terminalChanged() override {
// snippet-setup-start
        Tui::ZWindow *win = new Tui::ZWindow("Hello World", this);
        win->setGeometry({5, 3, 20, 10});
        QObject::connect(new Tui::ZShortcut(Tui::ZKeySequence::forKey(Qt::Key_Escape),
                                            this,
                                            Qt::ApplicationShortcut),
                         &Tui::ZShortcut::activated,
                         this, &Root::quit);
        Tui::ZButton *button = new Tui::ZButton(Tui::withMarkup, "<m>Q</m>uit", win);
        QObject::connect(button, &Tui::ZButton::clicked, this, &Root::quit);
        button->setGeometry({6, 7, 10, 1});
        button->setFocus();
// snippet-setup-end
    }

    void quit() {
        QCoreApplication::instance()->quit();
    }
};


// snippet-main-start
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    Tui::ZTerminal terminal;

    Root root;

    terminal.setMainWidget(&root);

    return app.exec();
}
// snippet-main-end
