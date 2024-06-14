FROM alpine:3.20 as builder

RUN apk update --no-cache; \
    apk add --no-cache cmake make gcc musl-dev;

WORKDIR /usr/src/violet
COPY . .

RUN cmake -B build; \
    cd build; \
    make -j2

FROM alpine:3.20 as app

RUN apk update --no-cache; \
    apk add --no-cache musl;

COPY --from=builder /usr/src/violet/build/violet /usr/local/bin/

ENTRYPOINT [ "/usr/local/bin/violet" ]
