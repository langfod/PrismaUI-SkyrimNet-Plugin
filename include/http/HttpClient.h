#pragma once

#include <string>

namespace SkyrimNetUI::Http {

    /**
     * @brief HTTP response with status code and body
     */
    struct Response {
        int status = 0;      ///< HTTP status code (0 = network/connection error)
        std::string body;    ///< Response body

        /// @return true if request completed with 2xx status
        [[nodiscard]] bool ok() const { return status >= 200 && status < 300; }

        /// @return true if request reached the server (status != 0)
        explicit operator bool() const { return status != 0; }
    };

    /**
     * @brief Performs HTTP GET request
     * @param url URL to request (e.g., "http://localhost:8080/path")
     * @return Response with status and body; status=0 on connection failure
     */
    Response Get(const std::string& url);

    /**
     * @brief Performs HTTP POST request with JSON payload
     * @param url URL to request (e.g., "http://localhost:8080/path")
     * @param jsonData JSON payload as string
     * @return Response with status and body; status=0 on connection failure
     */
    Response Post(const std::string& url, const std::string& jsonData);

}  // namespace SkyrimNetUI::Http
