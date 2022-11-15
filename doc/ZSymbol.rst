.. _ZSymbol:

ZSymbol
=======

Tui Widgets uses strings as identifiers in various places.
Using strings is helpful for namespace management when multiple independent developers create libraries and
identfiers are widely scoped.
To reduce possible performance impact of many string comparisons, strings are converted to ZSymbol instances.
The ZSymbol system maintains an internal mapping of all strings used as symbols to integer values,
so that simple integer comparisons can be done after lookup.

ZSymbol is used for storage and in places where implicit conversation from QString is not desired.
In constract ZImplicitSymbol can be used where ease of use requires implicit conversion.

.. c:macro:: TUISYM_LITERAL(x)

   Creates a static :cpp:class:`Tui::ZSymbol` symbol instance from the string literal ``x``.

   Example: ``widget.getColor(TUISYM_LITERAL("control.fg"))``

.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui::ZSymbol

   ZSymbol is copyable, assignable and supports all comparisons.

   .. cpp:function:: ZSymbol()

      Create a null symbol.

   .. cpp:function:: explicit ZSymbol(QString str)

      Creates a symbol representing the string ``str``

   .. cpp:function:: QString toString() const

      Returns the string that is represented by the symbol.

   .. cpp:function:: explicit operator bool() const

      Returns ``true`` if the symbol is not the null symbol (created by the default constructor)

   .. cpp:function:: friend unsigned int qHash(const ZSymbol &key)

      Returns a hash for qt's hash based data types.

.. rst-class:: tw-midspacebefore
.. cpp:class:: Tui::ZImplicitSymbol : public Tui::ZSymbol

   .. cpp:function:: ZImplicitSymbol(QString str)
   .. cpp:function:: template <int N> ZImplicitSymbol(const char(&literal)[N])

      Construct an ZImplicitSymbol as implicit type conversion from string literal or QString.
