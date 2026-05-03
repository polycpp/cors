Compatibility and Scope
=======================

``polycpp::cors`` is a C++ companion port of npm ``cors``. It ports the CORS
header policy behavior used by server applications, but it is not a drop-in
translation of the JavaScript middleware factory.

Supported in 1.0.0
------------------

- Default wildcard CORS policy.
- Disabled, fixed, reflected, exact, regex, and synchronous predicate origin policies.
- Methods, allowed headers, exposed headers, credentials, max-age, preflight continuation, and custom OPTIONS success status.
- Pure policy evaluation through ``CorsResult``.
- Header application to ``polycpp::http::Headers``.
- Template adapters for request/response-like objects.
- ``Vary`` mutation through ``polycpp::vary``.

Intentional C++ API shape
-------------------------

The upstream package exports ``cors(options?)`` as an Express/Connect
middleware factory. This port exposes explicit C++ policy objects and functions:

.. list-table::
   :header-rows: 1

   * - npm ``cors``
     - ``polycpp::cors``
   * - ``cors(options?)``
     - Build ``CorsOptions`` directly.
   * - Middleware callback
     - Call ``evaluate``, ``apply``, or ``handle``.
   * - Dynamic JavaScript option shapes
     - Use typed ``OriginSetting``, ``OriginMatcher``, and header lists.
   * - Error-first async callbacks
     - Use synchronous policy construction and predicates.

The pure ``evaluate`` API is usually the best integration boundary when adapting
a new server stack because it makes generated headers and control flow explicit.

Deferred or unsupported behavior
--------------------------------

- Express/Connect middleware registration API.
- Asynchronous JavaScript option callbacks and callback error propagation.
- Browser CORS enforcement or fetch integration.
- Full live Express integration tests until the companion ``express`` middleware ABI is stable.
- JavaScript object truthiness, prototype identity, and ``RegExp`` object identity.

Security boundary
-----------------

This library generates server response headers. It does not decide whether a
browser will permit a request, and it does not authenticate callers. Treat
``OriginSetting`` as one layer of HTTP response policy, not as an authorization
check.

Where to go next
----------------

- Start with :doc:`../getting-started/quickstart` for a minimal preflight example.
- Use :doc:`origin-and-preflight` when configuring origin and header behavior.
- Use :doc:`../api/cors` when wiring framework request/response objects.
