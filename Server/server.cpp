#include <udpdk_api.h>

int main()
{
    int socket = udpdk_socket(AF_INET, SOCK_STREAM, 0);
}