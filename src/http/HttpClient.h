#pragma once

#include <string>

namespace SkyrimNetUI {
namespace Http {

/**
 * @brief Performs HTTP GET request
 * @param url Wide string URL to request
 * @return Response body as string, empty on failure
 */
std::string Get(const std::wstring& url);

/**
 * @brief Performs HTTP POST request with JSON payload
 * @param url Wide string URL to request
 * @param jsonData JSON payload as string
 * @return true if successful (2xx status code), false otherwise
 */
bool Post(const std::wstring& url, const std::string& jsonData);

} // namespace Http
} // namespace SkyrimNetUI
