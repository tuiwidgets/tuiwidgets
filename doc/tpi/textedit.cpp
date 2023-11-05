#include "tpi-image-builder.h"

#include <QBuffer>
#include <QObject>
#include <QRect>

#include <Tui/ZDialog.h>
#include <Tui/ZTextEdit.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>

static void addText(Tui::ZTextEdit *textedit, const QString &text) {
    QByteArray x = text.toUtf8();
    QBuffer data(&x);
    data.open(QIODevice::ReadOnly);
    textedit->readFrom(&data);
}

void textedit() {
    Tui::ZTerminal terminal (Tui::ZTerminal::OffScreen(40, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);

    Tui::ZDialog dialog(&root);
    dialog.setGeometry({0, 0, 19, 10});
    dialog.setFocus();
    dialog.setBorderEdges({});

    // default
    {
        Tui::ZTextEdit textedit(terminal.textMetrics(), &dialog);
        textedit.setGeometry({1, 1, 17, 3});
        addText(&textedit, "Hello World,\nit is a good day");

        export_tpi(&terminal, "textedit", 0, 0, 19, 5);
    }

    // default-selection
    {
        Tui::ZTextEdit textedit(terminal.textMetrics(), &dialog);
        textedit.setGeometry({1, 1, 17, 3});
        addText(&textedit, "Hello World,\nit is a good day");
        textedit.setAnchorPosition(Tui::ZTextEdit::Position{6, 0});
        textedit.setCursorPosition(Tui::ZTextEdit::Position{12, 0}, true);

        export_tpi(&terminal, "textedit-selection", 0, 0, 19, 5);
    }

    // focus
    {
        Tui::ZTextEdit textedit(terminal.textMetrics(), &dialog);
        textedit.setGeometry({1, 1, 17, 3});
        addText(&textedit, "Hello World,\nit is a good day");
        textedit.setFocus();

        export_tpi(&terminal, "textedit-focus", 0, 0, 19, 5);
    }

    // focus-selection
    {
        Tui::ZTextEdit textedit(terminal.textMetrics(), &dialog);
        textedit.setGeometry({1, 1, 17, 3});
        addText(&textedit, "Hello World,\nit is a good day");
        textedit.setAnchorPosition(Tui::ZTextEdit::Position{6, 0});
        textedit.setCursorPosition(Tui::ZTextEdit::Position{12, 0}, true);
        textedit.setFocus();

        export_tpi(&terminal, "textedit-focus-selection", 0, 0, 19, 5);
    }

    // disabled
    {
        Tui::ZTextEdit textedit(terminal.textMetrics(), &dialog);
        textedit.setGeometry({1, 1, 17, 3});
        addText(&textedit, "Hello World,\nit is a good day");
        textedit.setEnabled(false);

        export_tpi(&terminal, "textedit-disabled", 0, 0, 19, 5);
    }
}
