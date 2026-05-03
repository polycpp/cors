#pragma once

/**
 * @file cors.hpp
 * @brief CORS policy helpers for polycpp HTTP request/response objects.
 *
 * C++ port of npm cors (https://github.com/expressjs/cors). The port exposes
 * a deterministic policy layer that can be used without a live server, plus
 * adapters for polycpp HTTP headers and ServerResponse-style handles.
 *
 * @since 1.0.0
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

/** @brief Header container used by the CORS policy layer. */
using Headers = polycpp::http::Headers;

/** @brief Ordered list of method or header names joined into comma-separated header values. */
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

    /** @brief Match one origin string exactly. */
    static OriginMatcher exact(std::string origin);

    /** @brief Match origins using std::regex_search with the provided pattern and flags. */
    static OriginMatcher regex(std::string pattern, std::regex::flag_type flags = std::regex::ECMAScript);

    /** @brief Match origins with a synchronous predicate. Throws TypeError when matcher is empty. */
    static OriginMatcher predicate(std::function<bool(std::string_view)> matcher);

    /** @brief Return the configured matcher kind. */
    Kind kind() const noexcept;

    /** @brief Return true when @p origin satisfies this matcher. */
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

    /** @brief Emit Access-Control-Allow-Origin: * without adding Vary: Origin. */
    static OriginSetting any();

    /** @brief Disable CORS header generation and leave requests to continue downstream. */
    static OriginSetting disabled();

    /** @brief Emit one configured origin value and add Vary: Origin. */
    static OriginSetting fixed(std::string origin);

    /** @brief Mirror the request Origin value when present and add Vary: Origin. */
    static OriginSetting reflect();

    /** @brief Reflect request origins accepted by at least one matcher and add Vary: Origin. */
    static OriginSetting allow_list(std::vector<OriginMatcher> matchers);

    /** @brief Return the configured origin mode. */
    Mode mode() const noexcept;

    /** @brief Return the value configured by fixed(). */
    const std::string& fixed_origin() const noexcept;

    /** @brief Return the matcher list configured by allow_list(). */
    const std::vector<OriginMatcher>& matchers() const noexcept;

private:
    Mode mode_ = Mode::Any;
    std::string fixed_origin_;
    std::vector<OriginMatcher> matchers_;
};

/** @brief CORS options equivalent to the static options accepted by npm cors. */
struct CorsOptions {
    /** @brief Origin policy. Defaults to wildcard. */
    OriginSetting origin = OriginSetting::any();

    /** @brief Methods emitted on preflight responses. Defaults to npm cors methods. */
    HeaderList methods = default_methods();

    /**
     * @brief Headers emitted as Access-Control-Allow-Headers on preflight responses.
     *
     * When absent, the request Access-Control-Request-Headers value is reflected
     * and Vary: Access-Control-Request-Headers is added. When present but empty,
     * no allow-header value is emitted.
     */
    std::optional<HeaderList> allowed_headers;

    /** @brief Exposed headers emitted on actual and preflight responses when present and non-empty. */
    std::optional<HeaderList> exposed_headers;

    /** @brief Emit Access-Control-Allow-Credentials: true when enabled. */
    bool credentials = false;

    /** @brief Emit Access-Control-Max-Age on preflight responses when present, including zero. */
    std::optional<long long> max_age;

    /** @brief Continue downstream after preflight headers instead of ending the response. */
    bool preflight_continue = false;

    /** @brief Status used when preflight_continue is false. Defaults to 204. */
    int options_success_status = 204;
};

/** @brief Explicit request boundary used by the pure policy evaluator. */
struct RequestView {
    /** @brief Incoming request method. OPTIONS is matched case-insensitively. */
    std::string_view method;

    /** @brief Incoming request headers. The view does not take ownership. */
    const Headers& headers;

    RequestView(std::string_view request_method, const Headers& request_headers) noexcept
        : method(request_method), headers(request_headers) {}
};

/** @brief Header and control-flow result produced by evaluating CORS policy. */
struct CorsResult {
    /** @brief Headers generated by policy evaluation. */
    Headers headers;

    /** @brief True when the request method is OPTIONS case-insensitively. */
    bool is_preflight = false;

    /** @brief Whether downstream request handling should continue. */
    bool should_continue = true;

    /** @brief Whether a response adapter should terminate the response. */
    bool should_end_response = false;

    /** @brief Status code for response termination, usually a preflight response. */
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
