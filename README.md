fifo-to-stdin
=============
## Use at your own risk
##### This program has been writen with no harms in mind, however:
##### no guarantee what-so-ever that this program actually does what it has been specified to
##### i am not responsible for any damage to you and/or your system and/or the persons using your system, resulting from the usage of this program
##### you have been warned.
<br><br>
program designed as a helper function for the start-stop-daemon.

the start-stop-daemon cannot redirect the stdin of the daemonized process.
this program redirects everything read from a fifo (via tail -f <fifo>) through a pipe to the program <prog>

### Usage: fi2sti \<fifo\> \<prog\> \<prog_arguments\>
where 
* fifo is a with "mkfifo" or "mknod p" created FIFO file<br>
* prog is the program which shall have stdin replaced in favor of the \<fifo\> file<br>
  * every further arguments are getting parsed, without modification, to the program \<prog\><br>

---

### Why not just pipe it like: `/bin/bash prog args < fifo`
As soon as one write to the fifo file (like `echo help > fifo`) is complete, the fifo signals an eof and the underlying program prog might stop or lose it's stdin.

### So how is it solved here?
The way to solve this generally is `tail -f fifo | prog args`.<br>
tail -f fifo will normally not stop reading out the growing lines of the specified file, 
except it receives a signal like SIGINT, SIGTERM, etc.
The pipe between tail and prog (under normal circumstances) is therefore not destroyed so even after you are done writing to the fifo you can open it again, write something to it and the text is being transmitted to stdin of the program prog.<br><br>
And that's almost what this program does, except it varies a little in order to make life a little easier when dealing with start-stop-daemon. <br> The start-stop-daemon only closes the process with a specified pid, so having that in mind this program does the following:

### How it works:
First it opens the program "tail" by creating a pipe, forking and invoking the shell (via popen()).<br>
"tail" is also being told to terminate if the process with the parent pid dies (via --pid=\<parent_pid\>). <br>
A file descriptor is being created from the output stream of "tail".<br>
The stdin file descriptor of the parent process is being copied to the file descriptor of the output of "tail" (via dup2()).<br>
Lastly the program \<prog\> is being launched with execvp.<br>
