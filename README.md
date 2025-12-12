

# 고성능 C++ 템플릿 메모리 풀 (High-Performance Memory Pool)
C++로 구현된 헤더 온리(Header-only) 고성능 오브젝트 풀(Object Pool / FreeList)입니다.

잦은 동적 할당(new/delete)으로 인한 오버헤드를 제거하고, LIFO(Stack) 구조를 채택하여 캐시 지역성(Cache Locality)을 극대화하도록 설계되었습니다.

## 🚀 주요 특징 (Key Features)
템플릿(Template) 기반: 기본 자료형부터 사용자 정의 클래스까지 모든 타입 지원.

LIFO (Last-In, First-Out) 전략: 가장 최근에 반환된(Hot) 메모리 블록을 우선 재사용하여 CPU 캐시 적중률(Cache Hit Rate) 향상.

할당과 생성의 완벽한 분리:

풀 초기화 시점에는 순수 메모리만 확보 (불필요한 기본 생성자 호출 방지).

사용 시점(Alloc)에 생성자 호출.

alignas와 byte 배열을 사용하여 메모리 정렬(Alignment) 준수.

가변 인자 템플릿(Variadic Templates) & 완벽한 전달(Perfect Forwarding):

Alloc(args...) 호출 시 인자의 개수나 타입에 상관없이 객체 생성자로 완벽하게 전달.

Placement New & 명시적 소멸자:

이미 할당된 메모리 위에 객체를 생성(placement new)하고, 반환 시 메모리 해제 없이 소멸자(~DATA)만 호출하여 재사용.

자동 확장(Automatic Expansion): 미리 확보한 노드가 고갈되면 자동으로 추가 메모리를 할당하여 확장.

## 📦 설치 방법 (Installation)
이 라이브러리는 **헤더 온리(Header-Only)**입니다.

별도의 빌드 과정 없이 MemoryPool.h 파일을 프로젝트 경로에 복사하고 include 하여 사용하세요.
```C++

#include "MemoryPool.h"
```
## 🛠 사용법 (Usage)
### 1. 기본 사용 (생성자/소멸자 호출 불필요 시)
구조체(struct)나 단순 데이터 타입처럼 생성자/소멸자 호출이 굳이 필요 없는 경우입니다.

```C++

struct Data {
    int x, y;
};
```

 1. 메모리 풀 생성 (초기 블록 10개)
// 두 번째 인자 기본값 false: 생성자/소멸자 호출 안 함
CMemoryPool<Data> pool(10); 

 2. 할당 (단순 메모리 포인터 반환)
Data* pData = pool.Alloc(); 
pData->x = 10;

 3. 반환 (풀로 복귀)
pool.Free(pData);
### 2. 객체 사용 (생성자 인자가 필요한 경우)
클래스처럼 생성자와 소멸자 호출이 필수적인 경우, bPlacementNew 옵션을 true로 설정해야 합니다.

```C++

class Player {
public:
    Player(int id, int hp) : m_id(id), m_hp(hp) {
        std::cout << "Player 생성됨" << std::endl;
    }
    ~Player() {
        std::cout << "Player 소멸됨" << std::endl;
    }
private:
    int m_id;
    int m_hp;
};

int main() {
    // [중요] 두 번째 인자를 'true'로 설정하여 Placement New 모드 활성화
    CMemoryPool<Player> playerPool(10, true);

    // Alloc에 인자를 전달하면 생성자가 호출됨
    // 내부 동작: new (ptr) Player(100, 500);
    Player* p1 = playerPool.Alloc(100, 500); 
    
    // 객체 반환 시 소멸자가 호출됨
    // 내부 동작: p1->~Player();
    playerPool.Free(p1); 

    return 0;
}
```
## 🧠 구현 상세 (Design Details)
### 1. 메모리 레이아웃 (st_BLOCK_NODE)
풀을 생성할 때 DATA 객체의 생성자가 호출되는 것을 막기 위해, 객체 자체 대신 메모리 바이트 배열을 사용합니다. 이때 alignas를 사용하여 CPU가 요구하는 메모리 정렬 조건을 만족시킵니다.
```C++

struct st_BLOCK_NODE {
    // DATA 크기만큼의 raw memory. 
    // alignas를 통해 DATA 타입의 정렬 요구사항을 준수함.
    alignas(DATA) unsigned char data[sizeof(DATA)]; 
    
    st_BLOCK_NODE* next; // 다음 노드를 가리키는 포인터
};
```
### 2. 가변 인자 템플릿을 통한 생성 (Alloc)
사용자가 Alloc에 어떤 인자를 넣든, std::forward를 통해 불필요한 복사 없이 생성자로 전달합니다.

```C++

template<typename... Args>
DATA* Alloc(Args&&... args) {
    // ... 노드 꺼내기 로직 ...
    
    // 단순 메모리(byte array) 주소를 DATA 포인터로 재해석
    DATA* pRet = reinterpret_cast<DATA*>(pNode->data);

    if (m_bPlacementNew) {
        // 해당 메모리 위치에 생성자 호출 (Placement New)
        new (pRet) DATA(std::forward<Args>(args)...);
    }
    return pRet;
}
```
### 3. 포인터 재해석을 통한 반환 (Free)
구조체의 첫 번째 멤버(data)의 주소는 구조체 전체(st_BLOCK_NODE)의 시작 주소와 동일하다는 C++ 표준을 이용하여, reinterpret_cast로 안전하게 노드를 복구합니다.

```C++

// pData 포인터를 다시 st_BLOCK_NODE 포인터로 변환하여 리스트에 연결
st_BLOCK_NODE* pNode = reinterpret_cast<st_BLOCK_NODE*>(pData);
```
