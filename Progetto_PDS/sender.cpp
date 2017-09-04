#include "sender.h"
#include <boost/asio.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "protoType.h"

void sendUDPMessage(std::string& username, status& current_status, std::atomic<bool>& exit_app) {


	boost::asio::io_service io_service;
	boost::asio::ip::udp::socket socket(io_service);
	boost::asio::ip::udp::endpoint local_endpoint;
	boost::asio::ip::udp::endpoint sender_endpoint;

	socket.open(boost::asio::ip::udp::v4());
	socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	socket.set_option(boost::asio::socket_base::broadcast(true));
	sender_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::broadcast(), 1500);

	while (!exit_app.load()) {
		if (current_status == STAT_ONLINE) {
			try {
				socket.send_to(boost::asio::buffer(username + "\r\nONLINE\r\n"), sender_endpoint);
			}
			catch (boost::system::system_error e) {
				std::cout << e.what() << std::endl;
			}
		}	
		else {
			try {
				socket.send_to(boost::asio::buffer(username + "\r\nOFFLINE\r\n"), sender_endpoint);
			}
			catch (boost::system::system_error e) {
				std::cout << e.what() << std::endl;
			}
		}
		Sleep(TIME_SEND_MESSAGE_UDP);	
		
	}
	socket.close();
	return;
}

