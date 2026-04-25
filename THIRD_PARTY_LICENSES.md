# Third-Party Licenses

## Upstream package: cors

- Source: https://github.com/expressjs/cors
- Version basis: 2.8.6
- License: MIT
- License evidence: upstream `package.json` and `LICENSE`
- Use in this repo: clean-room C++ port based on public behavior and tests; no upstream source is vendored

## Companion dependency: vary

- Source: https://github.com/jshttp/vary
- polycpp companion: https://github.com/polycpp/vary
- npm version analyzed through `cors` dependency graph: 1.1.2
- License: MIT
- License evidence: npm package metadata and existing polycpp companion license review
- Use in this repo: linked/reused as `polycpp::vary`; no npm source is vendored

## object-assign

- Source: https://github.com/sindresorhus/object-assign
- npm version analyzed through `cors` dependency graph: 4.1.1
- License: MIT
- License evidence: npm package metadata
- Use in this repo: not linked or vendored; behavior is replaced by direct C++ defaulted option structs and clean-room assignment/copy semantics
