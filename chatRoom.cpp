#include<iostream>
#include "chatRoom.hpp"

void Room::join(Participantptr participant){
    this->participants.insert(participant);
}

void Room:: leave(Participantptr participant){
    this->participants.erase(participant);
}

void Room::deliever(Participantptr participantPtr, Message &msg){
    msgQueue.push_back(msg);
    while(!msgQueue.empty()){
        Message message = msgQueue.front();
        msgQueue.pop_front();
        for(auto _participant : participants){
            if(_participant!=participantPtr){
                _participant->write(message);
            }
        }
    }
}


void Session::write(Message &msg){
    messageQueue.push_back(msg);
    while(!messageQueue.empty()){
        Message msg = messageQueue.front();
        messageQueue.pop_front();
        bool header_decoder_flag = msg.decodeHeader();
        if(header_decoder_flag){
            std::string body = msg.getBody();
            async_wite(body,msg.getBodyLength());
        }
        else{
            std::cout<<"Message length exceeds the max length"<<std::endl;
        }
    }
}

void Session:: deliever(Message &msg){
    room.deliever(shared_from_this(),msg);
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
            deliever(message);
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
void Session::async_wite(std::string mesgBody, size_t msgLen){
    auto write_handeler = [&](boost::system::error_code ec, std::size_t bytes_trasferred){
        if(!ec){
            std::cout<<"Data is writing to the socket"<<std::endl;
        }
        else{
            std::cerr<<"Write error: "<<ec.message()<<std::endl;
        }
    };
    boost::asio::async_write(clientSocket,boost::asio::buffer(mesgBody,msgLen),write_handeler);
}


void accept_connection(boost::asio::io_context &io, char * port, tcp::acceptor &acceptor, Room &room, const tcp::endpoint &endpoint){
    tcp::socket socket(io);
    acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket){
        if(!ec){
            std::shared_ptr<Session> session = std::make_shared<Session>(std::move(socket),room);
            session->start();
        }
        accept_connection(io,port,acceptor,room,endpoint);
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
    accept_connection(io_context,argv[1],acceptor,room,endpoint);
    io_context.run();
    }
    catch(const std::exception& e) {
        std::cout <<"Exception: "<<e.what()<<std::endl;
    }
    return 0;
}