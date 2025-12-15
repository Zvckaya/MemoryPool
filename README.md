🛡️ 고성능 안전 메모리 풀 (High-Performance & Safe Memory Pool)
C++로 구현된 헤더 온리(Header-only) 고성능 오브젝트 풀입니다.

잦은 동적 할당(new/delete) 오버헤드를 제거하여 성능을 극대화하면서도, **이중 해제(Double Free)**나 잘못된 포인터 해제(Invalid Free) 같은 치명적인 메모리 오류를 방지하기 위한 강력한 검증 로직을 포함하고 있습니다.

🚀 주요 특징 (Key Features)
1. 성능 최적화 (Performance)
LIFO (Last-In, First-Out) 전략: 가장 최근에 반환된(Hot) 메모리 블록을 우선 재사용하여 CPU 캐시 적중률(Cache Hit Rate)을 극대화합니다.

템플릿(Template) & 가변 인자(Variadic Templates): 모든 타입의 객체를 지원하며, Alloc 시 생성자 인자를 완벽하게 전달(Perfect Forwarding)합니다.

Placement New: 메모리 할당과 객체 생성을 분리하여 불필요한 초기화 비용을 제거했습니다.

2. 강력한 안전성 (Robust Safety)
Owner Check: 해당 메모리 풀에서 할당된 노드가 맞는지 검사하여, 다른 풀의 객체나 엉뚱한 포인터 해제를 방지합니다.

Double Free 방지: 매직 넘버(MAGIC_CODE)를 사용하여 이미 해제된 메모리를 다시 해제하려는 시도를 차단합니다.

Memory Corruption 감지: 메모리 블록 헤더의 무결성을 검사하여 오염 여부를 확인합니다.

Memory Leak 경고: 풀 소멸 시 반환되지 않은 객체가 있다면 경고 메시지를 출력합니다.

📦 설치 방법 (Installation)
이 라이브러리는 **헤더 온리(Header-Only)**입니다.

별도의 빌드 없이 MemoryPool.h 파일을 프로젝트에 포함하여 사용하세요.

```C++

#include "MemoryPool.h"
```
🛠 사용법 (Usage)
1. 기본 사용 (단순 구조체)
생성자/소멸자 호출이 필요 없는 구조체나 기본 타입(int, float 등)에 적합합니다.

```C++

struct Data {
    int x, y;
};
```
// 1. 풀 생성 (초기 블록 10개, 생성자 호출 false)
CMemoryPool<Data> pool(10); 

// 2. 할당 (단순 메모리 포인터 반환)
Data* pData = pool.Alloc(); 
pData->x = 10;

// 3. 반환
pool.Free(pData);
2. 객체 사용 (생성자/소멸자 필수)
클래스 객체 사용 시, 두 번째 인자를 true로 설정하여 Placement New를 활성화해야 합니다.

```C++

class Player {
public:
    Player(int id, int hp) : m_id(id), m_hp(hp) {
        std::cout << "Player 생성" << std::endl;
    }
    ~Player() {
        std::cout << "Player 소멸" << std::endl;
    }
private:
    int m_id, m_hp;
};

int main() {
    // [중요] 두 번째 인자를 'true'로 설정하여 생성자/소멸자 자동 호출 활성화
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
🧠 내부 구현 상세 (Implementation Details)
1. 안전한 노드 구조 (st_BLOCK_NODE)
데이터 앞에 보안 검사용 메타데이터(Header)를 배치하여 무결성을 보장합니다.

```C++

struct st_BLOCK_NODE {
    CMemoryPool* pOwner;     // 소유자 풀 확인 (Invalid Free 방지)
    unsigned int checkCode;  // 상태 코드 (Double Free 방지)
    st_BLOCK_NODE* next;     // 다음 노드 포인터
    
    // 실제 데이터 (Alignment 준수)
    alignas(DATA) unsigned char data[sizeof(DATA)]; 
};
```
2. 검증 로직 (Free)
사용자가 데이터 포인터(pData)를 반납하면, 역산(offset calculation)을 통해 헤더를 찾고 3단계 검증을 수행합니다.

Offset Calculation: pData 주소에서 offsetof(st_BLOCK_NODE, data)만큼 빼서 노드 시작 주소를 찾습니다.

Owner Check: pNode->pOwner == this인지 확인합니다.

State Check: checkCode가 CODE_ALLOC(사용 중)인지 확인합니다. 만약 CODE_FREE(이미 반납됨)라면 에러를 출력하고 차단합니다.

⚠️ 주의사항 (Notes)
디버깅 오버헤드: 안전성 검사를 위해 노드마다 메타데이터(pOwner, checkCode)가 추가되므로, 순수 데이터 크기보다 약간 더 많은 메모리를 사용합니다.

스레드 안전성: 본 라이브러리는 싱글 스레드 환경에 최적화되어 있습니다. 멀티 스레드 환경에서 사용 시 Lock 처리가 필요합니다.

미반환 객체: 풀이 소멸될 때까지 반환(Free)되지 않은 객체는 메모리 누수가 발생할 수 있으며, 소멸자에서 이에 대한 경고 메시지를 출력합니다.
