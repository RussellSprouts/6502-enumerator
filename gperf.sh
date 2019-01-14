#!/bin/bash

make enumerator2

env LD_LIBRARY_PATH=. ./enumerator2 40

google-pprof --web enumerator2 gperf-profile.log
google-pprof --callgrind enumerator2 gperf-profile.log > perf.callgrind

kcachegrind perf.callgrind
