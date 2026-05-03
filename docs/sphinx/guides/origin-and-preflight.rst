Origin and Preflight Semantics
==============================

Origin policies
---------------

``OriginSetting::any()`` emits ``Access-Control-Allow-Origin: *`` and does not
add ``Vary: Origin``.

``OriginSetting::disabled()`` emits no CORS headers and returns a continuing
result.

``OriginSetting::fixed(value)`` emits the configured value and adds
``Vary: Origin``.

``OriginSetting::reflect()`` mirrors the request ``Origin`` when present and
adds ``Vary: Origin``.

``OriginSetting::allow_list(...)`` reflects only matching request origins, but
still adds ``Vary: Origin`` so caches do not reuse a decision across origins.

Actual request handling
-----------------------

For non-preflight requests, evaluation emits origin and credential headers plus
``Access-Control-Expose-Headers`` when ``exposed_headers`` is configured and
non-empty. It does not emit preflight-only headers such as
``Access-Control-Allow-Methods``, ``Access-Control-Allow-Headers``,
``Access-Control-Max-Age``, or ``Content-Length``.

Preflight handling
------------------

A request method equal to ``OPTIONS`` case-insensitively is treated as
preflight. By default, preflight evaluation sets ``should_continue=false``,
``should_end_response=true``, status ``204``, and ``Content-Length: 0``.

Set ``preflight_continue=true`` when a caller wants to keep processing after
headers are generated. In that mode, the result still contains generated CORS
headers, but ``should_end_response`` remains false and ``Content-Length`` is not
added.

Preflight evaluation emits:

- ``Access-Control-Allow-Origin`` according to the configured origin policy.
- ``Access-Control-Allow-Credentials`` when ``credentials=true``.
- ``Access-Control-Allow-Methods`` from ``methods`` when the joined value is not empty.
- ``Access-Control-Allow-Headers`` from ``allowed_headers`` or reflected request headers.
- ``Access-Control-Max-Age`` when ``max_age`` is present, including ``0``.
- ``Access-Control-Expose-Headers`` when ``exposed_headers`` is present and non-empty.

Header reflection
-----------------

If ``allowed_headers`` is not set, preflight handling reflects the request
``Access-Control-Request-Headers`` value and adds
``Vary: Access-Control-Request-Headers``. If ``allowed_headers`` is set to an
empty vector, no allow-header is emitted and no request-header Vary field is
added.

Response application
--------------------

``evaluate`` is pure policy evaluation: it builds a ``CorsResult`` without
mutating caller-owned response headers. Use ``apply(request, headers, options)``
to evaluate and merge those headers into a ``polycpp::http::Headers`` object.
The merge preserves existing ``Vary`` values through ``polycpp::vary``.

Use ``handle(request, response, options)`` for response-like objects that expose
``setHeader(name, value)``, ``status(code)``, and ``end()``. It returns
``false`` when a default preflight response has been completed and downstream
handling should stop.

Validation behavior
-------------------

Header values are validated by ``polycpp::http::Headers`` while the result is
being built. When using ``apply(request, headers, options)``, invalid configured
values throw before the caller-owned response headers are mutated.
