#pragma once

#include <new>
#include <vector>
#include <utility>

template<class DATA>
class CContiguousPool
{
public:
	CContiguousPool(int iBlockNum, bool bPlacementnew = false)
		: m_bPlacementNew(bPlacementnew)
		, m_pOriginalBuffer(nullptr)
	{
		m_pOriginalBuffer = static_cast<char*>(::operator new(sizeof(DATA) * iBLockNum)); //생 메모리 할당 

		m_FreeStack.reserve(iBlockNum); //스택 크기 예약 

		//스타트는 버퍼 시작 위치 
		m_pBufferStart = m_pOriginalBuffer;
		//끝은 시작위치 + 사이즈만큼 
		m_pBufferEnd = m_pOriginalBuffer + (sizeof(DATA) * iBlockNum);

		for (int i = 0; i < iBlockNum; ++i) //실제로 크기만큼 증가하며 
		{
			char * pAddr = m_pOriginalBuffer + (i * sizeof(DATA)); //해당위치를 찝고 
			m_FreeStack.push_back(reinterpret_cast<DATA*>(pAddr)); //그 위치를 DATA*로 해석해서(즉 그 사이즈만큼) 스택에 넣는다. 
		}

	}

	~CContiguousPool()
	{
		for (DATA* pData : m_FreeStack) //스택을 돌면서 
		{
			//주소가 범위안에 있는지 꼭 체크해야 한다. 메모리 영역은 지정되있으니 
			bool bIsOrigin = (reinterpret_cast<char*>(pData) >= m_pBufferStart &&
				reinterpret_cast<char*>(pData) < m_pBufferEnd);

			if (!bIsOrigin)
			{
				// 범위 밖이라면 나중에 낱개로 추가 할당된 놈임.... 
				::operator delete(pData);
			}
		}

		// 만약 맞으면 초기지점 해제 
		if (m_pOriginBuffer)
		{
			::operator delete(m_pOriginBuffer);
		}
	}


	template<typename... Args>
	DATA* Alloc(Args&&... args)
	{
		DATA* pRet = nullptr;

		//스택이 비지 않았나? 즉 여유분이 있는가. 
		if (!m_FreeStack.empty())
		{
			//스택에서 마지막 꺼냄 
			pRet = m_FreeStack.back();
			m_FreeStack.pop_back();
		}
		else
		{
			//여유분 없을시 메모리만 할당  ,여기서부터는 연되지 않을 수 있다. 
			pRet = static_cast<DATA*>(::operator new(sizeof(DATA)));
		}

		//생성자 호출 
		if (m_bPlacementNew)
		{
			new (pRet) DATA(std::forward<Args>(args)...);
		}

		return pRet;
	}

	void Free(DATA* pData)
	{
		if (pData == nullptr) return;

		//소멸자 호출 
		if (m_bPlacementNew)
		{
			pData->~DATA();
		}

		//스택에 주소 반남 
		m_FreeStack.push_back(pData);
	}

	int GetFreeCount() { return (int)m_FreeStack.size(); }
private:
	std::vector<DATA*> m_FreeStack;

	char* m_pOriginalBuffer;
	char* m_pBufferStart;
	char* m_pBufferEnd;

	bool m_bPlacementNew;
};