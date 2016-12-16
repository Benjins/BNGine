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
	Vector<IDHandle<Entity>> clientIds;
	Vector<IDHandle<Entity>> localIds;

	IDHandle<Entity> LocalToClient(IDHandle<Entity> id);
	IDHandle<Entity> ClientToLocal(IDHandle<Entity> id);
};

void OnNetworkClientConnect(NetworkClientConn* client);
void OnNetworkClientDisconnect(NetworkClientConn* client);

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
		client.packetLoss = 10;
		currState = NSS_Begin;

		client.onConnect = OnNetworkClientConnect;
		client.onDisconnect = OnNetworkClientDisconnect;
	}

	void Initialize(short port) {
		client.Init(port);
		currState = NSS_Initialized;
	}

	void OpenNewConnection(IPV4Addr addr) {
		client.OpenNewConnection(addr);
		currState = NSS_Connected;
	}

	void RegisterPlayer(Entity* player) {
		playerEnt = GET_PTR_HANDLE(player);
	}

	void RegisterSpawnedEntity(Entity* ent){
		spawnedEnts.PushBack(GET_PTR_HANDLE(ent));
	}

	void ReadReliablePacket(MemStream& stream);
	void ReadStreamingPacket(MemStream& stream);

	void NetworkUpdate();
};

#endif
