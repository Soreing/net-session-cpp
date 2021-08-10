#ifndef SOCKHDR_H
#define SOCKHDR_H

typedef unsigned short u_short;
typedef unsigned long u_long;
typedef unsigned char u_char;
typedef unsigned int u_int;

class IPV4_HDR
{
public:
	u_char ip_hl : 4;				//Header Length
	u_char ip_v : 4;				//Version Number

	u_char  ip_tos;					//Type of service
	u_short ip_length;				//Total length
	u_short ip_id;					//Unique Identifier

	u_char ip_frag_offset : 5;		//Fragment offset field
	u_char ip_more_fragment : 1;	//Reserved
	u_char ip_dont_fragment : 1;	//Don't Fragment
	u_char ip_reserved_zero : 1;	//More Fragment
	u_char ip_frag_offset1;			//Fragment offset

	u_char  ip_ttl;					//Time to live
	u_char  ip_protocol;			//Protocol(TCP,UDP etc)
	u_short ip_checksum;			//IP checksum
	u_int   ip_srcaddr;				//Source address
	u_int   ip_dstaddr;				//Source address

public:
	IPV4_HDR();
	IPV4_HDR& setID(u_short id);
	IPV4_HDR& setProtocol(u_char ipproto);
	IPV4_HDR& setAddress(const char* dst, const char *src);
	IPV4_HDR& setAddress(u_long dst, u_long src);
	IPV4_HDR& checksum(u_short len);
	IPV4_HDR& load(char* buffer);
	IPV4_HDR& print();
};


class UDP_HDR
{
public:
	u_short srcport;	//SourcePort
	u_short dstport;	//Destination Port
	u_short length;		//Total length
	u_short chksum;		//Checksum

	UDP_HDR();
	UDP_HDR& setPort(u_short src, u_short dst);
	UDP_HDR& setLength(u_short len);
	UDP_HDR& checksum(const char* data, u_short len);
	UDP_HDR& load(char* buffer);
};



#endif