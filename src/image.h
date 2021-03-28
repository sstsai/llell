#include "http.h"
#include "uri.h"
#include "opengl.h"
#include <boost/mpl/vector.hpp>
#include <boost/gil/extension/io/jpeg.hpp>
#include <string_view>
#include <sstream>
#include <fstream>
namespace image {
auto read_jpeg(std::string_view resource)
    -> std::optional<boost::gil::rgba8_image_t>
{
    using namespace std::literals;
    constexpr auto http = "http"sv;
    constexpr auto https = "https"sv;
    if (auto [scheme, userinfo, host, port, path, query, fragment] =
            uri::parse(resource);
        scheme.size()) {
        if (scheme == http) {
            namespace net = boost::asio;
            net::io_context ioc;
            auto stream = http::make_stream(ioc);
            auto res = http::get(stream, host, path, port.size() ? port : "80");
            if (res.result_int() == 200) {
                using namespace boost::gil;
                std::stringstream in_buffer(
                    res.body(), std::ios_base::in | std::ios_base::binary);
                auto img = rgba8_image_t{};
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
                auto img = rgba8_image_t{};
                read_and_convert_image(in_buffer, img, jpeg_tag());
                //write_view("a.jpg", view(img), jpeg_tag());
                return img;
            }
        }
    } else {
        using namespace boost::gil;
        auto img = rgba8_image_t{};
        read_and_convert_image(std::string(resource), img, jpeg_tag());
        return img;
    }
    return {};
}
auto glimage(boost::gil::rgba8_view_t const &image)
{
    using namespace gl;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, static_cast<gl::GLsizei>(image.width()),
        static_cast<gl::GLsizei>(image.height()), 0, GL_RGBA, GL_UNSIGNED_BYTE,
        boost::gil::interleaved_view_get_raw_data(image));
}
} // namespace image
