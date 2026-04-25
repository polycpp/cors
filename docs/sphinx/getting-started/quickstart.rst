Quickstart
==========

This example evaluates a reflected-origin preflight request without starting a
server.

Program
-------

.. code-block:: cpp

   #include <iostream>

   #include <polycpp/cors/cors.hpp>

   int main() {
       polycpp::http::Headers request_headers;
       request_headers.set("Origin", "https://app.example");
       request_headers.set("Access-Control-Request-Headers", "x-api-key");

       polycpp::cors::CorsOptions options;
       options.origin = polycpp::cors::OriginSetting::reflect();
       options.credentials = true;
       options.max_age = 600;

       auto result = polycpp::cors::evaluate("OPTIONS", request_headers, options);

       std::cout << result.status_code << '\n';
       std::cout << result.headers.get("Access-Control-Allow-Origin").value_or("") << '\n';
       std::cout << result.headers.get("Access-Control-Allow-Credentials").value_or("") << '\n';
       return result.should_end_response ? 0 : 1;
   }

Expected output
---------------

.. code-block:: text

   204
   https://app.example
   true

Key behavior
------------

- Default preflight handling ends the response with status ``204`` and ``Content-Length: 0``.
- ``OriginSetting::reflect`` mirrors the request ``Origin`` and adds ``Vary: Origin``.
- Missing ``allowed_headers`` reflects ``Access-Control-Request-Headers`` and adds ``Vary: Access-Control-Request-Headers``.
- ``credentials=true`` emits ``Access-Control-Allow-Credentials: true``.
