# API Mapping

| Upstream symbol | C++ symbol | Status | Notes |
|---|---|---|---|
| `cors(options?)` middleware factory | `polycpp::cors::evaluate(RequestView, CorsOptions)` | adapted | C++ returns deterministic header/control-flow data instead of a JavaScript middleware closure. |
| `cors(options?)(req, res, next)` | `polycpp::cors::handle(req, res, options)` | adapted | Template adapter for `IncomingMessage`/`ServerResponse`-style objects; returns whether downstream handling should continue. |
| `req.method` | `RequestView::method` or `IncomingMessage::method()` | adapted | Uses explicit method string and case-insensitive OPTIONS detection. |
| `req.headers.origin` | `RequestView::headers.get("Origin")` | adapted | Uses `polycpp::http::Headers` case-insensitive lookup. |
| `req.headers['access-control-request-headers']` | `RequestView::headers.get("Access-Control-Request-Headers")` | adapted | Reflected for preflight when `allowed_headers` is absent. |
| `res.setHeader(name, value)` | `polycpp::cors::apply(result, polycpp::http::Headers&)` and response adapter | adapted | Header mutation uses base `polycpp::http::Headers` or response-like `setHeader`. |
| `res.statusCode = optionsSuccessStatus` | `CorsResult::status_code` and response adapter `status(code)` | adapted | Pure result records status; response adapter sets it only when preflight short-circuits. |
| `res.end()` | `CorsResult::end_response` and response adapter `end()` | adapted | Pure result records the control-flow decision; adapter can finish a `ServerResponse`. |
| `options.origin = '*'` | `OriginSetting::any()` | direct | Default behavior; emits `Access-Control-Allow-Origin: *` and no `Vary: Origin`. |
| `options.origin = false` | `OriginSetting::disabled()` | direct | Disables CORS header generation. |
| `options.origin = true` | `OriginSetting::reflect()` | direct | Reflects request origin and adds `Vary: Origin`. |
| `options.origin = 'https://example.com'` | `OriginSetting::fixed("https://example.com")` | direct | Emits fixed origin and adds `Vary: Origin`. |
| `options.origin = /.../` | `OriginSetting::allow_list({OriginMatcher::regex(...)})` | compatibility layer | Matches request origin and reflects it on success; adds `Vary: Origin`. |
| `options.origin = [string, regexp]` | `OriginSetting::allow_list(...)` | compatibility layer | Exact and regex matchers are explicit C++ values. |
| `options.origin = function` | `OriginMatcher::predicate(...)` or caller-built `CorsOptions` delegate | adapted | Synchronous predicate only; async error-first callback behavior is deferred. |
| `options.methods` | `CorsOptions::methods` | direct | Vector is joined with commas. |
| `options.allowedHeaders` / `options.headers` | `CorsOptions::allowed_headers` | direct | Empty optional reflects request headers and adds `Vary: Access-Control-Request-Headers`. |
| `options.exposedHeaders` | `CorsOptions::exposed_headers` | direct | Adds `Access-Control-Expose-Headers` on actual and preflight paths when non-empty. |
| `options.credentials` | `CorsOptions::credentials` | direct | Adds `Access-Control-Allow-Credentials: true` only when enabled. |
| `options.maxAge` | `CorsOptions::max_age` | direct | Adds `Access-Control-Max-Age`, including zero. |
| `options.preflightContinue` | `CorsOptions::preflight_continue` | direct | Determines whether a preflight result should continue or end. |
| `options.optionsSuccessStatus` | `CorsOptions::options_success_status` | direct | Defaults to 204. |
| `vary(res, 'Origin')` dependency | `polycpp::vary::vary(response_headers, "Origin")` | direct | No local Vary implementation. |
| `object-assign({}, defaults, options)` | defaulted `CorsOptions` value/copy | adapted | No dependency source is copied. |

## Framework object boundary review

- Upstream reads or mutates framework/request/response/context objects: yes, reads Node request-like objects and mutates response-like objects.
- Upstream fields or methods read: `req.method`, `req.headers.origin`, `req.headers['access-control-request-headers']`.
- Upstream fields or methods written/called: `res.setHeader`, `res.statusCode`, `res.end`.
- C++ adapter boundary: pure `RequestView`/`CorsResult` APIs plus adapters for `polycpp::http::Headers`, `IncomingMessage`, and `ServerResponse`-style objects.
- Partial mutation risk on validation failure: pure `evaluate` validates/builds into a local `Headers`; adapters apply after result construction, so invalid values do not partially update caller response headers.
- Local abstraction decision: no local `HeaderMap`, request class, or response class replaces `polycpp::http`; CORS-specific structs only describe policy/result state.
