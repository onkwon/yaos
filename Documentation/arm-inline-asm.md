다음 링크의 내용을 요약한 것입니다: [http://www.ethernut.de/en/documents/arm-inline-asm.html][1]

```
asm(code : output operand list : input operand list : clobber list);
```

기본 어셈블러 명령어 문자열(코드 파트)은 순수 어셈블리 프로그램과 매우 흡사합니다. 하지만 C와 연결되는 상수와 레지스터는 콜론으로 구분된 두번째, 세번째, 네번째 파트에서 다르게 표기됩니다.

사용되지 않는 파트는 생략할 수 있습니다만, 사용되는 파트가 뒤따라 올 경우 생략할 수 없습니다. 예를들면, 입력 피연산자 파트가 사용된다면 출력 피연산자 파트는 반드시 공백으로 표기되어야 합니다:

```
asm("msr cpsr,%[ps]" : : [ps]"r"(status));
```

`%`문자 뒤에 따라오는 대괄호로 감싸인 심볼릭 이름은 참조할 피연산자를 지칭합니다. 대괄호로 감싼 심볼릭 참조는 GCC 컴파일러 3.1 버전부터 지원합니다. 이전 버전에서는 대괄호로 감싼 심볼릭 참조 대신 `%숫자 `를 사용합니다. `숫자` 는 나열된 피연산자 순서를 말합니다.

공백 문자열로 코드 파트 역시 생략할 수 있습니다. 아래 구문은 컴파일러에게 메모리 내용이 변경됐다는 것을 알립니다:

```
	asm("":::"memory");
```

Input and output operands
-------------------------

어셈블리 명령은 특정한 피연산자 타입만을 수용합니다. 이를테면 *branch* 명령은 점프할 목적 주소를 넘겨받습니다. 하지만 모든 메모리 주소가 유효한 것은 아닙니다. opcode의 마지막 부분은 24-bit 옵셋만을 수용하기 때문입니다. 반면, `branch` 명령과 `exchange` 명령은 32-bit 목적 주소를 포함하는 레지스터를 넘겨받습니다. 어느 경우든 피연산자는 포인터로 동일하게 처리되어 C에서 인라인 어셈블러로 전달됩니다. 따라서 상수나 포인터, 변수를 인라인 어셈블리 구문으로 넘길 때는 인라인 어셈블러가 데이터 타입을 알 수 있도록 반드시 *constraints*를 함께 넘겨주어야 합니다.

* GCC 4에서 제공하는 ARM 프로세서 constraints

```
Constraint | Usage in ARM state                              | Usage in Thumb state
-----------|-------------------------------------------------|---------------------
 f         | Floating point registers f0..f7                 | Not available
 h         | Not available                                   | Registers r8..r15
 G         | Immediate floating point constant               | Not available
 H         | Same as G, but negated                          | Not available
 I         | Immediate value in data processing instructions | Constant in the range 0..255
           |  e.g. ORR R0, R0, #operand                      |  e.g. SWI operand
 J         | Indexing constants -4095..4095                  | Constand in the range -255..-1
           |  e.g. LDR R1, [PC, #operand]                    |  e.g. SUB R0, R0, #operand
 K         | Same as I, but inverted                         | Same as I, but shifted
 L         | Same as I, but negated                          | Constant in the range -7..7
           |                                                 |  e.g. SUB R0, R1, #operand
 l         | Same as r                                       | Registers r0..r7
           |                                                 |  e.g. PUSH operand
 M         | Costant in the range of 0..32 or a power of 2   | Constant that is a multiple of 4 in the range of 0..1020
           | e.g. MOV R2, R1, ROR #operand                   | e.g. ADD R0, SP, #operand 
 m         | Any valid memory address
 N         | Not available                                   | Costand in the range of 0..31
           |                                                 |  e.g. LSL R0, R1, #operand
 O         | Not available                                   | Constant that is a multiple of 4 in the range of -508..508
           |                                                 |  e.g. ADD, SP, #operand
 r         | General register r0..r15                        | Not available
           |  e.g. SUB operand1, operand2, operand3          |
 w         | Vector floating point registers s0..s31         | Not available
 X         | Any operand
```

*constraint modifier*를 *constraint* 앞에 표기할 수 있습니다. *modifier*가 없는 *constraints*는 읽기 전용 피연산자임을 말합니다.

* modifiers

```
Modifier | Specifies
---------|----------
 =       | Write-only operand, usually used for all output operands
 +       | Read-write operand, must be listed as an output operand
 &       | A register that shuld be used for output only
```

출력 피연산자는 반드시 쓰기 전용이어야 하며 lvalue여야 합니다. 입력 피연산자는 입력 전용입니다. 입력 피연산자는 출력용과는 달리 컴파일러에서 유효한 타입인지 체크하지 못합니다. 다만, 늦은 어셈블리 스테이지에서 이상한 오류 메시지와 함께 검출될 것입니다. 오류 메시지가 내부 컴파일러 문제라고 말하더라도 인라인 어셈블러 코드를 먼저 살펴보는 것이 좋을 것입니다.

입력 피연산자에는 절대 값을 쓰지 말아야 합니다. 입/출력 피연산자에 동일한 변수를 사용하고 싶다면 `+` *constraint modifier*를 사용해야 합니다:

```
asm("mov %[value], %[value], ror #1" : [value] "+r" (y));
```

`+` *modifier*를 지원하지 않는 예전 컴파일러 버전에서는 다음과 같이 n-th(0부터 시작) 피연산자 순서를 기입하므로 위와 같은 동일한 결과를 얻을 수 있습니다:

```
asm("mov %0, %0, ror #1" : "=r" (value) : "0" (value));
```

위와 같이 동일한 연산자가 지정되지 않더라도 컴파일러는 동일한 피연산자 레지스터를 할당할 수도 있습니다. 다음 구문은

```
asm("mov %[result],%[value],ror #1":[result] "=r" (y):[value] "r" (x));
```

아래와 같이 출력됩니다:

```
00309DE5    ldr   r3, [sp, #0]    @ x, x
E330A0E1    mov   r3, r3, ror #1  @ tmp, x
04308DE5    str   r3, [sp, #4]    @ tmp, y
```

대부분의 경우 이는 문제가 되지 않지만, 입력 피연산자가 사용되기 이전에 출력 피연산자가 어셈블러 코드에 의해 수정되면 치명적입니다. 입/출력에 다른 레지스터를 사용하려면 `&` constraint modifier를 출력 피연산자에 추가해야 합니다. 다음 코드를 봅시다:

```
asm volatile("ldr %0, [%1]"     "\n\t"
             "str %2, [%1, #4]" "\n\t"
             : "=&r" (rdv)
             : "r" (&table), "r" (wdv)
             : "memory");
```

컴파일러가 만약 입/출력 피연산자에 동일한 레지스터를 할당했다면, 출력 피연산자 값은 첫번째 어셈블러 명령에 의해 오염될 것입니다. 하지만 다행히 `&` modifier는 컴파일러에게 입력 피연산자에 사용된 어떤 레지스터도 출력 피연산자에 사용하지 않을 것을 알립니다.

GNU C 코드 최적화
--------------

코드 최적화는 의도한 것과 다른 결과를 유도할 수 있습니다. 다음은 컴파일러 최적화 후에 출력된 비트 로테이션 예제입니다:

```
00309DE5    ldr   r3, [sp, #0]    @ x, x
E330A0E1    mov   r3, r3, ror #1  @ tmp, x
04308DE5    str   r3, [sp, #4]    @ tmp, y
```

컴파일러는 비트 로테이션에 `r3` 레지스터를 할당했습니다. 다른 레지스터를 할당할 수도 있고 C 변수마다 각 각 레지스터를 할당할 수도 있습니다. 문제는 확실히 값을 읽어오거나 결과를 저장하지 않을 수 있습니다. 다음은 다른 컴파일러 버전으로 다른 컴파일 옵션으로 출력한 결과입니다:

```
E420A0E1    mov r2, r4, ror #1    @ y, x
```

컴파일러는 각 피연산자에 다른 레지스터를 할당했습니다. 이미 캐시된 `r4` 값을 `r2` 레지스터에 씁니다. 이는 더욱 심각해질 수 있습니다. 최적화는 인라인 어셈블러 코드 전체를 생략할 수도 있기 때문입니다.

이를 해결하기 위해 `volatile`를 사용합니다:

```
/* NOP example */
asm volatile("mov r0, r0");
```

최적화는 코드를 재배열할 수도 있습니다. 다음 예제를 봅시다:

```c
i++;
if (j == 1)
    x += 3;
i++;
```

최적화는 두 번의 증가 연산이 조건 구문에 끼치는 영향이 없다는 것을 인식하는 것은 물론이고, 한꺼번에 2를 증가하는 것이 단 하나의 ARM 명령을 소비한다는 것을 압니다. 따라서 코드는 다음과 같이 재배열됩니다:

```c
if (j == 1)
    x += 3;
i += 2;
```

이는 컴파일러가 소스 코드의 구문 순서를 유지할 지, 변경할 지 보장할 수 없음을 뜻합니다.

다음 코드를 통해 재배열 최적화가 어떻게 문제가 되는지 알아보도록 합시다:

```c
asm volatile("mrs r12, cpsr\n\t"
	     "orr r12, r12, #0xC0\n\t"
	     "msr cpsr_c, r12\n\t" ::: "r12", "cc");
c *= b; /* This may fail. */
asm volatile("mrs r12, cpsr\n"
  	     "bic r12, r12, #0xC0\n"
    	     "msr cpsr_c, r12" ::: "r12", "cc");
```

위 코드는 인터럽트 루틴에서 값이 변경될 지도 모를 변수 `c`와 `b`를 곱합니다. 따라서 해당 변수에 접근하기 전 인터럽트를 금지하는 것은 현명한 방법같습니다만, 최적화는 의도와는 달리 변수의 곱셉을 인터럽트 금지나 활성 명령 전/후로 재배열할 수 있습니다.

이는 *clobber* 파트로 해결할 수 있는데, 그전에 위 코드에서 사용한 *clobber* 리스트 내용을 알아보도록 합시다:

```
"r12", "cc"
```

`r12` 레지스터가 사용된다는 것과 condition code flags가 업데이트 된다는 것을 컴파일러에게 알립니다. 레지스터를 직접 지정하는 것은 최적화 전략에 좋지 않습니다. 일반적으로 변수를 넘기고 컴파일러가 레지스터를 할당하도록 하는 것이 좋습니다.

그리고 `memory` 키워드가 있습니다. 이 키워드는 어셈블러 명령이 메모리를 변경한다고 컴파일러에게 알립니다. 그러면 컴파일러는 사전에 캐시된 모든 값을 저장하고, 명령 연산이 끝난 후에 reload 합니다:

```c
asm volatile("mrs r12, cpsr\n\t"
 	     "orr r12, r12, #0xC0\n\t"
	     "msr cpsr_c, r12\n\t" :: : "r12", "cc", "memory");
c *= b; /* This is safe. */
asm volatile("mrs r12, cpsr\n"
	     "bic r12, r12, #0xC0\n"
	     "msr cpsr_c, r12" ::: "r12", "cc", "memory");
```

캐시된 값을 *invalidating* 하는 것은 차선일 수 있습니다. 더미 피연산자를 추가하므로 인위적인 의존성을 만들 수도 있습니다:

```c
asm volatile("mrs r12, cpsr\n\t"
	     "orr r12, r12, #0xC0\n\t"
	     "msr cpsr_c, r12\n\t" : "=X" (b) :: "r12", "cc");
c *= b; /* This is safe. */
asm volatile("mrs r12, cpsr\n"
	     "bic r12, r12, #0xC0\n"
	     "msr cpsr_c, r12" :: "X" (c) : "r12", "cc");
```

위 코드는 첫번째 `asm` 구문에서 `b` 변수를, 두번째 `asm` 구문에서 `c` 변수를 수정한다고 가장합니다. 이 방법으로 캐시된 다른 변수들을 *invalidating*할 필요없이 세 구문의 순서를 유지할 수 있습니다.

참고
--------

더 자세한 정보와 예제를 [원문][1]에서 볼 수 있습니다.

[1]: http://www.ethernut.de/en/documents/arm-inline-asm.html

[https://gcc.gnu.org/onlinedocs/gcc/Local-Reg-Vars.html#Local-Reg-Vars](https://gcc.gnu.org/onlinedocs/gcc/Local-Reg-Vars.html#Local-Reg-Vars)
[https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Extended-Asm](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Extended-Asm)
