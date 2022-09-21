.. tui widgets documentation main file

Tui Widgets
===========

Tui Widgets is high-level widget based toolkit for terminal application.

Its designed to be flexible and similar to use to qt grahical widgets.
Its look and feel is similar to graphical applications
or classic full screen terminal applications from the PC world of the 90ies.

Features
--------

* works with the qt event loop and object model
* ready made user interface elements (text entry, checkboxes, buttons, layout managers, menu etc)
* overlapping window support
* robust input handling, unknown key events are gracefully filtered out
* truecolor, soft line breaks, explicit control of trailing whitespace
* does not depend on correctly set $TERM or terminfo database
* tagged paste
* mostly utf-8 based, string width routines also handle utf-16 and utf-32
* offscreen surfaces/layers
* ABI stability is planned (but breaking changes are still happening)
* does not use global variables/singletons where possible, can handle multiple terminals in one process
* permissively licensed: Boost Software License 1.0 (but qt licensing is stricter)

Does not contain:

* support for non utf-8 capable terminals

.. Termpaint is meant as a basic building block to build more specific libraries upon. There are a lot
.. of different higher layer styles, so it's cleaner to have separate libraries for this.

Minimal example
---------------


A "hello world", with fixed widget positions and size:

See :doc:`getting-started` for full source.
 
.. literalinclude:: examples/getting-started/getting-started-index.cpp
    :caption: UI Setup
    :start-after: // snippet-setup-start
    :end-before: // snippet-setup-end
    :dedent: 8

.. literalinclude:: examples/getting-started/getting-started-index.cpp
    :caption: main code
    :start-after: // snippet-main-start
    :end-before: // snippet-main-end


Support
-------

It's known to work on
 * xterm
 * vte
 * rxvt-unicode
 * mintty
 * iTerm2
 * microsoft terminal
 * putty
 * konsole
 * linux
 * freebsd
 * and more.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   getting-started
   concepts
   building-tuiwidgets
   ZTerminal
   ZWidget
   events
   ZPainter
   ZCommon
   ZColor
   ZCommandManager
   ZShortcut
   ZImage
   ZLayout
   ZPalette
   ZSymbol
   ZTextMetrics
   ZTest
   ZWindow

Curent Status
-------------

Known pending changes:
 * layout memory management is unflexible and crash prone.


Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
