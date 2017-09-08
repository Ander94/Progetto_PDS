#pragma once
#include <boost/asio/connect.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/write.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/asio/read.hpp>

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;
using boost::lambda::bind;
using boost::lambda::var;
using boost::lambda::_1;
void write_line(boost::asio::io_service& io_service_, tcp::socket& socket_, deadline_timer& deadline_, const std::string& line,
	boost::posix_time::time_duration timeout);

std::string read_line(boost::asio::io_service& io_service_, tcp::socket& socket_, deadline_timer& deadline_, boost::posix_time::time_duration timeout);
void check_deadline(boost::asio::io_service& io_service_, tcp::socket& socket_, deadline_timer& deadline_);