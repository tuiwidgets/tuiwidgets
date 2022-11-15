.. _AbstractTableModelTrackBy:

AbstractTableModelTrackBy
=========================

The Qt model classes are optimized for situations where small changes are made to existing data.
In some situations new data is only available as an updated list of all items possible with additions, deletions and moves.
This utility class aims to make implementing a read-only model easier in such a situation.
If the items have a stable identifier such as a file path, an database id or similar this class can automatically
translate a new state with a full list into the changes the model system expects.
This class is intended for small to medium sized models.

As example the following code creates a model based on a filesystem directory assuming file name are stable identifiers
in this case.

A instance is created with the type of the identifier/key to use and how many columns the model should have:

.. literalinclude:: examples/modeltrackby/modeltrackby.cpp
    :caption: setup
    :start-after: // snippet-setup-start
    :end-before:  // snippet-setup-end

It can then be updated as data comes in and changes:

.. literalinclude:: examples/modeltrackby/modeltrackby.cpp
    :caption: setup
    :start-after: // snippet-update-start
    :end-before:  // snippet-update-end

Here ``files`` is the data to update the model to.
The code transforms the data into a :cpp:class:`QVector` of :cpp:class:`Tui::Misc::AbstractTableModelTrackBy::Row`
instances.

In each row the :cpp:member:`Tui::Misc::AbstractTableModelTrackBy::Row::key` is set to the stable identifier of the row.
The ``key`` is how row removal, addition and reordering is detected.
:cpp:member:`Tui::Misc::AbstractTableModelTrackBy::Row::columns` is a :cpp:class:`QVector` that stores the assocation of
Qt item roles to their data for each column.

Finally :cpp:func:`setData <void Tui::Misc::AbstractTableModelTrackBy::setData(const QVector<Tui::Misc::AbstractTableModelTrackBy::Row> &data)>`
updates the model.

.. cpp:class:: template <typename KEY> Tui::Misc::AbstractTableModelTrackBy : public QAbstractTableModel

   Utility class to implement a :cpp:class:`QAbstractTableModel` based on a list of rows with stable identifier.

   **Types**

   .. cpp:struct:: Row

      A row of data for use in :cpp:func:`setData <void Tui::Misc::AbstractTableModelTrackBy::setData(const QVector<Tui::Misc::AbstractTableModelTrackBy::Row> &data)>`.

      .. cpp:member:: KEY key

         The key to be used for row removal, addition and reordering detection.

         If a row is the "same" row as in a previous update it must have the same value for key.

      .. cpp:member:: QVector<QMap<int, QVariant>> columns

         A list of columns for this row.

         Each item is a mapping from Qt item roles to their data.

   **Constructors**

   .. cpp:function:: AbstractTableModelTrackBy(int columns)

      Creates an empty model with the number of columns set to ``columns``.

   **Functions**

   .. cpp:function:: void setData(const QVector<Tui::Misc::AbstractTableModelTrackBy::Row> &data)

      Updates the model using the new data from ``data``.

      Model events for row removal, addition and movements are generated as well as model events for data that has
      changed relative to the previous state in each table cell.
