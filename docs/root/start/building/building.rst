.. _building:

Building
========

------------
Requirements
------------

**Bazel 0.26.0** is the latest version of Bazel with which Envoy Mobile has been tested.

.. _android_aar:

-----------
Android AAR
-----------

Envoy Mobile can be compiled into an ``.aar`` file for use with Android apps.
This command is defined in the main :repo:`BUILD <BUILD>` file of the repo, and may be run locally:

``bazel build android_dist --config=android``

Upon completion of the build, you'll see an ``envoy.aar`` file at :repo:`dist/envoy.aar <dist>`.

The ``envoy_mobile_android`` Bazel rule defined in the :repo:`dist BUILD file <dist/BUILD>` provides an example of how this artifact may be used.

For a demo of a working app using this artifact, see the :ref:`hello_world` example.

.. _ios_framework:

--------------------
iOS static framework
--------------------

Ensure that your configuration meets the requirements defined in the :repo:`.bazelrc file <.bazelrc>`
(currently Xcode 10.2.1 / iOS 12.2 / Swift 5.0).

Envoy Mobile supports being compiled into a ``.framework`` file for consumption by iOS apps.
This command is defined in the main :repo:`BUILD <BUILD>` file of the repo, and may be run locally:

``bazel build ios_dist --config=ios``

Upon completion of the build, you'll see a ``Envoy.framework`` folder at :repo:`dist/Envoy.framework <dist>`.

The ``envoy_mobile_ios`` Bazel rule defined in the :repo:`dist BUILD file <dist/BUILD>` provides an example of how this artifact may be used.

For a demo of a working app using this artifact, see the :ref:`hello_world` example.
