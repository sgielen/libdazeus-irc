/**
 * Copyright (c) Sjors Gielen, 2010
 * See LICENSE for license.
 */

#ifndef USER_H
#define USER_H

#include <string>

class User;
class Network;

class User {
  public:
            User( const std::string &fullName, Network *n );
            User( const std::string &nick, const std::string &ident, const std::string &host, Network *n );
    void    setFullName( const std::string &fullName );
    void    setNick( const std::string &nick );
    void    setIdent( const std::string &ident );
    void    setHost( const std::string &host );
    void    setNetwork( Network *n );
    const std::string &fullName() const;
    const std::string &nickName() const;
    const std::string &nick() const { return nickName(); }
    const std::string &ident() const;
    const std::string &host() const;
    Network *network() const;
    std::string toString() const;

    bool    isMe() const;
    bool    isMe( const std::string &network ) const;
    bool    isSameUser( const User &other ) const;

    static bool isSameUser( const User &one, const User &two );
    static bool isMe( const User &which, const std::string &network );

  private:
    // explicitly disable copy constructor
    User(const User&);
    void operator=(const User&);

    std::string nick_;
    std::string ident_;
    std::string host_;
    Network    *network_;
};

#endif
