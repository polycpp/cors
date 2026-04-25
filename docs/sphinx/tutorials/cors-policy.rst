Evaluate And Apply CORS Policy
==============================

Start with incoming request headers:

.. code-block:: cpp

   polycpp::http::Headers request_headers;
   request_headers.set("Origin", "https://app.example");

Use wildcard CORS by default:

.. code-block:: cpp

   auto result = polycpp::cors::evaluate("GET", request_headers);
   // result.headers.get("Access-Control-Allow-Origin") == "*"

Reflect a request origin when that is the intended policy:

.. code-block:: cpp

   polycpp::cors::CorsOptions options;
   options.origin = polycpp::cors::OriginSetting::reflect();

   auto reflected = polycpp::cors::evaluate("GET", request_headers, options);
   // reflected.headers.get("Access-Control-Allow-Origin") == "https://app.example"
   // reflected.headers.get("Vary") == "Origin"

Restrict origins with exact, regex, or predicate matchers:

.. code-block:: cpp

   options.origin = polycpp::cors::OriginSetting::allow_list({
       polycpp::cors::OriginMatcher::exact("https://app.example"),
       polycpp::cors::OriginMatcher::regex(R"(://(.+\.)?example\.org$)"),
   });

Apply the result to an existing response header set:

.. code-block:: cpp

   polycpp::http::Headers response_headers;
   auto applied = polycpp::cors::apply(
       polycpp::cors::RequestView{"GET", request_headers},
       response_headers,
       options);

``apply`` uses ``polycpp::vary`` when merging ``Vary`` values, so existing
``Vary`` fields are preserved instead of overwritten.
