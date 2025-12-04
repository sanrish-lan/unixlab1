FROM alpine:latest

RUN sed -i 's/https/http/' /etc/apk/repositories

RUN apk add --no-cache util-linux

COPY worker.sh /usr/local/bin/worker.sh
RUN chmod +x /usr/local/bin/worker.sh

VOLUME /data
ENTRYPOINT ["/usr/local/bin/worker.sh"]
