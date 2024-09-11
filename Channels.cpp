#include "../headers/Client.hpp"
#include "../headers/Colors.hpp"
#include "../headers/Channels.hpp"
#include "../headers/IrcServer.hpp"


Channel::Channel(std::string name, Client *client)
{
	this->name = name;
	this->clients.push_back(*client);
	this->on_invit_only = false;
	this->need_password = false;
	this->topic_restriction = false;
	this->participant = 1;
}

Channel::Channel(std::string _name, bool _oninvitonly, bool _topicrestriction, std::string _password)
{
	this->name = _name;
	this->participant = 0;
	this->on_invit_only = _oninvitonly;
	this->topic_restriction = _topicrestriction;
	if (_password.size() == 0)
		this->need_password = false;
	else
	{
		this->need_password = true;
		//ajouter parsing pour password
		this->password = _password;
	}
}
Channel::~Channel()
{
}
std::string				Channel::getname()
{
	return (this->name);
}

unsigned int			Channel::get_participants()
{
	return (this->participant);
}

std::vector<Client>	Channel::get_clients()
{
	return (this->clients);
}

bool					Channel::add_participant(Client *client)
{
	std::vector<Client>::iterator it = this->clients.begin();
	for (;it != this->clients.end(); it++)
	{
		if (it->getNickname() == client->getNickname())
		{
			std::string msg_send = ITALIC BLUE "[You aleready are in this channel]\n" END_STYLE;
			send(client->getFdClient().fd, msg_send.c_str(), msg_send.size(), 0);
			return (false);
		}
	}
	this->clients.push_back(*client);
	this->participant++;
	return (true);
}
		
void					Channel::remove_participant(Client &client)
{
	std::vector<Client>::iterator it = this->clients.begin();
	while (it != this->clients.end())
	{
		if (it->getNickname() == client.getNickname())
			this->get_clients().erase(it);
		it++;
	}
	return ;
}

void					Channel::send_to_channel(std::string msg)
{
	std::string temp = PURPLE "[Channel: " + this->name + "] " + msg;
	std::vector<Client>::iterator it = this->clients.begin();
	while (it != this->clients.end())
	{
		send(it->getFdClient().fd, temp.c_str(), temp.size(), 0);
		it++;
	}
	return ;
}

bool					Channel::getOnInviteOnly()
{
	return (this->on_invit_only);
}

bool					Channel::getTopicRestriction()
{
	return (this->topic_restriction);
}

std::string				Channel::getPassword()
{
	return (this->password);
}

bool					Channel::getNeedPassword()
{
	return (this->need_password);
}

void					Channel::setOnInviteOnly(bool _oninvitonly)
{
	this->on_invit_only = _oninvitonly;
}

void					Channel::setTopicRestriction(bool _topicrestriction)
{
	this->topic_restriction = _topicrestriction;
}

void					Channel::setPassword(std::string _password)
{
	this->password = _password;
}

void					Channel::setNeedPassword(bool _needpassword)
{
	this->need_password = _needpassword;
}

bool					Channel::isAMember(std::string _user)
{
    for (unsigned int i = 0 ; i < clients.size() ; i++)
    {
        if (_user == clients[i].getNickname())
			return (true);
    }
	return (false);
}

void					Channel::deleteMember(std::string _user)
{
    for (std::vector<Client>::iterator it = clients.begin() ; it != clients.end() ; it++)
    {
        if (_user == it->getNickname())
		{
			clients.erase(it);
			return ;
		}
    }
}

std::string				Channel::get_modes()
{
	std::string res = " +";

	if (this->getOnInviteOnly() == true)
		res += "i";
	if (this->getTopicRestriction() == true)
		res += "t";
	if (this->getNeedPassword() == true)
	{
		res += "k";
		res += " " + this->getPassword();
	}
	if (res.size() > 2)
		return (res);
	else
		return ("");
		
}

void 					Channel::setLimit(int _limit)
{
	this->limit = _limit;
}

void					Channel::setHasLimit(bool _haslimit)
{
	this->has_limit = _haslimit;
}

int					Channel::getLimit(void)
{
	return (this->limit);
}

bool					Channel::getHasLimit(void)
{
	return (this->has_limit);
}

void					Channel::setTopic(std::string _topic)
{
	this->topic = _topic;
	return ;
}

std::string				Channel::getTopic()
{
	return (this->topic);
}