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
		config->nickName = "n1CKn4M3";
		config->userName = "us3RN4m3";
		config->fullName = "fuLLn4m3";
		config->password = "p4sSW0rd";

		dazeus::ServerConfigPtr server = std::make_shared<dazeus::ServerConfig>();
		server->host = argv[1];
		server->network = config;
		server->port = port;
		config->servers.push_back(server);

		dazeus::Network *n = new dazeus::Network(config);
		if(!n) return 4;

		n->connectToNetwork(false);
		// run until disconnect
		n->run();

		delete n;
	} catch(std::runtime_error e) {
		return 1;
	}
	return 0;
}
