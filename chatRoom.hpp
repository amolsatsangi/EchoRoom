#include<iostream>
#include "message.hpp"
#include<set>
#include<memory>
#include<sys/socket.h>
#include<unistd.h>
#include<boost/asio.hpp>
#include<deque>



#ifndef CHATROOM_HPP
#define CHATROOM_HPP

using boost::asio::ip::tcp;

class Participant{
    public:
        virtual void deliver(Message &msg) = 0;
        virtual void write(Message &msg) = 0;
        virtual ~Participant() = default;
};

typedef std::shared_ptr<Participant> Participantptr;


class Room{
    public:
        void join(Participantptr participant);
        void leave(Participantptr participant);
        void deliver(Participantptr participantPtr, Message &msg);
        
    private:
        enum{MaxParticipant = 100};
        std::set<Participantptr> participants;
};

class Session: public Participant, public std::enable_shared_from_this<Session>{
    public:
        Session(tcp::socket s, Room &room);
        void start();
        void write(Message &msg);
        void deliver(Message &msg);
        void async_read();
        void async_write(std::string mesgBody, size_t bodyLength);
    private:
        tcp::socket clientSocket;
        boost::asio::streambuf buffer;
        Room &room;
        std::deque<Message> messageQueue;
};
#endif