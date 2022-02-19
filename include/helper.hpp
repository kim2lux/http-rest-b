#pragma once

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <iostream>

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>

// Report a failure
void fail(beast::error_code ec, char const *what);
