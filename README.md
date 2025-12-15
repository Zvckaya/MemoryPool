# 🛡️ 고성능 안전 메모리 풀 (High-Performance & Safe Memory Pool)
C++로 구현된 헤더 온리(Header-only) 고성능 오브젝트 풀입니다.

잦은 동적 할당(new/delete) 오버헤드를 제거하여 성능을 극대화하면서도, **이중 해제(Double Free)**나 잘못된 포인터 해제(Invalid Free) 같은 치명적인 메모리 오류를 방지하기 위한 강력한 검증 로직을 포함하고 있습니다.

## 🚀 주요 특징 (Key Features)
1. 두 가지 동작 모드 지원 (Dual Modes)
Persistent Mode (false): 객체를 최초 1회만 생성하고, 반납 시 소멸시키지 않고 유지합니다. 재사용 시 초기화 비용이 0에 가깝습니다. (MMORPG의 몬스터, 패킷 등 빈번한 재사용 객체에 최적)

Transient Mode (true): 할당 시마다 생성자, 반납 시마다 소멸자를 호출합니다. 일반적인 new/delete와 동일한 라이프사이클을 가집니다.

2. 성능 최적화 (Performance)
LIFO (Last-In, First-Out): 가장 최근에 반환된(Hot) 메모리 블록을 우선 재사용하여 **CPU 캐시 적중률(Cache Hit Rate)**을 극대화합니다.

완벽한 전달(Perfect Forwarding): 가변 인자 템플릿을 사용하여 Alloc 시 전달된 인자를 생성자로 완벽하게 전달합니다.

Placement New: 메모리 할당과 객체 생성을 분리하여 불필요한 연산을 제거했습니다.

3. 강력한 안전성 (Robust Safety)
Owner Check: 해당 메모리 풀에서 할당된 노드가 맞는지 검사하여, 다른 풀의 객체나 엉뚱한 포인터 해제를 방지합니다.

Double Free 방지: 매직 넘버(MAGIC_CODE)를 사용하여 이미 해제된 메모리를 다시 해제하려는 시도를 차단합니다.

Memory Corruption 감지: 메모리 블록 헤더의 무결성을 검사하여 오염 여부를 확인합니다.

Memory Leak 경고: 풀 소멸 시 반환되지 않은 객체가 있다면 경고 메시지를 출력합니다.

## 📦 설치 방법 (Installation)
이 라이브러리는 **헤더 온리(Header-Only)**입니다.

별도의 빌드 없이 MemoryPool.h 파일을 프로젝트에 포함하여 사용하세요.

```C++

#include "MemoryPool.h"

```
## 🛠 사용법 (Usage)
1. Transient Mode (매번 생성/파괴)
일반적인 new / delete를 대체하는 모드입니다.

생성자의 두 번째 인자를 **true**로 설정합니다.

Alloc: 메모리 할당 + 생성자 호출

Free: 소멸자 호출 + 메모리 반납

```C++

class Player {
public:
    Player(int id) : m_id(id) { std::cout << "Player 생성" << std::endl; }
    ~Player() { std::cout << "Player 소멸" << std::endl; }
    int m_id;
};

int main() {
    // [Mode: true] 할당/해제 시마다 생성자/소멸자가 호출됨
    CMemoryPool<Player> pool(10, true);

    // 내부 동작: new (ptr) Player(100);
    Player* p1 = pool.Alloc(100); 
    
    // 내부 동작: p1->~Player();
    pool.Free(p1); 

    return 0;
}
```
2. Persistent Mode (최초 1회 생성 후 재사용)
객체를 부수지 않고 값만 초기화해서 계속 재사용하는 초고성능 모드입니다.

생성자의 두 번째 인자를 false (기본값)로 설정합니다.

Alloc (첫 호출): 메모리 할당 + 생성자 호출

Alloc (재사용): 생성자 호출 건너뜀 (기존 객체 리턴)

Free: 소멸자 호출 안 함 (메모리만 반납)

```C++

class Monster {
public:
    Monster() { std::cout << "최초 1회만 호출됨" << std::endl; }
    void Reset(int hp) { m_hp = hp; } // 재사용을 위한 초기화 함수
    int m_hp;
};

int main() {
    // [Mode: false] 객체를 유지하며 재사용 (기본값)
    CMemoryPool<Monster> pool(10, false);

    // 1. 최초 할당: 생성자 호출됨
    Monster* m1 = pool.Alloc(); 
    m1->Reset(100);
    
    // 2. 반납: 소멸자 호출 안 됨 (객체 살아있음)
    pool.Free(m1);

    // 3. 재할당: 생성자 호출 안 됨! (아까 그 객체 재사용)
    Monster* m2 = pool.Alloc();
    m2->Reset(200); // 별도 초기화 필요

    return 0;
}
```
## 🧠 내부 구현 상세 (Implementation Details)
1. 안전한 노드 구조 (st_BLOCK_NODE)
데이터 앞에 보안 검사용 메타데이터(Header)를 배치하여 무결성을 보장합니다.

```C++

struct st_BLOCK_NODE {
    CMemoryPool* pOwner;     // 소유자 풀 확인 (Invalid Free 방지)
    unsigned int checkCode;  // 상태 코드 (Double Free 방지)
    st_BLOCK_NODE* next;     // 다음 노드 포인터
    bool isConstructed;      // 객체 생성 여부 플래그 (Persistent 모드용)

    // 실제 데이터 (Alignment 준수)
    alignas(DATA) unsigned char data[sizeof(DATA)]; 
};
```
2. 검증 로직 (Free)
사용자가 데이터 포인터(pData)를 반납하면, 역산(offset calculation)을 통해 헤더를 찾고 3단계 검증을 수행합니다.

Offset Calculation: pData 주소에서 offsetof(st_BLOCK_NODE, data)만큼 빼서 노드 시작 주소를 찾습니다.

Owner Check: pNode->pOwner == this인지 확인합니다.

State Check: checkCode가 CODE_ALLOC(사용 중)인지 확인합니다. 만약 CODE_FREE(이미 반납됨)라면 에러를 출력하고 차단합니다.

## ⚠️ 주의사항 (Notes)
Persistent Mode 주의점: false 모드 사용 시, Alloc으로 받은 객체는 이전 사용자가 남긴 데이터(Dirty State)를 그대로 가지고 있을 수 있습니다. 반드시 Reset() 함수 등을 통해 상태를 초기화하고 사용하세요.

스레드 안전성: 본 라이브러리는 싱글 스레드 환경에 최적화되어 있습니다. 멀티 스레드 환경에서 사용 시 Lock 처리가 필요합니다.

디버깅 오버헤드: 안전성 검사를 위해 노드마다 메타데이터(pOwner, checkCode 등)가 추가되므로, 순수 데이터 크기보다 약간 더 많은 메모리를 사용합니다.
