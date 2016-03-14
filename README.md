# Netdeploy - threaded deployment of local terminal applications to the network
Copyright (C) 2016, [Mark O'Neill](//markoneill.name/).  


## Overview

Netdeploy is a tool for deploying local terminal applications through
the network. Netdeploy allows multiple clients to simultaneously
access independent instances of the hosted application via threading.
Clients may access the deployed application via telnet, netcat, other
standard network utilities, or their own custom interfaces, via TCP.

Netdeploy operates similarly to netcat run with the "-e" option.
However, Netdeploy differs in several noteworthy ways:

- Netdeploy will continue to serve and listen for connections after
  a client disconnects
- Netdeploy allows multiple simultaneous clients to be connected to
  independent instances of the hosted application
- Netdeploy allows users to restart the hosted application after it
  terminates without having to reconnect to the service
- Netdeploy forces applications to flush output buffers by using the
  LD\_PRELOAD trick and overriding common output functions (printf,
  fprintf, etc.) to call fflush(). This allows hosted applications
  to be more interactive.

Netdeploy was inspired by the need for ethical hacking competitions
to host arbitrary vulnerable binaries for remote exploitation. It is
therefore intended for but not limited to this purpose.

Since Netdeploy allows arbitrary programs to be run by the user it is
imperative that these applications be vetted beforehand to ensure there
is no risk to the host machine if the hosted application is subject to
buffer overflow or similar attacks. Note also that Netdeploy has not
been subject to a thorough security analysis and may itself represent
a security hole (which is similar in spirit to the "gaping security
hole" nature of "netcat -e".

## Requirements

Netdeploy uses pthreads for threading and the LD\_PRELOAD trick for
forcing hosted applications to flush their output buffers when writing
to an output stream. Thus it will work best on Linux but should 
provide basic functionality on other operating systems (assuming they
have sufficient POSIX support). Note that Netdeploy has not yet been 
tested on any system besides Linux.


## Installation

    make


## Usage

    ./netdeploy [-t threads] [-p port] prog_name prog_arg1 prog_arg2 ...

Options: 

- "-t" assigns the number of threads (default 10) to use 
- "-p" assigns a listen port (default 8888)

Arguments:

- prog\_name path and name to the application to be hosted
- prog\_args\* (supports multiple) arguments for the hosted application


## Usage Examples

Run echo\_test (included) with 50 threads on port 8080

    ./netdeploy -t 50 -p 8080 echo_test

Run date +"%y-%m-%d" with 10 threads on port 3333

    ./netdeploy -t 10 -p 3333 /usr/bin/date +"%y-%m-%d"

## Development

Netdeploy is being developed on Github.  For bug reports, please use the Github
issue tracker.  For patch submissions, please send me pull requests.

https://github.com/PhoenixTeam1/netdeploy


## License

Netdeploy is released under the GNU Lesser General Public License v2.1.


## Contributors

-   Mark O'Neill (@PhoenixTeam1), main author
