# Ductwork Functions

This page lists some functions of the Ductwork library.

These are native functions, expressed here in C, but callable from
any language which compiles to native machine code.
These are functions that the stages call.

`pc` is a pointer to a "pipe connector" struct or "pipe descriptor".
Any practical stage will have at least one input PC or output PC.
Most stages will have both input and output and may have multiples of either or both.

`buffer` is a pointer to a record data buffer.

`buflen` is an integer indicating the size of the buffer (for input)
or the length of the record (for output). A positive return code
indicates the number of bytes in the record (for input) or the number
of bytes successfully written (for output).

## Ductwork Functions used by Stages

The following functions are available for Ductwork stages.

* peekto

Use the `peekto()` fuction to examine an input record without consuming it.

    rc = xfl_peekto(pc,buffer,buflen);

The pipe descriptor struct referenced by `pc` must be an input connector.

If `buffer` is NULL or if the size indicated by `buflen` is zero,
then `peekto()` will report the record size without saving it to the buffer.

The return code will indicate the actual number of bytes in the record.
A negative return code indicates an error.

`peekto()` is called similarly to the POSIX system `read()` function.
`peekto()` is inspired by the 'PEEKTO' command for Rexx based stages.

* readto

Use the `readto()` fuction to read (or just consume) an input record.

    rc = xfl_readto(pc,buffer,buflen);

The pipe descriptor struct referenced by `pc` must be an input connector.

If `buffer` is NULL or if the size indicated by `buflen` is zero,
then `readto()` will consume the record without saving it to the buffer.

Note that records can have zero length.
To read a zero-length record, supply a `buflen` of at least 1.
The return code will indicate the actual numberof bytes in the record.
A negative return code indicates an error.

`readto()` is called similarly to the POSIX system `read()` function.
`readto()` is inspired by the 'READTO' command for Rexx based stages.

* output

Use the `output()` function to write a record.
`output()` blocks until the record is consumed.

    rc = xfl_output(pc,buffer,buflen);

The pipe descriptor struct referenced by `pc` must be an output connector.

The return code will indicate the actual numberof bytes written.
A negative return code indicates an error.

`output()` is called similarly to the POSIX system `write()` function.
`output()` is inspired by the 'OUTPUT' command for Rexx based stages.

* sever

Stages can `sever` a connection anytime it is no longer needed.

Use the `sever()` function to sever a connection.
The connection can be either input or output.

    rc = xfl_sever(pc);

`sever()` takes one argument, the connector pointer.

* stagestart

Before a stage can process pipeline input or output it must instantiate
structures referencing the file descriptors supplied by the launcher.

Use the `stagestart()` function to initialize a stage.

This is mandatory. <br/>
Stages are launched by the operating system and will need structures
allocated and populated with essential information about the pipeline
they're running in.

    rc = xfl_stagestart(\&pc);

`stagestart()` takes one argument, a pointer to a pipeline struct anchor.
Note that is a pointer to a pointer, two levels of indirection.

* stagequit

Use the `stagequit()` function to terminate a stage cleanly.
While the operating system will clean-up resources when simply
exiting the stage program, calling stagequit() will perform
proper severing of the connectors allowing other stages to respond.

    rc = xfl_stagequit(pc);

`stagequit()` takes one argument, the pipeline struct anchor. (a pointer)


