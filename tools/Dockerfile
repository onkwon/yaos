FROM debian:9

ENV REFRESHED_AT 2018-11-18

RUN apt-get update \
	&& apt-get -y install bzip2 wget git make gawk vim-common gcc-6 build-essential

WORKDIR /toolchain

RUN wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2 \
	&& tar -jxvf gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2 \
	&& rm gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2

ENV PATH $PATH:/toolchain/gcc-arm-none-eabi-7-2018-q2-update/bin

WORKDIR /app
