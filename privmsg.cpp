#include "../headers/IrcServer.hpp"
#include "../headers/Colors.hpp"
#include "sstream"

//remplacer nicknameAlreadyUsed par nicknameExist

int	IrcServer::splitPrivmsgCommand(int _pos, std::string _msg, std::vector<std::string>& _receivers)
{
	std::istringstream split_msg(_msg);
	std::string token, msg_send;
	std::vector<std::string> command;
	unsigned int i;

	while(split_msg >> token)
		command.push_back(token);
	
	if (command.size() < 3 || command[2].find(':') == std::string::npos || _msg.size() == _msg.find(':'))
	{
		// ERR_NEEDMOREPARAMS
		msg_send = RED ITALIC "[Error: Invalid command: syntax = PRIVMSG name :message]\n" END_STYLE;
		send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
		return (0);
	}

	for (i = 0 ; i < command[1].size() ; i++)
	{
		std::string receiver;
		while (i < command[1].size() && command[1][i] != ',')
			receiver += command[1][i++];
		_receivers.push_back(receiver);
		if (i == command[1].size())
			break;
	}

	return (1);
}

void    IrcServer::checkPrivmsg(std::string _msg, int _pos)
{
	std::string msg_send;
	std::vector<std::string> receivers;
	unsigned int i = 0;
	int user, channel;

	if (!splitPrivmsgCommand(_pos, _msg, receivers))
		return ;
	while (i < receivers.size())
	{
		user = userExist(receivers[i]);
		channel = channelExist(receivers[i]);

		msg_send = _msg.substr(_msg.find(':') + 1, _msg.size()) + "\n";
		if (user == -1 && channel == -1)
		{
			msg_send = RED ITALIC "[Error: This nickname does not match any user or channel]\n" END_STYLE;
			send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
		}
		else if (user != -1)
		{
			std::string temp = PURPLE "[From: " + clients[_pos - 1]->getNickname() + "] => " END_STYLE BOLD + msg_send + END_STYLE;
			send(clients[user]->getFdClient().fd, temp.c_str(), temp.size(), 0);
		}
		else if (channel != -1)
			find_channel(receivers[i], msg_send, _pos);
		i++;
	}
}

int    IrcServer::find_channel(std::string target, std::string msg, int _pos)
{
    std::vector<Channel*>::iterator it = this->channels.begin();
    for (; it != this->channels.end(); it++)
    {
        if ((*it)->getname() == target)
        {
            std::string temp = PURPLE "[From: " + this->clients[_pos -1]->getNickname() + "] => " END_STYLE BOLD + msg + END_STYLE;
            (*it)->send_to_channel(temp);
            return (1);
        }
    }
    return (0);
}