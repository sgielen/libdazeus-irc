#include <network.h>
#include <server.h>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	if(argc != 3) {
		fprintf(stderr, "Usage: %s host port\n", argv[0]);
		return 10;
	}

	uint16_t port = strtoul(argv[2], NULL, 10);

	try {
		dazeus::NetworkConfig config;
		config.name = "test";
		config.displayName = "test";
		config.nickName = "connectone";

		dazeus::ServerConfig server;
		server.host = argv[1];
		server.port = port;
		config.servers.push_back(server);

		dazeus::Network n(config);

		n.connectToNetwork(false);
		n.run();
		config.nickName = "connecttwo";
		n.resetConfig(config);
		n.connectToNetwork(true);
		n.run();

		return 0;
	} catch(std::runtime_error e) {
		return 1;
	}
}
