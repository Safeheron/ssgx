#ifndef SSGXLIB_SSGX_HTTP_T_FILTER_H
#define SSGXLIB_SSGX_HTTP_T_FILTER_H

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ssgx_http_t_structs.h"

namespace ssgx {
namespace http_t {
/**
 * @brief Abstract base class for HTTP request filters.
 *
 * Filters are used to modify or inspect HTTP requests and responses before or after handling them. Derived classes can
 * override the `Before` and `After` methods to implement custom filtering logic.
 */
class Filter {
  public:
    /**
     * @brief Perform filtering before the request handler is executed.
     *
     * This method is called before the handler for the HTTP request is executed. It allows you to modify or inspect the
     * request and response.
     *
     * @param request The HTTP request object.
     * @param response The HTTP response object.
     * @return True to continue processing, or false to stop and return the response early.
     */
    virtual bool Before(Request& request, Response& response) {
        return true;
    }

    /**
     * @brief Perform filtering after the request handler is executed.
     *
     * This method is called after the handler for the HTTP request is executed. It allows you to modify or inspect the
     * request and response.
     *
     * @param request The HTTP request object.
     * @param response The HTTP response object.
     */
    virtual void After(Request& request, Response& response) {
    }

    /**
     * @brief Destructor.
     */
    virtual ~Filter() = default;
};

/**
 * @brief A class to manage a chain of filters for request processing.
 *
 * This class allows you to add filters to a chain and execute them before and after processing a request with a
 * handler. It ensures that each filter's `Before` and `After` methods are called in sequence, and handles the
 * request-response flow.
 */
class FilterChain {
  public:
    /**
     * @brief Add a filter to the filter chain.
     *
     * This method adds a filter to the chain. The filter will be cloned and added as part of the execution sequence for
     * incoming requests.
     *
     * @param filter An unique pointer to the filter to add.
     */
    void AddFilter(std::unique_ptr<Filter>&& filter) {
        filters_.emplace_back(std::move(filter));
    }
    /**
     * @brief Execute the filter chain for a request.
     *
     * This method executes the filter chain for a given request, running each filter's `Before` method before the
     * request handler and each filter's `After` method after the handler.
     *
     * @param request The HTTP request object to process.
     * @param handler The handler function to process the request.
     * @return The response after executing the handler and all filters.
     */
    Response Execute(Request& request, const std::function<void(Request&, Response&)>& handler) {
        Response response;

        for (auto& filter : filters_) {
            if (!filter->Before(request, response)) {
                return response;
            }
        }

        try {
            handler(request, response);
        } catch (const std::exception& ex) {
            response.SetResp("Internal Server Error: " + std::string(ex.what()), "text/plain", HttpStatusCode::InternalServerError500);
        } catch (...) {
            response.SetResp("Internal Server Error: Unknown exception", "text/plain", HttpStatusCode::InternalServerError500);
        }

        for (auto& filter : filters_) {
            filter->After(request, response);
        }

        return response;
    }

  private:
    std::vector<std::unique_ptr<Filter>> filters_;
};

} // namespace http_t
} // namespace ssgx

#endif // SSGXLIB_SSGX_HTTP_T_FILTER_H
