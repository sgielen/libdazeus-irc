#include <network.h>
#include <stdexcept>

int main() {
	try {
		dazeus::NetworkConfig config;
		dazeus::Network *n = new dazeus::Network(config);
		if(!n) return 3;
		delete n;
	} catch(std::runtime_error e) {
		return 1;
	}
	return 0;
}
