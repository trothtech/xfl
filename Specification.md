# Ductwork Specification

This document outlines the design specification of
the Ductwork project and enumerates the essential features.

## Ductwork Design

Ductwork is a pipeline environment supporting "flow programming".
It is inspired by CMS/TSO Pipelines and is intended to support
a subset of the capability and syntax of that implementation.

A basic Ductwork pipeline looks like a shell pipeline.
Unix and Linux shell users will recognize the syntax.

A Ductwork pipeline can have multiple streams.
Users of CMS/TSO Pipelines will immediately be able to construct
pipelines that they could not with shell alone.

Features of the Ductwork project:

* out-of-band bounding

Records (or "packets" or "messages") in a Ductwork pipeline are
conceptually similar to lines of text in a plain text file or stream,
such as program source. But records in Ductwork are delimited out of
band so they can contain anything. (Embedded newline characters,
or any characters including NULL, do not affect the structure.)

* independent stages

Pipelines are composed of independent programs called stages.
Traffic flows through the pipeline moving from the output of one stage
to the input of another stage.

* packetized flow (bounded messages, record-oriented)

Records (or packets, or messages) flow through the pipeline
as whole entities. A record may correspond to a line of text.
(Though traditional Unix newline interpolation is opaque.
A record may contain any number of newline characters
without affecting the structure of the record.)

* peek first, then consume

A record can be examined without being consumed.
This allows changes of flow based on content.

A stage can "peek" at an input record and deliver it (or its effects)
downstream before actually consuming the input record. In CMS/TSO Pipelines
a stage with this characteristic is said to "not delay the record".
To delay the record (or not) has a significant effect on traffic flow.

* labeled streams

The parser can assemble multi-stream pipelines using labels.
This makes for a superset of shell pipeline syntax.

## Differences from CMS/TSO Pipelines

Ductwork uses the operating system kernel for scheduling.
It does not have its own dispatcher. This differs from CMS/TSO Pipelines
which has a dispatcher independent from the operating system dispatcher.

CMS/TSO Pipelines concepts such as commit levels are not supported in Ductwork.

## Differences from Shell Pipes

It's possible to construct multi-stream pipelines in some shells,
but the syntax is difficult.

Writes in shell pipes do not block.
While a non-blocking write sounds convenient, it negates flow control.

## Differences from Named Pipes

Named pipes make multi-stream pipelines easier to describe in shell speak.

However, a read is not guaranteed to return only one chunk at a time.

Also, writes do not block and there is no traffic flow.

## Differences from Named Sockets

Named sockets, being local, allow delivery of the entire message immediately.

It's not clear that reads are quantized.

Writes do not block, no traffic flow.

## Differences from TCP Sockets

TCP sockets explicitly do not guarantee message boundaries.
Bytes (octets) are assured to arrive in order, but traffic may be
quantized arbitrarily from source endpoint to target endpoint.

Writes with TCP Sockets do not block.


