cors.hpp
========

Type Aliases
------------

.. doxygentypedef:: polycpp::cors::Headers

.. doxygentypedef:: polycpp::cors::HeaderList

Policy Types
------------

.. doxygenclass:: polycpp::cors::OriginMatcher
   :members:

.. doxygenclass:: polycpp::cors::OriginSetting
   :members:

.. doxygenstruct:: polycpp::cors::CorsOptions
   :members:

.. doxygenstruct:: polycpp::cors::RequestView
   :members:

.. doxygenstruct:: polycpp::cors::CorsResult
   :members:

Functions
---------

.. doxygenfunction:: polycpp::cors::default_methods

.. doxygenfunction:: polycpp::cors::join

.. doxygenfunction:: polycpp::cors::is_preflight_method

.. doxygenfunction:: polycpp::cors::evaluate(RequestView, const CorsOptions&)

.. doxygenfunction:: polycpp::cors::evaluate(std::string_view, const Headers&, const CorsOptions&)

.. doxygenfunction:: polycpp::cors::apply(const CorsResult&, Headers&)

.. doxygenfunction:: polycpp::cors::apply(RequestView, Headers&, const CorsOptions&)

Response-like adapters
----------------------

``cors.hpp`` also provides templated overloads for request/response-like objects:

- ``evaluate(request, options)`` expects ``request.method()`` and ``request.headers()``.
- ``apply(result, response)`` expects ``setHeader(name, value)`` and, for preflight termination, ``status(code)`` and ``end()``.
- ``handle(request, response, options)`` evaluates and applies in one call, returning whether downstream handling should continue.

These adapters are intended for ``polycpp::http::IncomingMessage`` and
``polycpp::http::ServerResponse``-style handles.
