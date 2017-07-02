#include "NetworkingServer.h"

struct ClientConn {
	Socket sock;
	int timeOut;
	bool sentP2PDetails;

	ClientConn() {
		timeOut = 0;
		sentP2PDetails = false;
	}
};

int maxClientTimeout = 5*1000;

void DoServer() {
	Socket sock1;
	sock1.Create(SP_TCP, SBT_NonBlocking);
	sock1.Bind(8099);

	sock1.Listen(50);

	ClientConn connections[50] = {};
	int connectionCount = 0;

	char packet[1024] = {};
	while (true) {
		if (sock1.AcceptConnection(&connections[connectionCount].sock)) {
			char addrString[128] = {};
			connections[connectionCount].sock.destination.WriteToString(addrString, sizeof(addrString));
			printf("Accepting connection from '%s'.\n", addrString);
			connections[connectionCount].timeOut = 0;
			connectionCount++;

			if (connectionCount == 2) {
				int bytesSent = 0;
				connections[0].sock.SendData(&connections[1].sock.destination, sizeof(IPV4Addr), &bytesSent);
				if (bytesSent == sizeof(IPV4Addr)) {
					connections[0].sentP2PDetails = true;
					connections[1].sock.Destroy();
					connectionCount--;
				}
				else {
					printf("ERROR:Could not send details to client at '%s'\n", addrString);
				}
			}
		}

		for (int i = 0; i < connectionCount; i++) {
			IPV4Addr addr;
			int bytesReceived;
			if (connections[i].sock.ReceiveData(packet, sizeof(packet), &bytesReceived, &addr)) {
				connections[i].sock.Destroy();
				connections[i] = connections[connectionCount - 1];
				connectionCount--;
			}
			else if (connections[i].sentP2PDetails){
				connections[i].timeOut += 10;

				if (connections[i].timeOut >= maxClientTimeout) {
					connections[i].sock.Destroy();
					connections[i] = connections[connectionCount - 1];
					connectionCount--;
				}
			}
		}

#if defined(_WIN32)
		Sleep(10);
#else
		usleep(10000);
#endif
	}
}

