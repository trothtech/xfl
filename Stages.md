# XFL Stages

This document lists a number of POSIX Pipelines stages
which must be included in a basic implementation.

## XFL Stages

The currently available stages are listed here in no particular order.


* buffer

Use the `buffer` stage to hold all input records
until the source stage terminates.


* console

The `console` stage is so named for compatibility with CMS/TSO Pipelines.
In POSIX Pipelines, `console` serves as a gateway between shell pipes and POSIX Pipelines stages.

When `console` is the first stage of a pipeline, it reads lines of text from file descriptor zero (0)
delimited by newline characters. In this mode, its output must be connected
to another POSIX Pipelines stage and its output is records with newline characters removed.
Output records are bounded where newlines occur on input.

When `console` is NOT the first stage of a pipeline,
its input must be connected to another POSIX Pipelines stage. It then reads input records
and writes to file descriptor one (1) with newline characters appended to each line.
In this mode, `console` may also be connected on output to a following POSIX Pipelines stage
to which it will write the input records unaltered. (No newline inserted.)


* cp

Use the `cp` stage to issue VM (CP) commands and recover their output.

This stage requires the z/VM hypervisor.
It really only works on z/Linux when hosted by z/VM.

This stage requires privileges. Usually, one must be root to issue CP commands.


* fanin

Use the `fanin` stage to collect multiple input streams
into a single output.


* filer, aliased as "&lt;"

Use `<` to read from a file.


* filew, aliased as "&gt;"

Use `>` to write to a file.


* literal

Use the `literal` stage to insert a line of literal text into a stream.


* locate

Use the `locate` stage to find occurrences of the specified string in the input stream.


* strliteral

Use the `strliteral` stage to insert a line of literal text into a stream.


## XFL Stage Operation

Stages run as independent programs.
<br/>
They inherit open file descriptors which are described in an
inherited environment variable and then use `peekto()`, `output()`,
and `readto()` functions to process incoming and outgoing records.


