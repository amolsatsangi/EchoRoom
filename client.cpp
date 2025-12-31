#include <iostream>
#include <thread>
#include "message.hpp"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: client <port>\n";
        return 1;
    }

    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        tcp::resolver resolver(io_context);

        boost::asio::connect(
            socket,
            resolver.resolve("127.0.0.1", argv[1])
        );

        // ---------- Reader thread (framed read) ----------
        std::thread reader([&]() {
            try {
                while (true) {
                    Message msg;

                    // Read header
                    boost::asio::read(
                        socket,
                        boost::asio::buffer(
                            msg.mutableData(),
                            Message::header_size
                        )
                    );

                    if (!msg.decodeHeader()) {
                        std::cerr << "Invalid message header\n";
                        break;
                    }

                    // Read body
                    boost::asio::read(
                        socket,
                        boost::asio::buffer(
                            msg.mutableData() + Message::header_size,
                            msg.getBodyLength()
                        )
                    );

                    std::cout << "Server: " << msg.getBody() << std::endl;
                }
            } catch (...) {
                std::cout << "Disconnected from server\n";
            }
        });

        // ---------- Writer loop (framed write) ----------
        while (true) {
            std::string input;
            std::cout << "Enter message: ";
            std::getline(std::cin, input);

            Message msg(input);

            boost::asio::write(
                socket,
                boost::asio::buffer(
                    msg.mutableData(),
                    Message::header_size + msg.getBodyLength()
                )
            );
        }

        reader.join();
    }
    catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }

    return 0;
}
