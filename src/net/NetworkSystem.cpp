#include "NetworkSystem.h"

#include "../core/Scene.h"

void OnNetworkClientConnect(NetworkClientConn* client) {
	//int index = GlobalScene->net.client.conns - client;
}

void OnNetworkClientDisconnect(NetworkClientConn* client) {
	int index = GlobalScene->net.client.conns - client;

	int lastIndex = GlobalScene->net.client.connectionCount - 1;
	GlobalScene->net.idMappings[index] = GlobalScene->net.idMappings[lastIndex];
	GlobalScene->net.otherPlayerEnts[index] = GlobalScene->net.otherPlayerEnts[lastIndex];
}

IDHandle<Entity> ClientEntityMapping::LocalToClient(IDHandle<Entity> id){
	for (int i = 0; i < localIds.count; i++) {
		if (localIds.data[i] == id) {
			return clientIds.data[i];
		}
	}

	ASSERT_WARN("Could not find local id: %d.", id.id);
	return IDHandle<Entity>(-1);
}

IDHandle<Entity> ClientEntityMapping::ClientToLocal(IDHandle<Entity> id){
	for (int i = 0; i < clientIds.count; i++) {
		if (clientIds.data[i] == id) {
			return localIds.data[i];
		}
	}

	ASSERT_WARN("Could not find client id: %d.", id.id);
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

		ownedEnts.PushBack(spawnedEnts.Get(i));
	}

	spawnedEnts.Clear();

	// Find destroyed ents
	for (int i = 0; i < ownedEnts.count; i++) {
		if (GlobalScene->entities.GetById(ownedEnts.data[i]) == nullptr) {
			// It's been destroyed...
		}
	}

	MemStream streamPacketData;

	// Send position updates

	client.StreamDataToAll(streamPacketData.readHead, streamPacketData.GetLength());
	client.SendDataReliableToAll(reliablePacketData.readHead, reliablePacketData.GetLength());

	if (updateSeconds >= tickTime) {
		client.PollNetworkUpdate();
		updateSeconds = 0;
	}

	// Get messages from clients
}

void ReadReliablePacket(MemStream& stream) {
	
}

void ReadStreamingPacket(MemStream& stream) {
	
}

