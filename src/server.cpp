/**
 * Copyright (c) Sjors Gielen, 2010-2012
 * See LICENSE for license.
 */

#include <cassert>
#include <iostream>
#include <sstream>

// libircclient.h needs cstdlib, don't remove the inclusion
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libircclient.h>

#include "server.h"

// #define DEBUG

#define IRC (irc_session_t*)irc_

std::string Server::toString(const Server *s)
{
	std::stringstream res;
	res << "Server[";
	if(s  == 0 ) {
		res << "0";
	} else {
		const ServerConfig *sc = s->config();
		res << sc->host << ":" << sc->port;
	}
	res << "]";

	return res.str();
}

Server::Server( const ServerConfig *c, Network *n )
: config_(c)
, motd_()
, network_(n)
, irc_(0)
, in_whois_for_()
, whois_identified_(false)
, in_names_()
{
}

Server::~Server()
{
	irc_destroy_session(IRC);
}

const ServerConfig *Server::config() const
{
	return config_;
}

void Server::disconnectFromServer( Network::DisconnectReason reason )
{
	std::string reasonString;
	switch( reason )
	{
	case Network::ShutdownReason:
		reasonString = "Shutting down";
		break;
	case Network::ConfigurationReloadReason:
		reasonString = "Reloading configuration";
		break;
	case Network::SwitchingServersReason:
		reasonString = "Switching servers";
		break;
	case Network::ErrorReason:
		reasonString = "Unknown error";
		break;
	case Network::AdminRequestReason:
		reasonString = "An admin asked me to disconnect";
		break;
	case Network::UnknownReason:
	default:
		reasonString = "See you around!";
	}

	quit( reasonString );
}


std::string Server::motd() const
{
	fprintf(stderr, "MOTD cannot be retrieved.\n");
	return std::string();
}

void Server::quit( const std::string &reason ) {
	irc_cmd_quit(IRC, reason.c_str());
}

void Server::whois( const std::string &destination ) {
	irc_cmd_whois(IRC, destination.c_str());
}

void Server::ctcpAction( const std::string &destination, const std::string &message ) {
	irc_cmd_me(IRC, destination.c_str(), message.c_str());
}

void Server::names( const std::string &channel ) {
	irc_cmd_names(IRC, channel.c_str());
}

void Server::ctcpRequest( const std::string &destination, const std::string &message ) {
	irc_cmd_ctcp_request(IRC, destination.c_str(), message.c_str());
}

void Server::join( const std::string &channel, const std::string &key ) {
	irc_cmd_join(IRC, channel.c_str(), key.c_str());
}

void Server::part( const std::string &channel, const std::string &) {
	// TODO: also use "reason" here (patch libircclient for this)
	irc_cmd_part(IRC, channel.c_str());
}

void Server::message( const std::string &destination, const std::string &message ) {
	std::stringstream ss(message);
	std::string line;
	while(std::getline(ss, line)) {
		irc_cmd_msg(IRC, destination.c_str(), line.c_str());
	}
}

void Server::addDescriptors(fd_set *in_set, fd_set *out_set, int *maxfd) {
	irc_add_select_descriptors(IRC, in_set, out_set, maxfd);
}

void Server::processDescriptors(fd_set *in_set, fd_set *out_set) {
	irc_process_select_descriptors(IRC, in_set, out_set);
}

void Server::slotNumericMessageReceived( const std::string &origin, unsigned int code,
	const std::vector<std::string> &args )
{
	assert( network_ != 0 );
	assert( network_->activeServer() == this );
	// Also send out some other interesting events
	if(code == 311) {
		in_whois_for_ = args[1];
		assert( !whois_identified_ );
	}
	// TODO: should use CAP IDENTIFY_MSG for this:
	else if(code == 307 || code == 330) // 330 means "logged in as", but doesn't check whether nick is grouped
	{
		whois_identified_ = true;
	}
	else if(code == 318)
	{
		network_->slotWhoisReceived( origin, in_whois_for_, whois_identified_ );
		std::vector<std::string> parameters;
		parameters.push_back(in_whois_for_);
		parameters.push_back(whois_identified_ ? "true" : "false");
		network_->slotIrcEvent( "WHOIS", origin, parameters );
		whois_identified_ = false;
		in_whois_for_.clear();
	}
	// part of NAMES
	else if(code == 353)
	{
		std::vector<std::string> names;
		std::stringstream ss(args.back());
		std::string name;
		while(std::getline(ss, name, ' ')) {
			in_names_.push_back(name);
		}
	}
	else if(code == 366)
	{
		network_->slotNamesReceived( origin, args.at(1), in_names_, args.at(0) );
		std::vector<std::string> parameters;
		parameters.push_back(args.at(1));
		std::vector<std::string>::const_iterator it;
		for(it = in_names_.begin(); it != in_names_.end(); ++it) {
			parameters.push_back(*it);
		}
		network_->slotIrcEvent( "NAMES", origin, parameters );
		in_names_.clear();
	}
	else if(code == 332)
	{
		std::vector<std::string> parameters;
		parameters.push_back(args.at(1));
		parameters.push_back(args.at(2));
		network_->slotIrcEvent( "TOPIC", origin, parameters );
	}
	std::stringstream codestream;
	codestream << code;
	std::vector<std::string> params;
	params.push_back(codestream.str());
	std::vector<std::string>::const_iterator it;
	for(it = args.begin(); it != args.end(); ++it) {
		params.push_back(*it);
	}
	network_->slotIrcEvent( "NUMERIC", origin, params );
}

void Server::slotDisconnected()
{
	network_->onFailedConnection();
}

void Server::slotIrcEvent(const std::string &event, const std::string &origin, const std::vector<std::string> &args)
{
	assert(network_ != 0);
	assert(network_->activeServer() == this);
	network_->slotIrcEvent(event, origin, args);
}

void irc_eventcode_callback(irc_session_t *s, unsigned int event, const char *origin, const char **p, unsigned int count) {
	Server *server = (Server*) irc_get_ctx(s);
	std::vector<std::string> params;
	for(unsigned int i = 0; i < count; ++i) {
		params.push_back(std::string(p[i]));
	}
	server->slotNumericMessageReceived(std::string(origin), event, params);
}

void irc_callback(irc_session_t *s, const char *e, const char *o, const char **params, unsigned int count) {
	Server *server = (Server*) irc_get_ctx(s);

	std::string event(e);
	// From libircclient docs, but CHANNEL_NOTICE is bullshit...
	if(event == "CHANNEL_NOTICE") {
		event = "NOTICE";
	}

	// for now, keep these std::strings:
	std::string origin;
	if(o != NULL)
		origin = std::string(o);
	size_t exclamMark = origin.find('!');
	if(exclamMark != std::string::npos) {
		origin = origin.substr(0, exclamMark);
	}

	std::vector<std::string> arguments;
	for(unsigned int i = 0; i < count; ++i) {
		arguments.push_back(std::string(params[i]));
	}

#ifdef DEBUG
	fprintf(stderr, "%s - %s from %s\n", Server::toString(server).c_str(), event.c_str(), origin.c_str());
#endif

	// TODO: handle disconnects nicely (probably using some ping and LIBIRC_ERR_CLOSED
	if(event == "ERROR") {
		fprintf(stderr, "Error received from libircclient; origin=%s.\n", origin.c_str());
		server->slotDisconnected();
	} else if(event == "CONNECT") {
		printf("Connected to server: %s\n", Server::toString(server).c_str());
	}

	server->slotIrcEvent(event, origin, arguments);
}

void Server::connectToServer()
{
	printf("Connecting to server: %s\n", toString(this).c_str());
	assert( !config_->network->nickName.length() == 0 );

	irc_callbacks_t callbacks;
	callbacks.event_connect = irc_callback;
	callbacks.event_nick = irc_callback;
	callbacks.event_quit = irc_callback;
	callbacks.event_join = irc_callback;
	callbacks.event_part = irc_callback;
	callbacks.event_mode = irc_callback;
	callbacks.event_umode = irc_callback;
	callbacks.event_topic = irc_callback;
	callbacks.event_kick = irc_callback;
	callbacks.event_channel = irc_callback;
	callbacks.event_privmsg = irc_callback;
	callbacks.event_notice = irc_callback;
	callbacks.event_invite = irc_callback;
	callbacks.event_ctcp_req = irc_callback;
	callbacks.event_ctcp_rep = irc_callback;
	callbacks.event_ctcp_action = irc_callback;
	callbacks.event_unknown = irc_callback;
	callbacks.event_numeric = irc_eventcode_callback;
	callbacks.event_dcc_chat_req = NULL;
	callbacks.event_dcc_send_req = NULL;

	irc_ = (void*)irc_create_session(&callbacks);
	if(!irc_) {
		std::cerr << "Couldn't create IRC session in Server.";
		abort();
	}
	irc_set_ctx(IRC, this);

	assert( config_->network->nickName.length() != 0 );
	irc_connect(IRC, config_->host.c_str(),
		config_->port,
		config_->network->password.c_str(),
		config_->network->nickName.c_str(),
		config_->network->userName.c_str(),
		config_->network->fullName.c_str());
}
