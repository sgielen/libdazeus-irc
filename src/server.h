/**
 * Copyright (c) Sjors Gielen, 2010-2012
 * See LICENSE for license.
 */

#ifndef SERVER_H
#define SERVER_H

#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <stdint.h>
#include <memory>

#include "network.h"
#include "config.h"

// #define SERVER_FULLDEBUG

namespace dazeus {

class Server
{
public:
	Server(const ServerConfig &sc, Network *n);
	~Server();
	const ServerConfig &config() const;
	std::string motd() const;
	void addDescriptors(fd_set *in_set, fd_set *out_set, int *maxfd);
	void processDescriptors(fd_set *in_set, fd_set *out_set);
	static std::string toString(const Server*);

	void connectToServer();
	void disconnectFromServer( Network::DisconnectReason );
	void quit( const std::string &reason );
	void whois( const std::string &destination );
	void ctcpAction( const std::string &destination, const std::string &message );
	void ctcpRequest( const std::string &destination, const std::string &message );
	void ctcpReply( const std::string &destination, const std::string &message );
	void join( const std::string &channel, const std::string &key = std::string() );
	void part( const std::string &channel, const std::string &reason = std::string() );
	void message( const std::string &destination, const std::string &message );
	void notice( const std::string &destination, const std::string &message );
	void names( const std::string &channel );
	void ping();
	void slotNumericMessageReceived( const std::string &origin, unsigned int code, const std::vector<std::string> &params);
	void slotIrcEvent(const std::string &event, const std::string &origin, const std::vector<std::string> &params);
	void slotDisconnected();

private:
	// explicitly disable copy constructor
	Server(const Server&);
	void operator=(const Server&);

	void ircEventMe( const std::string &eventname, const std::string &destination, const std::string &message);

	ServerConfig config_;
	std::string   motd_;
	Network  *network_;
	void *irc_;
	std::string in_whois_for_;
	bool whois_identified_;
	std::vector<std::string> in_names_;
};

}

#endif
