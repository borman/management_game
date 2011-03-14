#ifndef PROTOCOL_H
#define PROTOCOL_H

class ProtocolFactory
{
  public:
    virtual Protocol *create(Connection &connection) = 0;
};

class Protocol
{
  public:
    virtual handleIncoming(

    // Commands
    virtual authPlayer(const std::string &name);
    virtual joinGame();
};

#endif

