/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AliceInNov <alice@alicenov.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 20:31:14 by AliceInNov        #+#    #+#             */
/*   Updated: 2024/06/23 18:50:32 by AliceInNov       ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#ifndef utils_HPP
#define utils_HPP

#include <iostream>
#include <mutex>
#include "data.hpp"
#include "string.h"

// Asynchronous output
static std::mutex coutLock;
struct coutAsync
{
	std::unique_lock<std::mutex> lk;
	coutAsync() : lk(std::unique_lock<std::mutex>(coutLock)) {}

	template <typename T>
	coutAsync& operator<<(const T& _t)
	{
		std::cout << _t;
		return *this;
	}

	coutAsync& operator<<(std::ostream& (*fp)(std::ostream&))
	{
		std::cout << fp;
		return *this;
	}
};

static std::mutex cerrLock;
struct cerrAsync
{
	std::unique_lock<std::mutex> lk;
	cerrAsync() : lk(std::unique_lock<std::mutex>(coutLock)) {}

	template <typename T>
	cerrAsync& operator<<(const T& _t)
	{
		std::cerr << _t;
		return *this;
	}

	cerrAsync& operator<<(std::ostream& (*fp)(std::ostream&))
	{
		std::cerr << fp;
		return *this;
	}
};

static void PrintServerState(std::string msg)
{
	coutAsync() << C_STATE << msg << C_NOMAL << std::endl << std::endl;
}

static void PrintNotification(std::string msg)
{
	coutAsync() << C_NOTIY << msg << C_NOMAL << std::endl << std::endl;
}

static void PrintText(std::string msg)
{
	coutAsync() << C_NOMAL << msg << C_NOMAL << std::endl << std::endl;
}

static void PrintError(std::string msg)
{
	cerrAsync() << C_ERROR << msg << C_NOMAL << std::endl << std::endl;
}

inline std::string pTime()
{
	std::time_t now = std::time(0);
	std::tm* now_tm = std::gmtime(&now);
	char buf[42];
	std::strftime(buf, 42, "%Y/%m/%d %X", now_tm);
	return buf;
}

inline t_Message CreateMessage(int fd, int msgType, t_Arg msgArg)
{
	t_Message msg;

	msg.fd = fd;
	msgType = htonl(msgType);
	memcpy(msg.msg, &msgType, MSG_TYPE_SIZE);
	memcpy(&msg.msg[MSG_TYPE_SIZE], msgArg.data, MSG_ARG_SIZE);
	return msg;
}

inline std::string MessageTypeName(int messageType)
{
	switch (messageType) {
		case MT_NOTING:
			return std::string("MT_NOTING");
		case MT_MESSEGE:
			return std::string("MT_MESSEGE");
		case MT_SET_NAME:
			return std::string("MT_SET_NAME");
		case MT_MATCHQ_JOIN:
			return std::string("MT_MATCHQ_JOIN");
		case MT_ACTIVE_USER:
			return std::string("MT_ACTIVE_USER");
		case MT_GAME_RESULT:
			return std::string("MT_GAME_RESULT");
		case MT_USER_ACTION:
			return std::string("MT_USER_ACTION");
		case MT_ROOM_CREATED:
			return std::string("MT_ROOM_CREATED");
	}
	return std::string("MT_UNKOWN");
}

#endif