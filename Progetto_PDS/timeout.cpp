#include "timeout.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <future>
#include "wx\wx.h"


int read_some(tcp::socket &s, char* buf, size_t dim_buf) {

	// inizia entrambi i procedimenti

		std::future<int> f = std::async(std::launch::async, [&]() {
			char *buffer = (char*)malloc((dim_buf + 1) * sizeof(char));
			int dim_read;
			try {
				dim_read = s.read_some(boost::asio::buffer(buffer, dim_buf));
				memcpy(buf, buffer, dim_read);
				free(buffer);
				return dim_read;
			}
			catch (...) {
				free(buffer);
				return -1;
			}
		});
		int dim_read = -1;
		std::future_status state = f.wait_for(std::chrono::seconds(5));
		if (state != std::future_status::ready) {
			throw std::invalid_argument("Attenzione: connessione internet assente.\nControllare la connessione del proprio pc.");
		}
		else {
			dim_read = f.get();
			if (dim_read < 0) {
				throw std::invalid_argument("Attenzione: l'utente ha interrotto l'invio del file.");
			}
		}
		return dim_read;
}
