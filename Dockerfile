FROM ubuntu:17.10
RUN apt-get update -y
RUN apt-get install -y make git gcc 
RUN apt-get install -y nasm vim
RUN apt-get install -y g++
RUN apt-get install -y gdb
WORKDIR /root
ADD . /root/

