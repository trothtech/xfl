# POSIX Pipelines Protocol

This page describes the traffic flow over POSIX pipes.

## POSIX Pipelines (XFL) Consumer/Producer Handshake

The implementation uses a pair of ordinary Unix pipes, created with
the `pipe()` system call, to effect the managed flow of data from one
stage to the next. It combines a pair of unstructured pipes to form
a structured pipeline.

When the producer (upstream) stage performs a write, it blocks
until the consumer (downstream) is able to ingest the record.

## XFL Connector

The connection between stages is represented in source code by a C struct.


                        producer            consumer
                     (read) ctrl <--------- ctrl (write)
                    (write) data ---------> data (read)
                     mode=OUTPUT            mode=INPUT


## XFL Protocol

When the producer stage wants to write a record,
it blocks on a POSIX read awaiting instructions via the `ctrl` channel
from the consumer stage. The consumer sends one of the following signals.

The "commands" the producer might receive include ...

* `STAT`

When the consumer sends "STAT" on the control pipe,
the producer must send a description of the waiting record.
At this point in the development, the only meta data that a
producer sends would be "`DATA` *bytes* *seq*", and *seq*
is optional.

* `PEEK`

Think PIPLOCAT to examine a record, or the Rexx command '`PEEKTO`'.

The producer sends data, the content of the record.

* `NEXT`

Think PIPINPUT (sort of) and the consumer consumes the record.

The producer sends advances the sequence count.
The library function returns and the producer stage is unblocked.

* `QUIT`

This is for SEVER operation.

The producer sends "`OKAY`".

And we must have ...

* `FAIL` *errorcode*

if something went wrong.

## Example Write and Read of one Record

The producer calls `output(,srcbuf,)` to write a record.

The consumer calls `readto(,dstbuf,)` to read a record.

                          producer            consumer
                      read(ctrl,,) <--------- write(ctrl,"STAT",)
     write(data,"DATA bytes seq",) ---------> read(data,,)
                      read(ctrl,,) <--------- write(ctrl,"PEEK",)
          write(data,srcbuf,bytes) ---------> read(data,dstbuf,bytes)
                      read(ctrl,,) <--------- write(ctrl,"NEXT",)

## A word about Shared Memory

CMS/TSO Pipelines gets a major performance advantage by sharing memory
from stage to stage. Please understand that here we're using POSIX sockets.
In the name of "keep it simple", use of POSIX pipes is the starting point.
But shared memory is planned for the future. Even so, there will always
be situations where memory cannot be reliably shared, so POSIX pipes
(file descriptors) are always available.


