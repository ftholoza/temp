#include "../headers/Client.hpp"

//est ce qu'on considere qu'un client est connecte une fois qu'il est connecte au port? ou quand il a mis le bon mdp?

Client::Client(struct pollfd _fdclient, struct sockaddr_in _addrclient) : connected(false), valid_pswd(false), fd_client(_fdclient), addr_client(_addrclient) {}

Client::~Client() {}

std::string Client::getNickname(void)
{
    return (nickname);
}

bool Client::getConnected(void)
{
    return (connected);
}

bool Client::getValidPswd(void)
{
    return (valid_pswd);
}

struct pollfd& Client::getFdClient(void)
{
    return (fd_client);
}

struct sockaddr_in& Client::getAddrClient(void)
{
    return (addr_client);
}

std::vector<Channel*>   Client::getOperatorChannels(void)
{
    return (operator_channels);
}

void Client::setNickname(std::string _nickname)
{
    nickname = _nickname;
}

void Client::setConnected(bool _connected)
{
    connected = _connected;
}

void Client::setValidPswd(bool _validpswd)
{
    valid_pswd = _validpswd;
}

void Client::addOperatorChannels(Channel* _channel)
{
    operator_channels.push_back(_channel);
}

void Client::removeOperatorChannels(Channel* _channel)
{
    std::vector<Channel*>::iterator it;

    for (it = operator_channels.begin() ; it != operator_channels.end() ; it++)
    {
        if (*it == _channel)
        {
            operator_channels.erase(it);
            return ;
        }
    }
}

void Client::addInvitationsChannels(std::string _channel)
{
    invitations_channels.push_back(_channel);
}

std::vector<std::string> Client::getInvitationsChannels()
{
    return (this->invitations_channels);
}

bool Client::operatorOfThisChannel(std::string _channel)
{
    for(unsigned int i = 0 ; i < operator_channels.size() ; i++)
    {
        if (operator_channels[i]->getname() == _channel)
            return (true);
    }
    return (false);
}

bool Client::check_if_invited(std::string _channel)
{
    for (unsigned int  i = 0; i != this->getInvitationsChannels().size(); i++)
    {
        if (this->getInvitationsChannels()[i] == _channel)
            return (true);
    }
    return (false);
}

std::string	&Client::get_buff()
{
    return (this->buffer);
}
		
void	Client::set_buff(std::string _buff)
{
    this->buffer = _buff;
}

std::string Client::get_username()
{
    return (this->username);
}

std::string Client::get_hostname()
{
    return (this->hostname);
}

std::string Client::get_servername()
{
    return (this->servername);
}

std::string Client::get_realname()
{
    return (this->realname);
}

void		Client::set_username(std::string _username)
{
    this->username = _username;
}

void		Client::set_hostname(std::string _hostname)
{
    this->hostname = _hostname;
}

void		Client::set_servername(std::string _servername)
{
    this->servername = _servername;
}

void    Client::set_realname(std::string _realname)
{
    this->realname = _realname;
}