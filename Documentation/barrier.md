배리어는 크게 두 종류로 분류할 수 있다. 컴파일러 최적화 기능으로 인한 코드 재배치 문제를 해결하기 위한 "컴파일러 배리어compiler barrier"와 프로세서 최적화 전략으로 인한 보장되지 않은 실제 실행 순서 문제를 해결하기 위한 "프로세서 배리어processor barrier"가 있다.

컴파일러에 의한 코드 재배치 문제는 하단 *Problems with reordering code*를 참고하라. 우선 프로세서 배리어에 대해 알아본다. ARM 아키텍처 기준으로 세가지 명령어가 제공된다: DMB, DSB, ISB

* dmb

dmb 이전에 명시된 메모리 접근이 모두 완료된 후에 dmb 이후에 명시된 메모리 접근을 시작 하도록 보장한다.

> '명시된 메모리 접근'은 메모리 접근을 수반하는 명령어가 실행되었으나 메모리 접근이 실제로 실행되거나 완료되지 않은 상태를 뜻한다.

* dsb

dmb 명령이 메모리 접근순서만을 보장하는 반면 dsb 명령은 메모리 접근뿐만 아니라 명령어 실행 역시 동기화시킨다. 즉, dsb 명령이 이슈되면 모든 메모리 읽기가 완료되고 write buffer가 비워질 때(쓰기가 완료될 때) 까지 다른 명령어의 실행이 중지stall 된다.

* isb

앞서 메모리 배리어인 두 명령어와 달리 이 명령은 명령어 실행 순서를 보장하는 명령어 배리어다. isb 명령이 실행되면 파이프라인을 비우고 캐시나 메모리로부터 명령어를 다시 페치한다. isb 이전에 수행된 ASID 변경이나 TLB, 분기예측, CP15 레지스터 변경이 isb 이후 페치된 명령어들에 적용되는 것을 보장한다.

(추가)
다음 링크에 관련 문제가 잘 정리되어 있다: [http://community.arm.com/groups/processors/blog/2011/03/22/memory-access-ordering--an-introduction](http://community.arm.com/groups/processors/blog/2011/03/22/memory-access-ordering--an-introduction)

## 14.1 Problems with reordering code(avr-libc 1.8.0 user manual)

원문 작성자: Jan Waclawek

프로그램은 구문의 연속으로 이루어져 있고 기본적으로 컴파일러는 사용자가 입력한 순서대로 구문을 실행할 것이다. 하지만 최적화 기능은 net effect가 같기만 하다면, 구문의 순서를-그 부분마저도- 재정렬할 수 있다. net effect란 스탠다드에서 side effects라 불리는 것으로, volatile 지시자로 선언된 변수의 배타적 접근으로 해결된다. 그래서 휘발성volatile 읽기와 쓰기가 정확히 같은 주소를 지시하고, 정확히 같은 순서로 실행된다면(그리고 정확히 같은 값을 쓴다면) 내부의 다른 연산은 고려할 필요없이 프로그램은 올바른 것이다. (여기에서 연속적인 volatile 변수로의 접근 시간은 고려되지 않았음을 명심하라.)

하지만, 불행히도 volatile 접근으로 커버되지 않는 연산이 있다. avr-gcc/avr-libc에서는 *cli()*와 *sei()* 매크로가 그 사례가 될 것이다. 이 두 매크로는 *<avr/interrupt.h>*에 정의되어 있으며,*\__asm__()* 구문을 사용하여 어셈블러 니모닉으로 바로 변환된다. 이 매크로들은 volatile이 아닐 뿐더러 변수 접근과는 아예 무관하므로 컴파일러는 이 매크로를 어디로든 재정렬할 수 있다. *\__asm__()* 구문에 volatile 지시자를 선언할 수는 있지만, 관련 문서에는 그 효과에 대한 명확한 명시가 없다 (그리고 그것은 단지 최적화에 의해 구문이 제거되지 않게 하는 기능에 가깝다). 다음 관련 내용을 참고하라: 

> volatile 어셈 명령조차도 다른 코드로(심지어 *jump* 명령 사이로) 이동될 수 있다. [...] 그러므로, 사용자는 *volatile asm* 명령이 완전히 연속적인 순서로 배치될 것이라고 기대할 수는 없다.

또한, 다음 링크를 참조하라: [http://gcc.gnu.org/onlinedocs/gcc-4.3.4/gcc/Extended-Asm.html](http://gcc.gnu.org/onlinedocs/gcc-4.3.4/gcc/Extended-Asm.html)

memory barriers 매커니즘으로 문제는 해결될 수도 있다. 인라인 어셈 구문에 *memory* clobber를 추가하는 것인데, 이는 구문 실행전에 모든 변수의 값을 레지스터에서 메모리로 적재flush하고 구문 실행 뒤에 새로 읽어 들이는 것이다. 사실, memory barriers는 코드 정렬를 강제하는 것과는 좀 다른 목적을 갖고 있다. memory barriers는 레지스터 값을 변경해도 아무 문제가 없도록 레지스터에 캐쉬된 변수를 실제 메모리에 적재하기flush 위해 설계되었다. 멀티테스킹 운영체제에서의 컨테스트 스위치가 한 예가 될 것이다.

어쨌든, memory barrier는 barrier 전/후의 주어진 volatile 접근 순서를 보장하며 제대로 동작한다. 그러나 volatile 접근이 아닌 구문의 경우, 그 구문은 주어진 순서를 벗어나 barrier 사이로 재정렬될 수 있다. Peter Dannegger가 제공한 이 문제에 대한 코드를 보자: 

```c
#define cli() __asm volatile( "cli" ::: "memory" ) 
#define sei() __asm volatile( "sei" ::: "memory" ) 

unsigned int ivar; 

void test2( unsigned int val ) 
{ 
    val = 65535U / val; 
    cli(); 
    ivar = val; 
    sei(); 
}
```

컴파일 최적화 옵션: -Os 

```
00000112  : 
112: bc 01          movw r22, r24 
114: f8 94          cli 
116: 8f ef          ldi r24, 0xFF ; 255 
118: 9f ef          ldi r25, 0xFF ; 255 
11a: 0e 94 96 00    call 0x12c ; 0x12c <__udivmodhi4>
11e: 70 93 01 02    sts 0x0201, r23 
122: 60 93 00 02    sts 0x0200, r22 
126: 78 94          sei 
128: 08 95          ret
```

나눗셈 연산이 *cli()* 뒤로 이동되면서 의도한 것보다 오랜 시간 인터럽트가 비활성화 되었다. volatile 접근인 *cli()*와 *sei()*는 의도한 순서대로 발생하므로 net effect는 스탠다드에 의해 목적한 대로 달성된다. 문제는 인터럽트가 얼마나 오래 비활성화되었는가다. 시간은 임베디드 프로그램에서 중요한 부분이고 필수적인 요소이다. 다음 링크를 참고하라: [https://www.mikrocontroller.net/topic/65923](https://www.mikrocontroller.net/topic/65923)

불행히도 현재로써는 avr-gcc나 C standard나 사용자에 의해 쓰여진 코드 그대로 실행되는 코드정렬 매커니즘은 없다. 물론 최적화 기능을 끄거나*(-O0)* 주요부분을 어셈블리로 쓰면 되지만.

요약하자면:

* memory barrier는 volatile 변수에 주어진 접근 순서를 보장한다.
* memory barrier는 그 외의 일반접근에 주어진 접근 순서를 보장하지 않는다 (barrier 사이로 재정렬될 수 있다).
