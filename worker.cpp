#include <functional>
#include <iostream>
#include <string>

#include "messages.hpp"
#include "zmq.hpp"

namespace tasker {
class Worker {
   private:
    std::string SPACE = " ";
    zmq::socket_t *socket;
    const std::string &id;
    std::function<void(std::string)> on_message;

   public:
    Worker(const std::string &id) : id(id) {
    }

    void SetOnMessage(const std::function<void(std::string)> &on_message) {
        this->on_message = on_message;
    }

    void Send(const std::string &msg) {
        std::cout << "Sending message : " << msg << std::endl;
        zmq::message_t message(msg.size());
        std::memcpy(message.data(), msg.data(), msg.size());
        socket->send(message, zmq::send_flags::none);
    }

    int Start() {
        //connect to the driver
        zmq::context_t ctx{1};  // 1 IO thread

        this->socket = new zmq::socket_t{ctx, zmq::socket_type::dealer};
        this->socket->setsockopt(ZMQ_IDENTITY, this->id.c_str(), 7);

        std::cout << "Connecting to the driver..." << std::endl;
        this->socket->connect("tcp://localhost:5050");

        if (socket->connected()) {
            std::cout << "Connected to the server..." << std::endl;
        } else {
            std::cout << "Not connected to the server..." << std::endl;
            return 500;
        }

        // sending join message
        std::string cmd = tasker::GetCommand(tasker::JOIN);
        cmd.append(SPACE);
        cmd.append(this->id);
        Send(cmd);

        // now start continuous listening
        while (true) {
            zmq::message_t request;

            // receive a request from client
            std::cout << "Waiting for command.." << std::endl;
            socket->recv(request, zmq::recv_flags::none);

            std::string msg = request.to_string();
            std::string cmd = msg.substr(0, 3);
            std::string params = msg.substr(4, msg.length());

            int status = -1;
            if (tasker::GetCommand(tasker::Commands::MESSAGE).compare(cmd) == 0) {
            } else {
                std::cout << "Unknown message : " << msg << std::endl;
            }
        }
        return 0;
    }
};
}  // namespace tasker

int main(int argc, char *argv[]) {
    tasker::Worker worker(argv[1]);
    worker.SetOnMessage([](std::string msg) {
        std::cout << "Message received from server : " << msg << std::endl;
    });
    worker.Start();
    return 0;
}