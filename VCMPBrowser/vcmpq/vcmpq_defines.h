#pragma once

#define VCMPQ_STACK_ALLOCATION_SIZE 128
#define VCMPQ_MAX_STRING_LEN 250

typedef enum __VCMPQ_HEADER_TYPE
{
	VCMPQ_HEADERTYPE_PING = 'p',
	VCMPQ_HEADERTYPE_QUERY = 'i',
	VCMPQ_HEADERTYPE_PLAYERS = 'c',

	VCMPQ_HEADERTYPE_ERROR = 0x7F,
	// Answer should be set via bitflags rather than directly
	VCMPQ_HEADERTYPE_ANSWER = 0x80
} vcmpq_headertype;

typedef struct
{
	unsigned long			ulIP;
	unsigned short			usPort;
	vcmpq_headertype		eRequestType;
} vcmpq_request;

typedef struct
{
	unsigned int			uiSocket;
	void*					pSockAddr;
	int						iSockAddrLen;

	const unsigned char*	szRcvBufStart;
	const unsigned char*	szRcvBufEnd;

	unsigned char*			szSndBufMin;
	unsigned char*			szSndBufStart;
	unsigned char*			szSndBufEnd;
	unsigned char*			szSndBufMax;

	struct
	{
		// Header Type
		vcmpq_headertype		eType;

		const unsigned char*	szBodyPos;
	} Head;
} vcmpq_parse_info;
