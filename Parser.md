# Ductwork Parser

This page describes the pipeline parsing in Ductwork.

When a pipeline is instantiated from a shell,
care must be taken that the pipeline specification be quoted.
This is especially important for the stage separator so that it
will be interpreted by the parser and not by the shell.

## Stage Separator

The default stage separator character is a vertical bar `\|`.

Stages in Ductwork, as with CMS/TSO Pipelines,
are distinguished by a separator character. By default,
this character is a vertical bar so that Ductwork pipelines
look visually similar to shell pipelines.

    --stagesep char
    --separator char

You cannot specify left parenthesis, right parenthesis, asterisk (*), 
period, colon (:), or blank for the stage separator character. 

## Pipeline Separator (Endchar)

The default pipeline separator character is exclamation `\!`.

Multiple simultaneous streams can be defined, typically with
connections between pipelines. (Not strictly required.)
Stream definitions are distinquished by the pipeline separator.

    --endchar char

You cannot specify left parenthesis, right parenthesis, asterisk (*), 
period, colon (:), or blank as the pipeline end character. 

## Pipeline Escape Character

When a special character, such as the stage separator
or the end character, must be part of an argument or a literal,
it must be "escaped". For that, use the escape character.

The default pipeline escape character is backslash `\\`.

    --escape char

You cannot specify left parenthesis, right parenthesis, asterisk (*), 
period, colon (:), or blank for the escape character.

## Command Options

The main Ductwork command allows options to be specified using
VM/CMS style, for nominal compatibility with CMS/TSOPipelines,
or using Unix style as is somewhat easier on other systems.

    (stagesep char endchar char escape char)

Open parenthesis has special meaning for the shell,
so the above must be enclosed within quotes.

Furthermore, the standard stage separator also has meaning to the shell,
so the entire pipeline needs to be enclosed in quotes.

You can put CMS/TSO syntax options (that is, with parenthesis)
in the same quoted argument as the pipeline.

All options, either syntax, must come before the pipeline.

Unix style options should come before TSO/CMS style otions.


