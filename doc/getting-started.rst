.. _Getting_Started:

Getting started
===============

Tui Widgets is a high level terminal interface library using widgets.
It uses the operating system abstractions and object model of Qt.

Applications are generelly assembled from components called widgets.
An applications typically uses a mix of predefined widgets and application defined widgets.


A simple dialog with a button
-----------------------------

.. figure:: tpi/demo_gettingstarted.tpi

First you need to include the pkg-config dependency named ``TuiWidgets`` into
your application's build. Next include the needed headers into your source:

.. literalinclude:: examples/getting-started/getting-started.cpp
    :caption: includes
    :start-after: // snippet-header-start
    :end-before:  // snippet-header-end

The the main function contains general setup for Qt and the Terminal:

.. literalinclude:: examples/getting-started/getting-started.cpp
    :caption: main()
    :start-after: // snippet-main-start
    :end-before:  // snippet-main-end

This first initializes Qt for non graphical usage by creating an instance of ``QCoreApplication``.
An instance of ``QCoreApplication`` is always needed for Tui Widgets applications.

Next it creates a ZTerminal instance using its default constructor.
The default constructor connects the terminal instance to the terminal from which the application was run.
Next it creates an instance of the ``Root`` class, which is a custom widget, and connects it as the root to the terminal.

At last it passes control to the Qt event loop by calling ``app.exec()``.
This starts the application and terminal initialization.

When the event loop starts running, ZTerminal connects to the terminal and executes its terminal auto detection.
This auto detection is asynchronous and its results are needed for setup of the user interface.

When the terminal detection completes, all widgets connected to the terminal will receive a terminal changed notification.
This is the point where the application can start its user interface setup.

In this example this happens in the custom widget ``Root`` that is defined as follows:

.. literalinclude:: examples/getting-started/getting-started.h
    :caption: root widget header

(remember to setup your build system to run moc on the header file)

.. literalinclude:: examples/getting-started/getting-started.cpp
    :caption: root widget implementation
    :start-after: // snippet-root-start
    :end-before:  // snippet-root-end

The ``Root`` class defines a widget derived from ZRoot.
ZRoot implements common functionality like switching between windows and other window related tasks.

The ``Root`` widget reimplements the ``terminalChanged`` method to setup the applications user interface.
If the application does not move widgets to a additional terminal, ``terminalChanged`` will only be called once in the
lifetime of the root widget.

In ``(1)``, a global (``Tui::ApplicationShortcut``) keyboard shortcut is set up for the escape key.
The shortcut is then connected to the quit method. This allows quitting the application by pressing the Escape key.

In ``(2)`` the implementation proceeds with creating a window (using a ZWindow instance) with the title "Hello World".
It then assigns the window a position relative to the root using the ``setGeometry`` call, passing it an QRect with
the needed coordinates.

Lastly, in ``(3)`` a button is defined as child of the window. The button uses text with markup.
The ``<m>`` element marks the keyboard mnemonic of the button.
In this case, the button can be activated from anywhere within its parent window by pressing :kbd:`Alt+Q`.
Next, the clicked signal of the button is also connected to the ``quit()`` method of the class
so activating the button quits the application.

The button then is given its position relative to its parent widget (the window) and finally is given keyboard focus.

In ``(4)`` the ``quit`` method is implemented.
It simply calls ``quit`` of QCoreApplication to exit the event loop started in the last line of the ``main()`` function.

You might have noticed the use of raw ``new`` in the example code.
While unusual in modern c++ code, this is the right thing to do within the qt object model,
as long as the objects have a parent QObject set.
The parent automatically deletes the child objects in its destructor.
Memory leaks are therefore avoided.

On the other hand, your application has to make sure that it does not delete objects that
Qt has already deleted. The quick rule is that objects that have a parent QObject either need to be lifetime managed
by Qt, need to have a shorter lifetime than the parent (Qt detects if the application deletes the object)
or it needs to be removed from its parent before the parent is deleted.


See :download:`getting-started.cpp<examples/getting-started/getting-started.cpp>`
and :download:`getting-started.h<examples/getting-started/getting-started.h>` for the whole source of this example.


A note on building with meson
-----------------------------

In case you want to use meson as build system, here is a quick starter project file

.. code-block:: meson

   project('getting-started', ['cpp'], default_options : ['buildtype=debugoptimized', 'cpp_std=c++17'])

   add_project_arguments('-Wall', '-Wextra', language: 'cpp')
   add_project_arguments('-Werror=return-type', language: 'cpp')


   qt5 = import('qt5')
   qt5_dep = dependency('qt5', modules: 'Core')

   tuiwidgets_dep = dependency('TuiWidgets')

   sources = [
     'getting-started.cpp',
   ]
   
   moc_headers = [
     'getting-started.h',
   ]
   
   executable('getting-started', sources,
     qt5.preprocess(moc_headers: moc_headers),
     dependencies : [qt5_dep, tuiwidgets_dep])

If you have setup Tui Widgets as described in :ref:`Building Tui Widgets <library_setup>`, invoke meson like this

.. code-block:: shell

   PKG_CONFIG_PATH=$HOME/opt/tuiwidgets-prefix/lib/x86_64-linux-gnu/pkgconfig meson setup _build
   meson compile -C _build
   
   Then run ``_build/getting-started``

