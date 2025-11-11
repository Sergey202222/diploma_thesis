#pragma once
#include <iostream>
#include <fstream>
#include <regex>
#include <set>

#include "data_base.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

/*  Beast functions   */
beast::string_view mime_type(beast::string_view path);// Return a reasonable mime type based on the extension of a file.
std::string path_cat(beast::string_view base, beast::string_view path);// Append an HTTP rel-path to a local filesystem path. The returned path is normalized for the platform.
void fail(beast::error_code ec, char const* what); // Report a failure


/*   My functions   */
std::string open_start_file_search_result(const std::string& file_path); //получить содержимое файла html для вывода результата клиенту
bool split_str_content(const std::string& source_str, std::string& start_str, std::string& end_str); //разделить строку на 2 части по делимитеру "<!--search result below-->"
std::string clear_request_string(const std::string& source_str); //очистить строку поиска от служебного содержимого
std::set<std::string> get_words_request_set(const std::string& source_str);  //создать set из слов запроса
bool urls_vector_cmp(std::pair<std::string, int> pair_a, std::pair<std::string, int> pair_b); //сравнение пар урл-значение
std::string get_post_request_result_string(const std::string& request_string, Data_base* data_base, int search_results);  //получить строку с результатами поиска по словам
std::string prepare_body_string(const std::string& path, const std::string& request_string, const std::string& search_result_string);


// Return a response for the given request.
// The concrete type of the response message (which depends on the request), is type-erased in message_generator.
template <class Body, class Allocator>
http::message_generator handle_request(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>>&& req,
    int search_results,
    Data_base* data_base)
{
    // Returns a bad request response
    auto const bad_request =
        [&req](beast::string_view why)
        {
            http::response<http::string_body> res{ http::status::bad_request, req.version() };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "<p>" + std::string(why) + "</p>";
            res.prepare_payload();
            return res;
        };

    // Returns a not found response
    auto const not_found =
        [&req](beast::string_view target)
        {
            http::response<http::string_body> res{ http::status::not_found, req.version() };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "<p>The resource '" + std::string(target) + "' was not found.</p><p>Try from <a href=\"index.html\">the start page</a></p>";
            res.prepare_payload();
            return res;
        };

    // Returns a server error response
    auto const server_error =
        [&req](beast::string_view what)
        {
            http::response<http::string_body> res{ http::status::internal_server_error, req.version() };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "<p>An error occurred: '" + std::string(what) + "'</p>";
            res.prepare_payload();
            return res;
        };

    // Make sure we can handle the method
    if (req.method() != http::verb::get &&
        req.method() != http::verb::head &&
        req.method() != http::verb::post)
        return bad_request("Unknown HTTP-method");

    // Request path must be absolute and not contain "..".
    if (req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return bad_request("Illegal request-target");

    // Build the path to the requested file
    std::string path = path_cat(doc_root, req.target());
    if (req.target().back() == '/')
        path.append("index.html");

    // Attempt to open the file
    beast::error_code ec;
    http::file_body::value_type body;

    body.open(path.c_str(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if (ec == beast::errc::no_such_file_or_directory)
        return not_found(req.target());

    // Handle an unknown error
    if (ec)
        return server_error(ec.message());

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if (req.method() == http::verb::head)
    {
        http::response<http::empty_body> res{ http::status::ok, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    }

    // Respond to GET request
    else if (req.method() == http::verb::get)
    {
        http::response<http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version()) };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    }

    //POST request

    std::string request_string = clear_request_string(req.body());
    std::cout << "request_string = " << request_string << "\n\n";
    std::string search_result_string = get_post_request_result_string(request_string, data_base, search_results); //получить строку с результатами поиска по словам                                             

    http::response<http::string_body> res{ http::status::ok, req.version() };
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    //res.set(http::field::content_type, "text/html");
    res.set(http::field::content_type, mime_type(path));
    res.keep_alive(req.keep_alive());
    res.body() = prepare_body_string(path, request_string, search_result_string);
    res.content_length(res.body().size());
    res.prepare_payload();
    std::cout << "get req search_results = " << search_results << "\n";
    return res;
}