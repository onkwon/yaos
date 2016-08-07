sleep mode 와 deep sleep mode. 전자는 프로세서 클럭을 정지한다. 주변장치를 포함한 시스템의 다른 클럭들은 여전히 활성상태일 수 있다. 반면 후자는 대부분의 클럭들을 정지한다. product level에서 이는 stop 또는 standby 상태와 일치한다. stop 모드에서는 레지스터와 RAM을 포함한 문맥이 유지되는 반면 standby 모드는 엔트리 포인트가 다른 리셋상태라고 볼 수 있다. 낮은 I/O 클럭 주파수를 사용하고 I/O핀이 AIN 상태일 때, 그리고 시스템 동작 클럭 주파수가 낮을수록 전력소모는 낮아진다.

## Entering sleep mode

### WFI

### WFE

### Sleep-on-Exit

Sleep-on-Exit는 interrupt-driven applications에 이상적이다. 예외 핸들러의 수행을 종료하고 스레드 모드로 복귀하는 동시에 sleep 모드로 전환한다. 이는 예외처리 후 WFI 명령을 바로 실행하는 것과 유사하며(추가적인 스택연산이 수행되지만), 마이크로컨트롤러가 최대한 sleep 모드에 머무를 수 있도록 한다. 시스템 초기화가 완료된 시점에 Sleep-on-Exit 기능을 활성화하도록 해야한다. 너무 일찍 해당 기능이 활성화되어있다면 초기화가 완료되기 이전에 인터럽트가 발생함으로써 sleep에 빠질 수 있기 때문이다.

## Wakeup from sleep mode

### WFI or sleep-on-exit

충분한 우선순위의 예외가 발생하면 프로세서는 깨어난다. wakeup시 상황에 따라 시스템은 인터럽트 핸들러를 수행하기 전에 시스템 복구 작업을 해야할 필요가 있을 수 있다. 이를 달성하기 위해 PRIMASK 레지스터로 인터럽트를 마스킹하고(해당 인터럽트는 pending 상태) 복구 작업이 끝난 후 클리어한다.

### WFE

SEVONPEND 비트가 셋되어 있다면 인터럽트가 비활성화되어 있고 낮은 우선순위의 인터럽트가 발생하더라도 어떤 pending 인터럽트나 프로세서를 깨울 수 있다.

## Wakeup Interrupt Controller(WIC)

WIC는 deep sleep 모드에서 프로세서의 모든 클럭이 정지했을 때 NVIC의 wakeup decision functionality를 반영하기 위한 추가적 장치이다. 이는 ultralow-power 상태에서도 인터럽트에 의해 거의 바로 깨어날 수 있도록 한다. WIC는 추가적인 레지스터를 필요로 하지 않지만, 시스템 레벨의 전원관리 유닛(PMU:power management unit)을 필요로 한다. WIC는 deep sleep 모드에서만 사용된다.

wakeup을 위한 인터럽트 검출과 마스킹 작업이 WIC로 이전되었기 때문에 프로세서는 어떤 클럭 사이클도 소비할 필요 없이 low-power 모드에 머무를 수 있다. WIC는 device-specific PMU 설정과 deep sleep 활성화외에는 추가적인 프로그래밍 요구하지 않는다. deep sleep 중에 systick 타이머가 비활성화 될 수 있다(device specific인 듯).

임베디드 OS를 사용하고 sleep 모드에서도 스케줄러가 실행되어야 한다면 다음 중 하나가 해결책이 될 수 있다:

* deep sleep 모드에 영향을 받지 않는 별개의 타이머를 활성화해서 스케줄링 타임에 프로세서를 깨울 수 있도록 한다.
* WIC를 비활성화한다.
* deep sleep 모드를 사용하지 않는다. 기본 sleep 모드를 대신 사용한다.

## 참고자료

[http://www.embedded.com/design/mcus-processors-and-socs/4230085/The-basics-of-low-power-programming-on-the-Cortex-M0](http://www.embedded.com/design/mcus-processors-and-socs/4230085/The-basics-of-low-power-programming-on-the-Cortex-M0)
