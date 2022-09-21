#pragma once

#include <Tui/ZRoot.h>

class Root : public Tui::ZRoot {
    Q_OBJECT

public:
    void terminalChanged() override;
    void quit();
};

