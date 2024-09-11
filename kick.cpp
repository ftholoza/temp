#include "../headers/IrcServer.hpp"
#include "../headers/Colors.hpp"
#include "../headers/Error.hpp"
#include "sstream"

int	IrcServer::splitKickCommand(int _pos, std::string _msg, std::vector<std::string>& _users, std::vector<std::string>& _channels)
{
	std::istringstream split_msg(_msg);
	std::string token, msg_send;
	std::vector<std::string> command;
	unsigned int i;

	while(split_msg >> token)
		command.push_back(token);
	
	if (command.size() < 3)
	{
		msg_send = ERR_NEEDMOREPARAMS(clients[_pos - 1]->getNickname(), "KICK");
		send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
		return (0);
	}
	else if (command.size() > 3 && command[3].find(':') == std::string::npos)
	{
		msg_send = RED ITALIC "[Error: Invalid command: syntax = KICK channel user :...]\n" END_STYLE;
		send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
		return (0);
	}

	for (i = 0 ; i < command[1].size() ; i++)
	{
		std::string channel;
		while (i < command[1].size() && command[1][i] != ',')
			channel += command[1][i++];
		_channels.push_back(channel);
		if (i == command[1].size())
			break;
	}

	for (i = 0 ; i < command[2].size() ; i++)
	{
		std::string user;
		while (i < command[2].size() && command[2][i] != ',')
			user += command[2][i++];
		_users.push_back(user);
		if (i == command[2].size())
			break;
	}

	return (1);
}

void    IrcServer::checkKick(std::string _msg, int _pos)
{
	//voir	 ERR_BADCHANMASK
	std::string msg_send;
	std::vector<std::string> users, chan;
	int target_user, target_channel;
	unsigned int i = 0;

	if (!splitKickCommand(_pos, _msg, users, chan))
		return ;
	while (i < users.size())
	{
		target_user = userExist(users[i]);
		target_channel = channelExist(chan[i]);

		if (target_user == -1)
		{
			msg_send = BLUE ITALIC "[User '" + users[i] + "' doesn't exist.]\n" END_STYLE;
			send(fd_sockets[target_user + 1].fd, msg_send.c_str(), msg_send.size(), 0);
		}
		else if (target_channel == -1)
		{
			std::string msg = ERR_NOSUCHCHANNEL(clients[_pos - 1]->getNickname(), chan[i]);
			send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
		}
		else if (searchUserInChannel(users[i], chan[i]) == -1)
		{
			std::string msg = ERR_NOTONCHANNEL(users[i]);
			send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
		}
		else if (!clients[_pos - 1]->operatorOfThisChannel(chan[i]))
		{
			std::string msg = ERR_CHANOPRIVSNEEDED(clients[_pos -1]->getNickname(), chan[i]);
			send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
		}
		else
		{
			channels[target_channel]->deleteMember(users[i]);
			if (_msg.find(':') == std::string::npos)
				msg_send = BLUE ITALIC "[You were kicked from " + chan[i] + " channel]\n" END_STYLE;
			else
				msg_send = BLUE ITALIC "[You were kicked from " + chan[i] + " channel. Reason: " + _msg.substr(_msg.find(':') + 1, _msg.size()) + "]\n" END_STYLE;
			send(fd_sockets[target_user + 1].fd, msg_send.c_str(), msg_send.size(), 0);
		}
		i++;
	}
}