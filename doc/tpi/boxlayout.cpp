#include "tpi-image-builder.h"

#include <QObject>
#include <QRect>

#include <Tui/ZButton.h>
#include <Tui/ZHBoxLayout.h>
#include <Tui/ZVBoxLayout.h>
#include <Tui/ZInputBox.h>
#include <Tui/ZDialog.h>
#include <Tui/ZLabel.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>

void boxlayout() {
    Tui::ZTerminal terminal(Tui::ZTerminal::OffScreen(80, 40));

    Tui::ZRoot root;
    terminal.setMainWidget(&root);

    // h intro
    {
        Tui::ZDialog dialog(&root);
        dialog.setFocus();
        dialog.setBorderEdges({});
        dialog.setContentsMargins({1, 1, 1, 1});

        dialog.setGeometry({0, 0, 42, 3});
        Tui::ZHBoxLayout layout;
        layout.setSpacing(1);
        dialog.setLayout(&layout);

        Tui::ZInputBox item1(&dialog);
        item1.setText("widget 1");
        layout.addWidget(&item1);

        Tui::ZInputBox item2(&dialog);
        item2.setText("widget 2");
        layout.addWidget(&item2);

        Tui::ZInputBox item3(&dialog);
        item3.setText("widget 3");
        layout.addWidget(&item3);

        export_tpi(&terminal, "boxlayouts-intro-h", 0, 0, 42, 3);
    }

    // v intro
    {
        Tui::ZDialog dialog(&root);
        dialog.setFocus();
        dialog.setBorderEdges({});
        dialog.setContentsMargins({1, 1, 1, 1});

        dialog.setGeometry({0, 0, 12, 7});
        Tui::ZVBoxLayout layout;
        layout.setSpacing(1);
        dialog.setLayout(&layout);

        Tui::ZInputBox item1(&dialog);
        item1.setText("widget 1");
        layout.addWidget(&item1);

        Tui::ZInputBox item2(&dialog);
        item2.setText("widget 2");
        layout.addWidget(&item2);

        Tui::ZInputBox item3(&dialog);
        item3.setText("widget 3");
        layout.addWidget(&item3);

        export_tpi(&terminal, "boxlayouts-intro-v", 0, 0, 12, 7);
    }

    // stretch start
    {
        Tui::ZDialog dialog(&root);
        dialog.setFocus();
        dialog.setBorderEdges({});
        dialog.setContentsMargins({1, 1, 1, 1});

        dialog.setGeometry({0, 0, 47, 3});
        Tui::ZHBoxLayout layout;
        layout.setSpacing(1);
        dialog.setLayout(&layout);

        layout.addStretch();

        Tui::ZButton item1(Tui::withMarkup, "Button1", &dialog);
        layout.addWidget(&item1);

        Tui::ZButton item2(Tui::withMarkup, "Button2", &dialog);
        layout.addWidget(&item2);

        export_tpi(&terminal, "boxlayouts-stretch-start", 0, 0, 47, 3);
    }

    // stretch mid
    {
        Tui::ZDialog dialog(&root);
        dialog.setFocus();
        dialog.setBorderEdges({});
        dialog.setContentsMargins({1, 1, 1, 1});

        dialog.setGeometry({0, 0, 47, 3});
        Tui::ZHBoxLayout layout;
        layout.setSpacing(1);
        dialog.setLayout(&layout);

        Tui::ZButton item1(Tui::withMarkup, "Button1", &dialog);
        layout.addWidget(&item1);

        layout.addStretch();

        Tui::ZButton item2(Tui::withMarkup, "Button2", &dialog);
        layout.addWidget(&item2);

        export_tpi(&terminal, "boxlayouts-stretch-mid", 0, 0, 47, 3);
    }

    // nested 1
    {
        Tui::ZDialog dialog(&root);
        dialog.setFocus();
        dialog.setBorderEdges({});
        dialog.setContentsMargins({1, 1, 1, 1});

        dialog.setGeometry({0, 0, 47, 5});

        Tui::ZVBoxLayout *vbox = new Tui::ZVBoxLayout();
        Tui::ZLabel *labelFind;
        Tui::ZInputBox *searchText;
        Tui::ZLabel *labelReplace;
        Tui::ZInputBox *replaceText;
        
        vbox->setSpacing(1);
        {
            Tui::ZHBoxLayout *hbox = new Tui::ZHBoxLayout();
            hbox->setSpacing(1);

            labelFind = new Tui::ZLabel(Tui::withMarkup, "F<m>i</m>nd", &dialog);
            hbox->addWidget(labelFind);

            searchText = new Tui::ZInputBox(&dialog);
            labelFind->setBuddy(searchText);
            searchText->setText("one");
            hbox->addWidget(searchText);

            vbox->add(hbox);
        }

        {
            Tui::ZHBoxLayout *hbox = new Tui::ZHBoxLayout();
            hbox->setSpacing(1);

            labelReplace = new Tui::ZLabel(Tui::withMarkup, "Replace", &dialog);
            hbox->addWidget(labelReplace);

            replaceText = new Tui::ZInputBox(&dialog);
            labelReplace->setBuddy(replaceText);
            replaceText->setText("two");
            hbox->addWidget(replaceText);

            vbox->add(hbox);
        }
        
        dialog.setLayout(vbox);

        export_tpi(&terminal, "boxlayouts-nested1", 0, 0, 47, 5);
    }

    // nested 2
    {
        Tui::ZDialog dialog(&root);
        dialog.setFocus();
        dialog.setBorderEdges({});
        dialog.setContentsMargins({1, 1, 1, 1});

        dialog.setGeometry({0, 0, 47, 5});

        Tui::ZHBoxLayout *hbox = new Tui::ZHBoxLayout();

        Tui::ZLabel *labelFind;
        Tui::ZInputBox *searchText;
        Tui::ZLabel *labelReplace;
        Tui::ZInputBox *replaceText;
        
        hbox->setSpacing(1);
        {
            Tui::ZVBoxLayout *vbox = new Tui::ZVBoxLayout();
            vbox->setSpacing(1);

            labelFind = new Tui::ZLabel(Tui::withMarkup, "F<m>i</m>nd", &dialog);
            vbox->addWidget(labelFind);

            labelReplace = new Tui::ZLabel(Tui::withMarkup, "Replace", &dialog);
            vbox->addWidget(labelReplace);

            hbox->add(vbox);
        }

        {
            Tui::ZVBoxLayout *vbox = new Tui::ZVBoxLayout();
            vbox->setSpacing(1);

            searchText = new Tui::ZInputBox(&dialog);
            labelFind->setBuddy(searchText);
            searchText->setText("one");
            vbox->addWidget(searchText);

            replaceText = new Tui::ZInputBox(&dialog);
            labelReplace->setBuddy(replaceText);
            replaceText->setText("two");
            vbox->addWidget(replaceText);

            hbox->add(vbox);
        }

        dialog.setLayout(hbox);

        export_tpi(&terminal, "boxlayouts-nested2", 0, 0, 47, 5);
    }

}
