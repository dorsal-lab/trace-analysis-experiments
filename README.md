# trace-analysis-experiments
This repository regroups different experiment to parse and analyse traces

## Parsing speed

The goal of this experiment is to measure the minimum time required to count trace events. Usually most trace analyses do not require all event types, so in this case, we only increment a counter and moves the cursor to skip the event. This program uses event types of ~27 bytes which is what is obtained using a kernel trace with suggested tracepoints in https://github.com/tuxology/tracevizlab/tree/master/labs/003-record-kernel-trace-lttng.

### Steps

1- Record a kernel trace with lttng by following this tutorial: https://github.com/tuxology/tracevizlab/tree/master/labs/003-record-kernel-trace-lttng
2- Measure the time it takes for babeltrace2 to count the events with ``