#ifndef  NETWORKSYSTEM_H
#define  NETWORKSYSTEM_H

#pragma once

#include "NetMsg.h"

#include "../core/Entity.h"

enum ReliableGameMessage{
	RGM_EntitySpawn,
	RGM_EntityDestroy,
	RGM_DoneWithMessages,
	RGM_Count
};

enum NetworkSystemState {
	NSS_Begin,
	NSS_Initialized,
	NSS_Connected
};

struct ClientEntityMapping {
	Vector<uint32> clientIds;
	Vector<uint32> localIds;

	IDHandle<Entity> LocalToClient(IDHandle<Entity> id);
	IDHandle<Entity> ClientToLocal(IDHandle<Entity> id);
};

void OnNetworkClientConnect(NetworkClientConn* client, void*);
void OnNetworkClientDisconnect(NetworkClientConn* client, void*);

struct NetworkSystem {
	NetworkClient client;

	IDHandle<Prefab> playerPrefab;

	IDHandle<Entity> playerEnt;
	Vector<IDHandle<Entity>> ownedEnts;
	Vector<IDHandle<Entity>> spawnedEnts;

	IDHandle<Entity> otherPlayerEnts[MAX_CLIENT_CONNECTION_COUNT];
	ClientEntityMapping idMappings[MAX_CLIENT_CONNECTION_COUNT];

	NetworkSystemState currState;

	int networkTicksPerSecond;
	float updateSeconds;

	short debugPortToConnectTo;

	NetworkSystem() {
		networkTicksPerSecond = 15;
		//client.packetLoss = 10;
		
		client.clientTimeout = 1800;
		
		currState = NSS_Begin;

		client.onConnect = OnNetworkClientConnect;
		client.onDisconnect = OnNetworkClientDisconnect;
	}

	void Initialize(short port) {
		client.Init(port);
		currState = NSS_Initialized;
	}

	void OpenNewConnection(IPV4Addr addr);

	void RegisterPlayer(IDHandle<Entity> player) {
		playerEnt = player;
		ownedEnts.PushBack(playerEnt);
	}

	void RegisterSpawnedEntity(Entity* ent){
		spawnedEnts.PushBack(GET_PTR_HANDLE(ent));
	}

	void ReadReliablePacket(int clientIndex, MemStream& stream);
	void ReadStreamingPacket(int clientIndex, MemStream& stream);

	void NetworkUpdate();
};

#endif
