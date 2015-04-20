# Kernel Module :  ASE ++ : Linux Kernel Programming
- AUTHORS : Quentin Warnant & Antoine Durigneux
- Date : 20/04/2015

# The project

The goal of this project is to write a kernel module which monitors the execution time of a task on the different cores of a processor. Here are the requirements:

1. The module initially installs file /proc/ase_cmd in the proc file system, and a directory /proc/ase/ which will contain the output of the module.
2. Then it waits for the user to write a valid process ID (PID) on the file (for example with echo). From then on, it starts to track the process execution time on an internal data structure. It also creates a file with name equal to the process ID in the directory /proc/ase/
3. When the user reads from this file, it outputs there the total execution time of the process until now.
4. When the process terminates, the module must understand this and close the corresponding file in the /proc/ase directory, and free the corresponding memory for the data structure.
5. You can think of giving an additional command in /proc/ase/ to stop tracking a certain process. In that case, even if the process has not yet terminated, the corresponding data structures are destroyed, and the memory is freed.
6. Track the execution time of a process on each core. This means that, if the process never migrates, the run time in one of the cores must be zero. Write a test by using affinities that verifies that this is indeed the case (_hint_: you must intercept the appropriate functions to identify when a process migrates).

# How to use 
You can launch the bash script ase_launch.sh in order to run the module. 
```bash
	sudo ./ase_launch.sh 
```
- The script installs the module in the kernel, launches two background commands (yes) and tracks the execution time of theses process between several sleep() calls.
- When the job is done, the script removes the module from the kernel : the folder data of the ase_cmd and the memory associated to the module are freed. 

# Additional information
- We have completed the five first check points of the project. 
- We have started (but not finished) the implementation of the multi-core module. In fact, as you can see in the proct structure, the time data are put in arrays in order to adapt the module for multi-core execution time tracking.

