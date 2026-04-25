Installation
============

cors targets C++20 and builds with clang >= 14 or gcc >= 11. It depends on the
base `polycpp <https://github.com/enricohuang/polycpp>`_ library and the
``polycpp::vary`` companion library.

CMake FetchContent (recommended)
--------------------------------

Add the library to your ``CMakeLists.txt``:

.. code-block:: cmake

   include(FetchContent)

   FetchContent_Declare(
       polycpp_cors
       GIT_REPOSITORY https://github.com/polycpp/cors.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_cors)

   add_executable(my_app main.cpp)
   target_link_libraries(my_app PRIVATE polycpp::cors)

The first configure pulls ``polycpp`` and ``polycpp::vary`` transitively, so the
build tree may be large. Pin ``GIT_TAG`` to a specific commit for reproducible
builds.

Using local clones
------------------

If you already have cors, vary, and polycpp checked out locally, tell CMake to
use them instead of fetching from GitHub:

.. code-block:: bash

   cmake -B build -G Ninja \
       -DPOLYCPP_SOURCE_DIR=/path/to/polycpp \
       -DPOLYCPP_VARY_SOURCE_DIR=/path/to/vary

This repo also uses ``/path/to/polycpp`` and ``polycpp/vary``
automatically when those local checkouts exist.

Build options
-------------

``POLYCPP_CORS_BUILD_TESTS``
    Build the GoogleTest suite. Defaults to ``ON`` for standalone builds and
    ``OFF`` when consumed via FetchContent.

``POLYCPP_CORS_BUILD_EXAMPLES``
    Build runnable examples under ``examples/``. Defaults to ``OFF``.

``POLYCPP_SOURCE_DIR``
    Optional local base polycpp checkout.

``POLYCPP_VARY_SOURCE_DIR``
    Optional local ``polycpp::vary`` checkout.

Verifying the install
---------------------

.. code-block:: bash

   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_CORS_BUILD_EXAMPLES=ON
   cmake --build build -j$(nproc)
   ctest --test-dir build --output-on-failure
   ./build/examples/policy

All tests should pass on a supported toolchain.
