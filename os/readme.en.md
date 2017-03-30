# Operating Systems

The `startup_simulator.ko` module is a next-generation startup simulator running in the Linux kernel (simulations need to be fast, and it is well known that everything is faster in the kernel). A startup simulation can be started by feeding the simulator the startup's name and its business plan as a PDF document. Following patented calculations on that file, the simulator predicts with revolutionary accuracy the net worth of the business for the next 16 years.

The module can be controlled through the special file `/dev/startup_simulator`. Your goal is to create a program that controls the simulator and exposes its capabilities through a TCP socket so that you can yourself become rich by selling access to ~~gullible~~visionary entrepreneurs.

You may use the language of your choice to complete this task. Do note, however, that only a C source template is provided. This template establishes the network connection and pre-defines the structures that you need to use to communicate with the driver.

## Client Interface: requests

> To help you, the competition package includes a `startup_simulator.py` script, which implements a client that you can use to test your server. You may also refer to its implementation.

The server must listen on port 4321.

The communication protocol with TCP clients is based on synchronous commands: the client sends a request and the server replies. Connections are terminated unceremoniously: either side merely needs to close their socket. Connections manage a single startup simulation and are not reusable.

**Every integral value is sent in network byte order** (see `man htonl` if you aren't familiar with this notion). This fact is reiterated in this document at every opportunity.

The command and response header is made of two fields with a total size of 4 bytes:

* 2 bytes: complete size of the command (header included), in network byte order;
* 2 bytes: command type, in network byte order.

The size of a command's or response's body is therefore (total size - 4).

The body's payload must be interpreted according to the command type. The client can send two types of commands:

* `BEGIN_STARTUP` (0)
* `BUSINESS_PLAN_DATA` (1)

### BEGIN_STARTUP

The body of the `BEGIN_STARTUP` command starts with the capital, in dollars, that the enterprise starts with. This number is encoded over 4 bytes in network byte order. The rest of the body encodes the name of the startup in UTF-8 and without a null terminator. The name can be at most 99 bytes long.

(20 bytes, type 0: enterprise "Smartsuppose", initial capital 0x8344 $)

	00000000  00 14:00 00:00 00 83 44: 53 6d 61 72 74 73 75 70  |.......DSmartsup|
	00000010  70 6f 73 65                                       |pose|

Since connections are not reusable, it is an error to send a second `BEGIN_STARTUP` message. It is also an error to send any other command until an initial `BEGIN_STARTUP` command has been sent.

### BUSINESS_PLAN_DATA

The body of a `BUSINESS_PLAN_DATA` command is a chunk of the business plan in PDF format. The client must send this command as many times as necessary to complete the transfer. It signals the end of the transfer by sending a packet of this same type with an empty body.

The size of the chunk is determined from the size of the command.

	00000000  1c 4f:00 01:25 50 44 46  2d 31 2e 35 0d 25 e2 e3  |.O..%PDF-1.5.%..|
	00000010  cf d3 0d 0a 31 39 38 20  30 20 6f 62 6a 0d 3c 3c  |....198 0 obj.<<|
	00000020  2f 4c 69 6e 65 61 72 69  7a 65 64 20 31 2f 4c 20  |/Linearized 1/L |
	00000030  ...

## Client Interface: responses

The server responds to the client using the same header format. The server's response codes are:

* `ACKNOWLEDGED` (2)
* `ANGEL_INVESTOR` (3)
* `PREDICTION_RESULT` (4)
* `ERROR` (5)

### ACKNOWLEDGED

The `ACKNOWLEDGED` response tells the client that they can resume sending data. It is the usual response to `BEGIN_STARTUP` and `BUSINESS_DATA_PLAN` commands. This response has no payload.

    00000000  00 04 00 02                                       |....|

### ANGEL_INVESTOR

The `ANGEL_INVESTOR` response tells the client that at some point since the last send, the simulation has found that an angel investor will take great interest in the business early on and will radically transform the project. Therefore, simulation must be stopped because the business plan will change that much. This response has no payload and terminates the connection.

    00000000  00 04 00 03                                       |....|

### PREDICTION_RESULT

The `PREDICTION_RESULT` response is used when the client has finished transferring his business plan and the simulation has completed. The response includes the value of the prediction, in network byte order. Refer to the *Driver Interface* for more information on the result of the simulation. This response terminates the connection.

(In this example, the business has failed and its value quickly falls to 0:)

	00000000  00 84:00 04:00 00 01 00  00 00 02 00:00 00 03 00  |.C..............|
	00000010  00 04 00 00:00 05 01 00  00 06 01 00:00 08 01 22  |..............."|
	00000020  00 08 f2 40:00 00 00 00  00 00 00 00:00 00 00 00  |...@............|
	00000030  00 00 00 00:00 00 00 00  00 00 00 00:00 00 00 00  |................|
	00000040  ...                                               |...|

> **WARNING!** If you write a C structure to represent this packet, be mindful that the native alignment of 64-bit integers might leave a 4-byte hole between the packet header and the first result.

### ERROR

The `ERROR` response tells the client that an error occurred. It is the standard mechanism for the server to report any problem that it finds, either with its internal state or because the client is requesting an impossible operation. The response's payload must include a character string explaining to the best of your ability what happened. This string should not be null-terminated. Errors that are caused by the client performing an illegal operation (for instance, sending data before starting the simulation) should not terminate the connection.

	00000000  00 1f:00 05:53 70 6c 69  6e 65 20 72 65 74 69 63  |....Spline retic|
	00000010  75 6c 61 74 69 6f 6e 20  66 61 69 6c 65 64 21     |ulation failed!|

## Driver Interface

Your first action should be to open `/dev/startup_simulator` in read-write mode. You can then use the `ioctl` system call to control the driver's state.

Note that while the special file can be opened multiple times, by one or many processes, there is only one instance of the simulator and every program communicates with it. Consequently, if you open the file multiple times and use the `CSGAMES_SET_STARTUP` ioctl on the first file descriptor, the change will also be visible on the second file descriptor, even in different processes.

In addition to regular read and write operations, the special file supports the following `ioctl` calls:

* `CSGAMES_NEW_STARTUP` (0x100)
* `CSGAMES_SET_STARTUP` (0x101)
* `CSGAMES_GET_PROCESSING_END` (0x102)
* `CSGAMES_GET_ANGELS_PICK` (0x103)

### CSGAMES_NEW_STARTUP

The `CSGAMES_NEW_STARTUP` ioctl accepts as a parameter the address of a `startup` structure:

	struct startup {
		char name[100];
		unsigned capital_in_dollars;
	};

The call starts the simulation of a new business with that name and capital, and selects this startup as the current startup (see `CSGAMES_SET_STARTUP`). The simulation ends and the associated resources are freed when results are read (see read and write operations below). The module can manage up to 8 startups simultaneously.

Possible errors:

* `EINTR` (operation interrupted by a signal, try again);
* `EFAULT` (invalid pointer);
* `EINVAL` (incorrect name);
* `ENFILE` (8 startups already being processed).

### CSGAMES_SET_STARTUP

The `CSGAMES_SET_STARTUP` ioctl selects the business on which read and write operations take effect. It accepts a character string as its input, representing the name of the startup, terminated by a null character.

Possible errors:

* `EINTR` (operation interrupted by a signal, try again);
* `EFAULT` (invalid pointer);
* `ENOENT` (no startup found with that name).

### CSGAMES_GET_PROCESSING_END

The `CSGAMES_GET_PROCESSING_END` ioctl accepts a pointer to a `timeval` structure. Simulating takes time, and the driver processes chunks of the business plan in the background. This system call tells you when the simulator will be ready to process the next chunk of the business plan (see read and write operations). Do note that you must respect this limit: if you attempt a new read or write operation on a startup that is not ready, you could increase the total processing time required for the simulation.

Possible errors:

* `EINTR` (operation interrupted by a signal, try again);
* `EFAULT` (invalid pointer);
* `EINVAL` (no startup selected).

### CSGAMES_GET_ANGELS_PICK

The `CSGAMES_GET_ANGELS_PICK` ioctl accepts as a parameter a memory area at least 100 bytes long and writes the name of a startup that will be chosen by an angel investor (see `SIGNGEL` below). This command frees the resources associated with the simulation of that startup.

Following this call, no startup is selected for the next read/write operation.

Possible errors:

* `EINTR` (operation interrupted by a signal, try again);
* `EFAULT` (invalid pointer);
* `EINVAL` (no startup picked).

### Read and write operations

Read and write operations apply to the startup selected with the `CSGAMES_SET_STARTUP` ioctl.

The write operation feeds the driver with the business plan data that the client provides. Note that only PDF files are accepted by the simulator.

The simulator does *not* offer strong guarantees about the amount of data that each write operation accepts. In other words, a `write` call passing in N bytes is likely to return a value R that is less than N, meaning that only R out of N bytes were accepted. Your program must account for this and issue another `write` call to write the data that was not accepted on the previous call.

For complex technical reasons, the driver will not accept buffers of less than 4 or more than 32768 bytes at a time. That said, depending on the style of the business plan, it is highly probable that the maximum will never be reached.

A write operation causes a delay in the processing of the simulation. Generally, it will not be possible to read or write to the same simulation within a few seconds. During that delay, it is not recommended to try to read or write more data to the same selected startup, because it could be throttled. (Nothing prevents you, however, from using the `CSGAMES_SET_STARTUP` ioctl to select a different startup and update its simulation independently.)

You can query the simulator for the time when it will be possible again to read or write to a given startup using the `CSGAMES_GET_PROCESSING_END` ioctl.

Possible errors for `write`:

* `EINTR` (operation interrupted by a signal, try again);
* `EFAULT` (invalid pointer);
* `EINVAL` (no startup picked; insufficient buffer size; invalid data);
* `EAGAIN` (simulator still processing previous chunk);
* `EPERM` (startup was picked by angel investor and state can't be modified anymore).

The read operation ends a simulation (releasing the associated resources) and reads the result. This result is returned as a `struct startup_net_worth`:

	struct startup_net_worth {
		struct startup startup;
		unsigned reserved;
		long worth_by_year[16];
	};

The `reserved` field is not currently used.

The driver verifies that the `read` buffer size is *exactly* the same as the size of that structure.

After the read operation, the selected startup is removed and no startup is selected for subsequent operations.

Possible errors for `read`:

* `EINTR` (operation interrupted by a signal, try again);
* `EFAULT` (invalid pointer);
* `EINVAL` (no startup picked; buffer size not equal to the size of a `struct startup_net_worth`; invalid data);
* `EAGAIN` (simulator still processing previous chunk);
* `EPERM` (startup was picked by angel investor and state can't be modified anymore).

### SIGNGEL

It is possible that the simulator detects that a startup has such potential that an angel investor will necessarily pick it up. The patented algorithm that leads to this finding operates asynchronously; therefore, it announces its results using the `SIGNGEL` signal (also known as `SIGUSR1`).

The signal is delivered to the first process that has opened `/dev/startup_simulator`. If this process closes the file, the signal will be delivered to the next process that opens it. If no process is available, the signal is simply not delivered. For instance, if process 188 opens the file, and then process 190 also opens it, the signal will be delivered to process 188. If 188 closes the file, the signal will not be delivered, even if the file is still open in process 190. If process 201 opens the file later, it will now receive the signal.

As explained in the client interface section, this information must be used to send an `ANGEL_INVESTOR` response to the client that has created the simulation and terminate the connection.

The name of the startup selected by the investor can be queried using the `CSGAMES_GET_ANGELS_PICK` ioctl. The chosen business can no longer be the target of read or write operations.

Note that the default action of the `SIGNGEL` signal is to terminate the process that receives it.

## YOUR PACKAGE CONTAINS

This package should contain the following files:

* readme.fr.pdf
* readme.en.pdf
* startup\_simulator.py
* csgames.c

Let an organizer know as soon as possible if files are missing.

### readme.fr.pdf

This document (in French).

### readme.en.pdf

This document.

### startup_simulator.py

A Python script that you can use to test your program.

### csgames.c

A C program skeleton that you can use to kickstart your program's development.

### poop.pdf

An example of a great business plan that you can use to test your program.

## ALSO INSTALLED

The `ngelctl` program allows you to control some aspects of the angel investor simulation. Four actions are possible:

* `ngelctl -d`: turns off angel investor simulation
* `ngelctl -e`: turns on angel investor simulation
* `ngelctl [-s] startup`: makes the angel investor pick that startup
* `ngelctl -c`: resets the angel investor's choice (clearing the startup in the process)

This program will obviously not be available in the grading environment.

## Grading sheet

Grading uses two main criteria: the correctness of the results and the performance of your server. Correctness points are granted entirely or not at all, depending on whether the feature works or not. Performance points are only granted if the feature works, and are granted as a ratio of how fast your program successfully executes the tasks versus your competitors. For instance, if:

* you succeed at a task that has 10 performance points;
* your program accomplishes that task in 4.3 seconds;
* the fastest team's program accomplishes that task in 3.5 seconds;
* the slowest team's program accomplishes that task in 7.2 seconds

then you get 10 - 10 * ((4.3-3.5)/(7.2-3.5)) = 7.84 points. Abstractly, the formula is `points - points * ((you - min) / (max - min))`. (If a single team completes a feature with performance points, full performance points are granted.)

In the grading environment, angel investor simulation is **enabled** during the feature test, and **disabled** during performance calculation.

| Feature | ✔︎ | ⏱ |
|------------------|-------------------------:|---------------------------:|
|The server accepts two consecutive connections.|1|0|
|The server can handle the creation of a startup.|1|0|
|The server reacts correctly when it receives `SIGNGEL` with a single simulated startup.|5|0|
|The server reacts correctly when it receives `SIGNGEL` with four simulated startups.|10|0|
|The server can handle a startup business plan.|5|15|
|The server can handle two startup business plans.|1|5|
|The server can handle three startup business plans.|1|0|
|The server can handle four startup business plans.|1|5|
|The server can handle five startup business plans.|1|0|
|The server can handle six startup business plans.|1|0|
|The server can handle seven startup business plans.|1|0|
|The server can handle eight startup business plans.|1|5|
|The server can handle ten startup business plans.|1|20|

Additionally, 25 more points will be granted according to how your server reacts on undisclosed edge cases and error conditions. Finally, up to 5 points can be granted for subjective appreciability of your code.

| Category | Points |
|:-----------|-------:|
|Features|30|
|Performance|50|
|Edge case behavior|25|
|Style and elegance|5|
|**Total**|110|

## Help and debugging

Here is a list of functions that you may want to look at.

* man 2 socket, bind, listen, accept
* man 2 open, flock, read, write, ioctl, close
* man 2 signal, kill, fork, waitpid
* man 2 gettimeofday
* man 3 htonl

The following tools are available in your development environment but will be **disabled** in the grading environment:

* dmesg
* ngelctl (see above)

### Gotchas that a lot of you will fall for and for which grading will be merciless

* Ensure that every integer value that you send over sockets is in network byte order.
* Every `ioctl` call handled by the startup simulator can fail with `EINTR` and just needs to be retried.
* Most `read` and `write` calls can fail with `EINTR` or only partially succeed and need to be retried/restarted where they stopped.
