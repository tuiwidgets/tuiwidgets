Building Tui Widgets
====================

(You can skip this section if you are using a prebuild Tui Widgets library, e.g. from your linux distribution)

Prerequists
-----------

You need a working C++17 compiler, `meson <https://mesonbuild.com/>`_, `ninja <https://ninja-build.org/>`_
and an installation of `QtCore <https://www.qt.io/>`_.

For example on debian:

.. code-block:: shell

   apt install build-essential git meson ninja-build pkg-config qttools5-dev-tools qtbase5-dev

termpaint
---------

You also need `termpaint <https://termpaint.namepad.de>`_.

.. code-block:: shell

   git clone https://github.com/termpaint/termpaint
   
   cd termpaint
   meson setup _build -Dprefix=$HOME/opt/tuiwidgets-prefix
   meson compile -C _build
   meson install -C _build


.. _library_setup:

Tui Widgets
-----------

.. code-block:: shell

   git clone https://github.com/tuiwidgets/tuiwidgets
   
   cd tuiwidgets
   PKG_CONFIG_PATH=$HOME/opt/tuiwidgets-prefix/lib/x86_64-linux-gnu/pkgconfig meson setup _build -Dprefix=$HOME/opt/tuiwidgets-prefix
   meson compile -C _build
   meson install -C _build

