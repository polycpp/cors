#include <polycpp/cors/cors.hpp>

#include <algorithm>
#include <cctype>
#include <sstream>
#include <utility>

namespace polycpp::cors {
namespace {

std::string upper_ascii(std::string_view value) {
    std::string result;
    result.reserve(value.size());
    for (char ch : value) {
        result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }
    return result;
}

bool has_text(std::string_view value) noexcept {
    return !value.empty();
}

void set_if_not_empty(Headers& headers, const std::string& name, const std::string& value) {
    if (!value.empty()) {
        headers.set(name, value);
    }
}

bool configure_origin(Headers& headers, RequestView request, const OriginSetting& origin) {
    const auto request_origin = request.headers.get("Origin");

    switch (origin.mode()) {
        case OriginSetting::Mode::Disabled:
            return false;
        case OriginSetting::Mode::Any:
            headers.set("Access-Control-Allow-Origin", "*");
            return true;
        case OriginSetting::Mode::Fixed:
            headers.set("Access-Control-Allow-Origin", origin.fixed_origin());
            polycpp::vary::vary(headers, "Origin");
            return true;
        case OriginSetting::Mode::Reflect:
            if (request_origin && has_text(*request_origin)) {
                headers.set("Access-Control-Allow-Origin", *request_origin);
            }
            polycpp::vary::vary(headers, "Origin");
            return true;
        case OriginSetting::Mode::AllowList: {
            bool allowed = false;
            if (request_origin) {
                for (const auto& matcher : origin.matchers()) {
                    if (matcher.matches(*request_origin)) {
                        allowed = true;
                        break;
                    }
                }
            }
            if (allowed && request_origin && has_text(*request_origin)) {
                headers.set("Access-Control-Allow-Origin", *request_origin);
            }
            polycpp::vary::vary(headers, "Origin");
            return true;
        }
    }
    return false;
}

void configure_credentials(Headers& headers, const CorsOptions& options) {
    if (options.credentials) {
        headers.set("Access-Control-Allow-Credentials", "true");
    }
}

void configure_methods(Headers& headers, const CorsOptions& options) {
    set_if_not_empty(headers, "Access-Control-Allow-Methods", join(options.methods));
}

void configure_allowed_headers(Headers& headers, RequestView request, const CorsOptions& options) {
    if (options.allowed_headers) {
        set_if_not_empty(headers, "Access-Control-Allow-Headers", join(*options.allowed_headers));
        return;
    }

    const auto requested = request.headers.get("Access-Control-Request-Headers");
    if (requested && has_text(*requested)) {
        headers.set("Access-Control-Allow-Headers", *requested);
    }
    polycpp::vary::vary(headers, "Access-Control-Request-Headers");
}

void configure_exposed_headers(Headers& headers, const CorsOptions& options) {
    if (options.exposed_headers) {
        set_if_not_empty(headers, "Access-Control-Expose-Headers", join(*options.exposed_headers));
    }
}

void configure_max_age(Headers& headers, const CorsOptions& options) {
    if (options.max_age) {
        headers.set("Access-Control-Max-Age", std::to_string(*options.max_age));
    }
}

}  // namespace

HeaderList default_methods() {
    return {"GET", "HEAD", "PUT", "PATCH", "POST", "DELETE"};
}

std::string join(const HeaderList& values) {
    std::ostringstream stream;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) stream << ',';
        stream << values[i];
    }
    return stream.str();
}

OriginMatcher OriginMatcher::exact(std::string origin) {
    OriginMatcher matcher;
    matcher.kind_ = Kind::Exact;
    matcher.exact_ = std::move(origin);
    return matcher;
}

OriginMatcher OriginMatcher::regex(std::string pattern, std::regex::flag_type flags) {
    OriginMatcher matcher;
    matcher.kind_ = Kind::Regex;
    matcher.regex_ = std::regex(std::move(pattern), flags);
    return matcher;
}

OriginMatcher OriginMatcher::predicate(std::function<bool(std::string_view)> predicate) {
    if (!predicate) {
        throw polycpp::TypeError("origin predicate is required");
    }
    OriginMatcher matcher;
    matcher.kind_ = Kind::Predicate;
    matcher.predicate_ = std::move(predicate);
    return matcher;
}

OriginMatcher::Kind OriginMatcher::kind() const noexcept { return kind_; }

bool OriginMatcher::matches(std::string_view origin) const {
    switch (kind_) {
        case Kind::Exact:
            return origin == exact_;
        case Kind::Regex:
            return std::regex_search(origin.begin(), origin.end(), regex_);
        case Kind::Predicate:
            return predicate_ && predicate_(origin);
    }
    return false;
}

OriginSetting OriginSetting::any() { return {}; }

OriginSetting OriginSetting::disabled() {
    OriginSetting setting;
    setting.mode_ = Mode::Disabled;
    return setting;
}

OriginSetting OriginSetting::fixed(std::string origin) {
    OriginSetting setting;
    setting.mode_ = Mode::Fixed;
    setting.fixed_origin_ = std::move(origin);
    return setting;
}

OriginSetting OriginSetting::reflect() {
    OriginSetting setting;
    setting.mode_ = Mode::Reflect;
    return setting;
}

OriginSetting OriginSetting::allow_list(std::vector<OriginMatcher> matchers) {
    OriginSetting setting;
    setting.mode_ = Mode::AllowList;
    setting.matchers_ = std::move(matchers);
    return setting;
}

OriginSetting::Mode OriginSetting::mode() const noexcept { return mode_; }
const std::string& OriginSetting::fixed_origin() const noexcept { return fixed_origin_; }
const std::vector<OriginMatcher>& OriginSetting::matchers() const noexcept { return matchers_; }

bool is_preflight_method(std::string_view method) noexcept {
    if (method.size() != 7) return false;
    const std::string upper = upper_ascii(method);
    return upper == "OPTIONS";
}

CorsResult evaluate(RequestView request, const CorsOptions& options) {
    CorsResult result;
    result.is_preflight = is_preflight_method(request.method);

    const bool cors_enabled = configure_origin(result.headers, request, options.origin);
    if (!cors_enabled) {
        return result;
    }

    configure_credentials(result.headers, options);

    if (result.is_preflight) {
        configure_methods(result.headers, options);
        configure_allowed_headers(result.headers, request, options);
        configure_max_age(result.headers, options);
        configure_exposed_headers(result.headers, options);

        if (!options.preflight_continue) {
            result.should_continue = false;
            result.should_end_response = true;
            result.status_code = options.options_success_status;
            result.headers.set("Content-Length", "0");
        }
        return result;
    }

    configure_exposed_headers(result.headers, options);
    return result;
}

CorsResult evaluate(std::string_view method, const Headers& request_headers, const CorsOptions& options) {
    return evaluate(RequestView{method, request_headers}, options);
}

void apply(const CorsResult& result, Headers& response_headers) {
    for (const auto& [name, value] : result.headers.raw()) {
        if (detail::equals_ignore_ascii_case(name, "Vary")) {
            polycpp::vary::vary(response_headers, value);
        } else {
            response_headers.set(name, value);
        }
    }
}

CorsResult apply(RequestView request, Headers& response_headers, const CorsOptions& options) {
    CorsResult result = evaluate(request, options);
    apply(result, response_headers);
    return result;
}

}  // namespace polycpp::cors
