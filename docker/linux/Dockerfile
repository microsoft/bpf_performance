# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: MIT

FROM ubuntu:24.04

WORKDIR /app

# Copy all files from the bin directory to /app in the container
COPY bin .

LABEL org.opencontainers.image.source=https://github.com/Alan-Jowett/bpf_performance
LABEL org.opencontainers.image.description="BPF Performance containter image"
LABEL org.opencontainers.image.license="MIT"

ENV LD_LIBRARY_PATH=/app:${LD_LIBRARY_PATH}

ENTRYPOINT [ "./bpf_performance_runner" ]