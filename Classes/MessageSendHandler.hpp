/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MessageSendHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AliceInNov <alice@alicenov.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 09:25:56 by AliceInNov        #+#    #+#             */
/*   Updated: 2024/06/12 14:16:42 by AliceInNov       ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#ifndef MessageSendHandler_HPP
#define MessageSendHandler_HPP

#include <unistd.h>
#include <thread>

#include "../Includes/data.hpp"

#define PIPE_IN 0
#define PIPE_OUT 1

class MessageSendHandler
{
   private:
	int _pipeFD[2];
	int _epollFD;
	t_epollEvent _epEvent;
	std::thread* _loopThread;

   private:
	void SendHandleLoop();
	void Serialize(const t_Message& message, char* buffer);
	void Deserialize(const char* buffer, t_Message& message);

   public:
	MessageSendHandler(void);
	MessageSendHandler(const MessageSendHandler& src) = delete;
	virtual ~MessageSendHandler(void);
	MessageSendHandler& operator=(MessageSendHandler const& obj) = delete;

   public:
	void SendMessage(const t_Message& msg);
};

#endif