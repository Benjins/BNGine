#include "NetMsg.h"

void NetworkClient::PollNetworkUpdate(){
	int bytesReceived;
	Packet newPacket;
	IPV4Addr addr;
	while (socket.ReceiveData(&newPacket, sizeof(newPacket), &bytesReceived, &addr)){
		int index = -1;
		for (int i = 0; i < connectionCount; i++){
			if (conns[i].addr == addr){
				index = i;
				break;
			}
		}

		if (index < 0 && newPacket.IsOpeningPacket()){
			AddConnection(addr);
			onConnect(&conns[connectionCount], &newPacket.packetData);

			char addrBuffer[32] = {};
			addr.WriteToString(addrBuffer, sizeof(addrBuffer));
			printf("Log: Now connected to '%s'\n", addrBuffer);
		}
		else if (index >= 0){
			bool isReliable = !newPacket.IsStreamingPacket();
			if (packetLoss > 0 && (rand() % 100) < packetLoss){
				//printf("Log: packet of index %d lost\n", newPacket.GetPacketIndex());
			}
			else {
				int packetIdx = newPacket.GetPacketIndex();
				if (isReliable){
					//printf ("Log: reliable packet received w/ index %d\n", packetIdx);
					if (packetIdx > conns[index].reliableReceivePacketIndex){
						int diff = packetIdx - conns[index].reliableReceivePacketIndex;
						ASSERT(diff > 0);
						conns[index].reliableReceivePacketAcks <<= diff;
						conns[index].reliableReceivePacketAcks |= 1;
						conns[index].reliableReceivePacketIndex = packetIdx;
						//printf("Log: received packet of idx %d, more than %d\n", packetIdx, conns[index].reliableReceivePacketIndex);
					}
					else{
						//printf("Log: received packet of idx %d, bitfield is %d on %d\n", packetIdx, 
							//conns[index].reliableReceivePacketAcks,
							//conns[index].reliableReceivePacketIndex);
						int diff = conns[index].reliableReceivePacketIndex - packetIdx;
						ASSERT(diff >= 0);
						//ASSERT((conns[index].reliableReceivePacketAcks & (1 << diff)) == 0);
						if((conns[index].reliableReceivePacketAcks & (1 << diff)) != 0){
							printf("Log: Double receive of packet idx: %d\n", packetIdx);
						}
						
						conns[index].reliableReceivePacketAcks |= (1 << diff);
						//printf("Log: received packet of idx %d, bitfield now %d\n", packetIdx, conns[index].reliableReceivePacketAcks);
					}
				}
				else{
					//printf("Log: streaming packet received of index: %d\n", packetIdx);
				}

				int ackIndex = newPacket.GetAckIndex();
				int ackBitfield = newPacket.GetAckBitfield();
				// If we're getting a later packet ack from the client
				if (ackIndex > conns[index].reliableClientReceivePacketIndex) {
					conns[index].reliableClientReceivePacketIndex = ackIndex;
					conns[index].reliableClientReceivePacketAcks = ackBitfield;

					conns[index].lastReceivedPacketTime = time(NULL);

					conns[index].packetsReceived.PushBack(newPacket);
				}
			}
		}
		else{
			char addrBuffer[32] = {};
			addr.WriteToString(addrBuffer, sizeof(addrBuffer));
			printf("Log: unidentified address sending data: '%s'", addrBuffer);
		}
	}
	
	for (int c = 0; c < connectionCount; c++){
		time_t currTime = time(NULL);

		if (currTime - conns[c].lastReceivedPacketTime >= clientTimeout){
			if (onDisconnect) {
				onDisconnect(&conns[c], nullptr);
			}

			char addrBuffer[32] = {};
			conns[c].addr.WriteToString(addrBuffer, sizeof(addrBuffer));
			printf("Log: Client at addr '%s' has timed out.\n", addrBuffer);
			conns[c] = conns[connectionCount - 1];
			c--;
			connectionCount--;
			continue;
		}

		for (int i = 0; i < conns[c].newReliablePacketStart; i++){
			int packetIdx = conns[c].reliablePacketsToSend.data[i].GetPacketIndex();
			int diff = conns[c].reliableClientReceivePacketIndex - packetIdx;

			// We know the packet has been received
			if (diff >= 0 && diff < 32 && (conns[c].reliableClientReceivePacketAcks & (1 << diff)) != 0){
				//printf("Log: Packet %d verfied as received.\n", packetIdx);
				conns[c].reliablePacketsToSend.Remove(i);
				conns[c].reliableSendTimes.Remove(i);
				conns[c].newReliablePacketStart--;
				i--;
			}
			else if (currTime - conns[c].reliableSendTimes.data[i] >= reliableTimeoutResend){
				// Resend the packet, it likely got lost
				SendPacketImm(&conns[c].reliablePacketsToSend.data[i], c);
				conns[c].reliableSendTimes.data[i] = currTime;
				//printf ("Log: Resending packet of idx %d\n", packetIdx);
			}
		}

		conns[c].reliableSendTimes.EnsureCapacity(conns[c].reliablePacketsToSend.count);
		for (int i = conns[c].newReliablePacketStart; i < conns[c].reliablePacketsToSend.count; i++){
			SendPacketImm(&conns[c].reliablePacketsToSend.data[i], c);
			conns[c].reliableSendTimes.PushBack(currTime);
		}

		conns[c].newReliablePacketStart = conns[c].reliablePacketsToSend.count;

		for (int i = 0; i < conns[c].streamPacketsToSend.count; i++){
			SendPacketImm(&conns[c].streamPacketsToSend.data[i], c);
		}

		conns[c].streamPacketsToSend.Clear();
	}
}