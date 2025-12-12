#include "CPlayer.h"
#include "CMemoryPool.h"

int main()
{
	CMemoryPool<CPlayer> playerPool = CMemoryPool<CPlayer>(10,true);

	CPlayer* p1 = playerPool.Alloc(10);

	std::cout << "È®ÀÎ";


}