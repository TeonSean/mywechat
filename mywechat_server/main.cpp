#include"server.h"

int main(int argc, char *argv[])
{
    Server* server = Server::getInstance();
    server->loop();
    return 0;
}
