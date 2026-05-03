#pragma once

/**
 * @file detail/aggregator.hpp
 * @brief Template adapters for polycpp/cors.
 * @since 1.0.0
 */

#include <polycpp/cors/cors.hpp>

#include <string>
#include <string_view>

namespace polycpp::cors::detail {

inline char lower_ascii(char ch) noexcept {
    if (ch >= 'A' && ch <= 'Z') return static_cast<char>(ch - 'A' + 'a');
    return ch;
}

inline bool equals_ignore_ascii_case(std::string_view left, std::string_view right) noexcept {
    if (left.size() != right.size()) return false;
    for (std::size_t i = 0; i < left.size(); ++i) {
        if (lower_ascii(left[i]) != lower_ascii(right[i])) return false;
    }
    return true;
}

template <typename Response>
inline void set_result_header(Response& response, const std::string& name, const std::string& value) {
    if (equals_ignore_ascii_case(name, "Vary")) {
        polycpp::vary::vary(response, value);
    } else {
        response.setHeader(name, value);
    }
}

}  // namespace polycpp::cors::detail

namespace polycpp::cors {

/**
 * @brief Evaluate policy from an IncomingMessage-style request object.
 *
 * The request object must expose method() and headers() members compatible with
 * std::string_view and polycpp::http::Headers respectively.
 */
template <typename Request>
CorsResult evaluate(const Request& request, const CorsOptions& options = {}) {
    return evaluate(RequestView{request.method(), request.headers()}, options);
}

/**
 * @brief Apply an evaluated result to a ServerResponse-style object.
 *
 * The response object must expose setHeader(name, value). If the result ends
 * the response, it must also expose status(code) and end().
 *
 * @return true when downstream handling should continue.
 */
template <typename Response>
bool apply(const CorsResult& result, Response& response) {
    for (const auto& [name, value] : result.headers.raw()) {
        detail::set_result_header(response, name, value);
    }

    if (result.should_end_response) {
        response.status(result.status_code);
        response.end();
    }

    return result.should_continue;
}

/**
 * @brief Evaluate and apply CORS policy to request/response-style objects.
 *
 * @return true when downstream handling should continue.
 */
template <typename Request, typename Response>
bool handle(const Request& request, Response& response, const CorsOptions& options = {}) {
    return apply(evaluate(request, options), response);
}

}  // namespace polycpp::cors
