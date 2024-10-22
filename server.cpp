// SPI_WEBSERVER.cpp : Defines the entry point for the application.
//

#include "server.h"

#include <array>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>


void client(int socket);

int main()
{
	int server_socket;
	struct sockaddr_in address;
	int opt = 1;
	socklen_t addrlen = sizeof(address);

	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(8080);

	if (bind(server_socket, (sockaddr*)&address, addrlen) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	constexpr int max_queued_connections = 3;
	if (listen(server_socket, max_queued_connections) < 0)
	{
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	while (true)
	{
		int new_socket;

		if ((new_socket = accept(server_socket, (sockaddr*)&address, &addrlen)) < 0)
		{
			perror("accept failed");
			exit(EXIT_FAILURE);
		}

		if (fork() == 0)
		{
			close(server_socket);
			client(new_socket);
			exit(0);
		}
		else
		{
			close(new_socket);
		}
	}

	return 0;
}

struct route_entry
{
	std::string interface;
	unsigned long dest;
	unsigned long gateway;
	unsigned short flags;
	unsigned short ref_count;
	unsigned short use;
	unsigned short metric;
	unsigned long mask;
	unsigned long mtu;
	unsigned short window;
	unsigned short irtt;
};

std::vector<route_entry> parse_route_file()
{
	std::fstream f;
	f.open("/proc/net/route", ios::in);

	if (!f.is_open())
		return {};

	vector<route_entry> entries;

	std::string line;
	//skip first line
	std::getline(f, line);

	while (f)
	{
		std::getline(f, line);
		if (line.empty())
			continue;

		std::stringstream s = std::stringstream(line);
		using namespace std;
		route_entry e;

		s >> e.interface;
		s >> hex >> e.dest;
		s >> hex >> e.gateway;
		s >> dec >> e.flags;
		s >> dec >> e.ref_count;
		s >> dec >> e.use;
		s >> dec >> e.metric;
		s >> hex >> e.mask;
		s >> dec >> e.mtu;
		s >> dec >> e.window;
		s >> dec >> e.irtt;
		entries.push_back(e);
	}
	return entries;
}


// flags
enum RouteFlag {
    RTF_UP = 0x0001,        // route usable
    RTF_GATEWAY = 0x0002,   // destination is a gateway
    RTF_HOST = 0x0004,      // host entry (net otherwise)
    RTF_REINSTATE = 0x0008, // reinstate route after tmout
    RTF_DYNAMIC = 0x0010,   // created dyn. (by redirect)
    RTF_MODIFIED = 0x0020,  // modified dyn. (by redirect)
    RTF_MTU = 0x0040,       // specific MTU for this route
    RTF_MSS = RTF_MTU,      // Compatibility :-(
    RTF_WINDOW = 0x0080,    // per route window clamping
    RTF_IRTT = 0x0100,      // Initial round trip time
    RTF_REJECT = 0x0200     // Reject route
};


// Array of flag names corresponding to the defined flags
std::array<std::string, 11> flags = {
	"RTF_UP",
	"RTF_GATEWAY",
	"RTF_HOST",
	"RTF_REINSTATE",
	"RTF_DYNAMIC",
	"RTF_MODIFIED",
	"RTF_MTU",
	"RTF_MSS",
	"RTF_WINDOW",
	"RTF_IRTT",
	"RTF_REJECT"
};

// Array of descriptions for each flag
std::array<std::string, 11> flags_descriptions = {
	"Route is usable.",
	"Destination is a gateway.",
	"Host entry (network otherwise).",
	"Reinstate route after timeout.",
	"Created dynamically (by redirect).",
	"Modified dynamically (by redirect).",
	"Specific MTU for this route.",
	"MSS (Maximum Segment Size), compatibility.",
	"Per route window clamping.",
	"Initial round trip time.",
	"Reject route."
};


std::string intToIP(unsigned long ip)
{
	std::string result;
	result += std::to_string(ip & 0xFF);
	result += ".";
	result += std::to_string((ip >> 8) & 0xFF);
	result += ".";
	result += std::to_string((ip >> 16) & 0xFF);
	result += ".";
	result += std::to_string((ip >> 24) & 0xFF);
	return result;
}

std::string flagsToString(int flagsi)
{
	std::string result;
	for (int i = 0; i < 11; i++)
		if (flagsi & (1 << i))
		{
			result += flags[i];
			result += " - ";
			result += flags_descriptions[i];
			result += "<br>";
		}
	return result;
}

std::string boldIf(std::string text, bool condition)
{
	if (condition)
		return "<b>" + text + "</b>";
	return text;
}

std::string maskSlash(uint32_t ip)
{
	// how many ones in the mask

	auto total = 0;
	for (int i = 0; i < 32; i++)
		if (ip & (1 << i))
			total++;

	return intToIP(ip) + "/" + std::to_string(total);
}

void client(int new_socket)
{
	char buffer[1024] = {0};
	read(new_socket, buffer, 1024);
	std::cout << "Message received: " << buffer << std::endl;
	//send(new_socket, "Hello from server", strlen("Hello from server"), 0);


	auto entries = parse_route_file();

	// create html table from entries


	std::string html =

		"<!DOCTYPE html>"
		"<html>"
		"<style>"
		"table {"
		"  font-family: arial, sans-serif;"
		"  border-collapse: collapse;"
		"  width: 100%;"
		"}"
		""
		"td, th {"
		"  border: 1px solid #dddddd;"
		"  text-align: left;"
		"  padding: 8px;"
		"}"
		""
		"tr:nth-child(even) {"
		"  background-color: #dddddd;"
		"}"
		"</style>"
		"<head><title>proc/net/route</title></head>"
		//"<html><head><title>Routing Table</title></head><body><table><tr><th>Interface</th><th>Destination</th><th>Mask</th><th>Gateway</th><th>Flags</th><th>Metric</th><th>RefCnt</th><th>Use</th><th>MTU</th><th>Window</th><th>IRTT</th></tr>";
		"<html><head><title>Routing Table</title></head><body><table><tr><th>Interface</th><th>Destination</th><th>Mask</th><th>Gateway</th><th>Flags</th><th>Metric</th></tr>";

	for (auto& entry : entries)
	{
		// only usable routes
		if (!(entry.flags & RTF_UP))
			continue;

		html += "<tr>";
		html += "<td>" + entry.interface + "</td>";
		html += "<td>" + intToIP(entry.dest) + "</td>";
		html += "<td>" + maskSlash(entry.mask) + "</td>";
		html += "<td>" + boldIf(intToIP(entry.gateway), !entry.mask) + "</td>";
		html += "<td>" + flagsToString(entry.flags) + "</td>";
		html += "<td>" + std::to_string(entry.metric) + "</td>";
	//	html += "<td>" + std::to_string(entry.ref_count) + "</td>";
	//	html += "<td>" + std::to_string(entry.use) + "</td>";
	//	html += "<td>" + std::to_string(entry.mtu) + "</td>";
	//	html += "<td>" + std::to_string(entry.window) + "</td>";
	//	html += "<td>" + std::to_string(entry.irtt) + "</td>";
		html += "</tr>";
	}
	html += "</table></body></html>";

	std::string header =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: " +
		std::to_string(html.size()) +
		"\r\n"
		"Connection: close\r\n"
		"\r\n" +
		html;

	// send html response
	send(new_socket, header.c_str(), header.size(), 0);
	close(new_socket);
}
