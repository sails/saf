# docker build -t sailsxu/saf ./
FROM sailsxu/protobuf
MAINTAINER sails xu <sailsxu@qq.com>

ENV LC_ALL C.UTF-8

# rank_service，需要hiredis
RUN DEBIAN_FRONTEND=noninteractive git clone --depth=1 --recursive https://github.com/redis/hiredis.git
RUN DEBIAN_FRONTEND=noninteractive cd hiredis && make && make install && rm -rf ../hiredis
RUN DEBIAN_FRONTEND=noninteractive ldconfig
# docker会缓存，所以这里通过不同的命令防止缓存，每次修改这个字符串即可
RUN DEBIAN_FRONTEND=noninteractive echo '2015121600' > /dev/null && git clone --depth=1 --recursive https://github.com/sails/saf.git
RUN DEBIAN_FRONTEND=noninteractive cd saf && mkdir build && cd build && cmake ../ && make
RUN DEBIAN_FRONTEND=noninteractive cd saf/example && ./build_test.sh
# 覆盖默认配置文件, Add命令的src可以是url，如果src是压缩文件，增加到dest时会自动解压；在kubernetes中，可能configMap可以更方便改变，不用重建image
# ADD <src> <dest>

#运行
#docker run -d -p 10000:8000 -p 10001:8001 -v /root/saf/v1/log:/saf/dist/log -w /saf/dist imageId ./saf
#在kuberntes中，不方便指定目录，所以要通过命令/bin/bash，两个参数： “-c” 和 “cd /saf/dist && ./saf” 来运行
