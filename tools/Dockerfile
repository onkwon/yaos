FROM debian:9

ENV REFRESHED_AT 2018-11-18

RUN apt-get update \
	&& apt-get -y --no-install-recommends install \
		bzip2=1.0.6-8.1 \
		wget=1.18-5+deb9u2 \
		git=1:2.11.0-3+deb9u4 \
		make=4.1-9.1 \
		gawk=1:4.1.4+dfsg-1 \
		vim-common=2:8.0.0197-4+deb9u1 \
		gcc-6=6.3.0-18+deb9u1 \
		build-essential=12.3 \
		ca-certificates=20161130+nmu1+deb9u1 \
		ruby=1:2.3.3 \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/*
RUN gem install ceedling -v 0.28.3

WORKDIR /toolchain

RUN wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2 \
	&& tar -jxvf gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2 \
	&& rm gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2

ENV PATH $PATH:/toolchain/gcc-arm-none-eabi-7-2018-q2-update/bin

WORKDIR /app
