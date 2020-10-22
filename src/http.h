#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <filesystem>
namespace http {
inline auto make_stream(boost::asio::io_context &ioc)
{
    namespace beast = boost::beast;
    return beast::tcp_stream(ioc);
}
template <typename Stream>
inline auto get(Stream &stream, std::string_view host, std::string_view target,
                std::string_view port = "80", unsigned version = 11)
{
    namespace beast = boost::beast; // from <boost/beast.hpp>
    namespace http = beast::http;   // from <boost/beast/http.hpp>
    namespace net = boost::asio;    // from <boost/asio.hpp>
    using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

    beast::error_code ec;
    // These objects perform our I/O
    net::ip::tcp::resolver resolver(stream.get_executor());

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    // Set up an HTTP GET request message
    http::request<http::string_body> req(
        http::verb::get, boost::string_view(target.data(), target.size()),
        version);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::string_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);

    // Gracefully close the socket
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    return res;
}
} // namespace http
namespace https {
inline auto make_ssl()
{
    namespace net = boost::asio;
    namespace ssl = net::ssl;
    ssl::context ctx(ssl::context::tlsv12_client);
    ctx.set_verify_mode(ssl::verify_none);
    return ctx;
}
inline auto make_stream(boost::asio::io_context &ioc)
{
    namespace beast = boost::beast;
    static auto ssl_ctx = make_ssl();
    return beast::ssl_stream<beast::tcp_stream>(ioc, ssl_ctx);
}
template <typename Stream>
inline auto get(Stream &stream, std::string_view host, std::string_view target,
                std::string_view port = "443", unsigned version = 11)
{
    namespace beast = boost::beast; // from <boost/beast.hpp>
    namespace http = beast::http;   // from <boost/beast/http.hpp>
    namespace net = boost::asio;    // from <boost/asio.hpp>
    namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>

    beast::error_code ec;
    // These objects perform our I/O
    net::ip::tcp::resolver resolver(stream.get_executor());

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.data())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()),
                             net::error::get_ssl_category()};
        throw beast::system_error{ec};
    }
    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(stream).connect(results);

    // Perform the SSL handshake
    stream.handshake(ssl::stream_base::client);

    // Set up an HTTP GET request message
    http::request<http::string_body> req(
        http::verb::get, boost::string_view(target.data(), target.size()),
        version);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::string_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);

    return res;
}
} // namespace https

#include <doctest/doctest.h>
TEST_CASE("http client")
{
    namespace net = boost::asio;
    net::io_context ioc;

    auto stream = http::make_stream(ioc);

    auto res = http::get(stream, "cdn.sci-news.com",
                         "/images/enlarge6/image_7735_1e-Curiosity.jpg");

    REQUIRE(res.result_int() == 200);
    // Write the message to standard out
    std::cout << res.base() << std::endl;
}

TEST_CASE("https client")
{
    namespace net = boost::asio;
    net::io_context ioc;

    auto stream = https::make_stream(ioc);

    auto res =
        https::get(stream, "cbsnews1.cbsistatic.com",
                   "/hub/i/2018/02/05/342997a2-34e5-489d-b5e7-9d2f463e5b16/"
                   "mars-rover-ap-18032563546287.jpg");
    REQUIRE(res.result_int() == 200);

    // Write the message to standard out
    std::cout << res.base() << std::endl;
}
