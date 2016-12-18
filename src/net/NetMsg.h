#ifndef NETMSG_H
#define NETMSG_H

#pragma once

#include "../../ext/CppUtils/socket.h"
#include "../../ext/CppUtils/strings.h"
#include "../../ext/CppUtils/vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PACKET_SIZE 512
#define PACKET_HEADER_SIZE 12
#define MAX_PACKET_BODY (MAX_PACKET_SIZE - PACKET_HEADER_SIZE)

struct Packet{
	/*1
		Packet header:
		 - 4 bytes: Latest index of reliable packet receveied, 31st bit is if opening packet
		 - 4 bytes: bitfield of ACKs for if 31 previous reliable packet indices were recevied
		 - 4 bytes: packet index, 31st bit is if packet is reliable or not

		All packet data should be in network endianness
	*/

	char packetHeader[PACKET_HEADER_SIZE];
	char packetData[MAX_PACKET_BODY];

	int GetAckIndex() const{
		return ntohl(*(int*)packetHeader);
	}

	int GetAckBitfield() const{
		return ntohl(*(int*)(packetHeader + 4));
	}

	void SetAckIndex(int idx) {
		int* str = (int*)packetHeader;
		*str = htonl(idx);
	}

	void SetAckBitfield(int bitfield) {
		*(int*)(packetHeader + 4) = htonl(bitfield);
	}

	void SetIsStreamingPacket(bool isStreaming){
		int* str = (int*)(packetHeader + 8);
		if (isStreaming){
			*str = htonl(ntohl(*str) & ~(1 << 31));
		}
		else{
			*str = htonl(ntohl(*str) |  (1 << 31));
		}
	}

	bool IsStreamingPacket(){
		return (ntohl(*(int*)(packetHeader + 8)) & (1 << 31)) == 0;
	}

	int GetPacketIndex(){
		int* str = (int*)(packetHeader + 8);
		return ntohl(*str) & ~(1 << 31); 
	}

	void SetPacketIndex(int idx){
		int* str = (int*)(packetHeader + 8);
		*str = htonl(idx & ~(1 << 31) | (ntohl(*str) & (1 << 31)));
	}

	void SetIsOpeningPacket(bool isOpening){
		if (isOpening){
			*(int*)packetHeader |= htonl(1 << 31); 
		}
		else{
			*(int*)packetHeader &= ~htonl(1 << 31);
		}
	}

	bool IsOpeningPacket(){
		return (*(int*)packetHeader & htonl(1 << 31)) != 0;
	}
};

struct NetworkClientConn{
	Vector<Packet> streamPacketsToSend;
	Vector<Packet> packetsReceived;

	Vector<Packet> reliablePacketsToSend;
	Vector<time_t> reliableSendTimes; 

	IPV4Addr addr;

	int streamSendPacketIndex;
	int newReliablePacketStart;
	int reliableSendPacketIndex;

	int reliableReceivePacketIndex;
	int reliableReceivePacketAcks;

	int reliableClientReceivePacketIndex;
	int reliableClientReceivePacketAcks;

	time_t lastReceivedPacketTime;

	NetworkClientConn(){
		Init();
	}

	void Init(){
		streamSendPacketIndex = 0;
		newReliablePacketStart = 0;
		reliableSendPacketIndex = 0;
		reliableReceivePacketIndex = -1;
		reliableReceivePacketAcks = 0;
		reliableClientReceivePacketIndex = -1;
		reliableClientReceivePacketAcks = 0;
	}

	void Reset(){
		streamPacketsToSend.Clear();
		packetsReceived.Clear();
		reliablePacketsToSend.Clear();
		reliableSendTimes.Clear(); 
		Init();
	}
};

typedef void (NetworkCallbackFunc)(NetworkClientConn* client, void* user);

#define MAX_CLIENT_CONNECTION_COUNT 8

struct NetworkClient{
	
	NetworkClientConn conns[MAX_CLIENT_CONNECTION_COUNT];
	int connectionCount;

	Socket socket;

	int packetLoss;
	int reliableTimeoutResend;
	int clientTimeout;

	NetworkCallbackFunc* onConnect;
	NetworkCallbackFunc* onDisconnect;

	NetworkClient(){
		packetLoss = 0;
		reliableTimeoutResend = 2;
		connectionCount = 0;
		clientTimeout = 10;

		onConnect = nullptr;
		onDisconnect = nullptr;
	}

	void Init(short port){
		socket.Create(SP_UDP, SBT_NonBlocking);
		socket.Bind(port);
	}

	void AddConnection(IPV4Addr addr){
		conns[connectionCount].Reset();
		conns[connectionCount].addr = addr;
		conns[connectionCount].lastReceivedPacketTime = time(NULL);
		connectionCount++;
	}

	void SendPacketImm(Packet* packet, int connIndex){
		ASSERT(connIndex >= 0 && connIndex < connectionCount);

		int bytesSent;
		socket.StreamDataTo(packet, sizeof(Packet), &bytesSent, conns[connIndex].addr);
		ASSERT(bytesSent == sizeof(Packet));
	}

	void PollNetworkUpdate();

	void StreamDataToAll(void* data, int bytes){
		Packet packet = {};
		ASSERT(bytes < MAX_PACKET_BODY);
		BNS_MEMCPY(&packet.packetData, data, bytes);

		packet.SetIsStreamingPacket(true);

		for (int c = 0; c < connectionCount; c++){
			packet.SetPacketIndex(conns[c].streamSendPacketIndex);
			conns[c].streamSendPacketIndex++;
			packet.SetAckIndex(conns[c].reliableReceivePacketIndex);
			packet.SetAckBitfield(conns[c].reliableReceivePacketAcks);
			
			ASSERT(packet.IsStreamingPacket());
			conns[c].streamPacketsToSend.PushBack(packet);
		}
	}

	void SendDataReliableToAll(void* data, int bytes){
		Packet packet = {};
		ASSERT(bytes < MAX_PACKET_BODY);
		BNS_MEMCPY(&packet.packetData, data, bytes);

		packet.SetIsStreamingPacket(false);

		for (int c = 0; c < connectionCount; c++){
			packet.SetPacketIndex(conns[c].reliableSendPacketIndex);
			ASSERT(!packet.IsStreamingPacket());
			conns[c].reliableSendPacketIndex++;
			packet.SetAckIndex(conns[c].reliableReceivePacketIndex);
			packet.SetAckBitfield(conns[c].reliableReceivePacketAcks);
			
			conns[c].reliablePacketsToSend.PushBack(packet);
		}
	}

	void OpenNewConnection(IPV4Addr addr, void* userData = nullptr, int userDataLen = 0){
		char addrBuffer[32] = {};
		addr.WriteToString(addrBuffer, sizeof(addrBuffer));
		//OutputDebugStringA(StringStackBuffer<256>("Log: Connecting to '%s'\n", addrBuffer).buffer);

		Packet op = {};
		op.SetIsOpeningPacket(true);

		if (userData != nullptr && userDataLen > 0) {
			BNS_MEMCPY(&op.packetData, userData, userDataLen);
		}

		AddConnection(addr);
		onConnect(&conns[connectionCount - 1], nullptr);
		SendPacketImm(&op, connectionCount - 1);
	}
};

#endif
