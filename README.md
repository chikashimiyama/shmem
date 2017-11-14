shmem is an external object for Max programming environment by Cycling'74 for exchanging data stored in buffer~ with another software faster than OSC-based communication between two software.
This object employs a named shared memory feature that windows API provides, thus, it is not compatible with MacOS.
This object is especially useful when you want to visualize the content of buffer~ with your own OpenGL software coded in C++, C, C# or Java.

A copy of 256 floating point numbers from buffer~ to shared memory takes usually less than 1 nano second.
