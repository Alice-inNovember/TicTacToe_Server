#ifndef data_HPP
#define data_HPP

//색상
#define C_NOMAL "\033[0m"
#define C_ERROR "\033[1;31m"
#define C_STATE "\033[1;32m"
#define C_NOTIY "\033[1;34m"

//서버 설정
#define MAX_USER 512		// 최대 접속자 수
#define MAX_USER_Q 32		// 최대 접속 대기 큐
#define EP_INIT_SIZE 32		// epoll_create() 초기 크기
#define EP_EVENT_SIZE 32	// epoll_wait() maxevents

//전송 데이터 사이즈
#define MSG_TYPE_SIZE 4
#define MSG_ARG_SIZE 128
#define MSG_TOTAL_SIZE 132

enum e_MessageType
{
	MT_NOTING = 0,
	MT_MATCHQ_JOIN,
	MT_SET_NAME,
	MT_MESSEGE,
	MT_ACTIVE_USER,
	MT_ROOM_CREATED,
	MT_GAME_RESULT,
	MT_USER_ACTION,
};

#pragma pack(push)
#pragma pack(1)
typedef struct s_Arg
{
	char data[MSG_ARG_SIZE];
} t_Arg;
#pragma pack(pop)

typedef struct s_Message
{
	int fd;
	char msg[MSG_TOTAL_SIZE];
} t_Message;

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>

//구조체 typedef
typedef struct sockaddr_in t_sockAddrIn;
typedef struct sockaddr t_sockAddr;
typedef struct epoll_event t_epollEvent;

#endif