GPU quick sort 

Summary 

This is the OmpSs version of the gpu-quicksort

    * License: GPL, MIT, BSD, Copyright, other
    * Access level: Public, Restricted, Private
    * Repository:  http://pm.bsc.es/svn/...
    * # of versions: 2
    * Contact person: rosa.m.badia@…
    * Integration level: Stand-alone, ... "???"
    * Original code (Link) 

Description 

The gpu-quicksort was modified to be executed using the OmpSs framework in order to exploit the parallelism.

Origin 

@article{Cederman:2010:GPQ:1498698.1564500,
 author = {Cederman, Daniel and Tsigas, Philippas},
 title = {GPU-Quicksort: A practical Quicksort algorithm for graphics processors},
 journal = {J. Exp. Algorithmics},
 issue_date = {December 2009},
 volume = {14},
 month = {January},
 year = {2010},
 issn = {1084-6654},
 pages = {4:1.4--4:1.24},
 articleno = {4},
 numpages = {.84},
 url = {http://doi.acm.org/10.1145/1498698.1564500},
 doi = {http://doi.acm.org/10.1145/1498698.1564500},
 acmid = {1564500},
 publisher = {ACM},
 address = {New York, NY, USA},
 keywords = {CUDA, GPGPU, Sorting, multicore, quicksort},
} 


General Algorithm 

Versions 

The code is still under development and there is no funcional version of it.

Know issues 

Crash after the beginning of the execution

Execution instructions 

Just use the c.sh do compile and execute the binary. A sample dataset of 300 points is sorted automatically.

Performance Results 

Results

References 

Downloads 

Source code 

Input sets 

Traces 

Other 
