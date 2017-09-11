#pragma once
#include <boost/asio/connect.hpp>
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
#include <condition_variable>
#include <mutex>
#include <thread>
#include <chrono>
using boost::asio::ip::tcp;

size_t read_some(tcp::socket &s, char* buf, size_t dim_buf); 