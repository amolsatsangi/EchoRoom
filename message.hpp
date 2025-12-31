#ifndef MESSAGE_HPP
#define MESSAGE_HPP
#include<array>
#include <cstring>
#include <string>
#include <cstdio>
#include <cstdlib>

class Message{
    public:
        Message() : bodyLength_{0}{
        }
        
        explicit Message(std::string message){
            bodyLength_ = message.size()>max_body_size?max_body_size:message.size();
            encodeHeader();
            memcpy(data_.data()+header_size,message.c_str(),bodyLength_);
        }
        size_t getBodyLength() const{
            return bodyLength_;
        } 
        std::string getData() const{
            int length = header_size+bodyLength_;
            std::string result(data_.data(),length);
            return result;
        }
        std::string getBody() const{
            std::string fullData = getData();
            std::string message = fullData.substr(header_size,bodyLength_);
            return message;
        }
        bool decodeHeader(){
            char newHeader[header_size+1] = "";
            std::memcpy(newHeader, data_.data(), header_size);
            newHeader[header_size] = '\0';
            std::size_t newHeaderValue = static_cast<std::size_t>(std::atoi(newHeader));
            if(newHeaderValue>max_body_size)
                return false;
            bodyLength_ = newHeaderValue;
            return true;

        }
        char* mutableData(){
           return data_.data();
        }

    private:
        static constexpr std::size_t header_size = 4;
        static constexpr std::size_t max_body_size = 512;
        std::array<char, header_size + max_body_size> data_{};
        size_t bodyLength_;

        void encodeHeader(){
            std::snprintf(data_.data(), header_size + 1, "%04zu", bodyLength_);
        }  
};
#endif