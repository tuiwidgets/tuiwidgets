#ifndef MENUE_H
#define MENUE_H

#include <QObject>
#include <QRect>
#include <QVector>

#include <Tui/ZCommandNotifier.h>
#include <Tui/ZImage.h>
#include <Tui/ZMenu.h>
#include <Tui/ZMenuItem.h>
#include <Tui/ZMenubar.h>
#include <Tui/ZRoot.h>
#include <Tui/ZTerminal.h>
#include <Tui/ZTest.h>
#include <Tui/ZWindow.h>

#include "export_tpi.h"

void menue();
void menuebar();
void submenu();
void popupmenu();

#endif // MENUE_H
