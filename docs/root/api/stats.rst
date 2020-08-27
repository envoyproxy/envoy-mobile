.. _api_stats:

Stats
=========================

----------------
``StatsClient``
----------------

To use Envoy Mobile's stats function, obtain an instance of ``StatsClient`` from ``Engine`` (refer to :ref:`api_starting_envoy` for building an engine instance), and store the stats client to create ``Counter`` instances.

**Kotlin example**::

  statsClient.counter(Element("foo"), Element"bar"))

**Swift example**::

  statsClient.counter(elements: ["foo", "bar"])

The ``counter`` method from stats client takes a variable number of elements and returns a ``Counter`` instance, the elements are used to for a dot(.) delimited string. For the example code above, the formed string is ``foo.bar``, this string serves as the identifier of the counter.

Store the counter instance, and call the ``increment`` method to increment the counter wherever it applies.
