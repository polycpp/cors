# Dependency and JavaScript API Analysis

Run `scripts/analyze-upstream-js.py` after upstream intake, then consume `.tmp/dependency-analysis.json` to fill this file.

- package: cors
- package version: 2.8.6
- package root: `.tmp/upstream/cors`
- analyzer json: `.tmp/dependency-analysis.json`
- published npm artifact path: not used; source repo contains the published `lib/` entry
- published npm artifact analyzed: not needed for this package because declared entry points exist in the source clone
- include dev dependencies: no
- dependency source install used: yes, analyzer installed runtime dependencies in a temporary npm analysis directory
- companion root checked: `polycpp companion repositories`

## Package entry metadata

- main: `./lib/index.js`
- module: none
- types: none
- exports: none
- bin: none
- missing declared entries in repo clone: none
- TypeScript source files detected: none
- source-vs-published artifact decision: use the Git source clone as the primary analyzer input because the runtime entry point is present and matches package metadata

## Direct dependencies

- `object-assign`: JavaScript object merge helper.
- `vary`: HTTP `Vary` response header helper.

## Dependency ownership decisions

For every direct runtime dependency, choose exactly one outcome:

- use existing polycpp companion
- create separate private polycpp companion repo
- implement private helper in this repo
- optional adapter
- deferred or unsupported feature

Before choosing `implement private helper in this repo`, verify that neither
base `polycpp` nor an existing companion library already provides the required
behavior. Record that decision in `docs/research.md` under
`## Polycpp ecosystem reuse analysis`.

For every dependency, also choose a license strategy:

- permissive dependency ok with notice
- use existing companion license
- GPL-compatible repo or defer
- AGPL-compatible repo or defer
- optional GPL adapter or defer
- optional AGPL adapter or defer
- document LGPL obligations and linking model
- keep MPL code separated or replace/defer
- clean-room replacement
- dev/test-only, not shipped
- manual license review required (analyzer-only state; strict readiness fails until this is resolved)

License evidence must be concrete enough for another agent to audit. Acceptable examples:

- package.json license field
- manual SPDX check: LICENSE confirms MIT
- existing companion repo license reviewed
- dev/test-only dependency, not shipped

Do not leave analyzer-only evidence such as `unknown`, `unverified`, or `heuristic` in this table before strict readiness.

| Package | Kind | Requested | Installed | License | License evidence | License impact | License strategy | Affects repo license | Deps | Source files | Node API calls | JS API calls | Recommendation | Rationale |
|---|---|---|---|---|---|---|---|---|---:|---:|---:|---:|---|---|
| object-assign | hard | ^4 | 4.1.1 | MIT | package.json license field | permissive | clean-room replacement | no | 0 | 1 | 0 | 6 | implement private helper in this repo | C++ option/default merging does not need a separate public companion and no source is vendored. |
| vary | hard | ^1 | 1.1.2 | MIT | package.json license field and existing companion repo reviewed | permissive | use existing companion license | no | 0 | 1 | 0 | 14 | use existing polycpp companion | Existing `polycpp/vary` provides `polycpp::vary::vary(polycpp::http::Headers&, ...)`; CORS must reuse it for Vary header mutation. |

## License impact summary

- upstream package license: MIT
- repo license decision: MIT with `polycpp contributors` as copyright holder
- GPL/AGPL dependencies: none detected
- LGPL/MPL dependencies: none detected
- permissive dependencies requiring notices: upstream `cors` MIT notice and `vary` MIT notice; `object-assign` behavior is clean-room C++ option merging and no source is vendored
- dev/test-only dependencies excluded from shipped artifacts: `after`, `eslint`, `express`, `mocha`, `nyc`, and `supertest`
- dependency license notices to add to `THIRD_PARTY_LICENSES.md`: upstream `cors` MIT notice and `vary` MIT notice

## Transitive dependency summary

- `object-assign` has no runtime dependencies.
- `vary` has no runtime dependencies.
- Analyzer-reported dependency licenses are permissive.
- No npm dependency source is vendored into this C++ port.

## Runtime API usage

### Target package

- entry points analyzed: `lib/index.js`
- source files analyzed by analyzer: 1 target file
- source files manually inspected: `lib/index.js`, `test/test.js`, `test/example-app.js`, `test/issue-2.js`, `test/error-response.js`
- external imports seen from target: `object-assign`, `vary`

### Analyzer porting gates

- polycpp reuse hints consumed: analyzer reported none, but manual review selected base `polycpp::http` types and existing `polycpp::vary`
- security hints consumed: analyzer did not mark the package security-sensitive; manual review treats CORS policy as security-sensitive because browser exposure depends on generated headers
- security-sensitive package: yes for policy correctness; fail closed on unsupported async callback behavior by not exposing that API shape

### Node.js API usage

- target package uses no Node core modules directly.
- tests use Node `events` and `util` for fake response inheritance only; C++ tests can use direct structs and gtest.

### JavaScript API usage

- `Array.isArray` maps to typed `std::vector` option fields.
- `Array.prototype.push` maps to `std::vector::push_back`.
- `RegExp.prototype.test` maps to `std::regex_match` or predicate matchers.
- JavaScript truthiness around `origin` maps to explicit `OriginSetting` modes.

### Framework object boundary usage

- analyzer-reported target accesses: `req.headers.origin`, `req.headers['access-control-request-headers']`, `req.method`, `res.setHeader`, `res.statusCode`, and `res.end`.
- analyzer-reported dependency accesses: `vary` reads/writes `res.getHeader`/`res.setHeader`; this is handled by reusing the `polycpp::vary` companion.
- manual review decision: expose pure evaluation over `RequestView` plus adapters for `polycpp::http::Headers` and `ServerResponse`-style objects. Do not introduce a custom header map, request type, or response type as the primary API.

## Porting decisions

- Implement a deterministic synchronous CORS policy layer, not a JavaScript middleware factory.
- Use `polycpp::http::Headers` for request and response headers.
- Use `polycpp::vary` for Vary mutation.
- Use explicit `OriginSetting` and `OriginMatcher` instead of JavaScript dynamic option shapes.
- Return a `CorsResult` so callers can inspect whether a preflight should end or continue.
- Provide templated `handle` helpers for `IncomingMessage`/`ServerResponse`-style objects after pure behavior is tested.

Each porting decision should be consistent with the ecosystem reuse decisions
recorded in `docs/research.md`.

## Analyzer warnings

- none
