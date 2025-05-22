# trace-analysis-experiments

This repository regroups different experiment to parse and analyse traces

## Parsing speed

The goal of this experiment is to measure the minimum time required to count trace events. Usually most trace analyses do not require all event types, so in this case, we only increment a counter and moves the cursor to skip the event. This program uses event types of ~27 bytes which is what is obtained using a kernel trace with suggested tracepoints in https://github.com/tuxology/tracevizlab/tree/master/labs/003-record-kernel-trace-lttng.

### Prerequisites

- [babeltrace2](https://babeltrace.org/#bt2-get)
- LTTng or CTF Trace file > 4 GiB

### Steps

1. Record a kernel trace with lttng by following this tutorial: https://github.com/tuxology/tracevizlab/tree/master/labs/003-record-kernel-trace-lttng
2. Measure the time it takes for babeltrace2 to count the events with `time babeltrace2 <trace-path> --component=sink.utils.counter`
3. Compile the TReadSpeed.c file with `gcc -O3 TReadSpeed.c -o TReadSpeed`
4. Compress the CTF trace file to one archive
5. Run TReadSpeed with `time TReadSpeed <archive-trace-path> <size-in-bytes>`


## SQLite writing speed

Most recent tracers (ftrace, LTTng, uftrace, perf, etc...) use a binary format to store events with the least overhead possible, however users like the capabilities that databases offer. Ideally, this would be done as a post-processing step to be able to query the events generated, but this step can be quite long for large traces. This experiment has been done to measure how fast one can insert events into an SQLite database file.

### Prerequisites

- SQLite 3 library (debian: libsqlite3-dev, centos: sqlite-devel arch-linux: sqlite)
- [LTTng-tools] (https://lttng.org/docs/v2.13/#doc-installing-lttng)
- [LTTng-UST] (https://lttng.org/docs/v2.13/#doc-installing-lttng)

### Steps

1. Compile SQLite intercept library with `gcc -shared -fPIC -O3 -o cyg-profile-sqlite.so trace-sqlite.c -lsqlite3`
2. Compile the main program with `gcc -finstrument-functions TWriteSpeed.c -o TWriteSpeed`
3. Trace the main program with LTTng-UST:
    ```
    lttng create
    lttng enable-channel --userspace --blocking-timeout=inf blocking-channel
    lttng enable-event --userspace --channel=blocking-channel --all
    lttng start
    time LTTNG_UST_ALLOW_BLOCKING=1 LD_PRELOAD=liblttng-ust-cyg-profile-fast.so ./TWriteSpeed
    lttng stop
    lttng destroy
    ```
4. Trace the main program with our SQLite tracer:
    ```
    time LD_PRELOAD=./cyg-profile-sqlite.so ./TWriteSpeed
    ```
