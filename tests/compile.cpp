#include <network.h>
#include <stdexcept>

int main() {
	try {
		dazeus::NetworkConfig *config = new dazeus::NetworkConfig();
		if(!config) return 2;
		dazeus::Network *n = new dazeus::Network(config);
		if(!n) return 3;
		delete n;
		delete config;
	} catch(std::runtime_error e) {
		return 1;
	}
	return 0;
}
