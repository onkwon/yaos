## Boot Sequence

전원인가시 ARM core는 off된 상태로 GPU에서 부팅이 시작된다.

1. 첫번째 부트로더는 RPi 생산시 SoC ROM에 프로그램된 것으로 사용자가 프로그램할 수 없다. 첫번째 부트로더는 SD카드의 FAT32 부트 파티션을 마운트해 다음 단계의 부트로더를(bootcode.bin) 읽는 역할을 한다.

2. 다음 단계의(두번째) 부트로더는 bootcode.bin이다. 첫번째 부트로더는 ARM core와 램을 초기화하지 않기 때문에 두번째 부트로더는 GPU의 128K 4-way set associative L2 cache에 로드되어 GPU에서 실행된다. 두번째 부트로더는 램을 활성화하고 SD카드로부터 GPU 펌웨어를(start.elf) 읽어들인다.

3. 세번째 부트로더인 start.elf는 GPU 펌웨어이다. 일종의 BIOS라고 할 수 있다(하지만, 단순한 펌웨어가 아니라 VideoCore OS라는 사유 소프트웨어이다(소스 공개됨). 리눅스는 mailbox messaging system을 사용하여 VCOS와 통신한다). GPU와 ARM core에서 사용하는 RAM을 배타적으로 할당하고(GPU와 CPU사이에 MMU가 동작, ARM core의 물리주소는 VideoCore에서 다른 주소에 맵핑) 디바이스의 각종 주파수와 같은 하드웨어 제어 설정(ARM core 깨우는 작업)을 한다.

4. 그리고 마지막으로 kernel.img를 로드하고(0x8000번지에 로드됨) ARM core를 깨운다. 그러면 ARM core는 kernel.img의 명령을 실행하기 시작한다.

펌웨어:  
[https://github.com/raspberrypi/firmware/tree/master/boot](https://github.com/raspberrypi/firmware/tree/master/boot "firmware")

참고자료:  
[https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=72260](https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=72260 "")

## Peripherals

* Peripheral Base Address : 0x20000000 (0x3f000000 for RPi2)

### System Timer

* Base Address : 0x3000

4개의 32비트 비교채널과 하나의 64비트 카운터. 각 채널은 인터럽트 시그널을 발생하는 ouput compare register 를 하나씩 갖는다. 인터럽트 서비스 루틴에서 다음 인터럽트 발생 시기를 설정하기 위해서는 해당 compare register 의 값을 다음 프리러닝 카운터 값으로 설정해주어야 한다.

> 비교 레지스터 값과 매칭이 되면 timer 인터럽트가 발생할 것 같은데 그렇지가 않다. 뭔가 추가적인 설정이 필요한 듯. 제공된 매뉴얼에는 정보가 부족하고, ARM 레퍼런스 매뉴얼 generic timer를 읽어볼 것.

프리러닝 카운터는 1MHz(1us) 주기로 값을 업데이트한다(실측값).

```c
struct system_timer {
	unsigned int status; /* 비교 레지스터와 카운터가 매치하면 해당 비트 셋 */
	union {
		struct {
			unsigned int counter_h;
			unsigned int counter_l;
		};
		unsigned long long counter;
	};
	unsigned int compare0;
	unsigned int compare1;
	unsigned int compare2;
	unsigned int compare3;
};
```

### Timer

* Base Address : 0xb000  
* Offset : 0x400

시스템 클럭 기반 타이머(이 클럭은 동적으로 변할 수 있다 e.g. low power mode. 따라서 정확한 타이밍을 위해서는 시스템 타이머를 사용하라).

프리스케일링 하지 않을 경우(\*1) 1MHz 단위로 뛴다(즉, APB버스 클럭이 초기 1MHz로 설정된 듯).

value 값이 0이 되면 인터럽트 발생시키고 value <- load 반복.

```c
struct timer {
	unsigned int load; /* 카운터 시작 지점 */
	unsigned int value; /* read-only */
	unsigned int control;
	unsigned int irq_clear;
	unsigned int irq_raw;
	unsigned int irq_masked;
	unsigned int reload;
	unsigned int divider;
	unsigned int counter;
};
```

### GPIO

* Base Address : 0x00200000

54개의 GPIO가 두개의 뱅크에 나뉘어 있다. 각 핀은 적어도 두개의 alternative functions 을 갖는다. 각 뱅크는 각각 하나의 인터럽트 라인을 갖는다. 그리고 하나의 인터럽트 라인을 공유한다.

1. 입력 모드일 경우, 센싱 종류를 설정한다(폴링, 라이징, 레벨).
2. 핀 출력 값을 설정(set or clear)
3. 모드 설정(function select)

핀맵(p.102)과 레지스터 상세(p.90)는 데이터시트 참고.

### UART

> http://elinux.org/BCM2835\_datasheet\_errata
> 위 링크 반드시 참고할 것. 데이터시트만 보다가 시간 엄청 허비 ㅠㅠ

두 개의 UART를 제공한다. UART0은 mini uart, UART1은 pl011 uart.

GPIO14,15 에 각각 TXD, RXD 핀이 맵핑되어 있다. UART1인 경우 ALT0에 맵핑되어 있고, UART0인 경우 ALT5에 맵핑되어 있다.

> 시스템 클럭은 500MHz(추정)

### Interrupts

* Base Address : 0xb000  
* Offset : 0x200

GPU 주변장치 혹은 ARM 제어 주변장치로부터 인터럽트가 발생할 수 있다. 기본 pending 레지스터 하나와 두개의 GPU pending 레지스터가 있다.

```c
struct interrupt_controller {
	unsigned int basic_pending;
	unsigned int pending1;
	unsigned int pending2;
	unsigned int fiq_control;
	unsigned int irq1_enable;
	unsigned int irq2_enable;
	unsigned int basic_enable;
	unsigned int irq1_disable;
	unsigned int irq2_disable;
	unsigned int basic_disable;
};
```

1. basic pending 레지스터 체크
2. pending register 1(8번째 비트)/pending register 2(9번째 비트)가 셋되어 있다면 해당 레지스터 체크
3. clear pending bit

## Blinking LED

* Status LED Pin : GPIO16 (GPIO47 for RPi2 and B+ Model)

1. SD 카드를 FAT32 파일시스템으로 포맷한다.
2. `bootcode.bin`와 `start.elf` 펌웨어를 SD 카드로 복사한다.
3. 아래 예제 프로그램 바이너리를 `kernel.img` 파일명으로 변경한 뒤 SD 카드로 복사한다.

```c
#define GPIO_INPUT              0
#define GPIO_OUTPUT             1

#define PERI_BASE               0x20000000
#define GPIO_BASE               (PERI_BASE + 0x00200000)

#define GPIO_FS(n)      \
        *(volatile unsigned int *)(GPIO_BASE + ((n / 10) * 4))
#define SET_GPIO_FS(n, v) {     \
        GPIO_FS(n) &= ~(7 << (n % 10 * 3)); \
        GPIO_FS(n) |= v << (n % 10 * 3); \
}

#define GPIO_SET(n) ({   \
        if (n / 32)             \
                *(volatile unsigned int *)(GPIO_BASE + 0x20) |= 1 << (n % 32); \
        else                    \
                *(volatile unsigned int *)(GPIO_BASE + 0x1c) |= 1 << (n % 32); \
})
#define GPIO_CLEAR(n) ({ \
        if (n / 32)             \
                *(volatile unsigned int *)(GPIO_BASE + 0x2c) |= 1 << (n % 32); \
        else                    \
                *(volatile unsigned int *)(GPIO_BASE + 0x28) |= 1 << (n % 32); \
})

#define LED                     16
#define DELAY                   5000000

int __attribute__((naked)) main()
{
        __asm__ __volatile__("ldr sp, =0x8000" ::: "memory");

       	SET_GPIO_FS(LED, GPIO_OUTPUT);

        unsigned int i;

       	while (1) {
		GPIO_CLEAR(LED);
       	        for (i = 0; i < DELAY; i++) __asm__ __volatile__("nop");
               	GPIO_SET(LED);
                for (i = 0; i < DELAY; i++) __asm__ __volatile__("nop");
       	}   

        return 0;
}
```

linker script:

```
        OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
        OUTPUT_ARCH(arm)

        SECTIONS
        {
                . = 0x8000;
                .text : { *(.text) }
                .data : { *(.data) }
                .bss : { *(.bss) }
        }
```

컴파일:

```
arm-linux-gnueabihf-gcc -O0 -T,rpi.lds -nostartfiles -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s main.c -o kernel.elf
arm-linux-gnueabihf-objcopy kernel.elf -O binary kernel.img
```

## 기타

### enabling SIMD and floating-point extension

CPACR.cp10와 CPACR.cp11의 값은 동일해야만 함.

```
	/* NSACP.cp[10-11] */
	mrc     p15, 0, r0, c1, c1, 2
	orr     r0, r0, #3 << 10        /* enable non-secure access */
	bic     r0, r0, #3 << 14        /* NSASEDIS */
	mcr     p15, 0, r0, c1, c1, 2
	mcr     p15, 0, r0, c7, c5, 4   /* ISB */
	/* HCPTR.cp[10-11] */
	mrc     p15, 4, r0, c1, c1, 2
	bic     r0, r0, #3 << 10        /* TCP10 and TCP11 */
	bic     r0, r0, #3 << 14        /* TASE */
	mcr     p15, 4, r0, c1, c1, 2
	mcr     p15, 0, r0, c7, c5, 4   /* ISB */
	/* FPEXC.en */
	mov     r3, #0x40000000
	vmsr    fpexc, r3
```
