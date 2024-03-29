#  Heap-Management-Operating-Systems
This program is my own implementation of malloc and free. 
Building and Running the Code:
The code compiles into four shared libraries and four test programs. To build the code, change to your top level assignment directory and type:
make
Once you have the library, you can use it to override the existing malloc by using LD_PRELOAD:
$ env LD_PRELOAD=lib/libmalloc-ff.so cat README.md
or
     
$ env LD_PRELOAD=lib/libmalloc-ff.so tests/test1
To run the other heap management schemes replace libmalloc-ff.so with the appropriate library:
Best-Fit:  libmalloc-bf.so
First-Fit: libmalloc-ff.so
Next-Fit:  libmalloc-nf.so
Worst-Fit: libmalloc-wf.so

What this program does?
Using the framework of malloc and free provided on the course github repository:
1. Implement splitting and coalescing of free blocks. If two free blocks are adjacent then combine them. If a free block is larger than the requested size then split the block into two.
2. Implement three additional heap management strategies: Next Fit, Worst Fit, Best Fit (First Fit has already been implemented for you).
3. Counters exist in the code for tracking of the following events:
• Number of times the user calls malloc successfully
• Number of times the user calls free successfully • Number of times we reuse an existing block
• Number of times we request a new block
• Number of times we split a block
• Number of times we coalesce blocks • Number blocks in free list
• Total amount of memory requested
• Maximum size of the heap
The code will print these statistics ( THESE STATS ARE FAKE) upon exit and should look like this:
mallocs:   8
frees:     8
reuses:    1
grows:     5
splits:    1
coalesces: 1
blocks:    5
requested: 7298
max heap:  4096
You will need to increment these counters where appropriate.
4. Four test programs are provided to help debug your code. They are located in the tests directory.
5. Implement realloc and calloc:
        void *calloc(size_t nmemb, size_t size);
        void *realloc(void *ptr, size_t size);
