#include <iostream>

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

    std::cout << result.status_code << "\n";
    std::cout << result.headers.get("Access-Control-Allow-Origin").value_or("") << "\n";
    std::cout << result.headers.get("Access-Control-Allow-Credentials").value_or("") << "\n";
    return result.should_end_response ? 0 : 1;
}
