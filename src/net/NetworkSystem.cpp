#include "NetworkSystem.h"

#include "../core/Scene.h"

void OnNetworkClientConnect(NetworkClientConn* client, void* opPacketData) {
	int index = client - GlobalScene->net.client.conns;

	//OutputDebugStringA(StringStackBuffer<256>("Connecting to client index: %d\n", index).buffer);

	if (opPacketData != nullptr) {
		{
			MemStream str;

			// There has got to be a better way to do mem stream reads w/o freeing mem...
			str.base = opPacketData;
			str.readHead = str.base;
			str.writeHead = VOID_PTR_ADD(str.base, MAX_PACKET_BODY);

			IDHandle<Prefab> spawnPlayerPrefab = IDHandle<Prefab>(str.Read<int>());
			int hostId = str.Read<int>();

			Vector3 spawnPos = str.Read<Vector3>();
			Quaternion spawnRot = str.Read<Quaternion>();

			Prefab* spawnPrefab = GlobalScene->res.prefabs.GetById(spawnPlayerPrefab);
			Entity* spawnedEnt = spawnPrefab->Instantiate(spawnPos, spawnRot);

			GlobalScene->net.idMappings[index].localIds.PushBack(spawnedEnt->id);
			GlobalScene->net.idMappings[index].clientIds.PushBack(hostId);

			GlobalScene->net.otherPlayerEnts[index] = GET_PTR_HANDLE(spawnedEnt);

			str.base = nullptr;
			str.readHead = nullptr;
			str.writeHead = nullptr;
		}

		MemStream responsePacketData;
		responsePacketData.Write(RGM_EntitySpawn);
		responsePacketData.Write(GlobalScene->net.playerPrefab.id);
		responsePacketData.Write(GlobalScene->net.playerEnt.id);

		Entity* ent = GlobalScene->entities.GetById(GlobalScene->net.playerEnt);
		Transform* trans = GlobalScene->transforms.GetById(ent->transform);

		responsePacketData.Write(trans->position);
		responsePacketData.Write(trans->rotation);

		responsePacketData.Write(RGM_DoneWithMessages);

		GlobalScene->net.client.SendDataReliableToAll(responsePacketData.readHead, responsePacketData.GetLength());
	}
}

void OnNetworkClientDisconnect(NetworkClientConn* client, void*) {
	int index = client - GlobalScene->net.client.conns;

	int lastIndex = GlobalScene->net.client.connectionCount - 1;
	GlobalScene->net.idMappings[index] = GlobalScene->net.idMappings[lastIndex];
	GlobalScene->net.otherPlayerEnts[index] = GlobalScene->net.otherPlayerEnts[lastIndex];
}

void NetworkSystem::OpenNewConnection(IPV4Addr addr) {
	MemStream str;
	str.Write(playerPrefab);
	str.Write(playerEnt);

	Entity* ent = GlobalScene->entities.GetById(playerEnt);
	Transform* trans = GlobalScene->transforms.GetById(ent->transform);

	str.Write(trans->position);
	str.Write(trans->rotation);

	client.OpenNewConnection(addr, str.readHead, str.GetLength());
	currState = NSS_Connected;
}

IDHandle<Entity> ClientEntityMapping::LocalToClient(IDHandle<Entity> id){
	for (int i = 0; i < localIds.count; i++) {
		if (localIds.data[i] == id.id) {
			return IDHandle<Entity>(clientIds.data[i]);
		}
	}

	//ASSERT_WARN("Could not find local id: %d.", id.id);
	return IDHandle<Entity>(-1);
}

IDHandle<Entity> ClientEntityMapping::ClientToLocal(IDHandle<Entity> id){
	for (int i = 0; i < clientIds.count; i++) {
		if (clientIds.data[i] == id.id) {
			return IDHandle<Entity>(localIds.data[i]);
		}
	}

	//ASSERT_WARN("Could not find client id: %d.", id.id);
	return IDHandle<Entity>(-1);
}

void NetworkSystem::NetworkUpdate() {
	updateSeconds += GlobalScene->GetDeltaTime();

	const float tickTime = 1.0f / networkTicksPerSecond;

	MemStream reliablePacketData;

	// Find new spawned ents
	for (int i = 0; i < spawnedEnts.count; i++) {
		PrefabInstanceComponent* instComp = FIND_COMPONENT_BY_ENTITY(PrefabInstanceComponent, spawnedEnts.Get(i));
		ASSERT_MSG(instComp != nullptr,
			"Warning, entity of id '%d' is not a prefab, but it was registered with network.",
			spawnedEnts.Get(i).id);

		reliablePacketData.Write(RGM_EntitySpawn);
		reliablePacketData.Write(instComp->prefab.id);
		reliablePacketData.Write(instComp->entity.id);

		Entity* ent = GlobalScene->entities.GetById(instComp->entity);
		Transform* trans = GlobalScene->transforms.GetById(ent->transform);

		reliablePacketData.Write(trans->position);
		reliablePacketData.Write(trans->rotation);

		ownedEnts.PushBack(spawnedEnts.Get(i));
	}

	spawnedEnts.Clear();

	// Find destroyed ents
	for (int i = 0; i < ownedEnts.count; i++) {
		if (GlobalScene->entities.GetById(ownedEnts.data[i]) == nullptr) {
			// It's been destroyed...
			reliablePacketData.Write(RGM_EntityDestroy);
			reliablePacketData.Write(ownedEnts.data[i].id);
			ownedEnts.Remove(i);
			i--;
		}
	}

	if (reliablePacketData.GetLength() > 0) {
		reliablePacketData.Write(RGM_DoneWithMessages);
		client.SendDataReliableToAll(reliablePacketData.readHead, reliablePacketData.GetLength());
	}

	MemStream streamPacketData;

	streamPacketData.Write(ownedEnts.count);

	// Send position updates
	for (int i = 0; i < ownedEnts.count; i++) {
		streamPacketData.Write(ownedEnts.Get(i).id);

		Entity* ent = GlobalScene->entities.GetById(ownedEnts.Get(i));
		Transform* trans = GlobalScene->transforms.GetById(ent->transform);
		streamPacketData.Write(trans->position);
		streamPacketData.Write(trans->rotation);
	}

	client.StreamDataToAll(streamPacketData.readHead, streamPacketData.GetLength());

	if (updateSeconds >= tickTime) {
		client.PollNetworkUpdate();
		updateSeconds = 0;
	}

	for (int c = 0; c < client.connectionCount; c++) {
		for (int i = 0; i < client.conns[c].packetsReceived.count; i++) {
			Packet* pkt = &client.conns[c].packetsReceived.data[i];

			MemStream str;

			// There has got to be a better way to do mem stream reads w/o freeing mem...
			str.base = &pkt->packetData;
			str.readHead = str.base;
			str.writeHead = VOID_PTR_ADD(str.base, MAX_PACKET_BODY);

			if (pkt->IsStreamingPacket()) {
				ReadStreamingPacket(c, str);
			}
			else{
				ReadReliablePacket(c, str);
			}

			str.base = nullptr;
			str.readHead = nullptr;
			str.writeHead = nullptr;
		}

		client.conns[c].packetsReceived.Clear();
	}
}

void NetworkSystem::ReadReliablePacket(int clientIndex, MemStream& stream) {
	bool reading = true;
	while (reading && stream.GetLength() > 0) {
		ReliableGameMessage msg = stream.Read<ReliableGameMessage>();
		switch (msg) {
		case RGM_EntitySpawn: {
			IDHandle<Prefab> prefabId = IDHandle<Prefab>(stream.Read<int>());
			int instanceId = stream.Read<int>();

			Vector3 pos = stream.Read<Vector3>();
			Quaternion rot = stream.Read<Quaternion>();

			Prefab* prefab = GlobalScene->res.prefabs.GetById(prefabId);
			Entity* ent = prefab->Instantiate(pos, rot);

			idMappings[clientIndex].clientIds.PushBack(instanceId);
			idMappings[clientIndex].localIds.PushBack(ent->id);

			if (prefabId == playerPrefab) {
				otherPlayerEnts[clientIndex] = GET_PTR_HANDLE(ent);
			}
		} break;
		case RGM_EntityDestroy: {
			int entId = stream.Read<int>();
			IDHandle<Entity> localId = idMappings[clientIndex].ClientToLocal(IDHandle<Entity>(entId));

			GlobalScene->DestroyEntity(localId);
		} break;
		case RGM_DoneWithMessages: {
			reading = false;
		} break;
		default: {
			ASSERT_WARN("Reliable Packet contains unknown message type: %d", (int)msg);
			reading = false;
		} break;
		}
	}
}

void NetworkSystem::ReadStreamingPacket(int clientIndex, MemStream& stream) {
	int entCount = stream.Read<int>();

	for (int i = 0; i < entCount; i++) {
		int entId = stream.Read<int>();
		Vector3 pos = stream.Read<Vector3>();
		Quaternion rot = stream.Read<Quaternion>();

		IDHandle<Entity> localId = idMappings[clientIndex].ClientToLocal(IDHandle<Entity>(entId));
		Entity* ent = GlobalScene->entities.GetById(localId);
		if (ent != nullptr) {
			Transform* trans = GlobalScene->transforms.GetById(ent->transform);
			ASSERT(trans != nullptr);

			trans->position = pos;
			trans->rotation = rot;
		}
	}
}

