/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MessageSendHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AliceInNov <alice@alicenov.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:09:25 by AliceInNov        #+#    #+#             */
/*   Updated: 2024/06/15 16:14:07 by AliceInNov       ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#include "MessageSendHandler.hpp"
#include <fcntl.h>
#include <string.h>
#include <string>
#include "../Includes/utils.hpp"

MessageSendHandler::MessageSendHandler(void)
{
	//pipe init
	{
		int retval;
		retval = pipe(_pipeFD);
		if (retval == -1)
			throw std::string("MessageSendHandler(), pipe()");

		// 파이프의 최대 버퍼 크기 설정
		// 버퍼 크기 미만의 데이터가 움직일시 아토믹 오퍼레이션, 쓰레드 동기화 가능.
		int new_size = 1048576;	   // 1MB
		retval = fcntl(_pipeFD[PIPE_OUT], F_SETPIPE_SZ, new_size);
		if (retval == -1)
			throw std::string("MessageSendHandler(), fcntl()");

		// 현재 파이프의 최대 버퍼 크기 확인
		int current_size = fcntl(_pipeFD[PIPE_OUT], F_GETPIPE_SZ);
		if (current_size == -1)
			throw std::string("MessageSendHandler(), fcntl()");
	}

	//epoll init
	{
		if ((_epollFD = epoll_create(MAX_USER)) == -1)
			throw std::string("ERROR  : MessageSendHandler(), epoll_create()");

		_epEvent.events = EPOLLIN;		// 이벤트 들어오면 알림
		_epEvent.data.fd = _epollFD;	// fd 설정

		//listenfd의 상태변화를 감시 대상에 등록
		if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, _pipeFD[PIPE_IN], &_epEvent) == -1)
			throw std::string("ERROR  : MessageSendHandler(), epoll_ctl()");
	}

	_loopThread = new std::thread(&MessageSendHandler::SendHandleLoop, this);
}

MessageSendHandler::~MessageSendHandler(void)
{
	_loopThread->join();
	delete _loopThread;
	close(_epollFD);
	close(_pipeFD[PIPE_IN]);
	close(_pipeFD[PIPE_OUT]);
}

void MessageSendHandler::SendMessage(const t_Message& msg)
{
	char buffer[sizeof(t_Message)];
	Serialize(msg, buffer);
	write(_pipeFD[PIPE_OUT], buffer, sizeof(t_Message));
}

void MessageSendHandler::Serialize(const t_Message& message, char* buffer)
{
	memcpy(buffer, &message.fd, sizeof(message.fd));
	memcpy(buffer + sizeof(message.fd), message.msg, sizeof(message.msg));
}

void MessageSendHandler::Deserialize(const char* buffer, t_Message& message)
{
	memcpy(&message.fd, buffer, sizeof(message.fd));
	memcpy(message.msg, buffer + sizeof(message.fd), sizeof(message.msg));
}

void MessageSendHandler::SendHandleLoop()
{
	try {
		t_epollEvent epEvents[EP_EVENT_SIZE];
		int eventCnt;

		while (1) {
			eventCnt = epoll_wait(_epollFD, epEvents, EP_EVENT_SIZE, -1);

			if (eventCnt == -1)
				throw std::string("ERROR  : SendHandleLoop(), epoll_wait()");

			for (int evIdx = 0; evIdx < eventCnt; evIdx++) {
				t_Message msg;
				char buffer[sizeof(t_Message)];
				int retval = 0;

				read(_pipeFD[PIPE_IN], buffer, sizeof(t_Message));
				Deserialize(buffer, msg);

				retval = write(msg.fd, msg.msg, MSG_TOTAL_SIZE);
				if (retval < MSG_TOTAL_SIZE) {}
			}
		}
	}
	catch (const std::string& e) {
		PrintError(e);
	}
	catch (const std::exception& e) {
		PrintError("Caught std::exception: " + std::string(e.what()));
	}
	catch (...) {
		PrintError("Caught unknown exception");
	}
}