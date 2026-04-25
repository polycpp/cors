# Divergences

## Intentional C++ API Shape

- Upstream exports a JavaScript middleware factory. This port exposes a synchronous C++ policy API: `evaluate`, `apply`, and `handle`.
- Upstream accepts dynamic JavaScript option shapes. This port uses explicit `CorsOptions`, `OriginSetting`, and `OriginMatcher` types.
- Upstream mutates duck-typed `req`/`res` objects. This port uses `polycpp::http::Headers` and adapters for polycpp HTTP request/response handles.
- Upstream callback delegates are asynchronous error-first callbacks. This port supports deterministic synchronous policy construction and predicates; async middleware callback parity is deferred.

## Supported Behavior

- Default wildcard origin.
- Disabled origin.
- Fixed origin.
- Reflected origin.
- Exact, regex, and predicate origin allow-lists.
- Methods, allowed headers, exposed headers, credentials, max-age, preflight continue, and custom preflight success status.
- Preflight response control-flow through `CorsResult`.
- `Vary` header mutation through `polycpp::vary`.

## Deferred Behavior

- Express/Connect middleware registration API.
- Asynchronous JavaScript option callbacks and callback error propagation.
- Browser CORS enforcement.
- Full live HTTP server integration tests until the companion `express` middleware ABI is stable.

## Unsupported Runtime-Specific Features

- JavaScript object truthiness/prototype identity is not represented.
- JavaScript `RegExp` object identity and flags are not represented; C++ uses `std::regex`.
- JavaScript `next(err)` propagation is not represented in the pure API.

## Compatibility Notes

- Static allow-list origin mismatch still adds `Vary: Origin`, matching upstream static array/regexp behavior.
- `OriginSetting::disabled()` emits no CORS headers, matching upstream `origin: false` wrapper behavior.
- `allowed_headers` absent reflects `Access-Control-Request-Headers` and adds `Vary: Access-Control-Request-Headers`; an empty vector emits no allow-header and no Vary.
