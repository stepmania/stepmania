#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <string>
#include <iostream>
#include <chrono>
#include <thread>

char const *owner = "wolfman2000";
char const *nick = "appveyor-sm5";
char const *server = "irc.freenode.net";
char const *channel = "#stepmania";

bool startsWithPing(char const *buffer)
{
	return std::strncmp(buffer, "PING ", 5) == 0;
}

int main(int argc, char* argv[])
{
	// The common return value.
	int ret;
	// Set up the buffer.
	char buffer[512];

	auto *repoName = argv[1];
	auto *repoVersion = argv[2];
	auto *repoHash = argv[3];
	auto *repoAuthor = argv[4];
	auto *repoBuildUrl = argv[5];
	auto *repoSuccess = argv[6];

	SOCKET sock;
	struct WSAData *wd = (struct WSAData*)malloc(sizeof(struct WSAData));
	ret = WSAStartup(MAKEWORD(2, 0), wd);
	free(wd);

	if (ret)
	{
		std::cout << "Error loading Windows Socket API" << std::endl;
		return 1;
	}

	struct addrinfo hints;
	struct addrinfo *ai;

	// Ensure the hints are cleared.
	memset(&hints, 0, sizeof(struct addrinfo));
	// IPv4 vs IPv6: should not matter.
	hints.ai_family = AF_UNSPEC;
	// IRC uses TCP sockets.
	hints.ai_socktype = SOCK_STREAM;
	// Be explicit about TCP.
	hints.ai_protocol = IPPROTO_TCP;

	ret = getaddrinfo(server, "6667", &hints, &ai);
	if (ret != 0)
	{
		std::cout << "Problem with getting the server information!" << std::endl;
		return 2;
	}

	sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (sock == -1)
	{
		std::cout << "Problem with setting up the socket!" << std::endl;
		return 3;
	}

	ret = connect(sock, ai->ai_addr, ai->ai_addrlen);
	if (ret == -1)
	{
		std::cout << "Problem with connecting to the IRC server!" << std::endl;
	}

	// The server data is not needed anymore.
	freeaddrinfo(ai);

	// Start some of the basic commands.
	std::sprintf(buffer, "USER %s 0 * :%s\r\n", nick, owner);
	send(sock, buffer, strlen(buffer), 0);
	std::sprintf(buffer, "NICK %s\r\n", nick);
	send(sock, buffer, strlen(buffer), 0);

	// Respect the continuous loop.
	for (;;)
	{
		// receive the bytes as we get them.
		int numBytes = recv(sock, buffer, 511, 0);
		if (numBytes <= 0)
		{
			// We have quit IRC. Get out of here.
			break;
		}
		buffer[numBytes] = '\0'; // Ensure a null terminated string.
		if (startsWithPing(buffer))
		{
			// We MUST reply to a PING with PONG. Otherwise, we get booted off.
			buffer[1] = 'O';
			send(sock, buffer, strlen(buffer), 0);
		}
		// System commands should have a colon.
		if (buffer[0] != ':')
		{
			continue;
		}

		// Only join once we have a 001. This indicates it's safe.
		if (std::strncmp(std::strchr(buffer, ' ') + 1, "001", 3))
		{
			std::sprintf(buffer, "JOIN %s\r\n", channel);
			send(sock, buffer, strlen(buffer), 0);
			continue;
		}
		if (std::strncmp(std::strchr(buffer, ' ') + 1, "366", 3))
		{
			if (std::strncmp(repoSuccess, "success", 7) == 0)
			{
				std::sprintf(buffer, "PRIVMSG %s :AppVeyor build report for fork %s, build %s, commit %s by %s: Compiles on Windows!\r\n",
					channel, repoName, repoVersion, repoHash, repoAuthor
				);
			}
			else
			{
				std::sprintf(buffer, "PRIVMSG %s :AppVeyor build report for fork %s, build %s, commit %s by %s: Did not compile on Windows!\r\n",
					channel, repoName, repoVersion, repoHash, repoAuthor
				);
			}
			send(sock, buffer, strlen(buffer), 0);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

			std::sprintf(buffer, "PRIVMSG %s :Build Log: %s\r\n", channel, repoBuildUrl);
			send(sock, buffer, strlen(buffer), 0);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

			std::sprintf(buffer, "QUIT Bye bye now.\r\n");
			send(sock, buffer, strlen(buffer), 0);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}
