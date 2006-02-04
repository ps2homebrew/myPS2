/*
=================================================================
Copyright (C) 2005-2006 Torben "ntba2" Koenke

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA  02110-1301, USA.
=================================================================
*/

/*
# myPS2
# Author: ntba2
# Date: 12-11-2005
#
# File: net.h
#
*/

#ifndef _NET_H
#define _NET_H

#include <ps2ip.h>

#define USER_AGENT			"myPS2/1.1"

#define SHOUTCAST_HOST		"www.shoutcast.com"
#define SHOUTCAST_FILE		"/sbin/xmllister.phtml?limit=500&service=aol&no_compress=1"
#define SHOUTCAST_PORT		80
#define SHOUTCAST_STATIONS	500

#define HTTP_MAX_FILESIZE	1024 * 1024 * 10

int HttpDownload( const char *pHost, int nPort, const char *pFile, char **pBuffer, int *pSize, int (*callback)(int, int) );
int TokenizeURL( const char *pUrl, char *pHost, char *pFileName, int *pPort );

#define HTTP_ERROR_BASE 55

enum {
	HTTP_ERROR_SOCKET	= HTTP_ERROR_BASE,
	HTTP_ERROR_RESOLVE,
	HTTP_ERROR_CONNECT,
	HTTP_ERROR_SEND,
	HTTP_ERROR_RECV,
	HTTP_ERROR_BADREQ,
	HTTP_ERROR_SIZE,
	HTTP_ERROR_MEMORY,
	
	HTTP_NUM_ERRORS
};

//
// copied from /iop/tcpip/

u32 inet_addr( const char *cp );
int inet_aton( const char *cp, struct in_addr *addr );

#if !defined(INADDR_NONE)
#define INADDR_NONE		((u32) 0xffffffff)  /* 255.255.255.255 */
#endif

#ifndef isascii
#define in_range(c, lo, up)	((u8)c >= lo && (u8)c <= up)
#define isascii(c)			in_range(c, 0x20, 0x7f)
#define isdigit(c)			in_range(c, '0', '9')
#define isxdigit(c)			(isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)			in_range(c, 'a', 'z')
#define isspace(c)			(c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#endif

//
// ps2sdksrc/iop/tcpip/dns/

typedef struct t_dns_cache
{
	struct t_dns_cache *next;
	char *hostName;
	struct in_addr ipAddr;
	int rv;
} t_dnsCache;

int dnsInit( const char *dns_addr ); // replaces _start
void dnsResolveInit();
int	dnsPrepareQueryPacket(u8 *packetBuf, char *hostName); // returns size of packet
int	dnsParseResponsePacket(u8 *packetBuf, int pktSize, struct in_addr *ip);
void dnsCacheAdd(t_dnsCache *entry);
t_dnsCache *dnsCacheFind(char *hostName);
int	getLabelLength(u8 *buffer);
int	getResourceRecordLength(u8 *buffer);
int gethostbyname(char *name, struct in_addr *ip);

#define DNS_ERROR_HOST_NOT_FOUND	-1
#define	DNS_ERROR_CONNECT			-2
#define	DNS_ERROR_PARSE				-3

typedef struct
{
	u16 id;
	u16 flags;
	u16 QDCOUNT;
	u16 ANCOUNT;
	u16 NSCOUNT;
	u16 ARCOUNT;
} t_dnsMessageHeader;

typedef struct
{
	u16 type;
	u16 class;
	u32 ttl;
	u16 rdlength;
	u8 rdata[4]; // always 4 for A type RR's
} t_dnsResourceRecordHostAddr __attribute__ ((packed));

#define RCODE_noError			0
#define RCODE_formatError		1
#define	RCODE_serverFailure		2
#define	RCODE_nameError			3
#define	RCODE_notImplemented	4
#define RCODE_refused			5

#define	TYPE_A					1	// host address
#define	TYPE_PTR				12	// domain name pointer

#define CLASS_IN				1	// internet class

#define	NAMESERVER_PORT			53

#endif
