#include "CPlayer.h"
#include "CMemoryPool.h"
#include <vector>


CMemoryPool<CPlayer> playerPool = CMemoryPool<CPlayer>(10, false);

int main()
{

	std::vector<CPlayer*> players(20);

	for (int i = 0; i < 20; ++i)
	{
		players[i] = playerPool.Alloc(i);
		if (players[i] == nullptr)
		{
			std::cerr << "메모리 할당 에러" << std::endl;
			return 1;
		}
	}

	for (auto i : players)
	{
		playerPool.Free(i); 
	}

	CPlayer* test1 = playerPool.Alloc(13);
	playerPool.Free(test1);
	playerPool.Free(test1);
	


}