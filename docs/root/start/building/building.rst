.. _building:

Building
========

.. _building_requirements:

In order to compile the artifacts used by the Envoy Mobile library,
your system must also be set up for building Envoy. To get started, you can use
`this quick start guide
<https://github.com/envoyproxy/envoy/tree/master/bazel#quick-start-bazel-build-for-developers>`_.

Ensure that the ``envoy`` **submodule** is initialized when cloning by using ``--recursive``:

``git clone https://github.com/lyft/envoy-mobile.git --recursive``

If the repo was not initially cloned recursively, initialize the Envoy
submodule with ``git submodule update --init``.

--------------------
Android requirements
--------------------

.. attention::
   Android currently fails to build on macOS.
   We are working to resolve this ASAP.
   More information in `this issue <https://github.com/lyft/envoy-mobile/issues/72>`_.

- Bazel 0.26.0
- Android SDK Platform 28
- Android NDK 19.2.5345600

----------------
iOS requirements
----------------

- Bazel 0.26.0
- Xcode 10.2.1
- iOS 12.2 / Swift 5.0
- Note: Requirements are listed in the :repo:`.bazelrc file <.bazelrc>`

.. _android_aar:

-----------
Android AAR
-----------

Envoy Mobile can be compiled into an ``.aar`` file for use with Android apps.
This command is defined in the main :repo:`BUILD <BUILD>` file of the repo, and may be run locally:

``bazel build android_dist --config=android``

Upon completion of the build, you'll see an ``envoy.aar`` file at :repo:`dist/envoy.aar <dist>`.

The ``envoy_mobile_android`` Bazel rule defined in the :repo:`dist BUILD file <dist/BUILD>` provides
an example of how this artifact may be used.

For a demo of a working app using this artifact, see the :ref:`hello_world` example.

.. _ios_framework:

--------------------
iOS static framework
--------------------

Envoy Mobile supports being compiled into a ``.framework`` directory for consumption by iOS apps.
This command is defined in the main :repo:`BUILD <BUILD>` file of the repo, and may be run locally:

``bazel build ios_dist --config=ios``

Upon completion of the build, you'll see a ``Envoy.framework`` directory at
:repo:`dist/Envoy.framework <dist>`.

The ``envoy_mobile_ios`` Bazel rule defined in the :repo:`dist BUILD file <dist/BUILD>` provides an
example of how this artifact may be used.

For a demo of a working app using this artifact, see the :ref:`hello_world` example.
