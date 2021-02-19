#pragma once
#include <boost/asio/async_result.hpp>
#include <boost/asio/query.hpp>
#include <boost/asio/io_context.hpp>
#include <tl/expected.hpp>
#include <fmt/printf.h>
#include <chrono>
namespace timeout {
namespace asio = boost::asio;
template <typename Stream> struct run_for {
    Stream &stream;
    std::chrono::steady_clock::duration timeout;
};
template <typename Stream>
run_for(Stream &, std::chrono::steady_clock::duration) -> run_for<Stream &>;
} // namespace timeout
namespace boost::asio {
using boost::system::error_code;
template <typename T, typename Stream>
class async_result<timeout::run_for<Stream>, void(error_code, T)> {
private:
    Stream &stream;
    std::chrono::steady_clock::duration timeout;
    error_code error;
    T t;

public:
    // An asynchronous operation's initiating function automatically creates an
    // completion_handler_type object from the token. This function object is
    // then called on completion of the asynchronous operation.
    class completion_handler_type {
    public:
        completion_handler_type(timeout::run_for<Stream> const &token)
            : token(token)
        {}
        void operator()(error_code const &error, T t)
        {
            *this->error = error;
            *this->t = t;
        }

    private:
        friend class async_result;
        timeout::run_for<Stream> token;
        error_code *error;
        T *t;
    };

    // The async_result constructor associates the completion handler object
    // with the result of the initiating function.
    explicit async_result(completion_handler_type &h)
        : stream(h.token.stream), timeout(h.token.timeout)
    {
        h.error = &error;
        h.t = &t;
    }

    // The return_type typedef determines the result type of the asynchronous
    // operation's initiating function.
    using return_type = tl::expected<T, error_code>;

    // The get() function is used to obtain the result of the asynchronous
    // operation's initiating function. For the close_after completion token, we
    // use this function to run the io_context until the operation is complete.
    return_type get()
    {
        namespace net = asio;
        net::io_context &io_context = static_cast<net::io_context &>(
            net::query(stream.get_executor(), net::execution::context));

        // Restart the io_context, as it may have been left in the "stopped"
        // state by a previous operation.
        io_context.restart();

        // Block until the asynchronous operation has completed, or timed out.
        // If the pending asynchronous operation is a composed operation, the
        // deadline applies to the entire operation, rather than individual
        // operations on the socket.
        io_context.run_for(timeout);

        // If the asynchronous operation completed successfully then the
        // io_context would have been stopped due to running out of work. If it
        // was not stopped, then the io_context::run_for call must have timed
        // out and the operation is still incomplete.
        if (!io_context.stopped()) {
            // Close the socket to cancel the outstanding asynchronous
            // operation.
            stream.close();

            // Run the io_context again until the operation completes.
            io_context.run();

            return tl::make_unexpected(asio::error::timed_out);
        }

        if (error)
            return tl::make_unexpected(error);
        return t;
    }
};
} // namespace boost::asio
