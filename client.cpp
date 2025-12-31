#include <iostream>
#include <thread>
#include "message.hpp"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

void print_prompt() {
    std::cout << "> " << std::flush;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "[system] Usage: client <port>\n";
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
                        std::cerr << "\r[system] Invalid message header\n";
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

                    std::cout << "\r[chat] " << msg.getBody() << "\n";
                    print_prompt();

                }
            } catch (...) {
                std::cout << "\r[system] Disconnected from server\n";
            }
        });

        // ---------- Writer loop (framed write) ----------
        while (true) {
            std::string input;
            print_prompt();
            std::getline(std::cin, input);
            if (input == "/exit") {
                std::cout << "[system] Exiting chat...\n";
                socket.close();
                break;
            }
            if (input == "/help") {
                std::cout <<
                    "\nCommands:\n"
                    "  /exit   Quit the chat\n"
                    "  /help   Show this help\n\n";
                continue;
            }

            if (input.empty())
                continue;

            Message msg(input);

            boost::asio::write(
                socket,
                boost::asio::buffer(
                    msg.mutableData(),
                    Message::header_size + msg.getBodyLength()
                )
            );
        }

        reader.detach();
    }
    catch (const std::exception& e) {
        std::cerr << "\r[system] Client error: " << e.what() << std::endl;
    }

    return 0;
}
