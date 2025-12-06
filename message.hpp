#include<iostream>
#include<memory>
#ifndef MESSAGE_HPP
#define MESSAGE_HPP
#include <cstring>
class Message{
    public:
        Message() : bodyLength_{0}{
        }
        Message(std::string message){
            bodyLength_ = message.size()>maxBytes?maxBytes:message.size();
            encodeHeader();
            memcpy(data+header,message.c_str(),bodyLength_);
        }
        size_t getBodyLength(){
            return bodyLength_;
        }
        void encodeHeader(){
            char new_header[header+1] = "";
            sprintf(new_header,"%4d", int(bodyLength_));
            memcpy(data,new_header,header);
        }   
        std::string getData(){
            int length = header+bodyLength_;
            std::string result(data,length);
            return result;
        }
        std::string getBody(){
            std::string fullData = getData();
            std::string message = fullData.substr(header,bodyLength_);
            return message;
        }
        bool decodeHeader(){
            char newHeader[header+1] = "";
            strncpy(newHeader,data,header);
            newHeader[header] = '\0';
            int newHeaderValue = atoi(newHeader);
            if(newHeaderValue>maxBytes)
                return false;
            bodyLength_ = newHeaderValue;
            return true;

        }
    private:
        enum {header = 4};
        enum {maxBytes = 512};
        char data[header+maxBytes];
        size_t bodyLength_;
        
    
};

#endif