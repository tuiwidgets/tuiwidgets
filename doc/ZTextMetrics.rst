.. _ZTextMetrics:

ZTextMetrics
============

.. cpp:class:: Tui::ZTextMetrics

   ZTextMetrics allows measureing the size of a given text and how it is placed into cells.
   As the details of this process depend on the charateristics of the specific terminal instances of ZTextMetrics
   can only be obtained directly (by :cpp:func:`ZTextMetrics Tui::ZTerminal::textMetrics() const`)
   or indirectly (by :cpp:func:`ZTextMetrics Tui::ZPainter::textMetrics() const`) from a terminal.


   .. cpp:struct:: ClusterSize

      .. cpp:member:: int codeUnits
      .. cpp:member:: int codePoints
      .. cpp:member:: int columns


   .. rst-class:: tw-midspacebefore
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize nextCluster(const QString &data, int offset) const;
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize nextCluster(const QChar *data, int size) const;
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize nextCluster(const char32_t *data, int size) const;
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize nextCluster(const char16_t *data, int size) const;
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize nextCluster(const char *stringUtf8, int utf8CodeUnits) const;
   .. rst-class:: tw-noconv
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize nextCluster(QStringView data) const
   .. rst-class:: tw-noconv
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize nextCluster(std::u16string_view data) const
   .. rst-class:: tw-noconv
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize nextCluster(std::string_view data) const

      |noconv|

      Return metrics for the next cluster found in the input string.

   .. rst-class:: tw-midspacebefore
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize splitByColumns(const QString &data, int maxWidth) const;
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize splitByColumns(const QChar *data, int size, int maxWidth) const;
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize splitByColumns(const char32_t *data, int size, int maxWidth) const;
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize splitByColumns(const char16_t *data, int size, int maxWidth) const;
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize splitByColumns(const char *stringUtf8, int utf8CodeUnits, int maxWidth) const;
   .. rst-class:: tw-noconv
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize splitByColumns(QStringView data, int maxWidth) const
   .. rst-class:: tw-noconv
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize splitByColumns(std::u16string_view data, int maxWidth) const
   .. rst-class:: tw-noconv
   .. cpp:function:: Tui::ZTextMetrics::ClusterSize splitByColumns(std::string_view data, int maxWidth) const

      |noconv|

      Return metrics for the part of the input string that fits into ``maxWidth`` columns.

   .. rst-class:: tw-midspacebefore
   .. cpp:function:: int sizeInColumns(const QString &data) const;
   .. cpp:function:: int sizeInColumns(const QChar *data, int size) const;
   .. cpp:function:: int sizeInColumns(const char32_t *data, int size) const;
   .. cpp:function:: int sizeInColumns(const char16_t *data, int size) const;
   .. cpp:function:: int sizeInColumns(const char *stringUtf8, int utf8CodeUnits) const;
   .. rst-class:: tw-noconv
   .. cpp:function:: int sizeInColumns(QStringView data) const
   .. rst-class:: tw-noconv
   .. cpp:function:: int sizeInColumns(std::u16string_view data) const
   .. rst-class:: tw-noconv
   .. cpp:function:: int sizeInColumns(std::string_view data) const

      |noconv|

      Return the size in columns for the passed input string.

   .. rst-class:: tw-midspacebefore
   .. cpp:function:: int sizeInClusters(const QString &data) const;
   .. cpp:function:: int sizeInClusters(const QChar *data, int size) const;
   .. cpp:function:: int sizeInClusters(const char32_t *data, int size) const;
   .. cpp:function:: int sizeInClusters(const char16_t *data, int size) const;
   .. cpp:function:: int sizeInClusters(const char *stringUtf8, int utf8CodeUnits) const;
   .. rst-class:: tw-noconv
   .. cpp:function:: int sizeInClusters(QStringView data) const
   .. rst-class:: tw-noconv
   .. cpp:function:: int sizeInClusters(std::u16string_view data) const
   .. rst-class:: tw-noconv
   .. cpp:function:: int sizeInClusters(std::string_view data) const

      |noconv|

      Return the size in clusters for the passed input string.

.. |noconv| replace:: The overloads marked with ``noconv`` participates in overload resolution only if the ``string``
   parameter matches without implicit conversion.
