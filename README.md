[![Build Status](https://travis-ci.org/onkwon/yaos.svg?branch=master)](https://travis-ci.org/onkwon/yaos)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/69c0ee97ee2843d9ac4b415d9ee21b6f)](https://app.codacy.com/app/onkwon/yaos?utm_source=github.com&utm_medium=referral&utm_content=onkwon/yaos&utm_campaign=Badge_Grade_Dashboard)
[![codecov](https://codecov.io/gh/onkwon/yaos/branch/master/graph/badge.svg)](https://codecov.io/gh/onkwon/yaos)

# [yaos](https://yaos.io)

## Introduction

This is an embedded operating system for Internet of Things(IoT) devices, specifically for a single-core processor without MMU virtualization. It is designed for energy efficiency and hardware independent development. 

Full documentation can be found in the "Documentation" directory. It is also available online at []().

## Features

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

What things you need to install the software and how to install them

```
Give examples
```

### Build

```bash
	$ git clone https://github.com/onkwon/yaos.git
	$ cd yaos
	$ make stm32f1-min (or specify your board. e.g. stm32f429i-disco)
	$ make
```

#### Supported boards at the moment

* [mango-z1](http://www.mangoboard.com/main/?cate1=9&cate2=26&cate3=36)
* [mycortex-stm32f4](http://www.withrobot.com/mycortex-stm32f4/)
* [nrf52](https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF52832)
* [stm32f429i-disco](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/32f429idiscovery.html)
* [stm32f469i-disco](http://www.st.com/en/evaluation-tools/32f469idiscovery.html)
* [stm32-lcd](https://www.olimex.com/Products/ARM/ST/STM32-LCD/)
* [ust-mpb-stm32f103](https://www.devicemart.co.kr/1089642)
* [stm32f1-min](https://www.aliexpress.com/item/mini-Stm32f103c8t6-system-board-stm32-learning-development-board/1609777521.html)

### Example

## Running the tests

Explain how to run the automated tests for this system

## License

Apache-2.0 © [Kyunghwan Kwon](https://github.com/onkwon)

## Acknowledgments

* [Unity](http://www.throwtheswitch.org/unity/) - Unit testing for C
* [tiny-AES-c](https://github.com/kokke/tiny-AES-c) - Small portable AES128/192/256 in C
* [Sphinx](http://www.sphinx-doc.org/en/master/) - Documentation
* [Hawkmoth](https://github.com/jnikula/hawkmoth) - Sphinx Autodoc for C
* [texane/stlink](https://github.com/texane/stlink) - STM32 discovery line linux programmer
