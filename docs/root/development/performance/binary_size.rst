.. _dev_performance_size:

Analysis of Binary Size
=======================

In order to be able to tackle binary size analysis,
the Envoy Mobile team has standardized the process in this document.

Object files analysis
---------------------

Getting a binary
~~~~~~~~~~~~~~~~

In order to have consistency of results this is the toolchain used to build the binary for analysis:

1. clang-8
2. lld (installed with clang)
3. arm64 machine

The binary being compiled is ``//library/common:test_binary_size``.
The binary is getting built with the following build command::

  bazel build //library/common:test_binary_size --config=sizeopt

``sizeopt`` has the following flags:

.. _envoy_docs: https://github.com/envoyproxy/envoy/blob/master/bazel/README.md#enabling-optional-features

1. ``-c opt``: bazel compilation option for size optimization.
2. ``--copt -Os``: optimize for size.
3. ``--copt=-ggdb3``: keep debug symbols. Later stripped with ``strip``
4. ``--linkopt=-fuse-ld=lld``: use the lld linker.
5. ``--define=google_grpc=disabled``: more info in the `envoy docs <envoy_docs>`_.
6. ``--define=signal_trace=disabled``: more info in the `envoy docs <envoy_docs>`_.
7. ``--define=tcmalloc=disabled``: more info in the `envoy docs <envoy_docs>`_.
8. ``--define=hot_restart=disabled``: more info in the `envoy docs <envoy_docs>`_.

After compiling, the binary can be stripped of all symbols by using ``strip``::

  strip -s bazel-bin/library/common/test_binary_size

The unstripped and stripped binary can then be used for analysis.

Analysis
~~~~~~~~

While there are a lot of tools out there that can be used for binary size analysis (otool, objdump, jtool),
`Bloaty <https://github.com/google/bloaty>`_ has been the tool of choice to run object file analysis.

Bloaty's layering of data sources is extremely helpful in being able to explode the binary in all sorts of different ways.
For example, one can look at the composition of each compile unit in terms of sections::

  $ bloaty --debug-file=bin/test_binary_size -c envoy.bloaty -d sections,bloaty_package,compileunits bin/test_binary_size.stripped
  ...
  7.7%   109Ki   7.7%   110Ki bazel-out/aarch64-opt/bin/external/envoy_api/envoy/api/v2/route/route.pb.cc
    81.9%  89.6Ki  81.4%  89.6Ki .text
    13.6%  14.9Ki  13.5%  14.9Ki .eh_frame
    3.1%  3.45Ki   3.1%  3.45Ki .eh_frame_hdr
    1.3%  1.48Ki   1.3%  1.48Ki .data
    0.0%       0   0.6%     672 .bss
    0.1%      72   0.1%      72 .rodata

Or one might want to see how sections of the binary map to compilation units::

  $ bloaty --debug-file=bin/test_binary_size -c envoy.bloaty -d bloaty_package,compileunits,sections bin/test_binary_size.stripped
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
The example config used in this type of analysis is::

  custom_data_source: {
    name: "bloaty_package"
    base_data_source: "compileunits"

    #envoy source code.
    rewrite: {
      pattern: "^(external/envoy/source/)(\\w+/)(\\w+)"
      replacement: "envoy \\2"
    }

    #envoy third party libraries.
    rewrite: {
        pattern: "^(external/)(\\w+/)"
        replacement: "\\2"
    }

    #all compiled protos.
    rewrite: {
        pattern: "([.pb.cc | .pb.validate.cc])$"
        replacement: "compiled protos"
    }
  }

Open issues regarding size
--------------------------


``perf/size`` is a label tagging all current open issues that can improve binary size.
Check out the issues `here <https://github.com/lyft/envoy-mobile/labels/perf%2Fsize>`_.
After performing any change that tries to address these issues you should run through the analysis pipeline described above, and make sure your changes match expectations.

The following issues are listed in priority order maximizing complexity vs. size win:

- https://github.com/lyft/envoy-mobile/issues/174
- https://github.com/lyft/envoy-mobile/issues/175
- https://github.com/lyft/envoy-mobile/issues/180
- https://github.com/lyft/envoy-mobile/issues/178
- https://github.com/lyft/envoy-mobile/issues/177
- https://github.com/lyft/envoy-mobile/issues/179
- https://github.com/lyft/envoy-mobile/issues/176

Current status
~~~~~~~~~~~~~~

As of https://github.com/lyft/envoy-mobile/tree/f17caebcfce09ec5dcda905dc8418fea4d382da7
The test_binary_size_size as built by the toolchain against the architecture described above
compiles to a stripped size of 8.9mb and a compressed size of 3mb.

CI Integration
--------------

TODO: add when the integration is live.
https://github.com/lyft/envoy-mobile/issues/181