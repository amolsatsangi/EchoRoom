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
    bool header_decoder_flag  = front.decodeHeader();
    if(!header_decoder_flag ){
        std::cout << "Message length exceeds max\n";
        messageQueue.pop_front();
        writeInProgress = false;
        return;
    }
    async_write(front.getBody(), front.getBodyLength());
}

void Session:: deliver(Message &msg){
    room.deliver(shared_from_this(),msg);
}
void Session:: start(){
    room.join(shared_from_this());
    async_read();
}
using boost::asio::ip::address_v4;
void Session::async_read(){
    auto self(shared_from_this());
    boost::asio::async_read_until(clientSocket, buffer,"\n", [this,self](boost::system::error_code ec, std::size_t bytes_transferred){
        if(!ec){
            std::string data(boost::asio::buffers_begin(buffer.data()),boost::asio::buffers_begin(buffer.data())+bytes_transferred);

            buffer.consume(bytes_transferred);
            std::cout<<"Recieved: "<<data<<std::endl;
            Message message(data);
            deliver(message);
            async_read();
        }
        else{
            room.leave(shared_from_this());
            if(ec==boost::asio::error::eof){
                std::cout<<"Connection closed by peer"<<std::endl;
            }
            else{
                std::cout<<"Read error: "<<ec.message()<<std::endl;
            }
        }

    });
}
Session::Session(tcp::socket s, Room& r): clientSocket(std::move(s)), room(r){};

void Session::async_write(std::string mesgBody, size_t msgLen){
    auto self(shared_from_this());
    boost::asio::async_write(
        clientSocket,
        boost::asio::buffer(mesgBody, msgLen),
        [this, self, mesgBody](boost::system::error_code ec, std::size_t){
            if(!ec){
                messageQueue.pop_front();
                if(!messageQueue.empty()){
                    Message& next = messageQueue.front();
                    async_write(next.getBody(), next.getBodyLength());
                } else {
                    writeInProgress = false;
                }
            } else {
                room.leave(shared_from_this());
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