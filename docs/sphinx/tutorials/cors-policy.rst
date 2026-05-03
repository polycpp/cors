Evaluate and Apply CORS Policy
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
       polycpp::cors::OriginMatcher::predicate(
           [](auto origin) { return origin.ends_with(".internal"); }),
   });

Allow-list policies reflect the request origin only when one matcher accepts it.
They still add ``Vary: Origin`` for non-matching requests so shared caches do
not reuse a denial or allowance across origins.

Apply the result to an existing response header set:

.. code-block:: cpp

   polycpp::http::Headers response_headers;
   auto applied = polycpp::cors::apply(
       polycpp::cors::RequestView{"GET", request_headers},
       response_headers,
       options);

``apply`` uses ``polycpp::vary`` when merging ``Vary`` values, so existing
``Vary`` fields are preserved instead of overwritten.

Handle a response-like object
-----------------------------

When integrating with a ``ServerResponse``-style object, use ``handle`` to
evaluate, set headers, and end a default preflight response in one call:

.. code-block:: cpp

   const bool should_continue = polycpp::cors::handle(request, response, options);
   if (!should_continue) {
       return;
   }

The request object must expose ``method()`` and ``headers()``. The response
object must expose ``setHeader(name, value)`` and, for preflight termination,
``status(code)`` and ``end()``.

Configure preflight responses
-----------------------------

.. code-block:: cpp

   polycpp::cors::CorsOptions preflight_options;
   preflight_options.methods = {"GET", "POST"};
   preflight_options.allowed_headers = polycpp::cors::HeaderList{"authorization", "content-type"};
   preflight_options.max_age = 600;
   preflight_options.options_success_status = 200;

   auto preflight = polycpp::cors::evaluate("OPTIONS", request_headers, preflight_options);

By default, a preflight result has ``should_continue=false``,
``should_end_response=true``, status ``204``, and ``Content-Length: 0``. Set
``preflight_continue=true`` when the caller should continue processing after
CORS headers are generated.
