Origin And Preflight Semantics
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

Preflight handling
------------------

A request method equal to ``OPTIONS`` case-insensitively is treated as
preflight. By default, preflight evaluation sets ``should_continue=false``,
``should_end_response=true``, status ``204``, and ``Content-Length: 0``.

Set ``preflight_continue=true`` when a caller wants to keep processing after
headers are generated.

Header reflection
-----------------

If ``allowed_headers`` is not set, preflight handling reflects the request
``Access-Control-Request-Headers`` value and adds
``Vary: Access-Control-Request-Headers``. If ``allowed_headers`` is set to an
empty vector, no allow-header is emitted and no request-header Vary field is
added.
