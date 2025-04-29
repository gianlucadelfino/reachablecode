# Doubly mapped lockfree shared memory queue

It's easy(ish) to create a ring-based lockfree queue that is Single Producer-
Single Consumer, but it gets tricky if one also needs varible size messages.
The problem with variable size messages is that the end of the ring and the
beginning of the ring could hold two halves of a single message, and why is that
a problem you ask? The issue is that, for a very small amount of users, copying
the messages in and out is not really an option, so messages *must* be read
straight off the queue buffer and this is not possible if the message is not
written in a contiguous address space.

There is one way (that I know of) to keep the ring and have a contiguous
address space that loops around, and it boils down to mmapping the ring region
twice and making sure the two mappings are next to one another. This is what
this project accomplishes.

## How to build
```
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . --parallel
```

## Usage
There are 2 example apps, a reader and a writer:

```
    taskset -c 5 ./apps/reader --queue_file queue.bin --control_block_file control_block.bin
    taskset -c 6 ./apps/writer --queue_file queue.bin --control_block_file control_block.bin
```
The parameters are all optional and default to the above values. The prepended taskset pin the execution to
a specific core, which is amenable for our purpose but it's also optional.

## More Info
[ReachableCode.com](https://www.reachablecode.com)
