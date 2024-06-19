/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AliceInNov <alice@alicenov.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 20:48:04 by AliceInNov        #+#    #+#             */
/*   Updated: 2024/06/15 16:30:28 by AliceInNov       ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <unistd.h>
#include <string>
#include "../Includes/utils.hpp"

Server::Server(int port)
{
	_portNbr = port;

	InitServerSocket();
	InitEpoll();
}

Server::~Server(void)
{
	close(_epollFD);
	close(_serverFD);
}

void Server::InitServerSocket()
{
	//open
	if ((_serverFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		throw std::string("ERROR : InitServerSocket(), socket()");

	//init
	t_sockAddrIn addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_portNbr);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//socketOpt
	int bEnable = 1;
	setsockopt(_serverFD, SOL_SOCKET, SO_KEEPALIVE, &bEnable, sizeof(int));
	setsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &bEnable, sizeof(int));
	setsockopt(_serverFD, IPPROTO_TCP, TCP_NODELAY, &bEnable, sizeof(int));

	//bind
	if (bind(_serverFD, (t_sockAddr*)&addr, sizeof(t_sockAddrIn)) == -1)
		throw std::string("ERROR  : InitServerSocket(), bind()");

	//listen
	if (listen(_serverFD, MAX_USER_Q) == -1)
		throw std::string("ERROR  : InitServerSocket(), listen()");

	PrintServerState("SERVER : Init Server Socket");
}

void Server::InitEpoll()
{
	t_epollEvent epEvent;

	if ((_epollFD = epoll_create(MAX_USER)) == -1)
		throw std::string("ERROR  : InitEpoll(), epoll_create()");

	epEvent.events = EPOLLIN;		// 이벤트 들어오면 알림
	epEvent.data.fd = _serverFD;	// fd 설정
	//userFd의 상태변화를 감시 대상에 등록
	epoll_ctl(_epollFD, EPOLL_CTL_ADD, _serverFD, &epEvent);
	PrintServerState("SERVER : Init Server Epoll");
}