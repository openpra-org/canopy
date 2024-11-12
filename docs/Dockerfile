FROM ubuntu:noble AS emitter
LABEL authors="Arjun Earthperson"

ENV DEBIAN_FRONTEND=noninteractive
ARG EMITTER_PACKAGES="doxygen graphviz"

RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    --mount=target=/var/cache/apt-fast,type=cache,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean && \
    apt update && \
    apt install -y --no-install-recommends $EMITTER_PACKAGES && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /data
COPY .. .
RUN doxygen docs/Doxyfile

FROM nginx:latest AS publisher
WORKDIR /usr/share/nginx/html/docs
COPY docs/nginx.conf /etc/nginx/nginx.conf
COPY --from=emitter /data/docs/emitted/html/ ./
