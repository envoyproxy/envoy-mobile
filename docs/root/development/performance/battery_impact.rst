.. _dev_performance_battery:

.. _ios_envoy_example_app: https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/swift/hello_world
.. _android_envoy_example_app: https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/kotlin/hello_world
.. _android_envoy_example_control_app: https://github.com/lyft/envoy-mobile/tree/ac/envoy-battery-cpu-branch/examples/kotlin/control

Analysis of battery impact
==========================
In order to identify how Envoy impacts an application, we have created a control application with modifications to our
current hello world example applications. To see the actual applications we used, you can go to `iOS Envoy example <ios_envoy_example_app>`_,
`Android control app <android_envoy_example_control_app>`_, `Android Envoy example <android_envoy_example_app>`_.

The current configurations we have are that we make a request every 200ms and disabled caching.

Experimentation method
~~~~~~~~~~~~~~~~~~~~~~

iOS
---
* TODO

.. _accu_battery: https://play.google.com/store/apps/details?id=com.digibites.accubattery&hl=en_US

Android
-------
Getting the build:
1. Build the library using ``bazel build android_dist --config=android``
2. Control: ``bazel mobile-install //examples/kotlin/control:hello_control_kt``
3. Envoy: ``bazel mobile-install //examples/kotlin/hello_world:hello_envoy_kt --fat_apk_cpu=armeabi-v7a``

Experiment steps:
1. Set a phone's display to sleep after 30minutes of inactivity
2. Unplug the phone from any power source
3. Open up an application
4. Wait for the phone to sleep
5. Look at the battery drain from an application like `AccuBattery <accu_battery>`_

Results
~~~~~~~

iOS
---

Android
-------
The results after an hour of running both the applications:
Envoy:
Control:


Analysis
~~~~~~~~

iOS
---
* TODO

Android
-------
There isn't a very 

Open issues regarding battery usage
-----------------------------------

Current status
~~~~~~~~~~~~~~
