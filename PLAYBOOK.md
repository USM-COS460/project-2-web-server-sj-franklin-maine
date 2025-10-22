## COS 460/540 - Computer Networks
# Project 2: HTTP Server

# SJ Franklin

This project is written in C++ on Linux.

## How to compile

You'll want to compile an object file with:

```bash
g++ -o <object-file-name> server.cpp -pthread
```
I also learned you CAN use the ssh command in Powershell to forward the traffic requests/results from a Linux machine to display on, for example, a Windows machine.

## How to run

After compiling, run the object file and pass the port number and the directory path for your webpage files on the command line.
Example:
```bash
<object-file-name> <PORT-NUMBER> <DIRECTORY-PATH>
```

## My experience with this project

This one was fun! Weirdly enough I felt like the part of the assignment that took me the longest amount of time was reading 
the terminating sequence and getting the contents of the file (I was trying to use FILE * and fopen() but was having some weird
stuff happen when trying to open files in binary mode). The rest of the parsing went fine once I switched to i/o stringstreams.
I was expecting the threading to be more difficult too, but I think testing the functionality to make sure all of the files were
being sent properly in the browser first, and then moving that to threads made the process smoother for me. That and someone on
stackoverflow had the same issue with pthreads I did 16 years ago, which made fixing it easy. I think I forgot to include the datetime
in the header response I send, and I think I could probably condense the file parsing into another function that just takes in an extension
string and its associated MIME type. I tested this in Firefox and Microsoft Edge and the test pages were loading fine, but I noticed that
sometimes the server seems to get stuck in a loop on the Brave browser. It sometimes fixes itself too, which is weird. I couldn't replicate
the issue in my other browsers, so maybe it's a Brave browser thing? I might look more into what they expect to receive from a server, and
see if there's something wonky there.
What I learned overall: web servers are simpler than I thought, and they have very organized structures that makes loading files convenient 
(just need to make sure that you are sending along the correct header and the right data). 