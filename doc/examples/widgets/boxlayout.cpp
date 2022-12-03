#include <Tui/ZButton.h>
#include <Tui/ZHBoxLayout.h>
#include <Tui/ZWidget.h>
#include <Tui/ZVBoxLayout.h>

#include <Tui/ZLabel.h>
#include <Tui/ZInputBox.h>


void exampleBoxlayoutStretchStart(Tui::ZWidget *widget) {
// snippet-stretchstart-start
    Tui::ZHBoxLayout *layout = new Tui::ZHBoxLayout();
    layout->setSpacing(1);
    widget->setLayout(layout);

    layout->addStretch();

    Tui::ZButton *item1 = new Tui::ZButton(Tui::withMarkup, "Button1", widget);
    layout->addWidget(item1);

    Tui::ZButton *item2 = new Tui::ZButton(Tui::withMarkup, "Button2", widget);
    layout->addWidget(item2);
// snippet-stretchstart-end
}

void exampleBoxlayoutStretchMid(Tui::ZWidget *widget) {
// snippet-stretchmid-start
    Tui::ZHBoxLayout *layout = new Tui::ZHBoxLayout();
    layout->setSpacing(1);
    widget->setLayout(layout);

    Tui::ZButton *item1 = new Tui::ZButton(Tui::withMarkup, "Button1", widget);
    layout->addWidget(item1);

    layout->addStretch();

    Tui::ZButton *item2 = new Tui::ZButton(Tui::withMarkup, "Button2", widget);
    layout->addWidget(item2);
// snippet-stretchmid-end
}

void exampleBoxlayoutNestedHV(Tui::ZWidget *widget) {
// snippet-nestedhv-start
        Tui::ZHBoxLayout *hbox = new Tui::ZHBoxLayout();
        hbox->setSpacing(1);
        widget->setLayout(hbox);
        
        Tui::ZVBoxLayout *vbox1 = new Tui::ZVBoxLayout();
        vbox1->setSpacing(1);

        Tui::ZLabel *labelFind = new Tui::ZLabel(Tui::withMarkup, "F<m>i</m>nd", widget);
        vbox1->addWidget(labelFind);
        Tui::ZLabel *labelReplace = new Tui::ZLabel(Tui::withMarkup, "Replace", widget);
        vbox1->addWidget(labelReplace);
        hbox->add(vbox1);

        Tui::ZVBoxLayout *vbox2 = new Tui::ZVBoxLayout();
        vbox2->setSpacing(1);
        Tui::ZInputBox *searchText = new Tui::ZInputBox(widget);
        labelFind->setBuddy(searchText);
        searchText->setText("one");
        vbox2->addWidget(searchText);
        Tui::ZInputBox *replaceText = new Tui::ZInputBox(widget);
        labelReplace->setBuddy(replaceText);
        replaceText->setText("two");
        vbox2->addWidget(replaceText);

        hbox->add(vbox2);
// snippet-nestedhv-end
}

void exampleBoxlayoutNestedVH(Tui::ZWidget *widget) {
// snippet-nestedvh-start
    Tui::ZVBoxLayout *vbox = new Tui::ZVBoxLayout();
    vbox->setSpacing(1);
    widget->setLayout(vbox);

    Tui::ZHBoxLayout *hbox1 = new Tui::ZHBoxLayout();
    hbox1->setSpacing(1);
    Tui::ZLabel *labelFind = new Tui::ZLabel(Tui::withMarkup, "F<m>i</m>nd", widget);
    hbox1->addWidget(labelFind);
    Tui::ZInputBox *searchText = new Tui::ZInputBox(widget);
    labelFind->setBuddy(searchText);
    searchText->setText("one");
    hbox1->addWidget(searchText);
    vbox->add(hbox1);
    
    Tui::ZHBoxLayout *hbox2 = new Tui::ZHBoxLayout();
    hbox2->setSpacing(1);
    Tui::ZLabel *labelReplace = new Tui::ZLabel(Tui::withMarkup, "Replace", widget);
    hbox2->addWidget(labelReplace);
    Tui::ZInputBox *replaceText = new Tui::ZInputBox(widget);
    labelReplace->setBuddy(replaceText);
    replaceText->setText("two");
    hbox2->addWidget(replaceText);

    vbox->add(hbox2);
// snippet-nestedvh-end
}

