#pragma once
#include <new>

template<class DATA>
class CMemoryPool
{
public:
	struct st_BLOCK_NODE
	{
		DATA	data; //데이터 클래스
		st_BLOCK_NODE* next; //다음 노드
	};

	CMemoryPool(int iBlockNum, bool bPlacementNew = false): //생성자, 풀 객체 크기및 생성자 호출여부를 정한다. 기본은 false 
		m_iCapacity(0),
		m_iUseCount(0),
		m_bPlacementNew(bPlacementNew),
		_pFreeNode(nullptr)
	{
		for (int i = 0; i < iBlockNum; ++i) //풀 객체 생성 
		{
			st_BLOCK_NODE* pNode= new st_BLOCK_NODE; //메모리 공간 확보 

			//나중에 Alloc을 하면 _pFreeNode가 가르키는 놈을 꺼내 쓸 것임
			//즉, 최근에 만든 노드가 항상 pFreeNode가 되어야한다.
			//[_pFreeNode]->[최근] -> [노드] -> [nullptr] 형태가 되어야함 
			pNode->next = _pFreeNode; 
			_pFreeNode = pNode;

			m_iCapacity++; //사용량 증가 
		}
	}

	virtual ~CMemoryPool()
	{
		while (_pFreeNode != nullptr) //
		{
			st_BLOCK_NODE* pDeleteNode = _pFreeNode; //현재 free리스트 부터 지우고 
			_pFreeNode = _pFreeNode->next;  //다음으로 갱신 

			delete pDeleteNode;

		}
	}

	DATA* Alloc()
	{
		st_BLOCK_NODE* pNode = nullptr;

		if (_pFreeNode == nullptr) //여유공간이 없을떄
		{
			pNode = new st_BLOCK_NODE; 
			m_iCapacity++;
		}
		else { //있을때
			pNode = _pFreeNode; 
			_pFreeNode = _pFreeNode->next;
		}

		m_iUseCount++; //사용량 증가 

		if (m_bPlacementNew)
		{
			new (&pNode->data) DATA(); //placement New 문법
			//할당된 공간에서 생성자만 호출한다.
		}

		return &pNode->data; //중요한건 Node를 주는게 아니라 data만 줌 
	}

	bool Free(DATA* pData) //반환 
	{
		if (pData == nullptr) //잘못된 반환 
			return false;

		if (m_bPlacementNew) //소멸자 호출해야 하나?
		{
			pData->~DATA();
		}

		st_BLOCK_NODE* pNode = reinterpret_cast<st_BLOCK_NODE*>(pData); //Data를 노드로 캐스팅, 그래야 next를 알 수 있음 
		//DATA 자체에는 뒤를 나타내는 노드가 없다 

		pNode->next = _pFreeNode;
		_pFreeNode = pNode;

		m_iUseCount--; //사용량 감소 

		return true;
	}

	int GetCapacityCount(void)
	{
		return m_iCapacity;
	}
	int GetUseCount(void)
	{
		return m_iUseCount;
	}


private:
	int m_iCapacity; //생성된 node 수 
	int m_iUseCount; //현재 사용중인 블럭 수
	bool m_bPlacementNew; //생성자 소멸자 호출 여부 플래그
public:
	st_BLOCK_NODE* _pFreeNode; //반환된 오브젝트 블럭 관리노드
};