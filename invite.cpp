#include "../headers/IrcServer.hpp"
#include "../headers/Colors.hpp"
#include "../headers/Error.hpp"
#include "sstream"

int IrcServer::userExist(std::string _nickname)
{
    for (unsigned int i = 0 ; i < clients.size() ; i++)
    {
        if (_nickname == clients[i]->getNickname())
            return (i);
    }
	return (-1);
}

int IrcServer::searchUserInChannel(std::string _user, std::string _channel)
{
    for (unsigned int i = 0 ; i < channels.size() ; i++)
    {
		if (channels[i]->getname() == _channel)
		{
			if (channels[i]->isAMember(_user))
				return (i);
		}
    }
	return (-1);
}

void    IrcServer::checkInvite(std::string _msg, int _pos)
{
	//voir	RPL_AWAY
	std::istringstream split_msg(_msg);
	std::string token, msg_send;
	std::vector<std::string> command;
	int target_user, target_channel;

	while(split_msg >> token)
		command.push_back(token);

	if (command.size() < 3)
	{
		msg_send = ERR_NEEDMOREPARAMS(clients[_pos - 1]->getNickname(), "INVITE");
		send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
		return;
	}

	target_user = userExist(command[1]);
	target_channel = channelExist(command[2]);

	if (target_user == -1)
	{
		// ERR_NOSUCHNICK
	}
	else if (target_channel == -1)
	{
		clients[target_user -1]->addInvitationsChannels(command[2]);
		msg_send = BLUE ITALIC "[" + clients[_pos - 1]->getNickname() + " invite you on " + command[2] + " channel]\n" END_STYLE;
		send(fd_sockets[target_user + 1].fd, msg_send.c_str(), msg_send.size(), 0);
		//RPL_INVITING
	}
	else if (searchUserInChannel(clients[_pos - 1]->getNickname(), command[2]) == -1)
	{
		std::string msg = ERR_NOTONCHANNEL(clients[_pos - 1]->getNickname());
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
	}
	else if (searchUserInChannel(command[1], command[2]) != -1)
	{
		// ERR_USERONCHANNEL
	}
	else
	{
		if (channels[target_channel]->getOnInviteOnly() == true)
		{
			if (!clients[_pos - 1]->operatorOfThisChannel(channels[target_channel]->getname()) == true)
			{
				std::string msg = ERR_CHANOPRIVSNEEDED(clients[_pos -1]->getNickname(), command[2]);
				send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
				return ;
			}
		}
		clients[target_user]->addInvitationsChannels(channels[target_channel]->getname());
		msg_send = BLUE ITALIC "[" + clients[_pos - 1]->getNickname() + " invite you on " + command[2] + " channel]\n" END_STYLE;
		send(fd_sockets[target_user + 1].fd, msg_send.c_str(), msg_send.size(), 0);
		//RPL_INVITING
	}
}