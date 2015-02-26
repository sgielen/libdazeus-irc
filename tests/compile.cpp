#include <network.h>
#include <stdexcept>

int main() {
	try {
		dazeus::NetworkConfig config;
		dazeus::Network n(config);
		return 0;
	} catch(std::runtime_error e) {
		return 1;
	}
}
