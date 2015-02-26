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
		dazeus::NetworkConfigPtr config = std::make_shared<dazeus::NetworkConfig>();
		config->name = "test";
		config->displayName = "test";
		config->nickName = "connectone";

		dazeus::ServerConfigPtr server = std::make_shared<dazeus::ServerConfig>();
		server->host = argv[1];
		server->port = port;
		config->servers.push_back(server);

		dazeus::Network *n = new dazeus::Network(*config);
		if(!n) return 4;

		n->connectToNetwork(false);
		n->run();
		config->nickName = "connecttwo";
		n->resetConfig(*config);
		n->connectToNetwork(true);
		n->run();

		delete n;
	} catch(std::runtime_error e) {
		return 1;
	}
	return 0;
}
