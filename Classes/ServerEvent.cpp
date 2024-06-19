/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerEvent.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: AliceInNov <alice@alicenov.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/15 22:28:49 by AliceInNov        #+#    #+#             */
/*   Updated: 2024/06/20 04:53:43 by AliceInNov       ###   ########seoul.kr  */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include "../Includes/utils.hpp"
#include "Server.hpp"

void Server::ServerLoop()
{
	t_epollEvent epEvents[EP_EVENT_SIZE];
	int eventCnt;

	PrintServerState("SERVER : Started Successfully");
	while (1) {
		eventCnt = epoll_wait(_epollFD, epEvents, EP_EVENT_SIZE, -1);
		if (eventCnt == -1) {
			throw std::string("ERROR  : ServerLoop(), epoll_wait()");
		}

		for (int evIdx = 0; evIdx < eventCnt; evIdx++) {
			if (epEvents[evIdx].data.fd == _serverFD) {
				UserCreate();
			}
			else {
				EventHandler((t_User*)epEvents[evIdx].data.ptr);
			}
		}
	}
}

int Server::ReadMessageType(t_User* user)
{
	int messageType;
	int retval = 0;

	retval = read(user->fd, &messageType, sizeof(int));
	if (retval < (int)sizeof(int))
		throw std::string("EventHandler : User Disconnected");
	messageType = ntohl(messageType);

	return messageType;
}

t_Arg Server::ReadMessageArg(t_User* user)
{
	t_Arg messageArg;
	int retval = 0;

	retval = read(user->fd, &messageArg, MSG_ARG_SIZE);
	if (retval < MSG_ARG_SIZE)
		throw std::string("EventHandler : User Disconnected");

	return messageArg;
}

void Server::EventHandler(t_User* user)
{
	int messageType;
	t_Arg messageArg;

	try {
		messageType = ReadMessageType(user);
		messageArg = ReadMessageArg(user);
	}
	catch (std::string error) {
		UserDelete(user->userID);
		PrintNotification(std::string("EventHandler : ") + error);
		return;
	}

	switch (messageType) {
		case MT_NOTING:
			return;

		case MT_MATCHQ_JOIN:
			MatchQueueJoin(user->userID);
			return;

		case MT_SET_NAME:
			PrintText(std::string("User Name Set : ") + messageArg.data);
			messageArg.data[17] = 0;
			user->name = messageArg.data;
			return;

		case MT_MESSEGE:
			ForwardMessage(user->userID, messageType, messageArg);
			return;

		case MT_ACTIVE_USER:
			t_Arg arg;
			memset(arg.data, 0, MSG_ARG_SIZE);
			strncat(arg.data, std::to_string(_userMap.size()).c_str(), MSG_ARG_SIZE);
			SendMessage(user->userID, MT_ACTIVE_USER, arg);
			return;

		case MT_ROOM_CREATED:
			return;

		case MT_GMAE_START:
			return;

		case MT_GAME_RESULT:
			ForwardMessage(user->userID, messageType, messageArg);
			RoomDelete(user->userID);
			return;

		case MT_USER_ACTION:
			ForwardMessage(user->userID, messageType, messageArg);
			return;
	}
	PrintNotification("EventHandler ELSE : " + MessageTypeName(messageType));
}

void Server::UserCreate()
{
	int userFd;
	t_sockAddrIn addr;
	t_User* user;
	t_epollEvent epEvent;
	socklen_t clientLen;

	//user accept()
	clientLen = sizeof(t_sockAddr);
	userFd = accept(_serverFD, (t_sockAddr*)&addr, &clientLen);
	if (userFd == -1) {
		PrintError("USER : New User Error");
		return;
	}
	PrintNotification("USER : New User Connected");

	//user 초기화
	user = new t_User;
	user->fd = userFd;
	user->playerType = PLAYER_NULL;
	user->isQueued = false;
	user->name = std::string("NO NAME");
	user->roomID = uuidGenerator.getUUID().bytes();
	user->userID = uuidGenerator.getUUID().bytes();
	_userMap.insert(std::pair<std::string, t_User*>(user->userID, user));

	//epoll
	epEvent.events = EPOLLIN;	 // 이벤트 들어오면 알림
	epEvent.data.fd = userFd;	 // fd 설정
	epEvent.data.ptr = user;
	epoll_ctl(_epollFD, EPOLL_CTL_ADD, userFd, &epEvent);
	PrintNotification("USER : Connected Success");
}

void Server::UserDelete(std::string userID)
{
	PrintText("USER : UserDelete");
	t_User* user = _userMap[userID];

	//게임 도중 나갈시 Room 의 처리
	if (_roomMap.find(user->roomID) != _roomMap.end()) {
		t_Room* room = _roomMap[user->roomID];
		t_User* otherUser = _userMap[room->userID[!user->playerType]];

		messageSendHandler.SendMessage(
			CreateMessage(otherUser->fd, MT_GAME_RESULT, {"Enemy escaped"}));

		RoomDelete(user->roomID);
	}
	//유저 삭제
	epoll_ctl(_epollFD, EPOLL_CTL_DEL, user->fd, NULL);
	_userMap.erase(user->userID);
	close(user->fd);
	delete user;
}

void Server::MatchQueueJoin(std::string userID)
{
	PrintText("USER : MatchQueueJoin");
	if (_userMap.find(userID) == _userMap.end() || _userMap[userID]->isQueued) {
		return;
	}

	_matchList.push_back(userID);
	_userMap[userID]->isQueued = true;

	if (_matchList.size() >= 2) {
		std::string tempID_01 = _matchList.front();
		if (_userMap.find(tempID_01) == _userMap.end()) {
			_matchList.pop_front();
			return;
		}
		_matchList.pop_front();

		std::string tempID_02 = _matchList.front();
		if (_userMap.find(tempID_02) == _userMap.end()) {
			_matchList.pop_front();
			_matchList.push_front(tempID_01);
			return;
		}
		_matchList.pop_front();
		RoomCreate(tempID_01, tempID_02);
	}
}

void Server::RoomCreate(std::string o_userID, std::string x_userID)
{
	PrintText("USER : RoomCreate");
	t_Room* room = new t_Room;
	std::string roomID = uuidGenerator.getUUID().bytes();

	room->userID[PLAYER_O] = o_userID;
	_userMap[o_userID]->playerType = PLAYER_O;
	_userMap[o_userID]->roomID = roomID;

	room->userID[PLAYER_X] = x_userID;
	_userMap[x_userID]->playerType = PLAYER_X;
	_userMap[x_userID]->roomID = roomID;

	_roomMap.insert(std::pair<std::string, t_Room*>(roomID, room));

	t_Arg argO;
	memset(argO.data, 0, MSG_ARG_SIZE);
	strcat(argO.data, "O | ");
	strcat(argO.data, _userMap[x_userID]->name.c_str());
	t_Arg argX;
	memset(argX.data, 0, MSG_ARG_SIZE);
	strcat(argX.data, "X | ");
	strcat(argX.data, _userMap[o_userID]->name.c_str());

	SendMessage(room->userID[PLAYER_O], MT_ROOM_CREATED, argO);
	SendMessage(room->userID[PLAYER_X], MT_ROOM_CREATED, argX);
}

void Server::RoomDelete(std::string roomID)
{
	PrintText("USER : RoomDelete");
	if (_roomMap.find(roomID) == _roomMap.end())
		return;
	t_Room* room = _roomMap[roomID];

	if (_userMap.find(room->userID[PLAYER_O]) != _userMap.end()) {
		t_User* player = _userMap[room->userID[PLAYER_O]];
		player->roomID = "";
		player->isQueued = false;
		player->playerType = PLAYER_NULL;
	}
	if (_userMap.find(room->userID[PLAYER_X]) != _userMap.end()) {
		t_User* player = _userMap[room->userID[PLAYER_X]];
		player->roomID = "";
		player->isQueued = false;
		player->playerType = PLAYER_NULL;
	}
	_roomMap.erase(roomID);
	delete room;
}

void Server::RoomMessage(std::string roomID, int type, t_Arg arg)
{
	if (_roomMap.find(roomID) == _roomMap.end())
		return;

	t_Room* room = _roomMap[roomID];
	SendMessage(room->userID[PLAYER_O], type, arg);
	SendMessage(room->userID[PLAYER_X], type, arg);
}

void Server::ForwardMessage(std::string senderID, int type, t_Arg arg)
{
	if (_userMap.find(senderID) == _userMap.end())
		return;
	t_User* sender = _userMap[senderID];

	if (_roomMap.find(sender->roomID) == _roomMap.end())
		return;
	t_Room* room = _roomMap[sender->roomID];

	if (_userMap.find(room->userID[!sender->playerType]) == _userMap.end())
		return;
	t_User* otherUser = _userMap[room->userID[!sender->playerType]];

	messageSendHandler.SendMessage(CreateMessage(otherUser->fd, type, arg));
}

void Server::SendMessage(std::string userID, int type, t_Arg arg)
{
	if (_userMap.find(userID) == _userMap.end()) {
		return;
	}
	messageSendHandler.SendMessage(CreateMessage(_userMap[userID]->fd, type, arg));
}
