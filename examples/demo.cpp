// Feel free to copy from this example to your own code
// SPDX-License-Identifier: 0BSD OR BSL-1.0 OR MIT-0

#include "demo.h"

#include <QCoreApplication>

#include <Tui/ZButton.h>
#include <Tui/ZCheckBox.h>
#include <Tui/ZColor.h>
#include <Tui/ZCommandNotifier.h>
#include <Tui/ZHBoxLayout.h>
#include <Tui/ZInputBox.h>
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

Demo::Demo() {
}

Demo::~Demo() {
}

void Demo::terminalChanged() {
    ensureCommandManager();

    Tui::ZVBoxLayout *vbox = new Tui::ZVBoxLayout();
    setLayout(vbox);

    Tui::ZMenubar *menu = new Tui::ZMenubar(this);
    menu->setItems({ { "<m>F</m>ile", "", {}, {
                       { "<m>B</m>utton", "", "ButtonDialog", {}},
                       { "<m>C</m>heckBox", "", "CheckBoxDialog", {}},
                       { "<m>I</m>nputBox", "", "InputBoxDialog", {}},
                       { "<m>R</m>adioButton", "", "RadioButtonDialog", {}},
                       { "<m>L</m>istView", "", "ListViewDialog", {}},
                       { "L<m>a</m>bel", "", "LabelDialog", {}},
                       {},
                       { "Search and Replace", "", "SearchAndReplaceDialog", {}},
                       {},
                       { "<m>Q</m>uit", "Ctrl-q", "Quit", {}},
                       } },
                   { "<m>H</m>elp", "", {}, {
                       { "<m>A</m>bout", "", "AboutDialog", {}}
                     }}});

    vbox->addWidget(menu);

    Tui::ZTextLine *message = new Tui::ZTextLine(this);
    message->setGeometry({20,0,100,1});
    Tui::ZPalette p;
    p.setColors({{"control.fg", Tui::Colors::blue}, {"control.bg", Tui::Colors::lightGray}});
    message->setPalette(p);

    QObject::connect(new Tui::ZCommandNotifier("ButtonDialog", this), &Tui::ZCommandNotifier::activated,
                     [this, message] {
        new ButtonDialog(this, message);
    });

    QObject::connect(new Tui::ZCommandNotifier("CheckBoxDialog", this), &Tui::ZCommandNotifier::activated,
                     [this, message] {
        new CheckBoxDialog(this, message);
    });

    QObject::connect(new Tui::ZCommandNotifier("InputBoxDialog", this), &Tui::ZCommandNotifier::activated,
                     [this] {
        new InputBoxDialog(this);
    });

    QObject::connect(new Tui::ZCommandNotifier("RadioButtonDialog", this), &Tui::ZCommandNotifier::activated,
                     [this, message] {
        new RadioButtonDialog(this, message);
    });

    QObject::connect(new Tui::ZCommandNotifier("ListViewDialog", this), &Tui::ZCommandNotifier::activated,
                     [this] {
        new ListViewDialog(this);
    });

    QObject::connect(new Tui::ZCommandNotifier("LabelDialog", this), &Tui::ZCommandNotifier::activated,
                     [this] {
        new LabelDialog(this);
    });

    QObject::connect(new Tui::ZCommandNotifier("SearchAndReplaceDialog", this), &Tui::ZCommandNotifier::activated,
                     [this] {
        new SearchAndReplaceDialog(this);
    });

    QObject::connect(new Tui::ZCommandNotifier("AboutDialog", this), &Tui::ZCommandNotifier::activated,
                     [this] {
        new AboutDialog(this);
    });

    QObject::connect(new Tui::ZShortcut(Tui::ZKeySequence::forShortcut("q"), this, Tui::ApplicationShortcut), &Tui::ZShortcut::activated,
                        this, &Demo::quit);
    QObject::connect(new Tui::ZCommandNotifier("Quit", this), &Tui::ZCommandNotifier::activated,
                     this, &Demo::quit);

    // Background
    setFillChar(u'▒');
}

void Demo::quit() {
    QCoreApplication::instance()->quit();
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    Tui::ZTerminal terminal;
    Demo root;
    terminal.setMainWidget(&root);
    root.setFocus();

    return app.exec();
}

ButtonDialog::ButtonDialog(Tui::ZWidget *parent, Tui::ZTextLine *message) : Tui::ZDialog(parent) {
    setOptions(Tui::ZWindow::CloseOption | Tui::ZWindow::DeleteOnClose
               | Tui::ZWindow::MoveOption | Tui::ZWindow::AutomaticOption | Tui::ZWindow::ResizeOption);
    setDefaultPlacement(Qt::AlignCenter);

    setWindowTitle("Button");
    setContentsMargins({1, 1, 1, 1});


    Tui::ZVBoxLayout *vbox = new Tui::ZVBoxLayout();
    setLayout(vbox);
    vbox->setSpacing(1);

    Tui::ZHBoxLayout *hbox1 = new Tui::ZHBoxLayout();

    Tui::ZButton *button1 = new Tui::ZButton(this);
    button1->setText("disabled");
    button1->setEnabled(false);
    hbox1->addWidget(button1);

    Tui::ZButton *button2 = new Tui::ZButton(this);
    button2->setText("inactive");
    hbox1->addWidget(button2);

    Tui::ZButton *button3 = new Tui::ZButton(this);
    button3->setMarkup("<m>d</m>efault");
    button3->setDefault(true);
    button3->setFocus();
    hbox1->addWidget(button3);

    vbox->add(hbox1);

    QObject::connect(button1, &Tui::ZButton::clicked,
                     [=] {
        message->setText("Button 1 clicked");
    });

    QObject::connect(button2, &Tui::ZButton::clicked,
                     [=] {
        message->setText("Button 2 clicked");
    });

    QObject::connect(button3, &Tui::ZButton::clicked,
                     [=] {
        message->setText("Button 3 clicked");
    });

}

CheckBoxDialog::CheckBoxDialog(Tui::ZWidget *parent, Tui::ZTextLine *message) : Tui::ZDialog(parent) {
    setOptions(Tui::ZWindow::CloseOption | Tui::ZWindow::DeleteOnClose
               | Tui::ZWindow::MoveOption | Tui::ZWindow::AutomaticOption | Tui::ZWindow::ResizeOption);
    setDefaultPlacement(Qt::AlignCenter);

    setFocus();
    setWindowTitle("CheckBox");
    setContentsMargins({1, 1, 1, 1});


    Tui::ZVBoxLayout *vbox = new Tui::ZVBoxLayout();
    setLayout(vbox);
    vbox->setSpacing(1);

    Tui::ZCheckBox *checkbox1 = new Tui::ZCheckBox(this);
    checkbox1->setText("disabled");
    checkbox1->setEnabled(false);
    vbox->addWidget(checkbox1);

    Tui::ZCheckBox *checkbox2 = new Tui::ZCheckBox(this);
    checkbox2->setMarkup("<m>i</m>nactive");
    vbox->addWidget(checkbox2);

    Tui::ZCheckBox *checkbox3 = new Tui::ZCheckBox(this);
    checkbox3->setMarkup("<m>s</m>elected");
    checkbox3->setFocus();
    checkbox3->setCheckState(Tui::CheckState::Checked);
    vbox->addWidget(checkbox3);

    Tui::ZCheckBox *checkbox4 = new Tui::ZCheckBox(this);
    checkbox4->setMarkup("<m>t</m>ristate");
    checkbox4->setTristate(true);
    checkbox4->setCheckState(Tui::CheckState::PartiallyChecked);
    vbox->addWidget(checkbox4);

    QObject::connect(checkbox2, &Tui::ZCheckBox::stateChanged,
                     [=] {
        if(checkbox2->checkState() == Tui::CheckState::Unchecked)
            message->setText("CheckBox 2 state changed: Unchecked");
        if(checkbox2->checkState() == Tui::CheckState::Checked)
            message->setText("CheckBox 2 state changed: Checked");
    });

    QObject::connect(checkbox3, &Tui::ZCheckBox::stateChanged,
                     [=] {
        if(checkbox3->checkState() == Tui::CheckState::Unchecked)
            message->setText("CheckBox 3 state changed: Unchecked");
        if(checkbox3->checkState() == Tui::CheckState::Checked)
            message->setText("CheckBox 3 state changed: Checked");
    });

    QObject::connect(checkbox4, &Tui::ZCheckBox::stateChanged,
                     [=] {
        if(checkbox4->checkState() == Tui::CheckState::Unchecked)
            message->setText("CheckBox 4 state changed: Unchecked");
        if(checkbox4->checkState() == Tui::CheckState::Checked)
            message->setText("CheckBox 4 state changed: Checked");
        if(checkbox4->checkState() == Tui::CheckState::PartiallyChecked)
            message->setText("CheckBox 4 state changed: PartiallyChecked");
    });
}

RadioButtonDialog::RadioButtonDialog(Tui::ZWidget *parent, Tui::ZTextLine *message) : Tui::ZDialog(parent) {
    setOptions(Tui::ZWindow::CloseOption | Tui::ZWindow::DeleteOnClose
               | Tui::ZWindow::MoveOption | Tui::ZWindow::AutomaticOption | Tui::ZWindow::ResizeOption);
    setDefaultPlacement(Qt::AlignCenter);

    setWindowTitle("RadioButton");
    setContentsMargins({1, 1, 1, 1});

    Tui::ZHBoxLayout *hbox = new Tui::ZHBoxLayout();
    hbox->setSpacing(1);
    setLayout(hbox);

    {
        Tui::ZVBoxLayout *vbox1 = new Tui::ZVBoxLayout();
        vbox1->setSpacing(1);

        Tui::ZWidget *groupA = new Tui::ZWidget(this);
        groupA->setLayout(vbox1);

        Tui::ZRadioButton *radioButtonA1 = new Tui::ZRadioButton(groupA);
        radioButtonA1->setMarkup("A <m>d</m>isabled");
        radioButtonA1->setChecked(true);
        radioButtonA1->setEnabled(false);
        vbox1->addWidget(radioButtonA1);

        Tui::ZRadioButton *radioButtonA2 = new Tui::ZRadioButton(groupA);
        radioButtonA2->setMarkup("A i<m>n</m>active");
        radioButtonA2->setFocus();
        vbox1->addWidget(radioButtonA2);

        hbox->addWidget(groupA);

        QObject::connect(radioButtonA1, &Tui::ZRadioButton::toggled,
                         [=] {
            if(radioButtonA1->checked() == true)
                message->setText("RadioButton A1: Checked");
        });

        QObject::connect(radioButtonA2, &Tui::ZRadioButton::toggled,
                         [=] {
            if(radioButtonA2->checked() == true)
                message->setText("RadioButton A2: Checked");
        });

    }
    {
        Tui::ZVBoxLayout *vbox2 = new Tui::ZVBoxLayout();
        vbox2->setSpacing(1);

        Tui::ZWidget *groupB = new Tui::ZWidget(this);
        groupB->setLayout(vbox2);

        Tui::ZRadioButton *radioButtonB1 = new Tui::ZRadioButton(groupB);
        radioButtonB1->setMarkup("B <m>a</m>ctive");
        radioButtonB1->setChecked(true);
        vbox2->addWidget(radioButtonB1);

        Tui::ZRadioButton *radioButtonB2 = new Tui::ZRadioButton(groupB);
        radioButtonB2->setMarkup("B <m>i</m>nactive");
        vbox2->addWidget(radioButtonB2);

        hbox->addWidget(groupB);

        QObject::connect(radioButtonB1, &Tui::ZRadioButton::toggled,
                         [=] {
            if(radioButtonB1->checked() == true)
                message->setText("RadioButton B1: Checked");
        });

        QObject::connect(radioButtonB2, &Tui::ZRadioButton::toggled,
                         [=] {
            if(radioButtonB2->checked() == true)
                message->setText("RadioButton B2: Checked");
        });
    }
}


InputBoxDialog::InputBoxDialog(Tui::ZWidget *parent) : Tui::ZDialog(parent) {
    setOptions(Tui::ZWindow::CloseOption | Tui::ZWindow::DeleteOnClose
               | Tui::ZWindow::MoveOption | Tui::ZWindow::AutomaticOption | Tui::ZWindow::ResizeOption);
    setDefaultPlacement(Qt::AlignCenter);

    setWindowTitle("InputBox");
    setContentsMargins({1, 1, 1, 1});

    Tui::ZVBoxLayout *vbox = new Tui::ZVBoxLayout();
    vbox->setSpacing(1);
    setLayout(vbox);

    Tui::ZInputBox *inputbox1 = new Tui::ZInputBox("😎👍🐧🚀", this);
    vbox->addWidget(inputbox1);

    Tui::ZInputBox *inputbox2 = new Tui::ZInputBox(this);
    inputbox2->setFocus();
    vbox->addWidget(inputbox2);
}


FileModel::FileModel(const QDir &dir) : AbstractTableModelTrackBy(1), _dir(dir) {
    _watcher.addPath(dir.absolutePath());
    QObject::connect(&_watcher, &QFileSystemWatcher::directoryChanged, this, &FileModel::update);
    update();
}

void FileModel::update() {
    QVector<Row> newData;

    QFileInfoList list;
    _dir.setFilter(QDir::AllEntries | QDir::NoDot);
    _dir.setSorting(QDir::DirsFirst | QDir::Name);

    for (const QFileInfo &fileInfo: _dir.entryInfoList()) {
        Row row;
        row.key = fileInfo.absoluteFilePath();
        QMap<int, QVariant> column;

        if( fileInfo.isDir()) {
            column[Qt::DisplayRole] = fileInfo.fileName()+"/";
        } else {
            column[Qt::DisplayRole] = fileInfo.fileName();
        }
        row.columns.append(std::move(column));
        newData.append(row);
    }
    setData(newData);
}

void FileModel::setDirectory(const QDir &dir) {
    if (_dir == dir) {
        return;
    }
    _dir = dir;
    _watcher.removePaths(_watcher.directories());
    _watcher.addPath(dir.absolutePath());
    update();
}


ListViewDialog::ListViewDialog(Tui::ZWidget *parent) : Tui::ZDialog(parent) {
    setOptions(Tui::ZWindow::CloseOption | Tui::ZWindow::DeleteOnClose
               | Tui::ZWindow::MoveOption | Tui::ZWindow::AutomaticOption | Tui::ZWindow::ResizeOption);
    setDefaultPlacement(Qt::AlignCenter);

    setWindowTitle("ListView");
    setContentsMargins({1, 1, 1, 1});

    Tui::ZHBoxLayout *hbox = new Tui::ZHBoxLayout();
    hbox->setSpacing(1);
    setLayout(hbox);

    _model = std::make_unique<FileModel>(_dir);

    _listview = new Tui::ZListView(this);
    _listview->setMinimumSize({44,5});
    _listview->setModel(_model.get());
    _listview->setFocus();
    hbox->addWidget(_listview);

    QObject::connect(_listview, &Tui::ZListView::enterPressed, [this](int selected){
        (void)selected;
        userInput(_listview->currentItem());
    });

    refreshFolder();
}

void ListViewDialog::refreshFolder() {
    _model->setDirectory(_dir);
    _listview->setCurrentIndex(_model->index(0, 0));
}

void ListViewDialog::userInput(QString filename) {
    if (QFileInfo(_dir.filePath(filename)).isDir()) {
        _dir.setPath(_dir.filePath(filename));
        _dir.makeAbsolute();
        refreshFolder();
    }
}

LabelDialog::LabelDialog(Tui::ZWidget *parent) : Tui::ZDialog(parent) {
    setOptions(Tui::ZWindow::CloseOption | Tui::ZWindow::DeleteOnClose
               | Tui::ZWindow::MoveOption | Tui::ZWindow::AutomaticOption | Tui::ZWindow::ResizeOption);
    setDefaultPlacement(Qt::AlignCenter);

    setFocus();
    setWindowTitle("Label");
    setContentsMargins({1, 1, 1, 1});

    Tui::ZVBoxLayout *vbox = new Tui::ZVBoxLayout();
    vbox->setSpacing(1);
    setLayout(vbox);

    Tui::ZLabel *label1 = new Tui::ZLabel(this);
    label1->setText("Label");
    label1->setFocus();
    vbox->addWidget(label1);

    Tui::ZLabel *label2 = new Tui::ZLabel(this);
    label2->setMarkup("l<m>a</m>bel");
    vbox->addWidget(label2);

    Tui::ZLabel *label3 = new Tui::ZLabel(Tui::withMarkup, "d<m>i</m>sabled", this);
    label3->setEnabled(false);
    vbox->addWidget(label3);

    Tui::ZPalette p;
    p.setColors({{"control.bg", Tui::Colors::red},
                 {"control.shortcut.bg", Tui::Colors::red},
                 {"control.fg", Tui::Colors::yellow}});

    Tui::ZLabel *label4 = new Tui::ZLabel(this);
    label4->setMarkup("C<m>o</m>loured");
    label4->setPalette(p);
    vbox->addWidget(label4);
}


SearchAndReplaceDialog::SearchAndReplaceDialog(Tui::ZWidget *parent) : Tui::ZDialog(parent) {
    setOptions(Tui::ZWindow::CloseOption | Tui::ZWindow::MoveOption | Tui::ZWindow::AutomaticOption);
    setDefaultPlacement(Tui::AlignBottom | Tui::AlignHCenter, {0, -2});
    setContentsMargins({1, 1, 2, 1});
    setGeometry({0, 0, 60, 15});

    setWindowTitle("Search and Replace");

    Tui::ZVBoxLayout *vbox = new Tui::ZVBoxLayout();
    setLayout(vbox);

    Tui::ZLabel *labelFind;
    vbox->setSpacing(1);
    {
        Tui::ZHBoxLayout *hbox = new Tui::ZHBoxLayout();
        hbox->setSpacing(2);

        labelFind = new Tui::ZLabel(Tui::withMarkup, "F<m>i</m>nd", this);
        hbox->addWidget(labelFind);

        Tui::ZInputBox *_searchText = new Tui::ZInputBox(this);
        labelFind->setBuddy(_searchText);
        _searchText->setFocus();
        hbox->addWidget(_searchText);

        vbox->add(hbox);
    }

    {
        Tui::ZHBoxLayout *hbox = new Tui::ZHBoxLayout();
        hbox->setSpacing(2);

        Tui::ZLabel *labelReplace = new Tui::ZLabel(Tui::withMarkup, "Replace", this);
        hbox->addWidget(labelReplace);

        Tui::ZInputBox *_replaceText = new Tui::ZInputBox(this);
        labelReplace->setBuddy(_replaceText);
        hbox->addWidget(_replaceText);

        labelFind->setMinimumSize(labelReplace->sizeHint());
        vbox->add(hbox);

    }

    {
        Tui::ZHBoxLayout *hbox = new Tui::ZHBoxLayout();
        hbox->setSpacing(3);

        {
            Tui::ZWidget *widgetA = new Tui::ZWidget(this);
            Tui::ZVBoxLayout *nbox = new Tui::ZVBoxLayout();
            widgetA->setLayout(nbox);

            Tui::ZLabel *tl = new Tui::ZLabel("Options", widgetA);
            nbox->addWidget(tl);

            Tui::ZCheckBox *_caseMatchBox = new Tui::ZCheckBox(Tui::withMarkup, "<m>M</m>atch case", widgetA);
            nbox->addWidget(_caseMatchBox);
            Tui::ZCheckBox *wordMatchBox = new Tui::ZCheckBox(Tui::withMarkup, "Match <m>e</m>ntire word only", widgetA);
            nbox->addWidget(wordMatchBox);
            wordMatchBox->setEnabled(false);
            Tui::ZCheckBox *_regexMatchBox = new Tui::ZCheckBox(Tui::withMarkup, "Re<m>g</m>ular expression", widgetA);
            nbox->addWidget(_regexMatchBox);
            Tui::ZCheckBox *_liveSearchBox = new Tui::ZCheckBox(Tui::withMarkup, "<m>L</m>ive search", widgetA);
            _liveSearchBox->setCheckState(Tui::Checked);
            nbox->addWidget(_liveSearchBox);

            hbox->addWidget(widgetA);
        }

        {
            Tui::ZWidget *widgetB = new Tui::ZWidget(this);
            Tui::ZVBoxLayout *nbox = new Tui::ZVBoxLayout();
            widgetB->setLayout(nbox);

            Tui::ZLabel *tl = new Tui::ZLabel("Direction", widgetB);
            nbox->addWidget(tl);

            Tui::ZRadioButton *_forward = new Tui::ZRadioButton(Tui::withMarkup, "<m>F</m>orward", widgetB);
            _forward->setChecked(true);
            nbox->addWidget(_forward);
            Tui::ZRadioButton *_backward = new Tui::ZRadioButton(Tui::withMarkup, "<m>B</m>ackward", widgetB);
            nbox->addWidget(_backward);

            Tui::ZCheckBox *_parseBox = new Tui::ZCheckBox(Tui::withMarkup, "escape sequence", widgetB);
            _parseBox->setCheckState(Tui::Checked);
            _parseBox->setEnabled(false);
            nbox->addWidget(_parseBox);

            Tui::ZCheckBox *_wrapBox = new Tui::ZCheckBox(Tui::withMarkup, "<m>W</m>rap around", widgetB);
            _wrapBox->setCheckState(Tui::Checked);
            nbox->addWidget(_wrapBox);

            hbox->addWidget(widgetB);
        }

        vbox->add(hbox);
    }

    vbox->addStretch();

    {
        Tui::ZHBoxLayout *hbox = new Tui::ZHBoxLayout();
        hbox->setSpacing(1);
        hbox->addStretch();

        Tui::ZButton *_findNextBtn = new Tui::ZButton(Tui::withMarkup, "<m>N</m>ext", this);
        _findNextBtn->setDefault(true);
        hbox->addWidget(_findNextBtn);

        Tui::ZButton *_replaceBtn = new Tui::ZButton(Tui::withMarkup, "<m>R</m>eplace", this);

        Tui::ZButton *_replaceAllBtn = new Tui::ZButton(Tui::withMarkup, "<m>A</m>ll", this);

        hbox->addWidget(_replaceBtn);
        hbox->addWidget(_replaceAllBtn);


        Tui::ZButton *_cancelBtn = new Tui::ZButton(Tui::withMarkup, "<m>C</m>lose", this);
        hbox->addWidget(_cancelBtn);

        hbox->addSpacing(3);

        vbox->add(hbox);
    }
}


AboutDialog::AboutDialog(Tui::ZWidget *parent) : Tui::ZWindow(parent) {
    setOptions(Tui::ZWindow::CloseOption | Tui::ZWindow::DeleteOnClose
               | Tui::ZWindow::MoveOption | Tui::ZWindow::AutomaticOption | Tui::ZWindow::ResizeOption);
    setDefaultPlacement(Qt::AlignCenter);

    setFocus();
    setWindowTitle("About");
    setContentsMargins({1, 1, 1, 1});
    setGeometry({0, 0, 40, 10});
}

void AboutDialog::paintEvent(Tui::ZPaintEvent *event) {
    Tui::ZWindow::paintEvent(event);
    Tui::ZTextLayout textline1(terminal()->textMetrics(),"Text");
    textline1.setText("Demo for TuiWidgets\n by Martin Hostettler and Christoph Hüffelmann\n written for FrOSCon 2022 BSL 1.0");
    textline1.doLayout(layoutArea().width());
    textline1.draw(*event->painter(),layoutArea().topLeft(),{Tui::Colors::brightWhite, Tui::Colors::blue},{});
}
