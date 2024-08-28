# XFL

XFL/Conduit - 
a utility for flow-based programming,
a CMS Pipelines workalike for POSIX systems

XFL provides on POSIX systems the same functionality
as CMS/TSO Pipelines provides on IBM mainframe systems CMS and TSO.

An alternate name for the project is "Conduit", provided by
the late Dave Jones, "Sir Dave the Generous". Other names include
"[ductwork](ductwork.md)" and "[plenum](plenum.md)". The confusion
is because to merely call it "Pipelines" conveys the implication that
it's just shell pipes. But this project is way more than shell pipes.

The XFL tag was provided by IBM,
recognizing the relationship between this project and CMS Pipelines
(with the tag FPL).

## Goals

Goals of the XFL project:

* simulate CMS/TSO Pipelines for POSIX environments
* allow stages to run independently alongside other programs
* impose minimal infrastructure and environmental requirements
* provide as many pre-defined stages as practical
* allow custom stages in any language

Each "stage" is a separate process.

Stages can be written in any language.

The "dispatcher" arranges file descriptors and spawns the stages.
It then lingers, much like the shell does when running multiple
stages (but the plumbing here is stronger), then exits when all
sub-processes have exited.

Hereafter we refer to it as the "launcher" because it is
not a dispatcher in the sense of scheduling threads.
That job is left to the operating system kernel.

## XFL

XFL is a CMS/TSO Pipelines work-alike for POSIX systems,
providing the same basic functionality as that of of CMS/TSO Pipelines,
but in a POSIX environment where IBM VM/CMS and IBM MVS/TSO are not available.

XFL is compatible, as much as is possible, with CMS Pipelines
at the command-line level. Dispatching is handled by the underlying
operating system (Unix, Linux, Windows). The difference in dispatching
is a major functional difference between XFL and CMS Pipelines.

XFL `pipe` command accepts traditional Unix style options (with dashes)
but also accepts traditional CMS style options (in parentheses).

XFL does not require a closed system such as containment within a
Java virtual machine (JVM). But this does not exclude use of Java
for writing XFL stages and a Java Native Interface is under development.

Too many technologies, services, systems, environments, projects,
and libraries refer to "pipelines", so we use the name XFL or Conduit
in hopes of conveying the full idea.

## Overview

XFL provides a command, '`pipe`'. The argument string to the
'`pipe`' command is the pipeline specification. '`pipe'` selects
programs to run and chains them together in a pipeline to pump data through.

XFL pipelines do not involve the usual POSIX stdin and stdout interface.
The chaining of XFL stages is more robust, bounded out of band,
and flow controlled. XFL has a library of built-in programs that
can be called in a pipeline specification. These built-in programs
interface with the operating system, and perform many utility functions.

Data on CMS is structured in logical records (or "packets" or "messages")
rather than a stream of bytes. For textual data, a line of text corresponds
to a logical record without a newline marker character. The data is passed
by XFL between the stages as logical records.

XFL users issue pipeline commands from the terminal, or in shell scripts,
or in any standard programming environment. Users can write XFL "stage"
programs in any language which compiles to native or in any interpreted
language which can call the support library. This includes the popular
Rexx language common on CMS and TSO. Home grown stages supplement
the built-in XFL programs.

This document does not attempt to explain CMS Pipelines or TSO Pipelines.
For general information about CMS/TSO Pipelines, see the web page

https://en.wikipedia.org/wiki/CMS_Pipelines

## Additional Links

http://trothtech.us/pipelines

http://www.casita.net/pub/xfl/


http://vm.marist.edu/~pipeline/

https://www.youtube.com/watch?v=AUsCdmjlaSU

http://www.rvdheij.nl/pdweb/

## Contributing

Here's
how we suggest you go about proposing a change to this project:

1. Fork this project to your account.
1. Create a branch for the change you intend to make.
1. Make your changes to your fork.
1. Send a pull request from your
fork's
branch to our master branch.

Using the web-based interface to make changes is fine too,
and will help you by automatically forking the project
and prompting to send a pull request too.








































































