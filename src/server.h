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

#include <libircclient.h>

#include "user.h"
#include "network.h"

// #define SERVER_FULLDEBUG

struct NetworkConfig;

struct ServerConfig {
  ServerConfig(std::string h = std::string(), NetworkConfig *n = 0, uint16_t p = 6667,
    bool s = false, uint8_t pr = 5) : host(h), port(p), priority(pr),
    network(n), ssl(s) {}
  ServerConfig(const ServerConfig &s) : host(s.host), port(s.port),
    priority(s.priority), network(s.network), ssl(s.ssl) {}
  const ServerConfig &operator=(const ServerConfig &s) { host = s.host; port = s.port;
    priority = s.priority; network = s.network; ssl = s.ssl; return *this; }
  std::string host;
  uint16_t port;
  uint8_t priority;
  NetworkConfig *network;
  bool ssl;
};

class Server
{


public:
	  Server(const ServerConfig *c, Network *n);
	  ~Server();
	const ServerConfig *config() const;
	std::string motd() const;
	void addDescriptors(fd_set *in_set, fd_set *out_set, int *maxfd);
	void processDescriptors(fd_set *in_set, fd_set *out_set);
	irc_session_t *getIrc() const;
	static std::string toString(const Server*);

	void connectToServer();
	void disconnectFromServer( Network::DisconnectReason );
	void quit( const std::string &reason );
	void whois( const std::string &destination );
	void ctcpAction( const std::string &destination, const std::string &message );
	void ctcpRequest( const std::string &destination, const std::string &message );
	void join( const std::string &channel, const std::string &key = std::string() );
	void part( const std::string &channel, const std::string &reason = std::string() );
	void message( const std::string &destination, const std::string &message );
	void names( const std::string &channel );
	void slotNumericMessageReceived( const std::string &origin, unsigned int code, const std::vector<std::string> &params);
	void slotIrcEvent(const std::string &event, const std::string &origin, const std::vector<std::string> &params);
	void slotDisconnected();

private:
	// explicitly disable copy constructor
	Server(const Server&);
	void operator=(const Server&);

	const ServerConfig *config_;
	std::string   motd_;
	Network  *network_;
	irc_session_t *irc_;
	std::string in_whois_for_;
	bool whois_identified_;
	std::vector<std::string> in_names_;
};

#endif
