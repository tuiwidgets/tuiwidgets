.. _SurrogateEscape:

SurrogateEscape
===============

Qt and thus Tui Widgets works with UTF-16 strings.
But external data rarely is in UTF-16 encoding.
Ideally it is wellformed data in a known encoding, and in that case Qt has suitable decoding tools to use.
But quite often data (such as "text" files) has no strong guarantees to be well-formed, but an application still
needs to process it on a best effort basis without destroying its content.

For the case that data is presumed to be UTF-8 but might contain non-compliant bytes the lossless "surrogate escape"[1]_
decoding scheme can be helpful.
It decodes invalid UTF-8 sequences to invalid UTF-16 sequences and has a matching encoder that can recreate the same
byte sequence from unmodified output of the decoder.
Thus it is possible to losslessly roundtrip data using this scheme.
Qt does not offer functions for this, but higher level parts of Tui Widgets support displaying strings in
surrogate escape encoding as special characters and thus it provides these functions.

The scheme for representing bytes from the not quite UTF-8 input is as follows:
 * start with a strict UTF-8 decoder (that rejects sequences that encode surrogate code points)
 * for all bytes that are not valid UTF-8 translate to a invalid unpaired unicode low surrogate code point.
   The original byte value x (which must be >= 0x80) is transformed to 0xdc00 + x.

This is based on Python's "surrogateescape" error handling mode specified in `PEP-0383 <https://peps.python.org/pep-0383/>`_,
which is based on a idea from Markus Kuhn that he called UTF-8b [2]_.

Changing the error handling when decoding UTF-8 can have security implications [3]_.
If the application does any filtering when using the codec the application has to consider how to handle
surrogate escape code units.
Also there is a possibility that raw bytes that where part of a invalid UTF-8 sequence will after some text processing
later on encode end up as a valid UTF-8 sequence which introduces unexpected characters into the result.

.. rst-class:: tw-invisible
.. cpp:class:: Tui::Misc::SurrogateEscape

   (actually a namespace)


.. cpp:function:: QString Tui::Misc::SurrogateEscape::decode(const QByteArray& data)
.. cpp:function:: QString Tui::Misc::SurrogateEscape::decode(const char *data, int len)

  Translates data that is presumed to be in UTF-8 encoding to a QString using the surrogate escape error handling
  scheme.

.. cpp:function:: QByteArray Tui::Misc::SurrogateEscape::encode(const QString& str)
.. cpp:function:: QByteArray Tui::Misc::SurrogateEscape::encode(const QChar* str, int len)

  Translates a string to a byte array. If the string is only valid UTF-16 the result is valid UTF-8, otherwise
  if it contains surrogate escape encoded raw bytes the result will likely not be valid UTF-8 but allows round tripping
  arbitrary data.


.. [1] Sometimes surrogate escape is also called UTF-8b (which is not a unicode standard)
.. [2] https://web.archive.org/web/20090830064219/http://mail.nl.linux.org/linux-utf8/2000-07/msg00040.html
       and https://web.archive.org/web/20070621091834/http://mail.nl.linux.org/linux-utf8/2000-07/msg00049.html
.. [3] Also see https://unicode.org/L2/L2009/09236-pep383-problems.html
