/**
 * Copyright (c) Sjors Gielen, 2010-2012
 * See LICENSE for license.
 */

#include "user.h"
#include "network.h"

User::User( const std::string &fullName, Network *n )
: nick_()
, ident_()
, host_()
, network_(n)
{
  setFullName( fullName );
}

User::User( const std::string &nick, const std::string &ident, const std::string &host, Network *n )
: nick_(nick)
, ident_(ident)
, host_(host)
, network_(n)
{
}

std::string User::toString() const
{
  return "User[" + nickName() + "]";
}


const std::string &User::host() const
{
  return host_;
}


const std::string &User::ident() const
{
  return ident_;
}


bool User::isMe() const
{
  return network()->user()->nick() == nick();
}


Network *User::network() const
{
  return network_;
}


const std::string &User::nickName() const
{
  return nick_;
}

void User::setHost( const std::string &host )
{
  host_ = host;
}


void User::setFullName( const std::string &fullName )
{
  size_t pos1 = fullName.find('!');
  size_t pos2 = fullName.find('@', pos1 == std::string::npos ? 0 : pos1 );

  // left(): The entire string is returned if n is [...] less than zero.
  setNick( fullName.substr(0, pos1 != std::string::npos ? pos1 : pos2 ) );
  setIdent( pos1 == std::string::npos ? std::string() : fullName.substr( pos1 + 1, pos2 - pos1 - 1 ) );
  setHost(  pos2 == std::string::npos ? std::string() : fullName.substr( pos2 + 1 ) );
  //setIdent( fullName.mid( pos1 + 1 ) );
  //setIdent( fullName.mid( pos1 + 1 ), pos2 - pos1 - 1 );
}


void User::setIdent( const std::string &ident )
{
  ident_ = ident;
}


void User::setNetwork( Network *n )
{
  network_ = n;
}


void User::setNick( const std::string &nick )
{
  nick_ = nick;
}
