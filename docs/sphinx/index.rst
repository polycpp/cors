cors
====

**Apply CORS response policy with polycpp HTTP types.**

``polycpp::cors`` is a C++20 companion port of the npm ``cors`` package. It
keeps the upstream header behavior for static policies while replacing the
JavaScript middleware factory with explicit C++ policy evaluation and response
adapters.

.. code-block:: cpp

   #include <polycpp/cors/cors.hpp>

   polycpp::http::Headers request_headers;
   request_headers.set("Origin", "https://app.example");

   polycpp::cors::CorsOptions options;
   options.origin = polycpp::cors::OriginSetting::reflect();

   auto result = polycpp::cors::evaluate("GET", request_headers, options);

.. grid:: 2

   .. grid-item-card:: Pure policy result
      :margin: 1

      ``evaluate`` returns headers and preflight control-flow decisions without
      mutating caller-owned response state.

   .. grid-item-card:: Polycpp HTTP integration
      :margin: 1

      Request and response boundaries use ``polycpp::http::Headers`` and
      ``ServerResponse``-style handles instead of local HTTP containers.

   .. grid-item-card:: Explicit origin modes
      :margin: 1

      Wildcard, disabled, fixed, reflected, exact, regex, and predicate origin
      policies are modeled as typed C++ values.

   .. grid-item-card:: Companion reuse
      :margin: 1

      ``Vary`` mutation delegates to ``polycpp::vary`` so CORS does not
      duplicate header-combination behavior.

Getting started
---------------

.. code-block:: cmake

   include(FetchContent)
   FetchContent_Declare(
       polycpp_cors
       GIT_REPOSITORY https://github.com/polycpp/cors.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_cors)
   target_link_libraries(my_app PRIVATE polycpp::cors)

:doc:`Installation <getting-started/installation>` | :doc:`Quickstart <getting-started/quickstart>` | :doc:`Tutorial <tutorials/cors-policy>` | :doc:`API reference <api/cors>`

.. toctree::
   :hidden:
   :caption: Getting started

   getting-started/installation
   getting-started/quickstart

.. toctree::
   :hidden:
   :caption: Tutorials

   tutorials/index

.. toctree::
   :hidden:
   :caption: How-to guides

   guides/index

.. toctree::
   :hidden:
   :caption: API reference

   api/index

.. toctree::
   :hidden:
   :caption: Examples

   examples/index
