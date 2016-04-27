FROM ubuntu:14.04
MAINTAINER Zhihao Yao <yzhh029@gmail.com>
MAINTAINER Cheng Huang

ADD . /bin

EXPOSE 12345

CMD ["/bin/raftfs", "-f", "/bin/hosts.txt", "-p", "12345"]
