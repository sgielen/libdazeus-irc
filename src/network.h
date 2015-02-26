/**
 * Copyright (c) Sjors Gielen, 2010-2012
 * See LICENSE for license.
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <sys/select.h> /* fd_set */
#include <string>
#include <map>
#include <memory>

namespace dazeus {

class Network;
class Server;

struct ServerConfig;
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

__attribute__((deprecated))
typedef std::shared_ptr<NetworkConfig> NetworkConfigPtr;

class NetworkListener
{
  public:
    virtual ~NetworkListener() {}
    virtual void ircEvent(const std::string &event, const std::string &origin,
                          const std::vector<std::string> &params, Network *n ) = 0;
};

class Network
{

  friend class Server;

  public:
    __attribute__((deprecated))
    Network( const NetworkConfigPtr c );

    Network(const NetworkConfig &c);
    ~Network();

    void resetConfig(const NetworkConfig &c);

    static std::string toString(const Network *n);
    void               addListener( NetworkListener *nl ) {
      networkListeners_.push_back(nl);
    }

    enum DisconnectReason {
      UnknownReason,
      ShutdownReason,
      ConfigurationReloadReason,
      SwitchingServersReason,
      ErrorReason,
      TimeoutReason,
      AdminRequestReason
    };

    enum ChannelMode {
      UserMode = 0,
      OpMode = 1,
      HalfOpMode = 2,
      VoiceMode = 4,
      UnknownMode = 8,
      OpAndVoiceMode = OpMode | VoiceMode
    };

    bool                        autoConnectEnabled() const;
    const std::vector<ServerConfigPtr> &servers() const;
    std::string                 nick() const;
    Server                     *activeServer() const;
    const NetworkConfig        &config() const;
    int                         serverUndesirability( const ServerConfig &sc ) const;
    std::string                 networkName() const;
    std::vector<std::string>    joinedChannels() const;
    std::map<std::string,std::string> topics() const;
    std::map<std::string,ChannelMode> usersInChannel(std::string channel) const;
    bool                        isIdentified(const std::string &user) const;
    bool                        isKnownUser(const std::string &user) const;

    void connectToNetwork( bool reconnect = false );
    void disconnectFromNetwork( DisconnectReason reason = UnknownReason );
    void checkTimeouts();
    void joinChannel( std::string channel );
    void leaveChannel( std::string channel );
    void say( std::string destination, std::string message );
    void notice( std::string destination, std::string message );
    void action( std::string destination, std::string message );
    void names( std::string channel );
    void ctcp( std::string destination, std::string message );
    void ctcpReply( std::string destination, std::string message );
    void sendWhois( std::string destination );
    void addDescriptors(fd_set *in_set, fd_set *out_set, int *maxfd);
    void processDescriptors(fd_set *in_set, fd_set *out_set);
    void run();
    static void run(std::vector<Network*> networks);

  private:
    // explicitly disable copy constructor
    Network(const Network&);
    void operator=(const Network&);

    void flagUndesirableServer( const ServerConfig &sc );
    void serverIsActuallyOkay( const ServerConfig &sc );
    void connectToServer( ServerConfigPtr conf, bool reconnect );

    Server               *activeServer_;
    NetworkConfig config_;
    std::map<std::string,int> undesirables_;
    bool                  deleteServer_;
    std::vector<std::string>        identifiedUsers_;
    std::map<std::string,std::vector<std::string> > knownUsers_;
    std::map<std::string,std::string> topics_;
    std::vector<NetworkListener*>   networkListeners_;
    std::string           nick_;
    time_t deadline_;
    time_t nextPongDeadline_;

    void onFailedConnection();
    void joinedChannel(const std::string &user, const std::string &receiver);
    void kickedChannel(const std::string &user, const std::string&, const std::string&, const std::string &receiver);
    void partedChannel(const std::string &user, const std::string &, const std::string &receiver);
    void slotQuit(const std::string &origin, const std::string&, const std::string &receiver);
    void slotWhoisReceived(const std::string &origin, const std::string &nick, bool identified);
    void slotNickChanged( const std::string &origin, const std::string &nick, const std::string &receiver );
    void slotNamesReceived(const std::string&, const std::string&, const std::vector<std::string> &names, const std::string &receiver );
    void slotTopicChanged(const std::string&, const std::string&, const std::string&);
    void slotIrcEvent(const std::string&, const std::string&, const std::vector<std::string>&);
};

}

#endif
