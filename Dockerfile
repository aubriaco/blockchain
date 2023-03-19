FROM alpine:latest

RUN apk update
RUN apk add g++ cmake make openssl-dev
COPY src/ src/
COPY CMakeLists.txt .
RUN mkdir build/
RUN cd build \
    && cmake .. \
    && make


ENTRYPOINT [ "build/blockchain" ]