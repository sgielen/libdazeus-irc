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
		dazeus::NetworkConfig *config = new dazeus::NetworkConfig();
		config->name = "test";
		config->displayName = "test";
		config->nickName = "connectone";
		if(!config) return 2;

		dazeus::ServerConfigPtr server = std::make_shared<dazeus::ServerConfig>(argv[1], config, port);
		config->servers.push_back(server);

		dazeus::Network *n = new dazeus::Network(config);
		if(!n) return 4;

		n->connectToNetwork(false);
		n->run();
		config->nickName = "connecttwo";
		n->connectToNetwork(true);
		n->run();

		delete n;
		delete config;
	} catch(std::runtime_error e) {
		return 1;
	}
	return 0;
}
