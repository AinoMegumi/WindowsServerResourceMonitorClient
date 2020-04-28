#pragma once
//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

// Performs an HTTP GET and prints the response
class session : public std::enable_shared_from_this<session> {
    tcp::resolver resolver_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_; // (Must persist between reads)
    http::request<http::empty_body> req_;
    http::response<http::string_body> res_;
    std::string ResStr;
public:
    // Objects are constructed with a strand to
    // ensure that handlers do not execute concurrently.
    explicit
        session(net::io_context& ioc)
        : resolver_(net::make_strand(ioc)), stream_(net::make_strand(ioc)), ResStr() {}

    // Start the asynchronous operation
    void run(char const* host, char const* port, char const* target, int version) {
        // Set up an HTTP GET request message
        this->req_.version(version);
        this->req_.method(http::verb::get);
        this->req_.target(target);
        this->req_.set(http::field::host, host);
        this->req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Look up the domain name
        this->resolver_.async_resolve(
            host,
            port,
            beast::bind_front_handler(
                &session::on_resolve,
                shared_from_this()));
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec) return fail(ec, "resolve");

        // Set a timeout on the operation
        this->stream_.expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        this->stream_.async_connect(
            results,
            beast::bind_front_handler(
                &session::on_connect,
                shared_from_this()));
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
        if (ec) return fail(ec, "connect");

        // Set a timeout on the operation
        this->stream_.expires_after(std::chrono::seconds(30));

        // Send the HTTP request to the remote host
        http::async_write(this->stream_, this->req_,
            beast::bind_front_handler(
                &session::on_write,
                shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec) return fail(ec, "write");

        // Receive the HTTP response
        http::async_read(this->stream_, this->buffer_, this->res_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec) return fail(ec, "read");

        std::stringstream ss{};

        // Write the message to standard out
        ss << this->res_ << std::endl;
        this->ResStr = ss.str();

        // Gracefully close the socket
        this->stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes so don't bother reporting it.
        if (ec && ec != beast::errc::not_connected) return fail(ec, "shutdown");

        // If we get here then the connection is closed gracefully
    }
};
