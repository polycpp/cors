#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <polycpp/cors/cors.hpp>

namespace cors = polycpp::cors;

namespace {

polycpp::http::Headers request_headers(std::string origin = "http://example.com",
                                       std::string request_headers_value = "x-header-1, x-header-2") {
    polycpp::http::Headers headers;
    if (!origin.empty()) {
        headers.set("Origin", origin);
    }
    if (!request_headers_value.empty()) {
        headers.set("Access-Control-Request-Headers", request_headers_value);
    }
    return headers;
}

std::string get(const polycpp::http::Headers& headers, const std::string& name) {
    return headers.get(name).value_or(std::string{});
}

struct FakeRequest {
    std::string method_value = "GET";
    polycpp::http::Headers headers_value = request_headers();

    const std::string& method() const noexcept { return method_value; }
    const polycpp::http::Headers& headers() const noexcept { return headers_value; }
};

struct FakeResponse {
    polycpp::http::Headers headers;
    int code = 200;
    bool ended = false;

    std::vector<std::string> getHeader(const std::string& name) const { return headers.getAll(name); }
    std::vector<std::string> getHeaderNames() const { return headers.names(); }
    FakeResponse& setHeader(const std::string& name, const std::string& value) {
        headers.set(name, value);
        return *this;
    }
    FakeResponse& status(int status_code) {
        code = status_code;
        return *this;
    }
    FakeResponse& end(const std::string& = "") {
        ended = true;
        return *this;
    }
};

}  // namespace

TEST(cors_policy, default_get_allows_all_origins_and_continues) {
    auto headers = request_headers();
    auto result = cors::evaluate("GET", headers);

    EXPECT_FALSE(result.is_preflight);
    EXPECT_TRUE(result.should_continue);
    EXPECT_FALSE(result.should_end_response);
    EXPECT_EQ(get(result.headers, "Access-Control-Allow-Origin"), "*");
    EXPECT_FALSE(result.headers.has("Access-Control-Allow-Methods"));
}

TEST(cors_policy, default_options_short_circuits_preflight) {
    auto headers = request_headers();
    auto result = cors::evaluate("OPTIONS", headers);

    EXPECT_TRUE(result.is_preflight);
    EXPECT_FALSE(result.should_continue);
    EXPECT_TRUE(result.should_end_response);
    EXPECT_EQ(result.status_code, 204);
    EXPECT_EQ(get(result.headers, "Access-Control-Allow-Origin"), "*");
    EXPECT_EQ(get(result.headers, "Access-Control-Allow-Methods"), "GET,HEAD,PUT,PATCH,POST,DELETE");
    EXPECT_EQ(get(result.headers, "Access-Control-Allow-Headers"), "x-header-1, x-header-2");
    EXPECT_EQ(get(result.headers, "Vary"), "Access-Control-Request-Headers");
    EXPECT_EQ(get(result.headers, "Content-Length"), "0");
}

TEST(cors_policy, preflight_status_and_continue_are_configurable) {
    auto headers = request_headers();

    cors::CorsOptions status_options;
    status_options.options_success_status = 200;
    auto status_result = cors::evaluate("options", headers, status_options);
    EXPECT_EQ(status_result.status_code, 200);
    EXPECT_TRUE(status_result.should_end_response);

    cors::CorsOptions continue_options;
    continue_options.preflight_continue = true;
    auto continue_result = cors::evaluate("OPTIONS", headers, continue_options);
    EXPECT_TRUE(continue_result.should_continue);
    EXPECT_FALSE(continue_result.should_end_response);
    EXPECT_FALSE(continue_result.headers.has("Content-Length"));
}

TEST(cors_policy, fixed_origin_adds_vary_origin) {
    cors::CorsOptions options;
    options.origin = cors::OriginSetting::fixed("http://fixed.example");

    auto result = cors::evaluate("GET", request_headers(), options);

    EXPECT_EQ(get(result.headers, "Access-Control-Allow-Origin"), "http://fixed.example");
    EXPECT_EQ(get(result.headers, "Vary"), "Origin");
}

TEST(cors_policy, apply_preserves_existing_vary_header) {
    cors::CorsOptions options;
    options.origin = cors::OriginSetting::fixed("http://fixed.example");

    polycpp::http::Headers response_headers;
    response_headers.set("Vary", "Accept-Encoding");

    auto result = cors::apply(cors::RequestView{"GET", request_headers()}, response_headers, options);

    EXPECT_TRUE(result.should_continue);
    EXPECT_EQ(get(response_headers, "Access-Control-Allow-Origin"), "http://fixed.example");
    EXPECT_EQ(get(response_headers, "Vary"), "Accept-Encoding, Origin");
}

TEST(cors_policy, disabled_origin_emits_no_cors_headers) {
    cors::CorsOptions options;
    options.origin = cors::OriginSetting::disabled();
    options.credentials = true;

    auto result = cors::evaluate("OPTIONS", request_headers(), options);

    EXPECT_TRUE(result.should_continue);
    EXPECT_FALSE(result.should_end_response);
    EXPECT_TRUE(result.headers.empty());
}

TEST(cors_policy, reflected_origin_mirrors_request_origin) {
    cors::CorsOptions options;
    options.origin = cors::OriginSetting::reflect();

    auto result = cors::evaluate("GET", request_headers("http://example.com"), options);

    EXPECT_EQ(get(result.headers, "Access-Control-Allow-Origin"), "http://example.com");
    EXPECT_EQ(get(result.headers, "Vary"), "Origin");
}

TEST(cors_policy, allow_list_supports_exact_regex_and_predicate_matchers) {
    cors::CorsOptions exact_options;
    exact_options.origin = cors::OriginSetting::allow_list({cors::OriginMatcher::exact("http://example.com")});
    auto exact = cors::evaluate("GET", request_headers("http://example.com"), exact_options);
    EXPECT_EQ(get(exact.headers, "Access-Control-Allow-Origin"), "http://example.com");
    EXPECT_EQ(get(exact.headers, "Vary"), "Origin");

    cors::CorsOptions regex_options;
    regex_options.origin = cors::OriginSetting::allow_list({cors::OriginMatcher::regex(R"(://(.+\.)?example\.org$)")});
    auto regex = cors::evaluate("GET", request_headers("https://api.example.org"), regex_options);
    EXPECT_EQ(get(regex.headers, "Access-Control-Allow-Origin"), "https://api.example.org");

    cors::CorsOptions predicate_options;
    predicate_options.origin = cors::OriginSetting::allow_list({
        cors::OriginMatcher::predicate([](std::string_view origin) { return origin.ends_with(".internal"); })
    });
    auto predicate = cors::evaluate("GET", request_headers("service.internal"), predicate_options);
    EXPECT_EQ(get(predicate.headers, "Access-Control-Allow-Origin"), "service.internal");
}

TEST(cors_policy, non_matching_static_origin_adds_vary_but_no_allow_origin) {
    cors::CorsOptions options;
    options.origin = cors::OriginSetting::allow_list({cors::OriginMatcher::exact("http://allowed.example")});
    options.credentials = true;

    auto result = cors::evaluate("GET", request_headers("http://denied.example"), options);

    EXPECT_FALSE(result.headers.has("Access-Control-Allow-Origin"));
    EXPECT_EQ(get(result.headers, "Vary"), "Origin");
    EXPECT_EQ(get(result.headers, "Access-Control-Allow-Credentials"), "true");
}

TEST(cors_policy, explicit_allowed_headers_do_not_add_request_headers_vary) {
    cors::CorsOptions options;
    options.allowed_headers = cors::HeaderList{"header1", "header2"};

    auto result = cors::evaluate("OPTIONS", request_headers(), options);

    EXPECT_EQ(get(result.headers, "Access-Control-Allow-Headers"), "header1,header2");
    EXPECT_FALSE(result.headers.has("Vary"));
}

TEST(cors_policy, empty_allowed_and_exposed_headers_emit_no_header) {
    cors::CorsOptions options;
    options.allowed_headers = cors::HeaderList{};
    options.exposed_headers = cors::HeaderList{};

    auto preflight = cors::evaluate("OPTIONS", request_headers(), options);
    auto actual = cors::evaluate("GET", request_headers(), options);

    EXPECT_FALSE(preflight.headers.has("Access-Control-Allow-Headers"));
    EXPECT_FALSE(preflight.headers.has("Vary"));
    EXPECT_FALSE(actual.headers.has("Access-Control-Expose-Headers"));
}

TEST(cors_policy, exposed_headers_credentials_and_max_age_are_supported) {
    cors::CorsOptions options;
    options.exposed_headers = cors::HeaderList{"x-exposed-1", "x-exposed-2"};
    options.credentials = true;
    options.max_age = 0;

    auto actual = cors::evaluate("GET", request_headers(), options);
    EXPECT_EQ(get(actual.headers, "Access-Control-Expose-Headers"), "x-exposed-1,x-exposed-2");
    EXPECT_EQ(get(actual.headers, "Access-Control-Allow-Credentials"), "true");
    EXPECT_FALSE(actual.headers.has("Access-Control-Max-Age"));

    auto preflight = cors::evaluate("OPTIONS", request_headers(), options);
    EXPECT_EQ(get(preflight.headers, "Access-Control-Expose-Headers"), "x-exposed-1,x-exposed-2");
    EXPECT_EQ(get(preflight.headers, "Access-Control-Allow-Credentials"), "true");
    EXPECT_EQ(get(preflight.headers, "Access-Control-Max-Age"), "0");
}

TEST(cors_policy, response_adapter_sets_headers_status_and_end) {
    FakeRequest request;
    request.method_value = "OPTIONS";
    FakeResponse response;
    response.headers.set("Vary", "Accept-Encoding");

    cors::CorsOptions options;
    options.origin = cors::OriginSetting::reflect();
    options.options_success_status = 200;

    const bool should_continue = cors::handle(request, response, options);

    EXPECT_FALSE(should_continue);
    EXPECT_TRUE(response.ended);
    EXPECT_EQ(response.code, 200);
    EXPECT_EQ(get(response.headers, "Access-Control-Allow-Origin"), "http://example.com");
    EXPECT_EQ(get(response.headers, "Content-Length"), "0");
    EXPECT_EQ(get(response.headers, "Vary"), "Accept-Encoding, Origin, Access-Control-Request-Headers");
}

TEST(cors_policy, invalid_configured_origin_fails_before_mutating_caller_headers) {
    cors::CorsOptions options;
    options.origin = cors::OriginSetting::fixed("bad\norigin");
    polycpp::http::Headers response;
    response.set("X-Stable", "kept");

    EXPECT_THROW(cors::apply(cors::RequestView{"GET", request_headers()}, response, options), polycpp::TypeError);
    EXPECT_EQ(get(response, "X-Stable"), "kept");
    EXPECT_FALSE(response.has("Access-Control-Allow-Origin"));
}
