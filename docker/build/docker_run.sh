#!/bin/bash

docker run \
      -it \
      -v $(pwd)/../..:/src \
      ssgx_build:1.2.0 \
      bash
