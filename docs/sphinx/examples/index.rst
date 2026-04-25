Examples
========

Runnable programs under ``examples/`` use only the public API.

.. toctree::
   :maxdepth: 1

   policy

Running examples
----------------

From the repository root:

.. code-block:: bash

   cmake -B build -G Ninja -DPOLYCPP_CORS_BUILD_EXAMPLES=ON
   cmake --build build --target policy
   ./build/examples/policy
