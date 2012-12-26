#include <network.h>
#include <stdexcept>

int main() {
	try {
		NetworkConfig *config = new NetworkConfig();
		if(!config) return 2;
		Network *n = new Network(config);
		if(!n) return 3;
		delete n;
		delete config;
	} catch(std::runtime_error e) {
		return 1;
	}
	return 0;
}
