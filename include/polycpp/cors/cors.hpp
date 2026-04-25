#pragma once

/**
 * @file cors.hpp
 * @brief CORS policy helpers for polycpp HTTP request/response objects.
 *
 * C++ port of npm cors (https://github.com/expressjs/cors). The port exposes
 * a deterministic policy layer that can be used without a live server, plus
 * adapters for polycpp HTTP headers and ServerResponse-style handles.
 *
 * @since 0.1.0
 */

#include <functional>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include <polycpp/core/error.hpp>
#include <polycpp/http/headers.hpp>
#include <polycpp/http/detail/headers_impl.hpp>
#include <polycpp/vary/vary.hpp>

namespace polycpp::cors {

using Headers = polycpp::http::Headers;
using HeaderList = std::vector<std::string>;

/** @brief Return the npm cors default method list. */
HeaderList default_methods();

/** @brief Join header/method values with commas, matching upstream array handling. */
std::string join(const HeaderList& values);

/** @brief A single origin allow-list matcher. */
class OriginMatcher {
public:
    enum class Kind {
        Exact,
        Regex,
        Predicate,
    };

    static OriginMatcher exact(std::string origin);
    static OriginMatcher regex(std::string pattern, std::regex::flag_type flags = std::regex::ECMAScript);
    static OriginMatcher predicate(std::function<bool(std::string_view)> matcher);

    Kind kind() const noexcept;
    bool matches(std::string_view origin) const;

private:
    Kind kind_ = Kind::Exact;
    std::string exact_;
    std::regex regex_;
    std::function<bool(std::string_view)> predicate_;
};

/** @brief CORS origin configuration with explicit C++ modes. */
class OriginSetting {
public:
    enum class Mode {
        Any,
        Disabled,
        Fixed,
        Reflect,
        AllowList,
    };

    static OriginSetting any();
    static OriginSetting disabled();
    static OriginSetting fixed(std::string origin);
    static OriginSetting reflect();
    static OriginSetting allow_list(std::vector<OriginMatcher> matchers);

    Mode mode() const noexcept;
    const std::string& fixed_origin() const noexcept;
    const std::vector<OriginMatcher>& matchers() const noexcept;

private:
    Mode mode_ = Mode::Any;
    std::string fixed_origin_;
    std::vector<OriginMatcher> matchers_;
};

/** @brief CORS options equivalent to the static options accepted by npm cors. */
struct CorsOptions {
    OriginSetting origin = OriginSetting::any();
    HeaderList methods = default_methods();
    std::optional<HeaderList> allowed_headers;
    std::optional<HeaderList> exposed_headers;
    bool credentials = false;
    std::optional<long long> max_age;
    bool preflight_continue = false;
    int options_success_status = 204;
};

/** @brief Explicit request boundary used by the pure policy evaluator. */
struct RequestView {
    std::string_view method;
    const Headers& headers;

    RequestView(std::string_view request_method, const Headers& request_headers) noexcept
        : method(request_method), headers(request_headers) {}
};

/** @brief Header and control-flow result produced by evaluating CORS policy. */
struct CorsResult {
    Headers headers;
    bool is_preflight = false;
    bool should_continue = true;
    bool should_end_response = false;
    int status_code = 200;
};

/** @brief Return true when @p method is an OPTIONS request. */
bool is_preflight_method(std::string_view method) noexcept;

/** @brief Evaluate CORS policy without mutating caller-owned response state. */
CorsResult evaluate(RequestView request, const CorsOptions& options = {});

/** @brief Convenience overload for method plus headers. */
CorsResult evaluate(std::string_view method, const Headers& request_headers, const CorsOptions& options = {});

/** @brief Apply evaluated CORS headers to an existing polycpp Headers object. */
void apply(const CorsResult& result, Headers& response_headers);

/** @brief Evaluate and apply CORS headers to an existing polycpp Headers object. */
CorsResult apply(RequestView request, Headers& response_headers, const CorsOptions& options = {});

}  // namespace polycpp::cors

#include <polycpp/cors/detail/aggregator.hpp>
