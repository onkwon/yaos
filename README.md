[![Build Status](https://travis-ci.org/onkwon/yaos.svg?branch=master)](https://travis-ci.org/onkwon/yaos)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/69c0ee97ee2843d9ac4b415d9ee21b6f)](https://app.codacy.com/app/onkwon/yaos?utm_source=github.com&utm_medium=referral&utm_content=onkwon/yaos&utm_campaign=Badge_Grade_Dashboard)
[![codecov](https://codecov.io/gh/onkwon/yaos/branch/master/graph/badge.svg)](https://codecov.io/gh/onkwon/yaos)

[yaos](https://yaos.io)
=======================

Introduction
------------

yaos 는 embedded system 을 위한 작은 운영체제입니다. 개인적인 호기심으로부터 출발한 사이드 프로젝트입니다. 필수 기능만을 작고 견고하게 유지하는 것, 그리고 읽기 즐거운 코드로 개선해가는 것이 목표입니다.

ARM Cortex-A 아키텍처인 라즈베리파이에 포팅한 패치가 있긴 하지만, 기본적으로 yaos 는 ARM Cortex-M 아키텍처를 타겟하고 있습니다.

API 를 비롯한 각종 문서는 [Documentation](./Documentation) 디렉토리 또는 [yaos.io](https://yaos.io) 를 참고하시기 바랍니다.

Features
--------

system call support

  desc.

user/kernel space seperation

mpu support

posix-like api interface

sleep, stop, and standby power modes support

secure boot and firmware update

virtual file system

Getting Started
---------------

### Fork and clone

`git clone git://github.com/onkwon/yaos`

### Get either of a toolchain or docker image

#### Toolchain

Get one from [here](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) if you don't have one installed yet. Or you can compile it from source code putting more effort, which is not recommended but still worth trying.

#### Docker image

`docker pull onkwon/yaos`

### Build

#### [stm32f1-min](https://www.aliexpress.com/item/mini-Stm32f103c8t6-system-board-stm32-learning-development-board/1609777521.html)

##### using toolchain

```
make stm32f1-min
make
```

##### using docker

```
docker run --rm -v $(pwd):/work/yaos -w /work/yaos onkwon/yaos make stm32f1-min
docker run --rm -v $(pwd):/work/yaos -w /work/yaos onkwon/yaos make
```

### Flash

```
make flash
```

#### jlink

#### stlink

#### openocd

#### gdb

## Supported boards at the moment

* [mango-z1](http://www.mangoboard.com/main/?cate1=9&cate2=26&cate3=36)
* [mycortex-stm32f4](http://www.withrobot.com/mycortex-stm32f4/)
* [nrf52](https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF52832)
* [stm32f429i-disco](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/32f429idiscovery.html)
* [stm32f469i-disco](http://www.st.com/en/evaluation-tools/32f469idiscovery.html)
* [stm32-lcd](https://www.olimex.com/Products/ARM/ST/STM32-LCD/)
* [ust-mpb-stm32f103](https://www.devicemart.co.kr/1089642)
* [stm32f1-min](https://www.aliexpress.com/item/mini-Stm32f103c8t6-system-board-stm32-learning-development-board/1609777521.html)

## Example

A LED example to take a taste of how the code look like.

```c
void main()
{
	int fd, led = 0;

	if ((fd = open("/dev/gpio20", O_WRONLY)) <= 0) {
		printf("can not open, %x\n", fd);
		return;
	}

	while (1) {
		write(fd, &led, 1);
		led ^= 1;
		sleep(1);
	}

	close(fd);
}
REGISTER_TASK(main, 0, DEFAULT_PRIORITY, STACK_SIZE_DEFAULT);
```

## Running the tests

`make test` or `docker run --rm -v $(pwd):/work/yaos -w /work/yaos onkwon/yaos make test`

## Acknowledgments

* [Unity](http://www.throwtheswitch.org/unity/) - Unit testing for C
* [tiny-AES-c](https://github.com/kokke/tiny-AES-c) - Small portable AES128/192/256 in C
* [Sphinx](http://www.sphinx-doc.org/en/master/) - Documentation
* [Hawkmoth](https://github.com/jnikula/hawkmoth) - Sphinx Autodoc for C
* [texane/stlink](https://github.com/texane/stlink) - STM32 discovery line linux programmer
