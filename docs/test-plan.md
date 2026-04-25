# Test Plan

## Unit tests

- Default GET request emits `Access-Control-Allow-Origin: *` and continues.
- Default OPTIONS request emits preflight headers, `Content-Length: 0`, status 204, and does not continue.
- `options_success_status` changes preflight status.
- `preflight_continue=true` emits preflight headers without ending the response.
- Method comparison normalizes `OPTIONS` case.
- Fixed origin emits fixed origin and `Vary: Origin`.
- Reflected origin mirrors request `Origin` and adds `Vary: Origin`.
- Disabled origin emits no CORS headers.
- Exact, regex, and predicate origin matchers reflect allowed origins.
- Non-matching static origin rules emit `Vary: Origin` but no allow-origin header.
- Allowed headers can be explicit or reflected from request headers.
- Exposed headers, credentials, and max-age are emitted only when configured.
- Existing `Vary` values are preserved and appended through `polycpp::vary`.

## Integration tests

- Apply `CorsResult` to `polycpp::http::Headers`.
- Use a response-like fake object with `getHeader`, `getHeaderNames`, `setHeader`, `status`, and `end` to verify adapter behavior.
- Build the example target with `POLYCPP_CORS_BUILD_EXAMPLES=ON`.

## Compatibility tests adapted from upstream

- Adapt core fake request/response cases from `test/test.js`.
- Adapt GET/HEAD/POST/DELETE/OPTIONS examples from `test/example-app.js` at the policy layer.
- Adapt reflected origin, credentials, method, and max-age cases from `test/issue-2.js`.
- Adapt error-response intent from `test/error-response.js` by verifying CORS headers are generated before downstream continuation.

## Security and fail-closed tests

- Invalid reflected header values throw through `polycpp::http::Headers` validation before caller-owned headers are mutated.
- Origin disabled emits no CORS headers.
- Non-matching origin allow-list does not emit `Access-Control-Allow-Origin`.
- Preflight short-circuit always sets `Content-Length: 0`.
- Unsupported async callback shape is absent from the API rather than emulated partially.

## Release-blocking behaviors

- Unit tests pass.
- Example compiles and runs.
- README examples match actual code.
- `python3 docs/build.py` passes.
- `python3 check-port-validation.py --run-docs-build <repo-root>` passes.

## Current validation

- pending implementation
