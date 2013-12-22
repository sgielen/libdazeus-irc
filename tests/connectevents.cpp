#include <network.h>
#include <server.h>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>

class TestListener : public dazeus::NetworkListener {
public:
	TestListener(dazeus::Network *n)
	: n_(n)
	, welcome(false)
	, motd(false)
	, motd2(false)
	, motdend(false)
	, connected(false)
	, mode(false)
	, noticesrv(false)
	, join(false)
	, topic(false)
	, privmsg(false) {}

#define mustbe(x, y) \
	if(!(x)) { fprintf(stderr, "Test error: %s\n", y); exit(9); }

	virtual void ircEvent(const std::string &event, const std::string &origin,
	  const std::vector<std::string> &params, dazeus::Network *n )
	{
		mustbe(n_ == n, "Network ptr is incorrect");
		mustbe(n->nick() == "Testbot", "Our nick is incorrect");
		mustbe(n->activeServer(), "Network has an active server");

		if(event == "NUMERIC" && params[0] == "1") {
			mustbe(!welcome, "Numeric 1 received twice");
			mustbe(params[1] == "Testbot", "Nickname in welcome wrong");
			mustbe(params[2] == "Welcome to this test server", "Body of numeric received incorrectly");
			welcome = true;
		} else if(event == "NUMERIC" && params[0] == "375") {
			mustbe(!motd, "Motd received twice");
			mustbe(params[1] == "Testbot", "Nickname in motd wrong");
			mustbe(params[2] == "server message of the day", "Body of motd received incorrectly");
			motd = true;
		} else if(event == "NUMERIC" && params[0] == "372") {
			mustbe(!motd2, "Motd body received twice");
			mustbe(params[1] == "Testbot", "Nickname in motd body wrong");
			mustbe(params[2] == "- MOTD", "Body of motd received incorrectly");
			motd2 = true;
		} else if(event == "NUMERIC" && params[0] == "376") {
			mustbe(!motdend, "Motd end received twice");
			mustbe(params[1] == "Testbot", "Nickname in motd end wrong");
			mustbe(params[2] == "End of message of the day.", "End of motd received incorrectly");
			motdend = true;
		} else if(event == "CONNECT") {
			mustbe(!connected, "Connected twice");
			connected = true;
		} else if(event == "MODE") {
			mustbe(!mode, "Received MODE twice");
			mustbe(origin == "Testbot", "MODE origin incorrect");
			mustbe(params[0] == "+x", "MODE parameter incorrect");
			mode = true;
		} else if(event == "NOTICE" && origin == "server") {
			mustbe(!noticesrv, "Received NOTICE from server twice");
			mustbe(params[0] == "Testbot", "NOTICE receiver incorrect");
			mustbe(params[1] == "A notice mE5sage", "NOTICE body incorrect");
			noticesrv = true;
		} else if(event == "JOIN") {
			mustbe(!join, "Received JOIN twice");
			mustbe(origin == "Testbot", "Incorrect JOINer");
			mustbe(params[0] == "##Ch4nN3l", "Incorrect channel");
			join = true;
		} else if(event == "TOPIC") {
			mustbe(!topic, "Received topic twice");
			mustbe(origin == "server", "Incorrect origin to TOPIC");
			mustbe(params[0] == "##Ch4nN3l", "Incorrect channel");
			mustbe(params[1] == "A T0p1C:!", "Incorrect topic");
			topic = true;
		} else if(event == "PRIVMSG") {
			mustbe(!privmsg, "Received privmsg twice");
			mustbe(origin == "t3ST", "Incorrect origin");
			mustbe(params[0] == "Testbot", "Incorrect receiver to privmsg");
			mustbe(params[1] == "Hell0 thEre!", "Incorrect body to privmsg");
			privmsg = true;
			// After this privmsg, we should have joined ##Ch4nN3l
			// Topic and users should be known, verify this
			std::vector<std::string> c = n->joinedChannels();
			mustbe(c.size() == 1 && c[0] == "##Ch4nN3l", "Channel is not registered as joined");
			std::map<std::string,std::string> t = n->topics();
			mustbe(t.size() == 1, "Topic for one channel is not known");
			mustbe(t.begin()->first == "##Ch4nN3l", "Topic for channel is not known");
			mustbe(t.begin()->second == "A T0p1C:!", "Topic for channel is incorrect");
			std::map<std::string,dazeus::Network::ChannelMode> u = n->usersInChannel("##Ch4nN3l");
			mustbe(u.size() == 5, "wrong number of users known");
			std::map<std::string,dazeus::Network::ChannelMode>::iterator it;
			bool op = false, voice = false, owner = false, normal = false, me = false;
			for(it = u.begin(); it != u.end(); ++it) {
				if(it->first == "Op3rAT0R") {
					mustbe(!op, "Operator is known twice");
					op = true;
				} else if(it->first == "V01CE") {
					mustbe(!voice, "Voice is known twice");
					voice = true;
				} else if(it->first == "OwN3R") {
					mustbe(!owner, "Owner is known twice");
					owner = true;
				} else if(it->first == "Norm4L") {
					mustbe(!normal, "Normal is known twice");
					normal = true;
				} else if(it->first == "Testbot") {
					mustbe(!me, "Me is known twice");
					me = true;
				} else {
					fprintf(stderr, "Nickname: %s\n", it->first.c_str());
					mustbe(false, "Didn't recognise a nickname");
				}
			}
			mustbe(op && voice && owner && normal && me, "Some nick wasn't seen");
		} else {
			// Test policy: More events are allowed and ignored,
			// but the required events must come in with the right
			// parameters
			std::cout << event << " " << origin << std::endl;
			for(unsigned i = 0; i < params.size(); ++i) {
				std::cout << "  " << i << ": " << params.at(i) << std::endl;
			}
		}

		if(welcome && motd && motd2 && motdend && connected && mode && noticesrv
		  && join && topic && privmsg) {
			exit(0);
		}
	}

private:
	dazeus::Network *n_;
	bool welcome, motd, motd2, motdend, connected, mode, noticesrv;
	bool join, topic, privmsg;
};

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
		config->nickName = "Testbot";
		if(!config) return 2;

		dazeus::ServerConfig *server = new dazeus::ServerConfig(argv[1], config, port);
		if(!server) return 3;
		config->servers.push_back(server);

		dazeus::Network *n = new dazeus::Network(config);
		if(!n) return 4;

		TestListener *l = new TestListener(n);
		n->addListener(l);
		n->connectToNetwork(false);
		n->run();

		delete l;
		delete n;
		delete server;
		delete config;
	} catch(std::runtime_error e) {
		return 1;
	}
	return 0;
}
