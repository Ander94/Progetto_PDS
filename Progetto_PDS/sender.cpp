#include "sender.h"
#include <boost/asio.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "protoType.h"

void sendUDPMessage(std::string& username, status& current_status) {


	boost::asio::io_service io_service;
	boost::asio::ip::udp::socket socket(io_service);
	boost::asio::ip::udp::endpoint local_endpoint;
	boost::asio::ip::udp::endpoint sender_endpoint;

	socket.open(boost::asio::ip::udp::v4());
	socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	socket.set_option(boost::asio::socket_base::broadcast(true));
	sender_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::broadcast(), 1500);

	while (1) {
		if (current_status == STAT_ONLINE) {
			try {
				//std::cout << "entro\n";
				socket.send_to(boost::asio::buffer(username), sender_endpoint);
			}
			catch (boost::system::system_error e) {
				std::cout << e.what() << std::endl;
			}
		}	
		Sleep(3000);	
		
	}
	socket.close();
	return;
}

