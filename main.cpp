#include "../headers/IrcServer.hpp"

int errorMsg(std::string msg)
{
    std::cout << msg << std::endl;
    return (1);
}

bool isInt(const char* str_nb)
{
    char* endptr = NULL;
    long nb = strtol(str_nb, &endptr, 10);

    while (*str_nb)
    {
        if (!isdigit(*str_nb))
            return (false);
        str_nb++;
    }
    if (*endptr != '\0' || nb < std::numeric_limits<int>::min() || nb > std::numeric_limits<int>::max())
        return (false);
        
    return (true);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        return (errorMsg("Error: Wrong number of arguments."));
    if (!isInt(argv[1]))
        return (errorMsg("Error: Invalid port number."));
    
    IrcServer server(argv[1], argv[2]);

    if (!server.validPort())
        return (errorMsg("Error: This port cannot be used."));
    server.handleConnections();

    return (0);
}