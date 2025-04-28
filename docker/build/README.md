# SSGX Build With Docker

We provide a Dockerfile to support building SSGX with docker.

### Prerequisites
 - **Docker** (Required)

### Building

Build an image with an environment for compiling SSGX.

```shell
$ bash ./build_ssgx_image.sh
```

Use the above image to compile SSGX, and the compiled products will be output to the current directory.

```shell
$ bash ./build_ssgx_within_docker.sh
```

When you have built the image, you can also enter docker to compile or run the code yourself.
```shell
$ bash ./docker_run.sh
```
