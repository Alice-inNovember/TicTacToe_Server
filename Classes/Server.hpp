/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AliceInNov <alice@alicenov.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 20:31:10 by AliceInNov        #+#    #+#             */
/*   Updated: 2024/06/12 09:40:28 by AliceInNov       ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#ifndef Server_HPP
#define Server_HPP

#include <sys/epoll.h>

#include "../Includes/data.hpp"
#include "MessageSendHandler.hpp"

#include <list>
#include <map>
#include <string>
#include "../Includes/uuid_v4.hpp"

enum e_PlayerType
{
	PLAYER_O = 0,
	PLAYER_X,
	PLAYER_NULL
};

typedef struct s_User
{
	//INFO
	int fd;
	int playerType;
	bool isQueued;

	//ID
	std::string userID;
	std::string roomID;
	std::string name;
} t_User;

typedef struct s_Room
{
	std::string roomID;
	std::string userID[2];
} t_Room;

class Server
{
	//Util
   private:
	MessageSendHandler messageSendHandler;
	UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;

	//Net
   private:
	int _serverFD;
	int _epollFD;
	int _portNbr;

	//Game
   private:
	std::map<std::string, t_User*> _userMap;
	std::map<std::string, t_Room*> _roomMap;
	std::list<std::string> _matchList;

	//Server.cpp
   private:
	void InitServerSocket();
	void InitEpoll();

   public:
	Server(void) = delete;
	Server(int port);
	Server(const Server& src) = delete;
	Server& operator=(Server const& obj) = delete;
	virtual ~Server(void);

	//ServerEvent.cpp
   public:
	void ServerLoop();

   private:
	int ReadMessageType(t_User* user);
	t_Arg ReadMessageArg(t_User* user);
	void EventHandler(t_User* user);
	void UserCreate();
	void UserDelete(std::string userID);
	void MatchQueueJoin(std::string userID);
	void RoomCreate(std::string o_userID, std::string x_userID);
	void RoomDelete(std::string roomID);
	void RoomMessage(std::string roomID, int type, t_Arg arg);
	void ForwardMessage(std::string senderID, int type, t_Arg arg);
	void SendMessage(std::string userID, int type, t_Arg arg);
};

#endif