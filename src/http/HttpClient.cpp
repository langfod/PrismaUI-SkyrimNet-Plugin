#include "http/HttpClient.h"

#include <httplib.h>

#include "pch.h"

namespace SkyrimNetUI::Http {

    // Split URL into base (scheme://host:port) and path
    static std::pair<std::string, std::string> SplitUrl(const std::string& url) {
        // Find the third slash (after scheme://)
        size_t schemeEnd = url.find("://");
        if (schemeEnd == std::string::npos) {
            return {url, "/"};
        }

        size_t pathStart = url.find('/', schemeEnd + 3);
        if (pathStart == std::string::npos) {
            return {url, "/"};
        }

        return {url.substr(0, pathStart), url.substr(pathStart)};
    }

    Response Get(const std::string& url) {
        try {
            auto [baseUrl, path] = SplitUrl(url);

            httplib::Client client(baseUrl);
            client.set_connection_timeout(30);
            client.set_read_timeout(30);
            client.set_follow_location(true);
            client.enable_server_certificate_verification(false);
            client.enable_server_hostname_verification(false);

            auto res = client.Get(path);

            if (!res) {
                logger::error("GET request failed: {}", httplib::to_string(res.error()));
                return {};
            }

            if (res->status >= 400) {
                logger::warn("GET request returned status {}: {}", res->status, url);
            }

            return {res->status, res->body};

        } catch (const std::exception& e) {
            logger::error("GET request exception: {}", e.what());
            return {};
        }
    }

    Response Post(const std::string& url, const std::string& jsonData) {
        logger::info("POST Request - URL: {}", url);
        logger::debug("POST Request - Payload: {}", jsonData);
        try {
            auto [baseUrl, path] = SplitUrl(url);

            httplib::Client client(baseUrl);
            client.set_connection_timeout(30);
            client.set_read_timeout(30);
            client.set_follow_location(true);
            client.enable_server_certificate_verification(false);
            client.enable_server_hostname_verification(false);

            auto res = client.Post(path, jsonData, "application/json");

            if (!res) {
                logger::error("POST request failed: {}", httplib::to_string(res.error()));
                return {};
            }

            logger::info("POST request status code: {}", res->status);

            if (res->status >= 400 && !res->body.empty()) {
                logger::error("Server error response: {}", res->body);
            }

            return {res->status, res->body};

        } catch (const std::exception& e) {
            logger::error("POST request exception: {}", e.what());
            return {};
        }
    }

}  // namespace SkyrimNetUI::Http
