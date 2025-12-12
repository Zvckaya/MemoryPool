#pragma once
#include <new>
#include <utility> 

template<class DATA>
class CMemoryPool
{
public:
	struct st_BLOCK_NODE
	{
		//템플릿으로 넘겨받은 객체를 직접 선언하지 않고, 그 크기만큼의 메모리 공간만 잡아놈
		// 처음에는 DATA를 놨으나.그러면 생성자 호출을 무조건 해야함. 
		//alignas이용 
		alignas(DATA) unsigned char data[sizeof(DATA)];

		st_BLOCK_NODE* next;
	};

	CMemoryPool(int iBlockNum, bool bPlacementNew = false) :
		m_iCapacity(0),
		m_iUseCount(0),
		m_bPlacementNew(bPlacementNew),
		_pFreeNode(nullptr)
	{
		for (int i = 0; i < iBlockNum; ++i)
		{
			// 여기서 new를 해도 st_BLOCK_NODE 안의 data는 단순 배열이므로
			// DATA의 생성자 호출을 막을 수 있다.
			st_BLOCK_NODE* pNode = new st_BLOCK_NODE;

			//LIFO 방식으로 연결(스택 형태)
			pNode->next = _pFreeNode;
			_pFreeNode = pNode;

			m_iCapacity++; //실제 사용량 증가 
		}
	}

	virtual ~CMemoryPool()
	{
		while (_pFreeNode != nullptr)
		{
			st_BLOCK_NODE* pDeleteNode = _pFreeNode;
			_pFreeNode = _pFreeNode->next; //노드 갱신
			delete pDeleteNode; //실제 메모리 삭제 
		}
	}

	//가변 인자 템플릿 적용 
	//몇개가 들어올지는  모르지만 일단 다 받겠다~...
	//아래의 의미는 타입이 여러개 오는 데,그걸 Args로 부르겠다 라는 의미.
	template<typename... Args>
	DATA* Alloc(Args&&... args) //타입에 맞는 변수도 여러개 오는데, 퉁처서 args라고 부르겠다
	{
		st_BLOCK_NODE* pNode = nullptr;

		if (_pFreeNode == nullptr)
		{
			pNode = new st_BLOCK_NODE;
			m_iCapacity++;
		}
		else
		{
			pNode = _pFreeNode;
			_pFreeNode = _pFreeNode->next;
		}

		m_iUseCount++;

		//배열의 시작 주소를 DATA*로 캐스팅 
		DATA* pRet = reinterpret_cast<DATA*>(pNode->data);

		if (m_bPlacementNew)
		{
			//new (Ret) <- placementNew 문법, 새롭게 메모리 할당 ㄴ
			//DATA(생성자...)
			//forword를 사용해서 넘어온 형태 그대로 넘긴다.
			new (pRet) DATA(std::forward<Args>(args)...);
		}

		return pRet;
	}

	bool Free(DATA* pData)
	{
		if (pData == nullptr)
			return false;

		if (m_bPlacementNew)
		{
			//만약 true 면 소멸자만 호출 
			pData->~DATA();
		}

		//다음 노드를 찾아야되니까 캐스팅, DATA상태로는 next노드를 알 수 업음
		st_BLOCK_NODE* pNode = reinterpret_cast<st_BLOCK_NODE*>(pData);

		pNode->next = _pFreeNode;
		_pFreeNode = pNode;

		m_iUseCount--;

		return true;
	}

	int GetCapacityCount(void) { return m_iCapacity; }
	int GetUseCount(void) { return m_iUseCount; }

private:
	int m_iCapacity;
	int m_iUseCount;
	bool m_bPlacementNew;

public:
	st_BLOCK_NODE* _pFreeNode;
};