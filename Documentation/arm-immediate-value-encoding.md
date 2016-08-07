ARM은 다른 RISC 프로세서들과 마찬가지로 고정된 32비트 명령어 크기를 갖는다. 이는 명령어 디코딩과 파이프라인 적용을 용이하게 하는 한편, 명령어와 함께 인코딩되는 상수immediate value값의 표현 범위를 제한한다.

데이터 연산 명령어의 25번째 비트는 operand2가 상수인지 레지스터인지 결정하는 플래그이다. operand2는 하위 12비트를 차지하는데 이는 0 ~ 4095 값만을 표현할 수 있다. 이런 제약을 돌파하기 위해 ARM은 12비트 중 상위 4비트를 로테이트 시프트 값으로, 하위 8비트를 상수값으로 사용한다.

4비트는 16가지(0~15) 경우의 수를 가지므로 8비트의 상수값을 32비트 값으로 확장하기에는 역부족이다. 하지만, 로테이트 값을 2로 곱하면 32비트 표현이(0~30, 단 짝수만) 가능해진다. 즉, 0~31까지의 2의 누승값을 표현할 수 있으므로, 하나의 명령어는 32비트 내의 어떤 비트든 조작할 수 있게된다.

참고:  

* [http://alisdair.mcdiarmid.org/arm-immediate-value-encoding](http://alisdair.mcdiarmid.org/arm-immediate-value-encoding/)
* [http://www.davespace.co.uk/arm/introduction-to-arm/immediates.html](http://www.davespace.co.uk/arm/introduction-to-arm/immediates.html)
