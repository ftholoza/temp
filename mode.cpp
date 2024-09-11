#include "../headers/IrcServer.hpp"
#include "../headers/Colors.hpp"
#include "sstream"
#include "../headers/Error.hpp"

void IrcServer::checkMode(std::string _msg, int _pos)
{
	std::istringstream split_msg(_msg);
	std::string token;
	std::string msg_send;
	std::vector<std::string> command;

	while(split_msg >> token)
		command.push_back(token);

	if (command.size() < 2)
	{
		msg_send = ERR_NEEDMOREPARAMS(clients[_pos - 1]->getNickname(), "MODE");
		send(fd_sockets[_pos].fd, msg_send.c_str(), msg_send.size(), 0);
		return ;
	}
	if (channelExist(command[1]) == -1)
	{
		std::string msg = ERR_NOSUCHCHANNEL(clients[_pos - 1]->getNickname(), command[1]);
        send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
		return ;
	}
	if (command.size() == 2)
	{
		std::string msg = RPL_CHANNELMODEIS(command[1], channels[channelExist(command[1])]->get_modes());
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
		return ;
	}
	if (channels[channelExist(command[1])]->isAMember(clients[_pos - 1]->getNickname()) == false)
	{
		std::string msg = ERR_NOTONCHANNEL(command[1]);
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
		return ;
	}
	unsigned int i = 2;
	while (i < command.size())
	{
		std::cout << command[i] << std::endl;
		if (command[i] == "+i" || command[i] == "-i")
			iFlag(command, command[i], _pos, i);
		else if (command[i] == "+t" || command[i] == "-t")
			tFlag(command, command[i], _pos, i);
		else if (command[i] == "+k" || command[i] == "-k")
		{
			kFlag(command, command[i], _pos, i);
			if (command[i] == "+k")
				i++;
		}
		else if (command[i] == "+o" || command[i] == "-o")
		{
			oFlag(command, command[i], _pos, i);
			i++;
		}
		else if (command[i] == "+l" || command[i] == "-l")
		{
			lFlag(command, command[i], _pos, i);
			if (command[i] == "+l")
				i++;
		}
		else
		{
			std::string msg = ERR_UNKNOWMODE(command[i]);
			send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
		}
		i++;
	}
}

int IrcServer::channelExist(std::string _name)
{
    for (unsigned int i = 0 ; i < channels.size() ; i++)
    {
        if (_name == channels[i]->getname())
            return (i);
    }
	return (-1);
}

void IrcServer::iFlag(std::vector<std::string> _command, std::string _flag, int _pos, unsigned int i)
{
	(void) i;
	int channel;

	channel = channelExist(_command[1]);
	if (_flag == "+i")
	{
		if (channels[channel]->getOnInviteOnly() == true)
		{
			std::string msg = ITALIC GREEN "[MODE: " + _command[1] + " channel aleready on invite only]\n" END_STYLE;
        	send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
			return ;
		}
		channels[channel]->setOnInviteOnly(true);
		std::string msg = ITALIC GREEN "[MODE: " + _command[1] + " channel on invite_only]\n" END_STYLE;
        channels[channel]->send_to_channel(msg);
	}
	else
	{
		if (channels[channel]->getOnInviteOnly() == false)
		{
			std::string msg = ITALIC GREEN "[MODE: " + _command[1] + " channel invite_only aleready disabled]\n" END_STYLE;
        	send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
			return ;
		}
		channels[channel]->setOnInviteOnly(false);
		std::string msg = ITALIC GREEN "[MODE: " + _command[1] + " channel invite_only disabled]\n" END_STYLE;
		channels[channel]->send_to_channel(msg);
	}
}

void IrcServer::tFlag(std::vector<std::string> _command, std::string _flag, int _pos, unsigned int i)
{
	(void) i;
	int channel;

	channel = channelExist(_command[1]);
	if (_flag == "+t")
	{
		if (channels[channel]->getTopicRestriction() == true)
		{
			std::string msg = ITALIC GREEN "[MODE: " + _command[1] + " channel Topic restriction aleready activated]\n" END_STYLE;
        	send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
			return ;
		}
		channels[channel]->setTopicRestriction(true);
		std::string msg = ITALIC GREEN "[MODE: " + _command[1] + " channel Topic restriction activated]\n" END_STYLE;
		channels[channel]->send_to_channel(msg);
	}
	else
	{
		if (channels[channel]->getTopicRestriction() == false)
		{
			std::string msg = ITALIC GREEN "[MODE: " + _command[1] + " channel Topic restriction aleready disabled]\n" END_STYLE;
        	send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
			return ;
		}
		channels[channel]->setTopicRestriction(false);
		std::string msg = ITALIC GREEN "[MODE: " + _command[1] + " channel Topic restriction disabled]\n" END_STYLE;
		channels[channel]->send_to_channel(msg);
	}
}

void IrcServer::kFlag(std::vector<std::string> _command, std::string _flag, int _pos, unsigned int i)
{
	int channel;

	channel = channelExist(_command[1]);
	if (_flag == "+k")
	{
		if (channels[channel]->getPassword().size() != 0)
		{
			std::string msg = ERR_KEYSET(channels[channel]->getname());
			send(clients[_pos - 1]->getFdClient().fd, msg.c_str(), msg.size(), 0);
			return ;
		}
		if (i >= _command.size() -1)
		{
			std::string msg = ERR_NEEDMOREPARAMS(clients[_pos - 1]->getNickname(), "MODE");
			send(clients[_pos - 1]->getFdClient().fd, msg.c_str(), msg.size(), 0);
			return ;
		}
		channels[channel]->setNeedPassword(true);
		channels[channel]->setPassword(_command[i + 1]);
		std::string msg = ITALIC GREEN "[MODE: " + _command[1] + " channel psw setled]\n" END_STYLE;
		channels[channel]->send_to_channel(msg);
	}
	else
	{
		if (channels[channel]->getNeedPassword() == false)
		{
			std::string msg = ITALIC GREEN "[MODE: " + _command[1] + " channel psw aleready disabled]\n" END_STYLE;
        	send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
			return ;
		}
		channels[channel]->setPassword("");
		channels[channel]->setNeedPassword(false);
		std::string msg = ITALIC GREEN "[MODE: " + _command[1] + " channel psw disabled]\n" END_STYLE;
		channels[channel]->send_to_channel(msg);
	}
}
                    
// ERR_USERSDONTMATCH
// RPL_UMODEIS
// ERR_UMODEUNKNOWNFLAG

void IrcServer::oFlag(std::vector<std::string> _command, std::string _flag, int _pos, unsigned int i)
{
	int channel, client;

	channel = channelExist(_command[1]);
	client = userExist(_command[3]);

	if (client == -1)
	{
		std::cout << "NOSUCHNICK" << std::endl;
		// ERR_NOSUCHNICK
	}
	else if (searchUserInChannel(_command[i + 1], _command[1]) == -1 || searchUserInChannel(clients[_pos - 1]->getNickname(), _command[1]) == -1)
	{
		std::string msg = ERR_NOTONCHANNEL(_command[1]);
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
	}
	else if (!clients[_pos - 1]->operatorOfThisChannel(_command[1]))
	{
		std::string msg = ERR_CHANOPRIVSNEEDED(clients[_pos -1]->getNickname(), _command[1]);
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
	}
	else if (_flag == "+o")
	{
		clients[client]->addOperatorChannels(channels[channel]);
		std::string msg = ITALIC PURPLE "[:" + clients[_pos - 1]->getNickname() + " MODE #" + _command[1] + " +o " + _command[i + 1] + "]\n" END_STYLE;
		channels[channel]->send_to_channel(msg);
	}
	else
	{
		clients[client]->removeOperatorChannels(channels[channel]);
		std::string msg = ITALIC PURPLE "[:" + clients[_pos - 1]->getNickname() + " MODE #" + _command[1] + " -o " + _command[i + 1] + "]\n" END_STYLE;
		channels[channel]->send_to_channel(msg);		
	}
}

void IrcServer::lFlag(std::vector<std::string> _command, std::string _flag, int _pos, unsigned int i)
{
	int channel;

	channel = channelExist(_command[1]);

	if (searchUserInChannel(clients[_pos - 1]->getNickname(), _command[1]) == -1 || searchUserInChannel(clients[_pos - 1]->getNickname(), _command[1]) == -1)
	{
		std::string msg = ERR_NOTONCHANNEL(_command[1]);
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
	}
	else if (!clients[_pos - 1]->operatorOfThisChannel(_command[1]))
	{
		std::string msg = ERR_CHANOPRIVSNEEDED(clients[_pos -1]->getNickname(), _command[1]);
		send(fd_sockets[_pos].fd, msg.c_str(), msg.size(), 0);
	}
	else if (_flag == "+l")
	{
		if (isInt(_command[i + 1].c_str()) && atoi(_command[i + 1].c_str()) > 0)
		{
			channels[channel]->setHasLimit(true);
			channels[channel]->setLimit(atoi(_command[i + 1].c_str()));
			std::string msg = ITALIC PURPLE "[:" + clients[_pos - 1]->getNickname() + " MODE #" + _command[1] + " +l " + _command[i + 1] + "]\n" END_STYLE;
			channels[channel]->send_to_channel(msg);
		}
		else
		{
			std::string msg = ITALIC RED "[MODE: Invalid limit: must be greater than 0]\n" END_STYLE;
			send(clients[_pos - 1]->getFdClient().fd, msg.c_str(), msg.size(), 0);
			// ou ERR_UNKNOWNCOMMAND
		}
	}
	else
	{
		channels[channel]->setHasLimit(false);
		std::string msg = ITALIC PURPLE "[:" + clients[_pos - 1]->getNickname() + " MODE #" + _command[1] + " -o]\n" END_STYLE;
		channels[channel]->send_to_channel(msg);		
	}	
}