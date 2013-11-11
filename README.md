sparsefileutils
===============

A collection of simple tools to copy backup and archive sparse files.

Sparse files, files that contain holes in the underlying filesystem,
are frequently used as image containers for virtual machines or remote
storage (iSCSI, nbd, ...). 
These tools faciliate the handling of sparse files because they only copy,
read, or write blocks to disk that really need to be written. This improves
data locality, block availability, and TRIM performance on flash based
media considerably. It also provides a means to do overcommitment on
hosting solutions.  

sbcp 
----
Copies one file to another, preserving sparse blocks and making all-zero
blocks sparse.

sbcat 
-----
Cats a file omitting the read of sparse blocks.

sbdiff 
------
Performs a binary diff over two files and produces a difference file with
sparse areas.

sbpatch 
-------
Applies a sparse patch file from sbdiff to the original file

sbmapcat 
--------
Produces a data file and a map file of an origial file with sparse blocks.

sbmapmerge 
----------
Merges a map and a data file created with sbmapcat into a sparse file.

sbsparsify (install disabled) 
-----------------------------
Scans a file for all-zero blocks and makes those blocks sparse. 
Note: This does not work reliably on all filesystems (i.e ext4).

sbinfo 
------
Shows sparse-block and all-zero-block information of files.

sbwrite
-------
Writes stdin to a file, omitting all all-zero blocks.
