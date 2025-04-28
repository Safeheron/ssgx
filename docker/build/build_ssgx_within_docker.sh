#!/bin/bash

docker run \
      --rm \
      -v $(pwd)/../..:/src \
      ssgx_build:1.2.0 \
      bash -c "cd /src  \
              && cmake --preset release-ssgx-config \
              && cmake --build --preset release-ssgx-build --verbose \
              && sudo cmake --install release-ssgx-config \
              && mkdir -p docker/build/output \
              && cp -r /opt/safeheron/ssgx docker/build/output/"






