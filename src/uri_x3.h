#pragma once
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/tuple.hpp>
#include <string_view>
#include <vector>
namespace boost::spirit::x3::traits {
template <typename Char, typename Trait>
struct is_range<std::basic_string_view<Char, Trait>> : mpl::true_ {};
} // namespace boost::spirit::x3::traits
namespace uri {
namespace parser {
namespace x3 = boost::spirit::x3;
template <class Subject> struct raw_directive : x3::raw_directive<Subject> {
    typedef x3::unary_parser<Subject, x3::raw_directive<Subject>> base_type;
    typedef x3::raw_attribute_type attribute_type;
    static bool const handles_container = Subject::handles_container;
    typedef Subject subject_type;
    using x3::raw_directive<Subject>::raw_directive;

    template <typename Iterator, typename Context, typename RContext,
              typename Attribute>
    bool parse(Iterator &first, Iterator const &last, Context const &context,
               RContext &rcontext, Attribute &attr) const
    {
        x3::skip_over(first, last, context);
        Iterator saved = first;
        if (this->subject.parse(first, last, context, rcontext, x3::unused)) {
            attr = Attribute(saved, first);
            // first = saved;
            return true;
        }
        return false;
    }
};
struct raw_gen {
    template <class Subject>
    raw_directive<typename x3::extension::as_parser<Subject>::value_type>
    operator[](Subject const &subject) const
    {
        return {x3::as_parser(subject)};
    }
};
inline auto constexpr raw = raw_gen{};
x3::rule<class sub_delims> const sub_delims = "sub_delims";
auto const sub_delims_def = x3::char_("!$&'()*+,;=");
BOOST_SPIRIT_DEFINE(sub_delims)
x3::rule<class gen_delims> const gen_delims = "gen_delims";
auto const gen_delims_def = x3::char_(":/?#[]@");
BOOST_SPIRIT_DEFINE(gen_delims)
x3::rule<class reserved> const reserved = "reserved";
auto const reserved_def = gen_delims | sub_delims;
BOOST_SPIRIT_DEFINE(reserved)
x3::rule<class unreserved> const unreserved = "unreserved";
auto const unreserved_def = x3::alpha | x3::digit | x3::char_("-._~");
BOOST_SPIRIT_DEFINE(unreserved)
x3::rule<class pct_encoded> const pct_encoded = "pct_encoded";
auto const pct_encoded_def = x3::lit("%") >> x3::xdigit >> x3::xdigit;
BOOST_SPIRIT_DEFINE(pct_encoded)
x3::rule<class pchar> const pchar = "pchar";
auto const pchar_def = unreserved | pct_encoded | sub_delims | x3::char_(":@");
BOOST_SPIRIT_DEFINE(pchar)
x3::rule<class query, std::string_view> const query = "query";
auto const query_def = raw[*(pchar | x3::char_("/?"))];
BOOST_SPIRIT_DEFINE(query)
x3::rule<class fragment, std::string_view> const fragment = "fragment";
auto const fragment_def = raw[*(pchar | x3::char_("/?"))];
BOOST_SPIRIT_DEFINE(fragment)
x3::rule<class segment> const segment = "segment";
auto const segment_def = *pchar;
BOOST_SPIRIT_DEFINE(segment)
x3::rule<class segment_nz> const segment_nz = "segment_nz";
auto const segment_nz_def = +pchar;
BOOST_SPIRIT_DEFINE(segment_nz)
x3::rule<class segment_nz_nc> const segment_nz_nc = "segment_nz_nc";
auto const segment_nz_nc_def = +(unreserved | pct_encoded | sub_delims | '@');
BOOST_SPIRIT_DEFINE(segment_nz_nc)
x3::rule<class path_empty> const path_empty = "path_empty";
auto const path_empty_def = x3::repeat(0)[pchar];
BOOST_SPIRIT_DEFINE(path_empty)
x3::rule<class path_rootless> const path_rootless = "path_rootless";
auto const path_rootless_def = segment_nz >> *('/' >> segment);
BOOST_SPIRIT_DEFINE(path_rootless)
x3::rule<class path_noscheme> const path_noscheme = "path_noscheme";
auto const path_noscheme_def = segment_nz_nc >> *('/' >> segment);
BOOST_SPIRIT_DEFINE(path_noscheme)
x3::rule<class path_absolute> const path_absolute = "path_absolute";
auto const path_absolute_def = x3::char_('/') >>
                               -(segment_nz >> *('/' >> segment));
BOOST_SPIRIT_DEFINE(path_absolute)
x3::rule<class path_abempty, std::string_view> const path_abempty =
    "path_abempty";
auto const path_abempty_def = raw[*(x3::char_('/') >> segment)];
BOOST_SPIRIT_DEFINE(path_abempty)
x3::rule<class path> const path = "path";
auto const path_def = path_abempty    // begins with "/" or is empty
                      | path_absolute // begins with "/" but not "//"
                      | path_noscheme // begins with a non-colon segment
                      | path_rootless // begins with a segment
                      | path_empty;   // zero characters
BOOST_SPIRIT_DEFINE(path)
x3::rule<class reg_name> const reg_name = "reg_name";
auto const reg_name_def = *(unreserved | pct_encoded | sub_delims);
BOOST_SPIRIT_DEFINE(reg_name)
x3::rule<class dec_octet> const dec_octet = "dec_octet";
auto const dec_octet_def = (x3::lit("25") >> x3::char_("0-5")) |
                           (x3::lit("2") >> x3::char_("0-4") >> x3::digit) |
                           (x3::lit("1") >> x3::repeat(2)[x3::digit]) |
                           (x3::char_("1-9") >> x3::digit) | x3::digit;
BOOST_SPIRIT_DEFINE(dec_octet)
x3::rule<class ip_v4_address> const ip_v4_address = "ip_v4_address";
auto const ip_v4_address_def =
    dec_octet >> "." >> dec_octet >> "." >> dec_octet >> "." >> dec_octet;
BOOST_SPIRIT_DEFINE(ip_v4_address)
x3::rule<class h16> const h16 = "h16";
auto const h16_def = x3::repeat(1, 4)[x3::xdigit];
BOOST_SPIRIT_DEFINE(h16)
x3::rule<class ls32> const ls32 = "ls32";
auto const ls32_def = (h16 >> ":" >> h16) | ip_v4_address;
BOOST_SPIRIT_DEFINE(ls32)
x3::rule<class ip_v6_address> const ip_v6_address = "ip_v6_address";
auto const ip_v6_address_def =
    (x3::repeat(6)[h16 >> ":"] >> ls32) |
    (x3::lit("::") >> x3::repeat(5)[h16 >> ":"] >> ls32) |
    (-h16 >> "::" >> x3::repeat(4)[h16 >> ":"] >> ls32) |
    (-(x3::repeat(0, 1)[h16 >> ":"] >> h16) >> "::" >>
     x3::repeat(3)[h16 >> ":"] >> ls32) |
    (-(h16 >> x3::repeat(0, 2)[x3::lit(":") >> h16]) >> "::" >>
     (ls32 >> x3::repeat(2)[":" >> h16])) |
    (-(h16 >> x3::repeat(0, 3)[":" >> h16]) >> "::" >> h16 >> ":" >> ls32) |
    (-(h16 >> x3::repeat(0, 4)[x3::lit(":") >> h16]) >> "::" >> ls32) |
    (-(h16 >> x3::repeat(0, 5)[x3::lit(":") >> h16]) >> "::" >> h16) |
    (-(h16 >> x3::repeat(0, 6)[x3::lit(":") >> h16]) >> "::");
BOOST_SPIRIT_DEFINE(ip_v6_address)
x3::rule<class ip_vfuture> const ip_vfuture = "ip_vfuture";
auto const ip_vfuture_def = x3::lit("v") >> +x3::xdigit >> "." >>
                            +(unreserved | sub_delims | ":");
BOOST_SPIRIT_DEFINE(ip_vfuture)
x3::rule<class ip_literal> const ip_literal = "ip_literal";
auto const ip_literal_def = x3::lit("[") >> (ip_v6_address | ip_vfuture) >> "]";
BOOST_SPIRIT_DEFINE(ip_literal)
x3::rule<class port, std::string_view> const port = "port";
auto const port_def = raw[*x3::digit];
BOOST_SPIRIT_DEFINE(port)
x3::rule<class host, std::string_view> const host = "host";
auto const host_def = raw[ip_literal | ip_v4_address | reg_name];
BOOST_SPIRIT_DEFINE(host)
x3::rule<class userinfo, std::string_view> const userinfo = "userinfo";
auto const userinfo_def = raw[*(unreserved | pct_encoded | sub_delims | ":")];
BOOST_SPIRIT_DEFINE(userinfo)
x3::rule<class authority, std::string_view> const authority = "authority";
auto const authority_def = raw[-(userinfo >> "@") >> host >> -(":" >> port)];
BOOST_SPIRIT_DEFINE(authority)
x3::rule<class scheme, std::string_view> const scheme = "scheme";
auto const scheme_def =
    raw[x3::alpha >> *(x3::alpha | x3::digit | x3::char_("+-."))];
BOOST_SPIRIT_DEFINE(scheme)
x3::rule<class relative_part> const relative_part = "relative_part";
auto const relative_part_def = (x3::lit("//") >> authority >> path_abempty) |
                               path_absolute | path_noscheme | path_empty;
BOOST_SPIRIT_DEFINE(relative_part)
x3::rule<class relative_ref> const relative_ref = "relative_ref";
auto const relative_ref_def = relative_part >> -("?" >> query) >>
                              -("#" >> fragment);
BOOST_SPIRIT_DEFINE(relative_ref)
x3::rule<class hier_part, std::string_view> const hier_part = "hier_part";
auto const hier_part_def = raw[(x3::lit("//") >> authority >> path_abempty) |
                               path_absolute | path_rootless | path_empty];
BOOST_SPIRIT_DEFINE(hier_part)
x3::rule<class uri> const uri = "uri";
auto const uri_def = scheme >> ":" >> hier_part >> -("?" >> query) >>
                     -("#" >> fragment);
BOOST_SPIRIT_DEFINE(uri)
x3::rule<class absolute_uri> const absolute_uri = "absolute_uri";
auto const absolute_uri_def = scheme >> ":" >> hier_part >> -("?" >> query);
BOOST_SPIRIT_DEFINE(absolute_uri)
x3::rule<class uri_reference> const uri_reference = "uri_reference";
auto const uri_reference_def = uri | relative_ref;
BOOST_SPIRIT_DEFINE(uri_reference)
} // namespace parser
struct parts {
    std::string_view scheme;
    std::string_view userinfo;
    std::string_view host;
    std::string_view port;
    std::string_view path;
    std::string_view query;
    std::string_view fragment;
};
inline constexpr auto parse(std::string_view uri) -> parts
{
    using namespace parser;
    auto iter = uri.begin();
    parts p;
    if (!x3::parse(iter, uri.end(), scheme >> ":", p.scheme))
        return {};
    if (x3::parse(iter, uri.end(), x3::lit("//"))) {
        if (!x3::parse(iter, uri.end(), userinfo >> "@", p.userinfo))
            p.userinfo = {};
        if (!x3::parse(iter, uri.end(), host, p.host))
            return {};
        if (!x3::parse(iter, uri.end(), -(":" >> port), p.port))
            return {};
        if (!x3::parse(iter, uri.end(), path_abempty, p.path))
            return {};
    } else {
        if (!x3::parse(iter, uri.end(),
                       raw[path_absolute | path_rootless | path_empty], p.path))
            return {};
    }
    if (!x3::parse(iter, uri.end(), -("?" >> query), p.query))
        return {};
    if (!x3::parse(iter, uri.end(), -("#" >> fragment), p.fragment))
        return {};
    return p;
}
} // namespace uri
#if __has_include(<doctest/doctest.h>)
#include <doctest/doctest.h>
TEST_CASE("uri parser")
{
    namespace x3 = boost::spirit::x3;
    using namespace std::literals;
    auto view = "http://www.google.com:80/hello?query#fragment"sv;
    auto parts = uri::parse(view);
     REQUIRE_EQ(parts.scheme, "http");
     REQUIRE_EQ(parts.userinfo, "");
     REQUIRE_EQ(parts.host, "www.google.com");
     REQUIRE_EQ(parts.port, "80");
     REQUIRE_EQ(parts.path, "/hello");
     REQUIRE_EQ(parts.query, "query");
     REQUIRE_EQ(parts.fragment, "fragment");
}
#endif