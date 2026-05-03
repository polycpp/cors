# Research

- package: cors
- npm url: https://www.npmjs.com/package/cors
- source url: https://github.com/expressjs/cors.git
- upstream version basis: 2.8.6
- upstream revision analyzed: 5317ebe670db2aaebc1d496eb5d33493deefb3ed
- upstream default branch: master
- license: MIT
- license evidence: package.json license field and upstream LICENSE
- category: HTTP middleware / CORS policy helper

## Package purpose

`cors` is Node.js middleware for applying Cross-Origin Resource Sharing response headers to Connect/Express-style request and response objects.

## Runtime assumptions

- browser: not a browser package; behavior targets HTTP server responses consumed by browsers
- node.js: upstream depends on Node/Express request and response object conventions
- filesystem: none
- network: no sockets opened by this package; it mutates HTTP response headers for already received requests
- crypto: none
- terminal: none

## Dependency summary

- package.json present: yes
- package main: `./lib/index.js`
- hard dependencies: `object-assign`, `vary`
- peer dependencies: none detected in package.json
- optional dependencies: none detected in package.json
- dependency analysis report: `docs/dependency-analysis.md`

## Upstream repo layout summary

Clone path used for analysis: `.tmp/upstream/cors`

Top files:

- `lib/index.js`
- `README.md`
- `LICENSE`
- `HISTORY.md`
- `package.json`
- `test/test.js`
- `test/example-app.js`
- `test/error-response.js`
- `test/issue-2.js`

Likely important implementation files:

- `lib/index.js`: all runtime behavior
- `test/test.js`: core fake request/response behavior for options, origins, headers, preflight, and callbacks
- `test/example-app.js`: Express integration examples for GET/HEAD/POST/DELETE/OPTIONS
- `test/issue-2.js`: reflected origin, credentials, methods, and max-age behavior
- `test/error-response.js`: confirms CORS headers are applied before downstream errors

## Entry points used by consumers

- `./lib/index.js`

Tests, fixtures, examples, and docs directories:

- `test`

## Important files and why they matter

- `lib/index.js`: source of truth for CORS header generation and middleware control-flow behavior.
- `test/test.js`: provides deterministic unit cases that can be ported without standing up a server.
- `test/example-app.js`: validates HTTP method behavior and preflight short-circuiting.
- `README.md`: documents options and user-facing examples.

## Files likely irrelevant to the C++ port

- `.eslintrc.yml`, `.github/*`, and `test/support/env.js`: JavaScript lint/CI/runtime setup.
- `HISTORY.md`: release history only; useful for context but not behavior.

## Test directories worth mining first

- `test`: adapt fake request/response unit cases for static options, reflected origins, Vary behavior, allowed/exposed headers, credentials, max-age, preflight response status, and preflight continuation.

## Implementation risks discovered from the source layout

- Upstream is a middleware factory with JavaScript callback-based option delegates; the C++ port needs a deterministic synchronous boundary.
- Upstream mutates duck-typed `req`/`res`; the C++ port must use `polycpp::http::Headers`, `IncomingMessage`, and `ServerResponse` rather than local HTTP containers.
- `Vary` handling must reuse the existing `polycpp::vary` companion and preserve append semantics.
- `origin: false` disables CORS entirely, while non-matching static origin rules still add `Vary: Origin`.
- Preflight requests may either end the response or continue depending on `preflightContinue`.

## Companion repo alignment

- companion repos inspected: `vary`, `forwarded`, `content-type`, `express`
- CMake target and alias pattern: `polycpp_cors` with alias `polycpp::cors`
- public header layout: `include/polycpp/cors/cors.hpp`
- detail/private header strategy: `include/polycpp/cors/detail/aggregator.hpp` for template adapters and small inline helpers
- aggregator header strategy: keep the public header as the main include and include the detail aggregator at the end, matching `vary`
- examples strategy: provide a small policy/evaluation example rather than an Express server example in 1.0.0
- documentation site strategy: use libgen Doxygen plus Sphinx scaffold and replace placeholder user pages before public release
- deliberate deviations from existing companions: add a dedicated `CorsResult` pure-evaluation type so behavior can be tested without a live HTTP server

## Polycpp ecosystem reuse analysis

- polycpp core paths inspected: `include/polycpp/http/headers.hpp`, `include/polycpp/http/http.hpp`, `include/polycpp/http/detail/incoming_message.hpp`, `include/polycpp/http/detail/server_response.hpp`
- polycpp core types/functions selected: `polycpp::http::Headers`, `IncomingMessage::method()`, `IncomingMessage::headers()`, `ServerResponse::setHeader()`, `ServerResponse::status()`, and `ServerResponse::end()`
- polycpp core types/functions rejected: local HTTP header/request/response containers are rejected because the base library already provides these concepts
- companion libs inspected for reusable APIs: `polycpp/vary`
- companion libs selected for reuse: `polycpp::vary::vary(polycpp::http::Headers&, ...)` for `Vary` mutation
- companion libs rejected or deferred: none for 1.0.0; `object-assign` is not a reusable C++ companion candidate because C++ option merging is direct struct copy/defaulting
- new local abstractions introduced: `CorsOptions`, `OriginSetting`, `OriginMatcher`, `RequestView`, and `CorsResult`; these are policy/result types, not replacement HTTP containers
- reuse risks or integration gaps: `polycpp::http::Headers::get()` comma-joins values with comma-space, so tests should assert CORS-visible behavior rather than raw JS object storage

## External SDK and native driver strategy

- upstream external services/protocols: CORS policy over HTTP headers only
- native SDKs/client libraries to use: none
- SDKs/protocols explicitly not reimplemented: no HTTP server, Express middleware engine, or browser CORS enforcement engine is implemented here
- adapter/linking strategy: link `polycpp` and existing `polycpp_vary`; expose header adapters for `polycpp::http` objects
- test environment needs: no external service; CMake/gtest is sufficient

## Security and fail-closed review

- security-sensitive behavior: yes, CORS decides which origins and headers are exposed to browsers
- trust boundary: request `Origin`, `Access-Control-Request-Headers`, and method are user-controlled HTTP inputs
- supported protocol or algorithm matrix: default wildcard origin, disabled origin, fixed origin, reflected origin, exact/regex/predicate allow-list, preflight methods, allowed headers, exposed headers, credentials, max-age, options success status
- unsupported behavior and fail-closed policy: asynchronous JavaScript callbacks are not represented; C++ delegates are synchronous and explicit
- key, secret, credential, or user-controlled input handling: no secrets; user-controlled origin/header strings are copied only into validated HTTP response headers through `polycpp::http::Headers`
- misuse cases that must be tested: reflected origins, non-matching origins, `origin=false`, credentials with wildcard origin, Vary append behavior, request-header reflection, preflight short-circuiting, and invalid header values

## Core use cases

- Apply default CORS headers to a normal request.
- Apply preflight CORS headers and generate a 204 empty response result.
- Reflect a request origin when `origin=true` or an allow-list matches.
- Append `Vary: Origin` and `Vary: Access-Control-Request-Headers` through `polycpp::vary`.
- Use the result with `polycpp::http::Headers` or a `ServerResponse`-style object.

## Key features to port first

- Static options and default options.
- Origin modes: wildcard, disabled, fixed, reflected, exact/regex/predicate allow-list.
- Methods, allowed headers, exposed headers, credentials, max-age, preflight continue, and options success status.
- Pure `evaluate(...)` and `apply(...)` APIs.
- Template adapters for `IncomingMessage`/`ServerResponse`-style objects.

## Features to defer

- JavaScript asynchronous error-first callbacks.
- Express/Connect middleware registration API.
- Browser-side CORS enforcement or fetch integration.
- Full Express integration tests until the `express` companion has a stable middleware ABI.

## 1.0.0 scope

- port version: 1.0.0
- versioning note: port version is independent from upstream versioning
- supported APIs: `CorsOptions`, `OriginSetting`, `OriginMatcher`, `RequestView`, `CorsResult`, `evaluate`, `apply`, and `handle`
- unsupported APIs: JavaScript middleware factory shape, asynchronous delegates, Node callback error propagation, and Express app registration
- dependency plan: reuse `polycpp::vary`; implement `object-assign` behavior as direct C++ option/default merging
- polycpp modules to use: `polycpp::http::Headers`, `polycpp::http::IncomingMessage`, `polycpp::http::ServerResponse`, `polycpp::TypeError`
- missing polycpp primitives: no missing primitive for 1.0.0; future Express integration may need a shared middleware ABI
