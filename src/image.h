#include "http.h"
#include "uri.h"
#include "opengl.h"
#include <boost/mpl/vector.hpp>
#include <boost/gil/extension/io/jpeg.hpp>
#include <string_view>
#include <sstream>
namespace image {
auto read_jpeg(std::string_view resource)
    -> std::optional<boost::gil::rgb8_image_t>
{
    using namespace std::literals;
    constexpr auto http = "http"sv;
    constexpr auto https = "https"sv;
    if (auto result = uri::scheme(uri::state{resource}); result.parsed.size()) {
        auto scheme = result.parsed;
        result = uri::authority(result);
        if (!result.parsed.size())
            return {};
        auto path = result.remaining;
        result = uri::userinfo(uri::state{result.parsed});
        auto userinfo = result.parsed;
        result = uri::host(result);
        auto host = result.parsed;
        result = uri::port(result);
        auto port = result.parsed;
        if (scheme == http) {
            namespace net = boost::asio;
            net::io_context ioc;
            auto stream = http::make_stream(ioc);
            auto res = http::get(stream, host, path, port.size() ? port : "80");
            if (res.result_int() == 200) {
                using namespace boost::gil;
                std::stringstream in_buffer(
                    res.body(), std::ios_base::in | std::ios_base::binary);
                auto img = rgb8_image_t{};
                read_and_convert_image(in_buffer, img, jpeg_tag());
                return img;
            }
        } else if (scheme == https) {
            namespace net = boost::asio;
            net::io_context ioc;
            auto stream = https::make_stream(ioc);
            auto res =
                https::get(stream, host, path, port.size() ? port : "443");
            if (res.result_int() == 200) {
                using namespace boost::gil;
                std::stringstream in_buffer(
                    res.body(), std::ios_base::in | std::ios_base::binary);
                auto img = rgb8_image_t{};
                read_and_convert_image(in_buffer, img, jpeg_tag());
                return img;
            }
        }
    } else {
        using namespace boost::gil;
        auto img = rgb8_image_t{};
        read_and_convert_image(std::string(resource), img, jpeg_tag());
        return img;
    }
    return {};
}
auto glimage(boost::gil::rgb8_view_t const &image)
{
    using namespace gl;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, static_cast<gl::GLsizei>(image.width()),
        static_cast<gl::GLsizei>(image.height()), 0, GL_RGB, GL_UNSIGNED_BYTE,
        boost::gil::interleaved_view_get_raw_data(image));
}
} // namespace image
