/**
 * Copyright (c) Sjors Gielen, 2010-2014
 * See LICENSE for license.
 */

#ifndef DAZEUS_CONFIG_H
#define DAZEUS_CONFIG_H

#include <vector>
#include <string>

namespace dazeus {

struct ServerConfig {
  ServerConfig() : port(6667), priority(5), ssl(false), ssl_verify(true) {}

  std::string toString() const;

  std::string host;
  uint16_t port;
  uint8_t priority;
  bool ssl;
  bool ssl_verify;
};

typedef std::shared_ptr<ServerConfig> ServerConfigPtr;

struct NetworkConfig {
  NetworkConfig() : nickName("DaZeus"), userName("dazeus"),
      fullName("DaZeus"), autoConnect(false), connectTimeout(10), pongTimeout(30) {}

  std::string name;
  std::string displayName;
  std::string nickName;
  std::string userName;
  std::string fullName;
  std::string password;
  std::vector<ServerConfigPtr> servers;
  bool autoConnect;
  time_t connectTimeout;
  time_t pongTimeout;
};

typedef std::shared_ptr<NetworkConfig> NetworkConfigPtr;

}

#endif
