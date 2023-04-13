#include "uri.h"
#include "opengl.h"
#include <boost/gil/extension/io/jpeg.hpp>
#include <boost/gil/extension/io/bmp.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <string_view>
#include <sstream>
#include <fstream>
#include <filesystem>
namespace image {
inline auto get(std::string_view resource)
    -> std::optional<boost::gil::rgba8_image_t>
{
    using namespace boost::gil;
    if (auto res = uri::get(resource); res) {
        std::stringstream in_buffer(res->body(),
                                    std::ios_base::in | std::ios_base::binary);
        auto img = rgba8_image_t{};
        auto mime = (*res)[boost::beast::http::field::content_type];
        if (mime == "image/jpeg")
            read_and_convert_image(in_buffer, img, jpeg_tag());
        // else if (mime == "image/tiff")
        //     read_and_convert_image(in_buffer, img, tiff_tag());
        else if (mime == "image/bmp")
            read_and_convert_image(in_buffer, img, bmp_tag());
        else if (mime == "image/png")
            read_and_convert_image(in_buffer, img, png_tag());
        else
            return {};
        return img;
    } else {
        using namespace std::filesystem;
        auto file = path(resource);
        if (is_regular_file(file)) {
            auto img = rgba8_image_t{};
            if (file.extension() == ".jpeg" || file.extension() == ".jpg")
                read_and_convert_image(file.c_str(), img, jpeg_tag());
            // else if (file.extension() == ".tiff" || file.extension() == ".tif")
            //     read_and_convert_image(file.c_str(), img, tiff_tag());
            else if (file.extension() == ".bmp")
                read_and_convert_image(file.c_str(), img, bmp_tag());
            else if (file.extension() == ".png")
                read_and_convert_image(file.c_str(), img, png_tag());
            else
                return {};
            return img;
        }
    }
    return {};
}
inline auto glimage(boost::gil::rgba8_view_t const &image)
{
    using namespace gl;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    auto step = static_cast<GLint>(
        image.axis_iterator<1>(boost::gil::point_t(0, 0)).step() /
        image.num_channels());
    glPixelStorei(GL_UNPACK_ROW_LENGTH, step);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, static_cast<gl::GLsizei>(image.width()),
        static_cast<gl::GLsizei>(image.height()), 0, GL_RGBA, GL_UNSIGNED_BYTE,
        boost::gil::interleaved_view_get_raw_data(image));
}
} // namespace image
