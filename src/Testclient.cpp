//
// Created by phmoll on 1/30/20.
//

#include <ndn-cxx/face.hpp>
//#include <cnl-cpp/generalized-object/generalized-object-handler.hpp>

class Client {

public:
    Client() {}

    void start()
    {
        while (1) {
            this->face.processEvents();
        }
    }

protected:
    ndn::Face face;
};

int main(int argc, char* argv[])
{
    //    Client c;
    //    c.start();
    ndn::Face localFace = ndn::Face();
    while (1) {
        localFace.processEvents();
        usleep(10000);
    }
}