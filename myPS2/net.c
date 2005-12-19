/*
=================================================================
Copyright (C) 2005 Torben "ntba2" Koenke

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
# File: net.c
#
*/

#include <tamtypes.h>
#include <sysconf.h>	// ReadBufLine
#include <string.h>
#include <stdio.h>
#include <ps2ip.h>
#include <net.h>

//
// HttpDownload -  Attempts to download a file off an HTTP server.
//
//				   pHost	: Name or address of HTTP server
//				   nPort	: HTTP server port
//				   pFile	: Path of remote file
//				   ppBuffer : Address of a pointer that will point to the
//							  memory allocated by the function after completion.
//				   pSize	: Size in bytes of allocated memory for pBuffer
//				   pCallback: Optional pointer to a callback function 
//
//				   Returns 1 on sucess, otherwise error number.
//

int HttpDownload( const char *pHost, int nPort, const char *pFile, char **pBuffer, int *pSize, int (*callback)(int, int) )
{
	int					s;
	struct sockaddr_in	addr;
	int					nRet, nRecv, nReqOK, nContentSize, nBreak, nRead;
	int					nTotal;

	char				strGet[256];
	char				strBuf[4096];
	char				strLine[1024];
	char				*pStr, *pLine;

	s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( s < 0 ) {
#ifdef _DEBUG
		printf("HttpDownload: socket() failed!\n");
#endif
		return HTTP_ERROR_SOCKET;
	}

	memset( &addr, 0, sizeof(addr) );

	addr.sin_family			= AF_INET;
	addr.sin_port			= htons(nPort);
	addr.sin_addr.s_addr	= inet_addr( pHost );

	// not an ip address, try to resolve the hostname
	if( addr.sin_addr.s_addr == INADDR_NONE ) {
		if( gethostbyname( (char*) pHost, &addr.sin_addr ) != 0 ) {
#ifdef _DEBUG
			printf("HttpDownload: Could not resolve hostname : %s\n", pHost);
#endif
			return HTTP_ERROR_RESOLVE;
		}
	}

	nRet = connect( s, (struct sockaddr*)&addr, sizeof(addr) );
	if( nRet < 0 ) {
#ifdef _DEBUG
		printf("HttpDownload: connect() failed!\n");
#endif
		disconnect(s);
		return HTTP_ERROR_CONNECT;
	}

	// prepare GET request
	sprintf( strGet,"GET %s HTTP/1.0\r\n\r\n", pFile );

	nRet = send( s, strGet, strlen(strGet), 0 );
	if( nRet < 0 ) {
#ifdef _DEBUG
		printf("HttpDownload: send() failed!\n");
#endif
		disconnect(s);
		return HTTP_ERROR_SEND;
	}

	// read HTTP header response
	nRecv			= 0;
	nReqOK			= 0;
	nContentSize	= 0;
	nBreak			= 0;

	// read header in 1024 chunks
	while(1) {
		nRet = recv( s, strBuf + nRecv, sizeof(strBuf) - 1 - nRecv, 0 );

		if( nRet < 0 || nRet == 0 ) {
#ifdef _DEBUG
			printf("HttpDownload: connection was reset!\n");
#endif
			disconnect(s);
			return HTTP_ERROR_RECV;
		}

		strBuf[nRet] = 0;
		pStr = strBuf;

		while(1) {
			nRead = ReadBufLine( &pStr, strLine );

			if( nRead == 0 ) {
				memcpy( strBuf, pStr, pStr - strBuf );
				nRecv = pStr - strBuf;
				break;
			}

			// end of HTTP header
			if( strLine[0] == '\r' && strLine[1] == '\n' ) {

				// remaining bytes are of file already
				nRecv = nRet - (pStr - strBuf);
				memcpy( strBuf, pStr, nRecv );

				// break out of loop
				nBreak = 1;
				break;
			}

			// check for HTTP error code...make this properly?
			if( strstr( strLine, "HTTP/1." ) ) {
				if( strstr( strLine, "200 OK" ) )
					nReqOK = 1;
			}
			else if( strstr( strLine, "Content-Length:" ) ) {

				// this is the file size in bytes
				pLine = strLine;
				pLine += strlen("Content-Length:");

				// skip whitespaces
				while( *pLine == ' ' || *pLine == '\t' )
					pLine++;

				nContentSize = atoi(pLine);
			}
		}

		// HTTP header end reached
		if( nBreak )
			break;
	}

	// file doesn't exist or some other HTTP error occured
	if( !nReqOK ) {
#ifdef _DEBUG
		printf("HttpDownload: nReqOK != 200 OK\n");
#endif
		disconnect(s);
		return HTTP_ERROR_BADREQ;
	}

	if( !nContentSize ) {
#ifdef _DEBUG
		printf("HttpDownload: (Notice) HTTP header did not contain Content-Length field!\n");
#endif
	}

	// check if it's not too big
	if( nContentSize > HTTP_MAX_FILESIZE ) {
#ifdef _DEBUG
		printf("HttpDownload: file size is too big (%i)!\n", nContentSize);
#endif
		disconnect(s);
		return HTTP_ERROR_SIZE;
	}

	*pBuffer	= NULL;
	nTotal		= 0;

	// nRecv is the number of bytes already belonging to the file that
	// were in the last strBuf chunk of the HTTP header.
	if( nRecv > 0 ) {
		*pBuffer = (char*) realloc( *pBuffer, nRecv );
		if( !(*pBuffer) ) {
#ifdef _DEBUG
			printf("HttpDownload: realloc failed for pBuffer!\n");
#endif
			disconnect(s);
			return HTTP_ERROR_MEMORY;
		}
		memcpy( *pBuffer, strBuf, nRecv );
		nTotal = nRecv;
	}

	// can start receiving the file now
	while( (nRet = recv( s, strBuf, sizeof(strBuf), 0 )) ) {

		if( nRet < 0 ) {
			printf("HttpDownload: recv() failed\n");

			free(*pBuffer);
			disconnect(s);
			return HTTP_ERROR_RECV;
		}

		// don't let the file get too big
		if( (nTotal + nRet) > HTTP_MAX_FILESIZE ) {
#ifdef _DEBUG
			printf("HttpDownload: file size exceeded HTTP_MAX_FILESIZE!\n");
#endif
			free(*pBuffer);
			disconnect(s);
			return HTTP_ERROR_SIZE;
		}

		*pBuffer = (char*) realloc( *pBuffer, nTotal + nRet );
		if( !(*pBuffer) ) {
#ifdef _DEBUG
			printf("HttpDownload: realloc failed for pBuffer!\n");
#endif
			disconnect(s);
			return HTTP_ERROR_MEMORY;
		}
		
		memcpy( (*pBuffer) + nTotal, strBuf, nRet );

		// total count of bytes received
		nTotal += nRet;

		// if a callback was specified call it
		if( callback )
			callback( nTotal, nContentSize );
	}

#ifdef _DEBUG
	if( (nContentSize > 0) && (nTotal == nContentSize) )
		printf("HttpDownload : This is good\n");
#endif

	*pSize = nTotal;

	disconnect(s);
	return 1;
}

const char *HttpErrors[] = 
{
	"Failed creating socket",
	"Could not resolve hostname",
	"Could not connect to server",
	"Error while sending data",
	"Error while receiving data",
	"Requested file is too big",
	"Memory allocation failed",
};

const char *GetHttpError( int errno )
{
	if( errno < HTTP_ERROR_BASE || errno >= HTTP_NUM_ERRORS )
		return NULL;

	return HttpErrors[errno - HTTP_ERROR_BASE];
}

//
// TokenizeURL - Extracts hostname, port and file from an URL in the
//				 form of http://hostname:port/file
//

int TokenizeURL( const char *pUrl, char *pHost, char *pFileName, int *pPort )
{
	char	*pStart, *pEnd;
	char	strTmp[128];
	int		nLen;

	*pPort = 80;

	if( (pStart = strchr( pUrl, ':' )) == NULL ) {
#ifdef _DEBUG
		printf("TokenizeURL: Invalid URL.\n");
#endif
		return 0;
	}

	// skip ':'
	pStart++;

	if( *(pStart++) != '/' || *(pStart++) != '/' ) {
#ifdef _DEBUG
		printf("TokenizeURL: Invalid URL.\n");
#endif
		return 0;
	}

	nLen = 0;

	// extract host
	while( *pStart ) {
		if( *pStart == ':' || *pStart == '/' )
			break;

		pHost[ nLen++ ]	= *pStart++;
		pHost[ nLen ]	= 0;
	}

	// parse custom port
	if( (*pStart) && (*pStart == ':') ) {

		// skip ':'
		pStart++;

		if( (pEnd = strchr( pStart, '/' )) != NULL ) {
			nLen = pEnd - pStart;
			strncpy( strTmp, pStart, nLen );

			strTmp[nLen]	= 0;
			pStart			= pEnd;
		}
		else {
			strcpy( strTmp, pStart );
		}

		*pPort = atoi(strTmp);
	}

	// parse filename
	if( (*pStart) && (*pStart == '/') )
		strcpy( pFileName, pStart );
	else
		strcpy( pFileName, "/" );

	return 1;
}

////////////////////////////////////////////////////////////////////////////////////
// copied from ps2sdksrc/iop/tcpip/dns/

t_dnsCache			cacheHead;			// fixme: thread
struct sockaddr_in	nameServerAddr;
u8					packetBuffer[2048]; // should be plenty!
int					currentId = 1;

//
// dnsInit - Needs to be called with address of DNS server
//			 before making any calls to gethostbyname.
//

int dnsInit( const char *dns_addr )
{
	u32 a;

	cacheHead.next			= NULL;
	cacheHead.hostName		= "";
	cacheHead.ipAddr.s_addr	= 0;
	cacheHead.rv			= 0;

	memset( &nameServerAddr, 0, sizeof(struct sockaddr_in) );

	nameServerAddr.sin_family		= AF_INET;
	nameServerAddr.sin_port			= htons(NAMESERVER_PORT);

	a = inet_addr( dns_addr );
	if( a == INADDR_NONE ) {
#ifdef _DEBUG
		printf("dnsInit : %s is not a valid address!\n", dns_addr);
#endif
		return 0;
	}

	nameServerAddr.sin_addr.s_addr = a;
	return 1;
}

void dnsSetNameserverAddress(struct in_addr *addr)
{
	memcpy(&nameServerAddr.sin_addr, addr, sizeof(struct in_addr));
}

int gethostbyname(char *name, struct in_addr *ip)
{
	t_dnsCache *cached = dnsCacheFind(name);
	int pktSize, sockFd, pos = 0, rv;
	t_dnsCache *newNode;

	memset(ip, 0, sizeof(struct in_addr));

	// Is the host already in the cache?
	if(cached != NULL)
	{
#ifdef DEBUG
		printf("host (%s) found in cache!\n", name);
#endif
		ip->s_addr = cached->ipAddr.s_addr;
		return cached->rv;
	}

	// prepare the query packet
	pktSize = dnsPrepareQueryPacket(&packetBuffer[2], name);

	// connect to dns server via TCP (UDP is standard, but TCP is easier :))
	sockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockFd < 0)
		return DNS_ERROR_CONNECT;

	if(connect(sockFd, (struct sockaddr *)&nameServerAddr, sizeof(struct sockaddr)) < 0)
	{
		disconnect(sockFd);
		return DNS_ERROR_CONNECT;
	}

#ifdef DEBUG
	printf("connected to nameserver!\n");
#endif

	// Send our packet - first send packet size
	*(u16 *)packetBuffer = htons(pktSize);
	pktSize += 2;

	while(pktSize)
	{
		int sent = send(sockFd, &packetBuffer[pos], pktSize, 0);
		
		if(sent < 0)
		{
			disconnect(sockFd);
			return DNS_ERROR_CONNECT;
		}

		pos += sent;
		pktSize -= sent;
	}
	
#ifdef DEBUG
	printf("packet sent!\n");
#endif

	rv = recv(sockFd, &packetBuffer[pktSize], sizeof(packetBuffer), 0);
	if(rv < 0)
	{
			disconnect(sockFd);
			return DNS_ERROR_CONNECT;
	}

	pktSize = ntohs(*(u16 *)packetBuffer);

	// should do proper loop to get all data if the entire packet didnt make it
	// through the first time.. maybe later :)
	disconnect(sockFd);
	if(pktSize != (rv - 2))
			return DNS_ERROR_CONNECT;

#ifdef DEBUG
	printf("received response! len = %d\n", pktSize);
#endif

	rv = dnsParseResponsePacket(&packetBuffer[2], pktSize, ip);
#ifdef DEBUG
	{
		u8 *ptr = (u8 *)ip;
		printf("resolved ip: %d.%d.%d.%d\n", ptr[0], ptr[1], ptr[2], ptr[3]);
	}
#endif

	newNode = (t_dnsCache *)malloc(sizeof(t_dnsCache));
	if(!newNode)
		return rv;

	newNode->ipAddr.s_addr = ip->s_addr;
	newNode->rv = rv;
	newNode->hostName = malloc(strlen(name) + 1);
	if(!newNode->hostName)
	{
		free(newNode);
		return rv;
	}

	strcpy(newNode->hostName, name);
	dnsCacheAdd(newNode);

	return rv;
}

int dnsPrepareQueryPacket(u8 *packetBuf, char *hostName)
{
	t_dnsMessageHeader *hdr = (t_dnsMessageHeader *)packetBuf;
	int currLen, pktSize = 0, left = strlen(hostName), i;

	memset(hdr, 0, sizeof(t_dnsMessageHeader));
	hdr->id = currentId++;
	hdr->flags = 0x100; // standard query, recursion desired
	hdr->QDCOUNT = 1;

	// convert header to network byte order
	for(i = 0; i < sizeof(t_dnsMessageHeader); i += 2)
		*(u16 *)&packetBuf[i] = htons(*(u16 *)&packetBuf[i]);

	pktSize += sizeof(t_dnsMessageHeader);
	packetBuf += sizeof(t_dnsMessageHeader);

	// Copy over QNAME
	while(left > 0)
	{
		for(currLen = 0; (hostName[currLen] != '.') && (hostName[currLen] != '\0'); currLen++);
		*(packetBuf++) = currLen;
		memcpy(packetBuf, hostName, currLen);

		hostName += currLen + 1;
		packetBuf += currLen;
		pktSize += currLen + 1;
		left -= currLen + 1;
	}

	// terminate QNAME
	*(packetBuf++) = '\0';
	pktSize++;

	// Set type = TYPE_A, class = CLASS_IN (the cheat way :P)
	*(packetBuf++) = 0;
	*(packetBuf++) = 1;
	*(packetBuf++) = 0;
	*(packetBuf++) = 1;
	pktSize += 4;

	return pktSize;
}

int getLabelLength(u8 *buffer)
{
	int len = 0;

	// if "compressed" label, simply skip 2 bytes
	if(buffer[len] == 0xC0)
		len += 2;
	else
	{
		// otherwise skip past null terminated string
		while(buffer[len] != '\0')
			len++;

		len++;
	}

	return len;
}

int getResourceRecordLength(u8 *buffer)
{
	int len = 0;
	u32 rdLen;

	len += getLabelLength(buffer);
	// skip to RDLENGTH
	len += 8;
	memcpy(&rdLen, &buffer[len], 2);
	rdLen = ntohs(rdLen);
	len += 2 + rdLen;
		
	return len;
}

int dnsParseResponsePacket(u8 *packetBuf, int pktSize, struct in_addr *ip)
{
	t_dnsMessageHeader *hdr = (t_dnsMessageHeader *)packetBuf;
	t_dnsResourceRecordHostAddr rrBody;
	u8 retCode;
	int pos, i;

	// convert header to host byte order and make sure everything is in order
	for(i = 0; i < sizeof(t_dnsMessageHeader); i += 2)
		*(u16 *)&packetBuf[i] = ntohs(*(u16 *)&packetBuf[i]);

	// make sure this is a response
	if(!(hdr->flags & 0x8000))
		return DNS_ERROR_PARSE;

	// make sure message isnt truncated
	if(hdr->flags & 0x200)
		return DNS_ERROR_PARSE;

	retCode = hdr->flags & 0x0F;
	if(retCode == RCODE_nameError)
		return DNS_ERROR_HOST_NOT_FOUND;
	else if(retCode != RCODE_noError)
		return DNS_ERROR_PARSE;

	// skip past original query to resource records
	pos = sizeof(t_dnsMessageHeader);
	while(packetBuf[pos] != '\0')
		pos++;
	pos += 5;

	// go through all resource records looking for host address type
	while(pos < pktSize)
	{
		memcpy(&rrBody, &packetBuf[pos + getLabelLength(&packetBuf[pos])], sizeof(t_dnsResourceRecordHostAddr));

		// keep searching till we find the right resource record (if any)
		if(	(ntohs(rrBody.type) != TYPE_A) ||
			(ntohs(rrBody.class) != CLASS_IN) ||
			(ntohs(rrBody.rdlength) != 4))
		{
			pos += getResourceRecordLength(&packetBuf[pos]);
			continue;
		}

		memcpy(ip, rrBody.rdata, 4);
		return 0;
	}

	return DNS_ERROR_PARSE;
}

void dnsCacheAdd(t_dnsCache *entry)
{
	t_dnsCache *last = &cacheHead;
	
	while(last->next != NULL)
		last = last->next;

	last->next = entry;
	entry->next = NULL;
}

t_dnsCache *dnsCacheFind(char *hostName)
{
	t_dnsCache *current;

	for(current = cacheHead.next; current != NULL; current = current->next)
	{
		if(!strcasecmp(current->hostName, hostName))
			return current;
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////
// copied from ps2sdksrc/iop/tcpip/

u32 inet_addr( const char *cp )
{
	struct in_addr val;
	
	if( inet_aton(cp, &val) )
		return(val.s_addr);
	
	return (INADDR_NONE);
}

/*
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */

/* inet_aton */
int inet_aton( const char *cp, struct in_addr *addr )
{
     u32 val;
     int base, n;
     char c;
     u32 parts[4];
     u32* pp = parts;

     c = *cp;
     for (;;) {
         /*
          * Collect number up to ``.''.
          * Values are specified as for C:
          * 0x=hex, 0=octal, isdigit=decimal.
          */
         if (!isdigit(c))
             return (0);
         val = 0; base = 10;
         if (c == '0') {
             c = *++cp;
             if (c == 'x' || c == 'X')
                 base = 16, c = *++cp;
             else
                 base = 8;
         }
         for (;;) {
             if (isascii(c) && isdigit(c)) {
                 val = (val * base) + (c - '0');
                 c = *++cp;
             } else if (base == 16 && isascii(c) && isxdigit(c)) {
                 val = (val << 4) |
                     (c + 10 - (islower(c) ? 'a' : 'A'));
                 c = *++cp;
             } else
             break;
         }
         if (c == '.') {
             /*
              * Internet format:
              *  a.b.c.d
              *  a.b.c   (with c treated as 16 bits)
              *  a.b (with b treated as 24 bits)
              */
             if (pp >= parts + 3)
                 return (0);
             *pp++ = val;
             c = *++cp;
         } else
             break;
     }
     /*
      * Check for trailing characters.
      */
     if (c != '\0' && (!isascii(c) || !isspace(c)))
         return (0);
     /*
      * Concoct the address according to
      * the number of parts specified.
      */
     n = pp - parts + 1;
     switch (n) {

     case 0:
         return (0);     /* initial nondigit */

     case 1:             /* a -- 32 bits */
         break;

     case 2:             /* a.b -- 8.24 bits */
         if (val > 0xffffff)
             return (0);
         val |= parts[0] << 24;
         break;

     case 3:             /* a.b.c -- 8.8.16 bits */
         if (val > 0xffff)
             return (0);
         val |= (parts[0] << 24) | (parts[1] << 16);
         break;

     case 4:             /* a.b.c.d -- 8.8.8.8 bits */
         if (val > 0xff)
             return (0);
         val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
         break;
     }
     if (addr)
         addr->s_addr = htonl(val);
     return (1);
}
