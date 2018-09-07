#### Compilation scripts

Directory contains set of scripts designed to be run in order to detect:
  - compilation issues,
  - runtime issues _(detectable by e.g. Thread and Address Sanitizers)_.

1. Scripts are designed to be executed locally or by CI flow within Docker container _(e.g. [Travis](https://travis-ci.com/) or [SemaphoreCI](https://semaphoreci.com/))_.

  - to run locally execute _(assuming your host provides necessary tooling like gcc >=8.1 clang >=5.0)_:
```bash
  $ <script-name>
```

  - to run within Docker container _(using Docker Wrapper `dw.sh` tool)_:
```bash
  $ dw.sh <script-name>
```

2. To run all the scripts _(within Docker container)_ execute:
```bash
  $ ./dw.sh ./script-runall.sh
```
  - `script-runall.sh` executes all scripts which file name matches with the `^[0-9]{4}-.*\.sh` regex. The order of execution is according to string numerical value like `sort -n` _(e.g. `0100-glut-node6-gcc-asan.sh` will be executed before `0200-wayland-node8-gcc-asan.sh`)_.
  - Error _(non-zero status)_ returned from any script terminates the whole execution chain.

  - It is recommended to run `script-clean.sh` before compiling Spark _(a.k.a pxScene)_ with a different configuration (e.g. compiler, compilation options, etc..).

Examples:

  a) Run all scripts within Docker container _(this is re-used by CI flow (e.g. Travis or Semaphoreci)_):
  ```bash
    $ ./ci/builds/dw.sh ./ci/builds/script-runall.sh
  ```

  b) Run single script
  ```bash
     # build needed externals
     $ ./ci/builds/dw.sh ./ci/builds/0010-external-breakpad.sh
     $ ./ci/builds/dw.sh ./ci/builds/0020-external-dukluv.sh
     # build Spark
     # if you were using different Spark build optionally clean previous artifacts
     $ ./ci/builds/dw.sh ./ci/builds/script-clean.sh
     # build Spark executable
     $ ./ci/builds/dw.sh ./ci/builds/0200-wayland-node8-gcc-asan.sh
     # run bash within Docker container
     $ ./ci/builds/dw.sh bash
     # run Spark within Docker
     $ cd examples/pxScene2d/src/
     $ ./pxscene
  ```

#### Docker wrapper - dw.sh
  `dw.sh` is a tool which allows you to execute any available commands or scripts within docker container. It:
  - maps the whole `$HOME` directory inside the container `$HOME`,
  - preserves your username, `uid` and `gid`,
  - exports the following environment variables from your host: `DISPLAY`, `WAYLAND_DISPLAY`, `XDG_RUNTIME_DIR`, `CC`, `CXX`,
  - preserves the current working directory _(the one it was invoked from)_.

Note: Please bear in mind that depending on you network speed running dw.sh for the first time might take even couple of minutes to cache and create the docker image.

  
  Example:
  ```bash
    $ pwd
    /home/dwrobel/projects/pxscene/pxCore
    $ ./ci/builds/dw.sh bash

    # from now on, we are executing commands within Docker container
    [dwrobel@cf5f655a7a45 pxCore]$ pwd
    /home/dwrobel/projects/pxscene/pxCore
    [dwrobel@cf5f655a7a45 pxCore]$ cd examples/pxScene2d/src
    [dwrobel@cf5f655a7a45 src]$ $ echo $WAYLAND_DISPLAY
    wayland-0
    [dwrobel@cf5f655a7a45 src]$ ./pxscene

    # if your compositor is not Wayland based
    # please make sure you run e.g. weston on your host (outside of docker)
    # and then export WAYLAND_DISPLAY before running Spark
    [dwrobel@cf5f655a7a45 src]$ WAYLAND_DISPLAY=wayland-0 ./pxscene
  ```
