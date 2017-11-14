shmem is a max object that allows users to use named shared memory in Max programming environment by Cycling'74 for exchanging data stored in buffer~ with another software faster than OSC-based communication between two software.
This is especially useful when you want to visualize the content of buffer~ with your own OpenGL software coded in C++, C, C# or Java.

The code uses Windows API, so that porting to Mac OSX is not trivial.
