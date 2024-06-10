#include <catch2/catch_test_macros.hpp>
#include "Host.h"
#include <thread>

TEST_CASE("Host Constructor"){
    SECTION("Initialize with valid parameters"){
        bool sucess = false;
        try{
            addressData hostAddress("127.0.0.1", 8001);
            Host Host(hostAddress);
            sucess = true;
        } catch(...){
            sucess = false;
        }
        
        REQUIRE(sucess == true);
}
}

TEST_CASE("Host Start"){
    SECTION("Start Host"){
    std::cout << "Starting " << std::endl;

    auto thread_1 = std::jthread([] () {
        std::cout << "Thread 1" << std::endl;
        addressData hostAddress("127.0.0.1", 8001);
        Host host(hostAddress);
        std::cout << "Host created" << std::endl;
        host.waitForHandshake();
        std::cout << "Connected" << std::endl;
        REQUIRE(host.isConnected() == true);
        host.stopHost();
        }
    );
    
    auto thread_2 = std::jthread([] (){
        std::cout << "Thread 2" << std::endl;
        addressData hostAddress("127.0.0.1", 8010);
        Host host(hostAddress);
        addressData remoteAdress("127.0.0.1", 8001);
        host.sendHandshake(remoteAdress);
        std::cout << "Sent connection request" << std::endl;
        host.stopHost();
        
    } 
    );
    thread_1.join();
    thread_2.join();
    std::cout << "Threads joined" << std::endl;
    }
}


