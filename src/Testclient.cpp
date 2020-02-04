//
// Created by phmoll on 1/30/20.
//

#include <iostream>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
//#include <cnl-cpp/generalized-object/generalized-object-handler.hpp>

class Client {

public:
    Client() {}

    void start()
    {

        static const std::string content("Hello, world!");

        // Create Data packet
        auto data = std::make_shared<ndn::Data>();
        data->setName("/just/a/name");
//        data->setFreshnessPeriod(10_s);
        data->setContent(reinterpret_cast<const uint8_t*>(content.data()), content.size());

        // Sign Data packet with default identity
        keyChain.sign(*data);
        // m_keyChain.sign(*data, signingByIdentity(<identityName>));
        // m_keyChain.sign(*data, signingByKey(<keyName>));
        // m_keyChain.sign(*data, signingByCertificate(<certName>));
        // m_keyChain.sign(*data, signingWithSha256());

        // Return Data packet to the requester
        std::cout << "<< D: " << data << std::endl;
        this->face.put(*data);

        while (1) {
            this->face.processEvents();
        }
    }

protected:
    ndn::Face face;
    ndn::KeyChain keyChain;
};

int main(int argc, char* argv[])
{
        Client c;
        c.start();
//    ndn::Face localFace = ndn::Face();
//    while (1) {
//        localFace.processEvents();
//        usleep(10000);
//    }
}