#include<iostream>
#include "chatRoom.hpp"

void Room::join(Participantptr participant){
    this->participants.insert(participant);
}

void Room:: leave(Participantptr participant){
    this->participants.erase(participant);
}

void Room::deliver(Participantptr participantPtr, Message &msg){
    for(auto _participant : participants){
        if(_participant!=participantPtr){
            _participant->write(msg);
        }
}
}


void Session::write(Message &msg){
    messageQueue.push_back(msg);
    if(writeInProgress)
        return;
    writeInProgress = true;
    Message& front = messageQueue.front();
    async_write(front);
}

void Session:: deliver(Message &msg){
    room.deliver(shared_from_this(),msg);
}
void Session:: start(){
    room.join(shared_from_this());
    async_read_header();
}
using boost::asio::ip::address_v4;
void Session::async_read_header() {
    auto self(shared_from_this());

    boost::asio::async_read(
        clientSocket,
        boost::asio::buffer(readMsg.mutableData(), Message::header_size),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec && readMsg.decodeHeader()) {
                async_read_body();
            } else {
                room.leave(self);
            }
        }
    );
}
void Session::async_read_body() {
    auto self(shared_from_this());

    boost::asio::async_read(
        clientSocket,
        boost::asio::buffer(
            readMsg.mutableData() + Message::header_size,
            readMsg.getBodyLength()
        ),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                deliver(readMsg);
                async_read_header();  // loop
            } else {
                room.leave(self);
            }
        }
    );
}

Session::Session(tcp::socket s, Room& r): clientSocket(std::move(s)), room(r){};

void Session::async_write(Message& msg) {
    auto self(shared_from_this());
    boost::asio::async_write(
        clientSocket,
        boost::asio::buffer(msg.mutableData(),Message::header_size + msg.getBodyLength()),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                messageQueue.pop_front();
                if (!messageQueue.empty()) {
                    async_write(messageQueue.front());
                } else {
                    writeInProgress = false;
                }
            } else {
                room.leave(self);
            }
        }
    );
}



void accept_connection( tcp::acceptor &acceptor, Room &room){
    acceptor.async_accept([&room, &acceptor](boost::system::error_code ec, tcp::socket socket){
        if(!ec){
            std::shared_ptr<Session> session = std::make_shared<Session>(std::move(socket),room);
            session->start();
        }
        accept_connection(acceptor,room);
    });
}

int main(int argc, char * argv[]){
    try{
        if(argc<2){
            std::cerr<<"Usage: server <port>";
            return 1;
        }
    Room room; 
    boost::asio::io_context io_context;
    tcp::endpoint endpoint(tcp::v4(), atoi(argv[1]));
    tcp::acceptor acceptor (io_context, endpoint);
    accept_connection(acceptor,room);
    io_context.run();
    }
    catch(const std::exception& e) {
        std::cout <<"Exception: "<<e.what()<<std::endl;
    }
    return 0;
}