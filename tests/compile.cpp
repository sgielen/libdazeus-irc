#include <network.h>
#include <stdexcept>

int main() {
	try {
		dazeus::NetworkConfigPtr config = std::make_shared<dazeus::NetworkConfig>();
		dazeus::Network *n = new dazeus::Network(config);
		if(!n) return 3;
		delete n;
	} catch(std::runtime_error e) {
		return 1;
	}
	return 0;
}
