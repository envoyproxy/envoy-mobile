.. _dev_performance_size:

Analysis of Binary Size
=======================

In order to be able to tackle binary size analysis,
the Envoy Mobile team has standardize the process in this document.

Object files analysis
---------------------

Getting a binary
~~~~~~~~~~~~~~~~

In order to have consistency of results these is the toolchain used to build the binaries for analysis:

1.  clang-8
2.  lld (installed with clang)
3.  arm64 machine

The binary being compiled is ``//library/common:test_binary``.
The binary is getting built with the following build command::

  bazel build //library/common:test_binary --config=sizeopt

``sizeopt`` has the following flags:

1.  ``-c opt``:
2.  ``--copt -Os``:
3.  ``--copt=-ggdb3``:
4.  ``--linkopt=-fuse-ld=lld``:
5.  ``--define=google_grpc=disabled``:
6.  ``--define=signal_trace=disabled``:
7.  ``--define=tcmalloc=disabled``:
8.  ``--define=hot_restart=disabled``:

After compiling, the binary can be stripped of all symbols by using ``strip``::

  strip -s bazel-bin/library/common/test_binary

The unstripped and stripped binary can then be used for analysis.

Analysis
~~~~~~~~

While there are a lot of tools out there that can be used for binary size analysis (otool, objdump, jtool),
[Bloaty](https://github.com/google/bloaty) has been more useful than `objdump`
when performing the investigation.

Bloaty's layering of data sources is extremely helpful in being able to explode the binary in all sorts of different ways.
For example one can look at the composition of each compile unit in terms of sections::

  $ bloaty --debug-file=bin/test_binary -c envoy.bloaty -d sections,bloaty_package,compileunits bin/test_binary.stripped
  ...
  7.7%   109Ki   7.7%   110Ki bazel-out/aarch64-opt/bin/external/envoy_api/envoy/api/v2/route/route.pb.cc
    81.9%  89.6Ki  81.4%  89.6Ki .text
    13.6%  14.9Ki  13.5%  14.9Ki .eh_frame
    3.1%  3.45Ki   3.1%  3.45Ki .eh_frame_hdr
    1.3%  1.48Ki   1.3%  1.48Ki .data
    0.0%       0   0.6%     672 .bss
    0.1%      72   0.1%      72 .rodata

Or one might want to see how sections of the binary map to compilation units::

  $ bloaty --debug-file=bin/test_binary -c envoy.bloaty -d bloaty_package,compileunits,sections bin/test_binary.stripped
  ...
  13.2%   929Ki  13.0%   929Ki .rodata
      81.2%   755Ki  81.2%   755Ki [section .rodata]
      15.4%   143Ki  15.4%   143Ki boringssl/
          37.9%  54.4Ki  37.9%  54.4Ki external/boringssl/src/crypto/obj/obj.c
          21.7%  31.1Ki  21.7%  31.1Ki external/boringssl/src/third_party/fiat/curve25519.c
          17.3%  24.9Ki  17.3%  24.9Ki external/boringssl/src/crypto/fipsmodule/bcm.c
          11.3%  16.2Ki  11.3%  16.2Ki external/boringssl/err_data.c
           4.5%  6.39Ki   4.5%  6.39Ki [55 Others]
           0.9%  1.26Ki   0.9%  1.26Ki external/boringssl/src/crypto/x509v3/v3_crld.c
           0.7%  1.06Ki   0.7%  1.06Ki external/boringssl/src/crypto/asn1/tasn_typ.c

These different representations will give you perspective about how different changes in the binary will affect size.
Note that the ``envoy.bloaty`` config refers to a bloaty config that has regexes to capture output.
The example config used in this type of analysis is HERETODOJOSE

Open issues regarding size
--------------------------

``perf/size`` is a label tagging all curent open issues that can improve binary size.
After performing any change that tries to address these issues you should run through the analysis pipeline described above, and make sure your changes match expectations.

CI Integration
--------------

TODOJOSE what is the CI JOB? What does it do?

This integration allows us to catch regressions on binary size.