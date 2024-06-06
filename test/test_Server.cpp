#include <catch2/catch_test_macros.hpp>
#include "Server.h"
#include <thread>

TEST_CASE("Server Constructor"){
    SECTION("Initialize with valid parameters"){
        bool sucess = false;
        try{
            Server server("127.0.0.1", 8001);
            sucess = true;
        } catch(...){
            sucess = false;
        }
        
        REQUIRE(sucess == true);
}
}

TEST_CASE("Server Start"){
    SECTION("Start server"){
    std::cout << "Starting " << std::endl;

    auto thread_1 = std::jthread([] () {
        std::cout << "Thread 1" << std::endl;
        Server server("127.0.0.1", 8001);
        std::cout << "Server created" << std::endl;
        server.waitForConnection();
        std::cout << "Connected" << std::endl;
        REQUIRE(server.isConnected() == true);
        }
    );
    
    auto thread_2 = std::jthread([] (){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Thread 2" << std::endl;
        Server client("127.0.0.1", 8010);
        client.inititalizeConnection("127.0.0.1", 8001);
        std::cout << "Sent connection request" << std::endl;
        
    } 
    );
    thread_1.join();
    thread_2.join();
    std::cout << "Threads joined" << std::endl;
    }
}


