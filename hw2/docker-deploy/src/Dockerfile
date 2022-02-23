FROM ubuntu:20.04
RUN apt-get update
RUN apt-get install -y g++ make
RUN mkdir /var/log/erss
RUN chmod 777 /var/log/erss
WORKDIR /
ADD . /
RUN chmod +x run.sh