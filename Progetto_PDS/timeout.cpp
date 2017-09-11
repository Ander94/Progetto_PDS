#include "timeout.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <future>


size_t read_some(tcp::socket &s, char* buf, size_t dim_buf) {
	size_t dim_read = 0;
	auto tp = std::chrono::system_clock::now() + std::chrono::seconds(2);
	// inizia entrambi i procedimenti
	try {
		auto f = std::async(std::launch::async, [&]() {
			char *buffer = (char*)malloc((dim_buf + 1) * sizeof(char));
			try {
				dim_read = s.read_some(boost::asio::buffer(buffer, dim_buf));
				memcpy(buf, buffer, dim_read);
				free(buffer);
			}
			catch (std::exception& e) {
				throw std::invalid_argument(e.what());
				free(buffer);
			}
		});
		std::future_status state = f.wait_until(tp);
		if (state != std::future_status::ready) {
			throw std::invalid_argument("Attenzione: connessione internet assente.\nControllare la connessione del proprio pc.");
		}
		else {
			return dim_read;
		}
	}
	catch (std::exception& e) {
		throw std::invalid_argument(e.what());
	}
}
