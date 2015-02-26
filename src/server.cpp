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

std::string dazeus::Server::toString(const Server *s)
{
	std::stringstream res;
	res << "Server[";
	if(s  == 0 ) {
		res << "0";
	} else {
		const ServerConfig &sc = s->config();
		res << sc.host << ":" << sc.port;
	}
	res << "]";

	return res.str();
}

std::string dazeus::ServerConfig::toString() const {
	std::stringstream ss;
	ss << "irc";
	if(ssl) {
		ss << "s";
	}
	ss << "://" << host << ":" << port;
	return "ServerConfig[" + ss.str() + "]";
}

dazeus::Server::Server(const ServerConfig &sc, Network *n)
: config_(sc)
, motd_()
, network_(n)
, irc_(0)
, in_whois_for_()
, whois_identified_(false)
, in_names_()
{
}

dazeus::Server::~Server()
{
	irc_destroy_session(IRC);
}

const dazeus::ServerConfig &dazeus::Server::config() const
{
	return config_;
}

void dazeus::Server::disconnectFromServer( Network::DisconnectReason reason )
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
	case Network::TimeoutReason:
		reasonString = "Timeout";
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


std::string dazeus::Server::motd() const
{
	fprintf(stderr, "MOTD cannot be retrieved.\n");
	return std::string();
}

void dazeus::Server::quit( const std::string &reason ) {
	irc_cmd_quit(IRC, reason.c_str());
}

void dazeus::Server::whois( const std::string &destination ) {
	irc_cmd_whois(IRC, destination.c_str());
}

/**
 * Echo an event back to the caller, with a specific echoing name. Used for IRC
 * commands that generate no replies from the server, such as PRIVMSG and an
 * ACTION message inside a CTCP message.
 */
void dazeus::Server::ircEventMe( const std::string &eventname, const std::string &destination, const std::string &message) {
	std::vector<std::string> parameters;
	parameters.push_back(destination);
	parameters.push_back(message);
	slotIrcEvent(eventname, network_->nick(), parameters);
}

void dazeus::Server::ctcpAction( const std::string &destination, const std::string &message ) {
	ircEventMe("ACTION_ME", destination, message);
	irc_cmd_me(IRC, destination.c_str(), message.c_str());
}

void dazeus::Server::names( const std::string &channel ) {
	irc_cmd_names(IRC, channel.c_str());
}

void dazeus::Server::ctcpRequest( const std::string &destination, const std::string &message ) {
	ircEventMe("CTCP_ME", destination, message);
	irc_cmd_ctcp_request(IRC, destination.c_str(), message.c_str());
}

void dazeus::Server::ctcpReply( const std::string &destination, const std::string &message ) {
	ircEventMe("CTCP_REP_ME", destination, message);
	irc_cmd_ctcp_reply(IRC, destination.c_str(), message.c_str());
}

void dazeus::Server::join( const std::string &channel, const std::string &key ) {
	irc_cmd_join(IRC, channel.c_str(), key.c_str());
}

void dazeus::Server::part( const std::string &channel, const std::string &) {
	// TODO: also use "reason" here (patch libircclient for this)
	irc_cmd_part(IRC, channel.c_str());
}

void dazeus::Server::message( const std::string &destination, const std::string &message ) {
	std::stringstream ss(message);
	std::string line;
	while(std::getline(ss, line)) {
		ircEventMe("PRIVMSG_ME", destination, message);
		irc_cmd_msg(IRC, destination.c_str(), line.c_str());
	}
}

void dazeus::Server::notice( const std::string &destination, const std::string &message ) {
	std::stringstream ss(message);
	std::string line;
	while(std::getline(ss, line)) {
		ircEventMe("NOTICE_ME", destination, message);
		irc_cmd_notice(IRC, destination.c_str(), line.c_str());
	}
}

void dazeus::Server::ping() {
	irc_send_raw(IRC, "PING");
}

void dazeus::Server::addDescriptors(fd_set *in_set, fd_set *out_set, int *maxfd) {
	irc_add_select_descriptors(IRC, in_set, out_set, maxfd);
}

void dazeus::Server::processDescriptors(fd_set *in_set, fd_set *out_set) {
	irc_process_select_descriptors(IRC, in_set, out_set);
}

void dazeus::Server::slotNumericMessageReceived( const std::string &origin, unsigned int code,
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
		slotIrcEvent( "WHOIS", origin, parameters );
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
		slotIrcEvent( "NAMES", origin, parameters );
		in_names_.clear();
	}
	else if(code == 332)
	{
		std::vector<std::string> parameters;
		parameters.push_back(args.at(1));
		parameters.push_back(args.at(2));
		slotIrcEvent( "TOPIC", origin, parameters );
	}
	std::stringstream codestream;
	codestream << code;
	std::vector<std::string> params;
	params.push_back(codestream.str());
	std::vector<std::string>::const_iterator it;
	for(it = args.begin(); it != args.end(); ++it) {
		params.push_back(*it);
	}
	slotIrcEvent( "NUMERIC", origin, params );
}

void dazeus::Server::slotDisconnected()
{
	network_->onFailedConnection();
}

void dazeus::Server::slotIrcEvent(const std::string &event, const std::string &origin, const std::vector<std::string> &args)
{
	assert(network_ != 0);
	assert(network_->activeServer() == this);
	network_->slotIrcEvent(event, origin, args);
}

void irc_eventcode_callback(irc_session_t *s, unsigned int event, const char *origin, const char **p, unsigned int count) {
	dazeus::Server *server = (dazeus::Server*) irc_get_ctx(s);
	std::vector<std::string> params;
	for(unsigned int i = 0; i < count; ++i) {
		params.push_back(std::string(p[i]));
	}
	server->slotNumericMessageReceived(std::string(origin), event, params);
}

void irc_callback(irc_session_t *s, const char *e, const char *o, const char **params, unsigned int count) {
	dazeus::Server *server = (dazeus::Server*) irc_get_ctx(s);

	std::string event(e);
	// From libircclient docs, but CHANNEL_* is bullshit...
	if(event == "CHANNEL_NOTICE") {
		event = "NOTICE";
	} else if(event == "CHANNEL") {
		event = "PRIVMSG";
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
	fprintf(stderr, "%s - %s from %s\n", dazeus::Server::toString(server).c_str(), event.c_str(), origin.c_str());
#endif

	// TODO: handle disconnects nicely (probably using some ping and LIBIRC_ERR_CLOSED
	if(event == "ERROR") {
		fprintf(stderr, "Error received from libircclient; origin=%s.\n", origin.c_str());
		server->slotDisconnected();
	} else if(event == "CONNECT") {
		printf("Connected to server: %s\n", dazeus::Server::toString(server).c_str());
	}

	server->slotIrcEvent(event, origin, arguments);
}

void dazeus::Server::connectToServer()
{
	printf("Connecting to server: %s\n", toString(this).c_str());

	irc_callbacks_t callbacks;
	memset(&callbacks, 0, sizeof(irc_callbacks_t));
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
#if LIBIRC_VERSION_HIGH > 1 || LIBIRC_VERSION_LOW >= 6
	callbacks.event_channel_notice = irc_callback;
#endif
	callbacks.event_privmsg = irc_callback;
	callbacks.event_notice = irc_callback;
	callbacks.event_invite = irc_callback;
	callbacks.event_ctcp_req = irc_callback;
	callbacks.event_ctcp_rep = irc_callback;
	callbacks.event_ctcp_action = irc_callback;
	callbacks.event_unknown = irc_callback;
	callbacks.event_numeric = irc_eventcode_callback;

	irc_ = (void*)irc_create_session(&callbacks);
	if(!irc_) {
		std::cerr << "Couldn't create IRC session in Server.";
		abort();
	}
	irc_set_ctx(IRC, this);

	assert(!network_->config().nickName.empty());
	std::string host = config_.host;
	if(config_.ssl) {
#if defined(LIBIRC_OPTION_SSL_NO_VERIFY)
		host = "#" + host;
		if(!config_.ssl_verify) {
			std::cerr << "Warning: connecting without SSL certificate verification." << std::endl;
			irc_option_set(IRC, LIBIRC_OPTION_SSL_NO_VERIFY);
		}
#else
		std::cerr << "Error: Your version of libircclient does not support SSL. Failing connection." << std::endl;
		slotDisconnected();
		return;
#endif
	}
	irc_connect(IRC, host.c_str(),
		config_.port,
		network_->config().password.c_str(),
		network_->config().nickName.c_str(),
		network_->config().userName.c_str(),
		network_->config().fullName.c_str());
}
