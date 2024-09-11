#include "../headers/Client.hpp"
#include "../headers/Colors.hpp"
#include "../headers/Channels.hpp"
#include "../headers/IrcServer.hpp"
#include "../headers/Error.hpp"
#include "sstream"

void    IrcServer::checkTopic(std::string _msg, int _pos)
{
	std::istringstream split_msg(_msg);
	std::string token;
	std::vector<std::string> command;
	std::string msg;

	while(split_msg >> token)
		command.push_back(token);
	
	if (command.size() < 2)
	{
		msg = ERR_NEEDMOREPARAMS(clients[_pos -1]->getNickname(), "TOPIC");
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
	}
	int	channel = channelExist(command[1]);
	if (channel == -1)
	{
		msg = ERR_NOSUCHCHANNEL(clients[_pos -1]->getNickname(), command[1]);
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
		return ;
	}
	if (command.size() >= 2)
	{
		if (channels[channel]->isAMember(clients[_pos -1]->getNickname()) == false)
		{
			msg = ERR_NOTONCHANNEL(command[1]);
			send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
			return ;
		}
	}
	if (command.size() == 2)
	{
		if (channels[channel]->getTopic().size() == 0)
		{
			msg = ERR_NOTOPIC(clients[_pos -1]->getNickname());
			send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
			return ;
		}
		msg = BLUE ITALIC "[Topic: " + channels[channel]->getname() + " channel: " + channels[channel]->getTopic() + "]\n" END_STYLE;
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
		return ;
	}
	if (channels[channel]->getTopicRestriction() == true && clients[_pos -1]->operatorOfThisChannel(command[1]) == false)
	{
		msg = ERR_CHANOPRIVSNEEDED(clients[_pos -1]->getNickname(), command[1]);
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
		return ;
	}
	else
	{
		if (_msg.size() -1 == _msg.find(':'))
		{
			msg = RED ITALIC "[Error: Incomplete command: syntax = TOPIC <channel> : <Topic>]\n" END_STYLE;
			send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
			return;
		}
        std::string topic = _msg.substr(_msg.find(':') + 1, _msg.size());
		channels[channel]->setTopic(topic);
		msg = GREEN ITALIC "[channel: " + command[1] + " Topic setled]\n" END_STYLE;
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
		msg = BLUE ITALIC "[channel: " + command[1] + " New Topic:" + topic + "]\n" END_STYLE;
		channels[channel]->send_to_channel(msg);
		return ;
	}
	return ;
 }