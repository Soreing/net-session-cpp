#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <net-session/cplatforms.h>
#include <net-session/sockhdr.h>

#include <iostream>
#include <string.h>

#if defined PLATFORM_WINDOWS
	#include <winsock2.h>
#elif defined PLATFORM_UNIX
	#include <arpa/inet.h>
	#include <netdb.h>
#endif


#pragma comment(lib,"ws2_32.lib")

IPV4_HDR::IPV4_HDR() 
	: 	ip_hl(5), ip_v(4),
		ip_tos(0), ip_length(0), ip_id(0), 
		ip_frag_offset(0), ip_more_fragment(0), ip_dont_fragment(1), ip_reserved_zero(0), ip_frag_offset1(0), 
		ip_ttl(0x80), ip_protocol(0), ip_checksum(0), ip_srcaddr(0), ip_dstaddr(0) 
{ 
}

IPV4_HDR& IPV4_HDR::setID(u_short id)
{	
	ip_id = ntohs(id); 
	return *this; 
}

IPV4_HDR& IPV4_HDR::setProtocol(u_char ipproto)
{ 
	ip_protocol = ipproto; 
	return *this; 
}

IPV4_HDR& IPV4_HDR::setAddress(u_long dst, u_long src)
{
	ip_dstaddr = dst;
	ip_srcaddr = src;
	return *this;
}

IPV4_HDR& IPV4_HDR::setAddress(const char* dst, const char *src)
{
	hostent* dsthost = gethostbyname(dst);
	ip_dstaddr = inet_addr(dst);

	hostent* srchost = gethostbyname(src);
	ip_srcaddr = inet_addr(src);

	return *this;
}

IPV4_HDR& IPV4_HDR::checksum(u_short len)
{
	ip_length = htons(len);
	unsigned short* ptr = (unsigned short*)this;
	long sum = 0, bytes = sizeof(IPV4_HDR);

	while (bytes>1)
	{	sum += *ptr++;
		bytes -= 2;
	}
	if (bytes == 1)
	{	sum += *(unsigned char*)ptr;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum = sum + (sum >> 16);
	ip_checksum = htons((short)~sum);

	return *this;
}

IPV4_HDR& IPV4_HDR::load(char* buffer)
{
	memcpy(buffer, (char*)this, sizeof(IPV4_HDR));
	return *this;
}

IPV4_HDR& IPV4_HDR::print()
{
	for (unsigned char h = 0, l = 0, i = 0; i < sizeof(IPV4_HDR); i++)
	{
		h = (((char*)this)[i] & 0xF0) >> 4;
		l = (((char*)this)[i] & 0x0F) >> 0;
		std::cout << (char)(h < 10 ? h + '0' : h + 'A' - 10) << (char)(l < 10 ? l + '0' : l + 'A' - 10) << " ";
	}

	return *this;
}


UDP_HDR::UDP_HDR() 
	: 	srcport(0), dstport(0), 
		length(0), chksum(0) 
{
}

UDP_HDR& UDP_HDR::setPort(u_short src, u_short dst)
{	
	srcport = htons(src); 
	dstport = htons(dst); 
	return *this;
}

UDP_HDR& UDP_HDR::setLength(u_short len)
{ 
	length = htons(len); 
	return *this; 
}

UDP_HDR& UDP_HDR::checksum(const char* data, u_short len) 
{
	//length = htons(len);
	//unsigned short* ptr = (unsigned short*)this;
	//long sum = 0, bytes = len;

	//while (bytes>1)
	//{
	//	sum += *ptr++;
	//	bytes -= 2;
	//}
	//if (bytes == 1)
	//{
	//	sum += *(unsigned char*)ptr;
	//}

	//sum = (sum >> 16) + (sum & 0xffff);
	//sum = sum + (sum >> 16);
	//ip_checksum = htons((short)~sum);

	return *this;
}

UDP_HDR& UDP_HDR::load(char* buffer)
{
	memcpy(buffer, (char*)this, sizeof(UDP_HDR));
	return *this;
}