// Feel free to copy from this example to your own code
// SPDX-License-Identifier: 0BSD OR BSL-1.0 OR MIT-0

#ifndef DEMO_H
#define DEMO_H

#include <QDir>
#include <QFileSystemWatcher>

#include <Tui/Misc/AbstractTableModelTrackBy.h>
#include <Tui/ZDialog.h>
#include <Tui/ZListView.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTextLine.h>

class Demo : public Tui::ZRoot {
    Q_OBJECT

public:
    Demo();
    ~Demo();

protected:
    void terminalChanged() override;

private:
    void quit();
};

class ButtonDialog : public Tui::ZDialog {
    Q_OBJECT

public:
    ButtonDialog (Tui::ZWidget *parent, Tui::ZTextLine *message);
};


class CheckBoxDialog : public Tui::ZDialog {
    Q_OBJECT
public:
    CheckBoxDialog (Tui::ZWidget *parent, Tui::ZTextLine *message);
};


class RadioButtonDialog : public Tui::ZDialog {
    Q_OBJECT
public:
    RadioButtonDialog (Tui::ZWidget *parent, Tui::ZTextLine *message);
};


class InputBoxDialog : public Tui::ZDialog {
    Q_OBJECT
public:
    InputBoxDialog (Tui::ZWidget *parent);
};

class FileModel : public Tui::Misc::AbstractTableModelTrackBy<QString> {
    Q_OBJECT
public:
    FileModel(const QDir &dir);

public:
    void update();
    void setDirectory(const QDir &dir);

private:
    QDir _dir;
    QFileSystemWatcher _watcher;
};

class ListViewDialog : public Tui::ZDialog {
    Q_OBJECT
public:
    ListViewDialog (Tui::ZWidget *parent);
private:
    void refreshFolder();
    void userInput(QString filename);

    Tui::ZListView *_listview;
    QDir _dir;
    std::unique_ptr<FileModel> _model;
};

class LabelDialog : public Tui::ZDialog {
    Q_OBJECT
public:
    LabelDialog (Tui::ZWidget *parent);
};

class SearchAndReplaceDialog : public Tui::ZDialog {
    Q_OBJECT
public:
    SearchAndReplaceDialog (Tui::ZWidget *parent);
};

class AboutDialog : public Tui::ZWindow {
    Q_OBJECT
public:
    AboutDialog (Tui::ZWidget *parent);
    void paintEvent(Tui::ZPaintEvent *event) override;
};

class StartDialog : public Tui::ZDialog {
    Q_OBJECT
public:
    StartDialog(Tui::ZWidget *parent);
    void paintEvent(Tui::ZPaintEvent *event) override;
};

#endif // DEMO_H
