#include "../headers/IrcServer.hpp"
#include "../headers/Colors.hpp"
#include "../headers/Channels.hpp"
#include "sstream"
#include "fcntl.h"

IrcServer::IrcServer(const char* _port, std::string _password): port(atoi(_port)), password(_password)
{
    struct pollfd fdserver;

    fdserver.fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    fdserver.events = POLLIN;
    fdserver.revents = 0;

    fd_sockets.push_back(fdserver);
}

IrcServer::~IrcServer() {}

bool IrcServer::validPort(void)
{
    struct sockaddr_in addrserver;

    if (port < 1 || port > 65535)
        return (false);

    addrserver.sin_family = AF_INET;
    addrserver.sin_addr.s_addr = INADDR_ANY;
    addrserver.sin_port = htons(port);

    if (bind(fd_sockets[0].fd, (struct sockaddr*)&addrserver, sizeof(addrserver)) < 0)
        return (false);

    addr_sockets.push_back(addrserver);
    return (true);
}

void IrcServer::handleConnections(void)
{
    unsigned int i;
    std::string msg;


    listen(fd_sockets[0].fd, SOMAXCONN);
    while (true)
    {
        if (poll(fd_sockets.data(), fd_sockets.size(), -1) > 0)
        {
            std::vector<int> to_delete;

            if (fd_sockets[0].revents == POLLIN)
            {
                if (acceptNewClient())
                {
                    std::string msg_send = BLUE ITALIC "[Welcome, please enter the password and your nickname]\n" END_STYLE;
                    send(fd_sockets.back().fd, msg_send.c_str(), msg_send.size(), 0);
                }
            }
            for (i = 1 ; i != fd_sockets.size() ; i++)
            {
                if (fd_sockets[i].revents == POLLIN)
                    readClientMsg(i, to_delete);
            }
            if (to_delete.size() > 0)
                deleteClients(to_delete);
        }
    }
}

bool IrcServer::acceptNewClient(void)
{
    struct sockaddr_in addr_client;
    struct pollfd fd_client;
    socklen_t len_addr = sizeof(addr_client);

    fd_client.fd = accept(fd_sockets[0].fd, (struct sockaddr*)&addr_client, &len_addr);
    int flag = fcntl(fd_client.fd, F_GETFL, 0);
    fcntl(fd_client.fd, F_SETFL, flag | O_NONBLOCK);
    if (fd_client.fd < 0)
    {
        std::cerr << RED ITALIC << "[Error: Unable to etablish connection with this client]\n" << END_STYLE << std::endl;
        return (false);
    }
    fd_client.events = POLLIN;
    fd_client.revents = 0;

    clients.push_back(new Client(fd_client, addr_client));
    fd_sockets.push_back(fd_client);
    addr_sockets.push_back(addr_client);

    return (true);
}

void IrcServer::readClientMsg(int _pos, std::vector<int>& _todelete)
{
    std::vector<char> msg(10);
    std::string         res;
    int nb_bytes;
    
    //voir ce que ca fait si on envoie juste un \n
    nb_bytes = recv(fd_sockets[_pos].fd, msg.data(), msg.size(), 0);
    if (!strchr(msg.data(), '\n'))
    {
        std::string buff = clients[_pos -1]->get_buff();
        buff += msg.data();
        clients[_pos -1]->set_buff(buff);
        return ;
    }
    if (nb_bytes < 0)
    {
        std::string msg_send = RED ITALIC "[Error: Try again please]\n" END_STYLE;
        send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0); //eventuellement gerer les cas specifiques de type : msg trop long
        return ;
    }
    else if (nb_bytes == 0)
    {
        _todelete.push_back(_pos);
        return;
    }
    res = clients[_pos -1]->get_buff() + msg.data();
    res[res.size() -1] = 0;
    res.erase(res.end() - 1);
    clients[_pos -1]->get_buff().erase();
    std::cout << "here1" << clients[_pos -1]->get_buff() << std::endl;
    std::cout << "here" << res << std::endl;
    if (clients[_pos -1]->getConnected() == false)
    {
        auth(res, _pos);
        bool    flag = false;
        if (clients[_pos -1]->getNickname().size() == 0 || clients[_pos -1]->get_hostname().size() == 0 || clients[_pos -1]->getValidPswd() == false)
        {
            std::string msg_send = BLUE ITALIC "[Required: " END_STYLE;
            send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
            flag = true;
        }
        if (clients[_pos -1]->getNickname().size() == 0)
        {
            std::string msg_send = BLUE ITALIC "[Nickname] " END_STYLE;
            send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
        }
        if (clients[_pos -1]->getValidPswd() == false)
        {
            std::string msg_send = BLUE ITALIC "[Password] " END_STYLE;
            send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
        }
        else if (clients[_pos -1]->get_username().size() == 0)
        {
            std::cout << "ok" << std::endl;
            std::string msg_send = BLUE ITALIC "[Username]" END_STYLE;
            send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
        }
        if (flag == true)
        {
            std::string msg_send = BLUE ITALIC "]\n" END_STYLE;
            send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
        }
    }
    else
    {
        switch (whichCommand(res))
        {
            case JOIN:
                join_channel(res, _pos);
                break;
            case PRIVMSG:
                checkPrivmsg(res, _pos);
                break;
            case MODE:
                checkMode(res, _pos);
                break;
            case INVITE:
                checkInvite(res, _pos);
                break;
            case KICK:
                checkKick(res, _pos);
                break;
            case TOPIC:
                checkTopic(res, _pos);
                break;
            default:
                std::string notif = BLUE ITALIC "[Available commands: JOIN, PRIVMSG, CPRIVMSG, MODE]\n" END_STYLE;
                send (fd_sockets[_pos].fd, notif.c_str(), notif.size(), 0);
        }
    }
}

void IrcServer::join_channel(std::string _command, int _pos)
{
    std::istringstream split_command(_command);
    std::vector<std::string> command;
    std::string token;
    std::string name;
    bool    flag = false;

    while(split_command >> token)
		command.push_back(token);
    if (command.size() < 2 || command.size() > 3)
    {
        std::string msg_send = RED ITALIC "[Error: syntax = JOIN channel_name psw]\n" END_STYLE;
        send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
        return ;
    }
    std::vector<Channel*>::iterator it = this->channels.begin();
    std::cout << command.size() << std::endl;
    while (it != this->channels.end())
    {
        if ((*it)->getname() == command[1])
        {
            if ((*it)->getNeedPassword() == true && (*it)->getOnInviteOnly() == false)
            {
                if (command.size() == 3)
                {
                    if ((*it)->getPassword() != command[3])
                    {
                        std::string msg = ITALIC RED "[Wrong channel password please retry]\n" END_STYLE;
                        send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
                        return ;
                    }
                }
                else
                {
                    std::string msg = ITALIC RED "[This channel need a psw]\n" END_STYLE;
                    send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
                    return ;
                }
            }
            if ((*it)->getOnInviteOnly() == true)
            {
                if (clients[_pos -1]->check_if_invited((*it)->getname()) == false)
                {
                    std::string msg = ITALIC RED "[this channel is on invitation only]\n" END_STYLE;
                    send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
                    return ;
                }
            }
            else if (command.size() > 2)
            {
                std::string msg = ITALIC RED "[Warning: this channel is public, no psw needed]\n" END_STYLE;
                send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
            }
            std::string notif = ITALIC PURPLE "[" + command[1] + " channel joined]\n" + END_STYLE;
            std::string notif2 = ITALIC PURPLE "[" + this->clients[_pos -1]->getNickname() + " joined the channel]\n" + END_STYLE;
            if ((*it)->add_participant(this->clients[_pos -1]) == true)
            {
                send(fd_sockets[_pos].fd, notif.c_str(), notif.size(), 0);
                (*it)->send_to_channel(notif2);
            }
            flag = true;
        }
        it++;
    }
    if (flag == false)
    {
        Channel *to_add = new Channel(command[1], this->clients[_pos -1]);
        this->channels.push_back(to_add);
        this->clients[_pos - 1]->addOperatorChannels(to_add);
        std::string notif = PURPLE ITALIC "[" + command[1] + " channel joined]\n" + END_STYLE;
        send(fd_sockets[_pos].fd, notif.c_str(), notif.size(), 0);
    }
}

void IrcServer::deleteClients(std::vector<int>& _todelete)
{
    std::vector<int>::iterator it;

    for (it = _todelete.begin() ; it != _todelete.end() ; it++)
    {
        close(fd_sockets[*it].fd);
        fd_sockets.erase(fd_sockets.begin() + *it);
        addr_sockets.erase(addr_sockets.begin() + *it);
        delete clients[*it - 1];
        clients.erase(clients.begin() + (*it - 1));
    }
}

int IrcServer::whichCommand(std::string _msg)
{
    if (_msg.size() >= 4 && _msg.substr(0, 4) == "PASS")
        return (PASS);
    else if (_msg.size() >= 4 && _msg.substr(0, 4) == "NICK")
        return (NICK);
    else if (_msg.size() >= 7 && _msg.substr(0, 7) == "PRIVMSG")
        return (PRIVMSG);
    else if (_msg.size() >= 4 && _msg.substr(0, 4) == "JOIN")
        return (JOIN);
    else if (_msg.size() >= 4 && _msg.substr(0, 4) == "MODE")
        return (MODE);
    else if (_msg.size() >= 6 && _msg.substr(0, 6) == "INVITE")
        return (INVITE);
    else if (_msg.size() >= 4 && _msg.substr(0, 4) == "KICK")
        return (KICK);
    else if (_msg.size() >= 5 && _msg.substr(0, 5) == "TOPIC")
        return (TOPIC);
    else if (_msg.size() >= 4 && _msg.substr(0, 4) == "USER")
        return (USER);
    else
        return (INVALID_COMMAND);
}