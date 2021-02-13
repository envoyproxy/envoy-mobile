.. _building:

Building
========

To build Envoy Mobile, your system must also be set up for building Envoy.
To get started, you can use `this quick start guide
<https://github.com/envoyproxy/envoy/tree/master/bazel#quick-start-bazel-build-for-developers>`_.

Ensure that the ``envoy`` **submodule** is initialized when cloning by using ``--recursive``:

``git clone https://github.com/lyft/envoy-mobile.git --recursive``

If the repo was not initially cloned recursively, you can manually initialize the Envoy submodule:

``git submodule update --init``

.. _releases: https://github.com/lyft/envoy-mobile/releases

------------------
Bazel requirements
------------------

Envoy Mobile is compiled using the version of Bazel specified in the
:repo:`.bazelversion <.bazelversion>` file.

To simplify build consistency across environments, bazelisk is used.
Follow `these Envoy instructions <https://github.com/envoyproxy/envoy/blob/master/bazel/README.md#installing-bazelisk-as-bazel>`_ to install bazelisk as bazel.

--------------------
Android requirements
--------------------

- Android SDK Platform 29
- Android NDK 21

----------------
iOS requirements
----------------

- Xcode 12.4
- Swift 5.x
- Note: Requirements are listed in the :repo:`.bazelrc file <.bazelrc>` and CI scripts

.. _android_aar:

-----------
Android AAR
-----------

Envoy Mobile can be compiled into an ``.aar`` file for use with Android apps.
This command is defined in the main :repo:`BUILD <BUILD>` file of the repo, and may be run locally:

``bazelisk build android_dist --config=android --fat_apk_cpu=<arch1,arch2>``

Upon completion of the build, you'll see an ``envoy.aar`` file at :repo:`dist/envoy.aar <dist>`.

Alternatively, you can use the **prebuilt artifact** from Envoy Mobile's releases_
or from :ref:`Maven <maven>`.

The ``envoy_mobile_android`` Bazel rule defined in the :repo:`dist BUILD file <dist/BUILD>` provides
an example of how this artifact may be used.

**When building the artifact for release** (usage outside of development), be sure to include the
``--config=release-android`` option, along with the architectures for which the artifact is being built:

``bazelisk build android_dist --config=release-android --fat_apk_cpu=x86,armeabi-v7a,arm64-v8a``

For a demo of a working app using this artifact, see the :ref:`hello_world` example.

.. _ios_framework:

--------------------
iOS static framework
--------------------

Envoy Mobile supports being compiled into a ``.framework`` for consumption by iOS apps.
This command is defined in the main :repo:`BUILD <BUILD>` file of the repo, and may be run locally:

``bazelisk build ios_dist --config=ios``

Upon completion of the build, you'll see a ``Envoy.framework`` directory at
:repo:`dist/Envoy.framework <dist>`.

Alternatively, you can use the prebuilt artifact from Envoy Mobile's releases_
or from :ref:`CocoaPods <cocoapods>`.

The ``envoy_mobile_ios`` Bazel rule defined in the :repo:`dist BUILD file <dist/BUILD>` provides an
example of how this artifact may be used.

**When building the artifact for release** (usage outside of development), be sure to include the
``--config=release-ios`` option, along with the architectures for which the artifact is being built:

``bazelisk build ios_dist --config=release-ios --ios_multi_cpus=i386,x86_64,armv7,arm64``

For a demo of a working app using this artifact, see the :ref:`hello_world` example.

.. _maven:

-----
Maven
-----

Envoy Mobile Android artifacts are also uploaded to Maven, and can be accessed/downloaded
`here <https://mvnrepository.com/artifact/io.envoyproxy.envoymobile/envoy>`_.

.. _cocoapods:

---------
CocoaPods
---------

If you use CocoaPods on iOS, you can add the following to your ``Podfile`` to use the latest version
of the prebuilt Envoy Mobile framework.

``pod 'EnvoyMobile'``

---------------------------------------------
Building Envoy Mobile with private Extensions
---------------------------------------------

Similar to Envoy, Envoy Mobile has bazel targets that allows the library to be built as a git
submodule in a consuming project. This setup enables creating private extensions, such as filters.

~~~~~~~~~~
Extensions
~~~~~~~~~~

The top-level `envoy_build_config` directory allows Envoy Mobile to tap into Envoy's already
existing `selective extensions system <https://github.com/envoyproxy/envoy/blob/master/bazel/README.md#disabling-extensions>`_.
Additionally, Envoy Mobile requires force registration
of extensions in the extension_registry.cc/h files due to static linking.

In order to override the extensions built into Envoy Mobile create an ``envoy_build_config`` directory
and include the following in the WORKSPACE file::

  local_repository(
    name = "envoy_build_config",
    # Relative paths are also supported.
    path = "/somewhere/on/filesystem/envoy_build_config",
  )

------------------------------
Deploying Envoy Mobile Locally
------------------------------

~~~~~~~
Android
~~~~~~~

To deploy Envoy Mobile's aar to your local maven repository, run the following commands::

    # To build Envoy Mobile. --fat_apk_cpu takes in a list of architectures: [x86|armeabi-v7a|arm64-v8a].
    bazelisk build android_dist --config=android --fat_apk_cpu=x86

    # To publish to local maven.
    dist/sonatype_nexus_upload.py --files dist/envoy.aar dist/envoy-pom.xml --local


The version deployed will be ``LOCAL-SNAPSHOT``. These artifacts can be found in your local maven directory (``~/.m2/repository/io/envoyproxy/envoymobile/envoy/LOCAL-SNAPSHOT/``)

~~~
iOS
~~~
TODO :issue:`#980 <980>`
