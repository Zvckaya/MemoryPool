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
	std::cout << "Player"<<id<<"»ý¼º" << std::endl;
}

CPlayer::~CPlayer()
{
	std::cout << "Player" << iPlayerId << "¼Ò¸ê" << std::endl;
}