/**
 * Copyright (c) Sjors Gielen, 2010-2012
 * See LICENSE for license.
 */

#include "network.h"
#include "server.h"
#include "utils.h"
#include <stdio.h>
#include <sys/select.h>

std::string dazeus::Network::toString(const Network *n)
{
	std::stringstream res;
	res << "Network[";
	if(n  == 0) {
		res << "0";
	} else {
		const NetworkConfig &nc = n->config();
		res << nc.displayName << ", " << Server::toString(n->activeServer());
	}
	res << "]";

	return res.str();
}

/**
 * @brief Constructor.
 * @deprecated NetworkConfigPtr is a deprecated type.
 */
dazeus::Network::Network( const NetworkConfigPtr c )
: activeServer_(0)
, config_(*c)
, undesirables_()
, deleteServer_(false)
, identifiedUsers_()
, knownUsers_()
, topics_()
, networkListeners_()
, nick_(c->nickName)
, deadline_(0)
, nextPongDeadline_(0)
{}

/**
 * @brief Constructor.
 */
dazeus::Network::Network(const NetworkConfig &c)
: activeServer_(0)
, config_(c)
, undesirables_()
, deleteServer_(false)
, identifiedUsers_()
, knownUsers_()
, topics_()
, networkListeners_()
, nick_(c.nickName)
, deadline_(0)
, nextPongDeadline_(0)
{}

void dazeus::Network::resetConfig(const NetworkConfig &c)
{
	config_ = c;
}

/**
 * @brief Destructor.
 */
dazeus::Network::~Network()
{
	disconnectFromNetwork();
}


/**
 * Returns the active server in this network, or 0 if there is none.
 */
dazeus::Server *dazeus::Network::activeServer() const
{
	return activeServer_;
}


/**
 * @brief Whether this network is marked for autoconnection.
 */
bool dazeus::Network::autoConnectEnabled() const
{
	return config_.autoConnect;
}

void dazeus::Network::action( std::string destination, std::string message )
{
	if( !activeServer_ )
		return;
	activeServer_->ctcpAction( destination, message );
}

void dazeus::Network::names( std::string channel )
{
	if( !activeServer_ )
		return;
	activeServer_->names( channel );
}

std::vector<std::string> dazeus::Network::joinedChannels() const
{
	std::vector<std::string> res;
	std::map<std::string,std::vector<std::string> >::const_iterator it;
	for(it = knownUsers_.begin(); it != knownUsers_.end(); ++it) {
		res.push_back(it->first);
	}
	return res;
}

std::map<std::string,std::string> dazeus::Network::topics() const
{
	return topics_;
}

std::map<std::string,dazeus::Network::ChannelMode> dazeus::Network::usersInChannel(std::string channel) const
{
	std::map<std::string, ChannelMode> res;
	std::map<std::string, std::vector<std::string> >::const_iterator it;
	for(it = knownUsers_.begin(); it != knownUsers_.end(); ++it) {
		if(it->first != channel)
			continue;
		std::vector<std::string> users = it->second;
		std::vector<std::string>::const_iterator it2;
		for(it2 = users.begin(); it2 != users.end(); ++it2) {
			// TODO: remember the mode
			res[*it2] = dazeus::Network::UserMode;
		}
	}
	return res;
}

/**
 * @brief Chooses one server or the other based on the priority and failure
 *        rate.
 * Failure rate is recorded in the Network object that the ServerConfig object
 * links to via the NetworkConfig, if there is such a NetworkConfig object.
 * You must give two servers of the same Network.
 */
namespace dazeus {
struct ServerSorter {
	private:
	Network *n_;
	std::map<std::string, int> randomValues;

	public:
	ServerSorter(Network *n) : n_(n) {}
	bool operator()(const ServerConfig c1, const ServerConfig c2) {
		int prio1 = c1.priority + n_->serverUndesirability(c1);
		int prio2 = c2.priority + n_->serverUndesirability(c2);

		if(prio1 != prio2) {
			return prio1 < prio2;
		}

		// Assign random values to ServerConfigs having the same
		// corrected prio's. These need to be stored because a
		// comparator must be consistent and transitive, so
		// subsequent calls to the operator()() function must agree
		// with the return value of this call.
		std::string id1 = c1.toString();
		std::string id2 = c2.toString();
		if(!contains(randomValues, id1)) {
			randomValues[id1] = rand();
		}
		if(!contains(randomValues, id2)) {
			randomValues[id2] = rand();
		}
		return randomValues[id1] < randomValues[id2];
	}
};
}

const dazeus::NetworkConfig &dazeus::Network::config() const
{
	return config_;
}

void dazeus::Network::connectToNetwork( bool reconnect )
{
	if( !reconnect && activeServer_ )
		return;

	printf("Connecting to network: %s\n", dazeus::Network::toString(this).c_str());

	// Check if there *is* a server to use
	if( servers().size() == 0 )
	{
		printf("Trying to connect to network '%s', but there are no servers to connect to!\n",
		config_.displayName.c_str());
		return;
	}

	// Find the best server to use

	// First, sort the list by priority and earlier failures
	std::vector<ServerConfig> sortedServers = servers();
	std::sort( sortedServers.begin(), sortedServers.end(), ServerSorter(this) );

	// Then, take the first one and create a Server around it
	const ServerConfig &best = sortedServers[0];

	// And set it as the active server, and connect.
	connectToServer( best, true );
}


void dazeus::Network::connectToServer(const ServerConfig &server, bool reconnect)
{
	if( !reconnect && activeServer_ )
		return;
	assert(knownUsers_.size() == 0);
	assert(identifiedUsers_.size() == 0);

	if( activeServer_ )
	{
		activeServer_->disconnectFromServer( SwitchingServersReason );
		// TODO: maybe deleteLater?
		delete(activeServer_);
	}

	activeServer_ = new Server(server, this);
	activeServer_->connectToServer();
	if(config_.connectTimeout > 0) {
		deadline_ = time(NULL) + config_.connectTimeout;
	}
}

void dazeus::Network::joinedChannel(const std::string &user, const std::string &receiver)
{
	if(user == nick_ && !contains_ci(knownUsers_, receiver)) {
		knownUsers_[receiver] = std::vector<std::string>();
	}
	std::vector<std::string> users = find_ci(knownUsers_, receiver)->second;
	if(!contains_ci(users, user))
		find_ci(knownUsers_, receiver)->second.push_back(user);
}

void dazeus::Network::partedChannel(const std::string &user, const std::string &, const std::string &receiver)
{
	if(user == nick_) {
		erase_ci(knownUsers_, receiver);
	} else {
		erase_ci(find_ci(knownUsers_, receiver)->second, user);
	}
	if(!isKnownUser(user)) {
		erase_ci(identifiedUsers_, user);
	}
}

void dazeus::Network::slotQuit(const std::string &origin, const std::string&, const std::string &)
{
	std::map<std::string,std::vector<std::string> >::iterator it;
	for(it = knownUsers_.begin(); it != knownUsers_.end(); ++it) {
		erase_ci(it->second, origin);
	}
	if(!isKnownUser(origin)) {
		erase_ci(identifiedUsers_, origin);
	}
}

void dazeus::Network::slotNickChanged( const std::string &origin, const std::string &nick, const std::string & )
{
	erase_ci(identifiedUsers_, origin);
	erase_ci(identifiedUsers_, nick);

	if(nick_ == origin)
		nick_ = nick;

	std::map<std::string,std::vector<std::string> >::iterator it;
	for(it = knownUsers_.begin(); it != knownUsers_.end(); ++it) {
		if(contains_ci(it->second, origin)) {
			erase_ci(it->second, origin);
			it->second.push_back(nick);
		}
	}
}

void dazeus::Network::kickedChannel(const std::string&, const std::string &user, const std::string&, const std::string &receiver)
{
	if(user == nick_) {
		erase_ci(knownUsers_, receiver);
	} else {
		erase_ci(find_ci(knownUsers_, receiver)->second, user);
	}
	if(!isKnownUser(user)) {
		erase_ci(identifiedUsers_, user);
	}
}

void dazeus::Network::onFailedConnection()
{
	fprintf(stderr, "Connection failed on %s\n", dazeus::Network::toString(this).c_str());

	identifiedUsers_.clear();
	knownUsers_.clear();

	slotIrcEvent("DISCONNECT", "", std::vector<std::string>());

	// Flag old server as undesirable
	// Don't destroy it here yet; it is still in the stack. It will be destroyed
	// in processDescriptors().
	flagUndesirableServer(activeServer_->config());
	deleteServer_ = true;
}


void dazeus::Network::ctcp( std::string destination, std::string message )
{
	if( !activeServer_ )
		return;
	activeServer_->ctcpRequest( destination, message );
}


void dazeus::Network::ctcpReply( std::string destination, std::string message )
{
	if( !activeServer_ )
		return;
	activeServer_->ctcpReply( destination, message );
}


/**
 * @brief Disconnect from the network.
 */
void dazeus::Network::disconnectFromNetwork( DisconnectReason reason )
{
	if( activeServer_ == 0 )
		return;

	identifiedUsers_.clear();
	knownUsers_.clear();

	activeServer_->disconnectFromServer( reason );
	// TODO: maybe deleteLater?
	delete activeServer_;
	activeServer_ = 0;
}


void dazeus::Network::joinChannel( std::string channel )
{
	if( !activeServer_ )
		return;
	activeServer_->join( channel );
}


void dazeus::Network::leaveChannel( std::string channel )
{
	if( !activeServer_ )
		return;
	activeServer_->part( channel );
}


void dazeus::Network::say( std::string destination, std::string message )
{
	if( !activeServer_ )
		return;
	activeServer_->message( destination, message );
}


void dazeus::Network::notice( std::string destination, std::string message )
{
	if( !activeServer_ )
		return;
	activeServer_->notice( destination, message );
}


void dazeus::Network::sendWhois( std::string destination )
{
	activeServer_->whois(destination);
}

const std::vector<dazeus::ServerConfig> &dazeus::Network::servers() const
{
	return config_.servers;
}


std::string dazeus::Network::nick() const
{
	return nick_;
}



std::string dazeus::Network::networkName() const
{
	return config_.name;
}



int dazeus::Network::serverUndesirability( const ServerConfig &sc ) const
{
	auto it = undesirables_.find(sc.toString());
	return it == undesirables_.end() ? 0 : it->second;
}

void dazeus::Network::flagUndesirableServer( const ServerConfig &sc )
{
	std::string i = sc.toString();
	auto it = undesirables_.find(sc.toString());
	if(it == undesirables_.end()) {
		undesirables_[i] = 1;
	} else {
		it->second += 1;
	}
}

void dazeus::Network::serverIsActuallyOkay( const ServerConfig &sc )
{
	undesirables_.erase(sc.toString());
}

bool dazeus::Network::isIdentified(const std::string &user) const {
	return contains_ci(identifiedUsers_, user);
}

bool dazeus::Network::isKnownUser(const std::string &user) const {
	std::map<std::string,std::vector<std::string> >::const_iterator it;
	for(it = knownUsers_.begin(); it != knownUsers_.end(); ++it) {
		if(contains_ci(it->second, user)) {
			return true;
		}
	}
	return false;
}

void dazeus::Network::slotWhoisReceived(const std::string &, const std::string &nick, bool identified) {
	if(!identified) {
		erase_ci(identifiedUsers_, nick);
	} else if(identified && !contains_ci(identifiedUsers_, nick) && isKnownUser(nick)) {
		identifiedUsers_.push_back(nick);
	}
}

void dazeus::Network::slotNamesReceived(const std::string&, const std::string &channel, const std::vector<std::string> &names, const std::string & ) {
	assert(contains_ci(knownUsers_, channel));
	std::vector<std::string> &users = find_ci(knownUsers_, channel)->second;
	std::vector<std::string>::const_iterator it;
	for(it = names.begin(); it != names.end(); ++it) {
		std::string n = *it;
		unsigned int nickStart;
		for(nickStart = 0; nickStart < n.length(); ++nickStart) {
			if(n[nickStart] != '@' && n[nickStart] != '~' && n[nickStart] != '+'
			&& n[nickStart] != '%' && n[nickStart] != '!') {
				break;
			}
		}
		n = n.substr(nickStart);
		if(!contains_ci(users, n))
			users.push_back(n);
	}
}

void dazeus::Network::slotTopicChanged(const std::string&, const std::string &channel, const std::string &topic) {
	topics_[channel] = topic;
}

void dazeus::Network::slotIrcEvent(const std::string &event, const std::string &origin, const std::vector<std::string> &params) {
	std::string receiver;
	if(params.size() > 0)
		receiver = params[0];

	if(event != "ERROR") {
		// a signal from the server means all is OK
		deadline_ = 0;
	}

#define MIN(a) if(params.size() < a) { fprintf(stderr, "Too few parameters for event %s\n", event.c_str()); return; }
	if(event == "CONNECT") {
		nextPongDeadline_ = time(NULL) + 30;
		serverIsActuallyOkay(activeServer_->config());
	} else if(event == "JOIN") {
		MIN(1);
		joinedChannel(origin, receiver);
	} else if(event == "PART") {
		MIN(1);
		partedChannel(origin, std::string(), receiver);
	} else if(event == "KICK") {
		MIN(2);
		kickedChannel(origin, params[1], std::string(), receiver);
	} else if(event == "QUIT") {
		std::string message;
		if(params.size() > 0) {
			message = params[0];
		}
		slotQuit(origin, message, receiver);
	} else if(event == "NICK") {
		MIN(1);
		slotNickChanged(origin, params[0], receiver);
	} else if(event == "TOPIC") {
		MIN(2);
		slotTopicChanged(origin, params[0], params[1]);
	}
#undef MIN
	std::vector<NetworkListener*>::iterator nlit;
	for(nlit = networkListeners_.begin(); nlit != networkListeners_.end();
	    nlit++) {
		(*nlit)->ircEvent(event, origin, params, this);
	}
}

void dazeus::Network::addDescriptors(fd_set *in_set, fd_set *out_set, int *maxfd) {
	activeServer_->addDescriptors(in_set, out_set, maxfd);
}

void dazeus::Network::processDescriptors(fd_set *in_set, fd_set *out_set) {
	if(deleteServer_) {
		deleteServer_ = false;
		delete activeServer_;
		activeServer_ = 0;
		connectToNetwork();
		return;
	}
	activeServer_->processDescriptors(in_set, out_set);
}

void dazeus::Network::checkTimeouts() {
	if(deadline_ > time(NULL)) {
		return; // deadline is set, not passed
	}
	if(deadline_ == 0) {
		if(time(NULL) > nextPongDeadline_) {
			// We've passed the nextPongDeadline, send the next PING
			nextPongDeadline_ = time(NULL) + 30;
			deadline_ = time(NULL) + config_.pongTimeout;
			activeServer_->ping();
		}
		return;
	}

	// Connection didn't finish in time, disconnect and try again
	flagUndesirableServer(activeServer()->config());
	activeServer_->disconnectFromServer(dazeus::Network::TimeoutReason);
	onFailedConnection();
	// no need to delete the server later, though: we can do it from here
	// TODO: use smart ptrs so this mess gets elegant by refcounting
	deleteServer_ = false;
	delete activeServer_;
	activeServer_ = 0;
	connectToNetwork(true);
}

void dazeus::Network::run() {
	std::vector<Network*> nets;
	nets << this;
	dazeus::Network::run(nets);
}

void dazeus::Network::run(std::vector<Network*> networks) {
	fd_set sockets, out_sockets;
	int highest;
	std::vector<Network*>::const_iterator nit;
	struct timeval timeout;
	while(1) {
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		highest = 0;
		FD_ZERO(&sockets);
		FD_ZERO(&out_sockets);
		for(nit = networks.begin(); nit != networks.end(); ++nit) {
			if((*nit)->activeServer()) {
				int ircmaxfd = 0;
				(*nit)->addDescriptors(&sockets, &out_sockets, &ircmaxfd);
				if(ircmaxfd > highest)
					highest = ircmaxfd;
			}
		}

		// If all networks are disconnected, nothing will happen
		// anymore; even the listeners won't be triggered anymore.
		// In such a case, break the event loop
		if(highest == 0) {
			return;
		}

		int socks = select(highest + 1, &sockets, &out_sockets, NULL, &timeout);
		if(socks < 0) {
			perror("select() failed");
			return;
		}
		else if(socks == 0) {
			continue;
		}
		for(nit = networks.begin(); nit != networks.end(); ++nit) {
			if((*nit)->activeServer()) {
				(*nit)->processDescriptors(&sockets, &out_sockets);
			}
		}
	}
}
