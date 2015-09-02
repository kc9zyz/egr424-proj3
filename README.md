# egr424-proj3
###Kernel for LM3S6965 Microcontroller
This project comprises of a pre-emptive scheduler to handle multiple threads of execution.

Threads may be added by adding an entry to the threadTable[] with the thread's function address.

###Details
The kernel runs threads in handler mode, controlling a series of processes running in thread mode. Processes have unprivileged access to resources, as well as separate stacks using the process stack pointer.
