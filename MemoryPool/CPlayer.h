#pragma once
#include <iostream>

class CPlayer
{
public:
	CPlayer(int id);
	~CPlayer();

private:
	int iPlayerId;
	int iHp;
	int iMp;
};

CPlayer::CPlayer(int id):iPlayerId(id), iHp(100),iMp(100)
{
	std::cout << "Player»ý¼º" << std::endl;
}

CPlayer::~CPlayer()
{
	std::cout << "Player¼Ò¸ê" << std::endl;
}