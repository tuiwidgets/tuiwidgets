Tui Widgets 0.2.2 (2025-04-03)
==============================

This is a feature release.
It is ABI and API compatible to 0.2.1.

New inline mode
---------------

It is now possible to use Tui Widgets to render in the normal flow of terminal
output without going to fullscreen mode.
This could be used for progress bars, prompts or other smaller unintrusive
interactions in a terminal commandline workflow.
Or when it is useful to have access to the previous output in the terminal
session while using an application.

See ZTerminal::Inline and ZTerminal::setInlineHeight.

Bug fixes
---------

* Fix crash in ZDocument finding methods.

Misc
----

* Add build configuration option `tests` and `rpath`.


Tui Widgets 0.2.1 (2023-12-10)
==========================

This is a feature release.
It is ABI and API compatible to 0.2.

New Features
------------

* Add multiline text edit support (widget ZTextEdit, model ZDocument)
  \{c2aa6fcb, 0f59eb52, 7da22427, d933e04e, 7da22427, 61b056d7, 3fdae8eb, b5ab375e, 4b5d0e57, 0394ae51, 4beb73a1, c3dbfd2d}
* Add internal clipboard: ZClipboard {5e197cb8}
* Add paste from internal clipboard to ZInputBox. {dbfba820}
* ZPainter: Add drawImage variant drawImageWithTiling that allows specifing tiling mode. {12a497a3}
* ZTerminal: Add terminalDetectionResultText() and terminalSelfReportedNameAndVersion() for terminal diagnostics. {9df107a1}
* Add ZTerminalDiagnosticsDialog to show a dialog with terminal diagnostics. {63b3262a}

Improvements
------------

* ZTextLayout performance improvements {ebb22fe7, 65c65ced}

Bug fixes
---------

* Fix scroll position handling in ZListView {d6bf805e, b7e89663, 3735d600}
* Fix compilation with catch2 version 3.x {c999c72b}
* Fix terminal detection when read and write modes for the terminal are only available on different file descriptors.
  \{ac08a809, 529e28dd}
* Fix crash in tests on NetBSD {2b6db1f4}

Other
-----

* Tests now run in verbose mode.
