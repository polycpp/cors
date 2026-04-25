# polycpp-cors

C++ companion port of [cors](https://www.npmjs.com/package/cors) for [polycpp](https://github.com/enricohuang/polycpp).

## Status

Port version: `0.1.0`

Initial port based on upstream version: `2.8.6`

Compatibility note:

- This repo does not imply full parity with upstream `cors`.
- Implemented and deferred behavior is tracked in `docs/research.md`, `docs/api-mapping.md`, and `docs/divergences.md`.

Implemented:

- Default wildcard CORS policy.
- Disabled, fixed, reflected, exact, regex, and predicate origin policies.
- Methods, allowed headers, exposed headers, credentials, max-age, preflight continue, and custom OPTIONS success status.
- Pure policy evaluation through `CorsResult`.
- Application to `polycpp::http::Headers`.
- Template adapters for `IncomingMessage`/`ServerResponse`-style objects.
- `Vary` mutation through the existing `polycpp::vary` companion library.

Deferred:

- Express/Connect middleware registration API.
- Asynchronous JavaScript option callbacks and callback error propagation.
- Browser CORS enforcement.
- Full live Express integration until the `express` companion middleware ABI is stable.

Known divergences:

- The C++ API is explicit and synchronous: `CorsOptions`, `OriginSetting`, `OriginMatcher`, `evaluate`, `apply`, and `handle` replace the JavaScript middleware factory shape.
- Dynamic JavaScript option shapes are represented as typed C++ configuration values.

## Prerequisites

- C++20 compiler
- CMake 3.20+
- Ninja recommended

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPOLYCPP_CORS_BUILD_EXAMPLES=ON
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
```

## Usage

```cpp
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

    // result.status_code == 204
    // result.should_end_response == true
    // result.headers contains Access-Control-Allow-Origin, credentials,
    // Access-Control-Allow-Methods, Access-Control-Allow-Headers,
    // Access-Control-Max-Age, Vary, and Content-Length.
}
```

To mutate an existing response header object:

```cpp
polycpp::http::Headers response_headers;
auto result = polycpp::cors::apply(
    polycpp::cors::RequestView{"GET", request_headers},
    response_headers,
    options);
```

For a `ServerResponse`-style object exposing `setHeader`, `status`, and `end`, use `polycpp::cors::handle(request, response, options)`.

## License

MIT
