#include "../headers/IrcServer.hpp"
#include "../headers/Colors.hpp"
#include "../headers/Channels.hpp"
#include "../headers/Error.hpp"
#include "sstream"
#include "fcntl.h"

void    IrcServer::auth(std::string msg, int _pos)
{
    std::string msg_send;
    switch (whichCommand(msg.data()))
    {
        case PASS:
            checkPassword(msg.data(), _pos);
            break;
        case NICK:
            checkNickname(msg.data(), _pos);
            break;
		case USER:
			checkUser(msg.data(), _pos);
			break;
        default:
            msg_send = RED ITALIC "[Invalid command: Availeble commands: PASS, NICK]\n" END_STYLE;
            send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
    }
    if (clients[_pos -1]->getNickname().size() > 0 && clients[_pos -1]->getValidPswd() == true && clients[_pos -1]->get_username().size() > 0)
    {
        msg_send = GREEN ITALIC "[You are now connected]\n" END_STYLE;
        clients[_pos - 1]->setConnected(true);
        send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
    }
}

void IrcServer::checkPassword(std::string _msg, int _pos)
{
    if (_msg.size() == 5 + password.size() && _msg.substr(5, password.size()) == password)
    {
        std::string msg_send = GREEN ITALIC "[Valid password]\n" END_STYLE;
        clients[_pos - 1]->setValidPswd(true);
        send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
    }
    else
    {
        std::string msg_send = RED ITALIC "[Wrong password please retry]\n" END_STYLE;
        send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
    }
}

void IrcServer::checkUser(std::string _msg, int _pos)
{
	std::istringstream split_msg(_msg);
	std::string token;
	std::string msg_send;
	std::string	name;
	std::vector<std::string> command;


	while(split_msg >> token)
	command.push_back(token);

	if (command.size() < 5)
	{
		msg_send = ERR_NEEDMOREPARAMS(clients[_pos -1]->getNickname(), "USER");
		send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
		return ;
	}
	if (check_names(command[1]) == false)
	{
		//err
		return;
	}
	if (command[4].find(':') == std::string::npos)
	{
		//err
		std::cout << "ok" << std::endl;
		return;
	}
	if (usernameUsed(command[1]) == false && realnameUsed(name))
	{
		clients[_pos -1]->set_username(command[1]);
		clients[_pos -1]->set_hostname("0");
		clients[_pos -1]->set_servername("0");
		clients[_pos -1]->set_realname(name);
		msg_send = GREEN ITALIC "[USER: setled]\n" END_STYLE;
	}
	return;
}

void IrcServer::checkNickname(std::string _msg, int _pos)
{
    std::string nick;

    if (clients[_pos -1]->getValidPswd() == false)
    {
        std::string msg_send = BLUE ITALIC "[Please enter password before]\n" END_STYLE;
        send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
        return ;
    }
    if (_msg.size() >= 5)
       nick = _msg.substr(5, _msg.size() - 5);
    if (nick.size() >= 3 && nick.size() <= 20)
    {
        if (!invalidNicknameCharacter(nick))
        {
            if (!nicknameAlreadyUsed(nick))
            {
                std::string msg_send = GREEN ITALIC "[Your nickname has been setled]\n" END_STYLE;
                send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
                clients[_pos - 1]->setNickname(nick);
            }
            else
            {
                std::string msg_send = BLUE ITALIC "[Sorry: this nickname is already used]\n" END_STYLE;
                send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
            }
        }
        else if (invalidNicknameCharacter(nick))
        {
            std::string msg_send = RED ITALIC "[Error: Your nickname must not contain any special characters other than '-' and '_']\n" END_STYLE;
            send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
        }
        else
        {
            std::string msg_send = GREEN ITALIC "[Your nickname has been setled]\n" END_STYLE;
            send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
        }
    }
    else
    {
        std::string msg_send = RED ITALIC "[Error: Your nickname must be between 3 and 20 characters long]\n" END_STYLE;
        send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
    }
}

bool IrcServer::invalidNicknameCharacter(std::string _nickname)
{
    unsigned int i;

    for (i = 0 ; i < _nickname.size() ; i++)
    {
        if (!isalnum(_nickname[i]) && _nickname[i] != '-' && _nickname[i] != '_')
            return (true);  
    }

    return(false);
}

bool IrcServer::nicknameAlreadyUsed(std::string _nickname)
{
    std::vector<Client*>::iterator it;

    for (it = clients.begin() ; it != clients.end() ; it++)
    {
        if (_nickname == (*it)->getNickname())
            return (true);
    }

    return (false);
}

bool    IrcServer::usernameUsed(std::string _username)
{
	std::vector<Client*>::iterator it;

    for (it = clients.begin() ; it != clients.end() ; it++)
    {
        if (_username == (*it)->get_username())
            return (true);
    }

    return (false);
}


bool    IrcServer::realnameUsed(std::string _realname)
{
	std::vector<Client*>::iterator it;

    for (it = clients.begin() ; it != clients.end() ; it++)
    {
        if (_realname == (*it)->get_realname())
            return (true);
    }

    return (false);
}

bool    IrcServer::check_names(std::string _name)
{
	if (_name.size() < 3)
		return (false);
	if (invalidNicknameCharacter(_name) == true)
		return (false);
	return (true);
}