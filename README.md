# Implementation of Distributed File System

##Introduction
Nowadays,replicating the same information across different locations is an important technique
for improving reliability and performance.For instance, replication can reduce the time needed to
access the data by load-balancing clients across the copies. Furthermore, data replication should
be transparent to the user. While there are multiple copies of the data, users should only see only
one logical copy and user operations should be appear to be executed atomically on the logical
copy.A Distributed file system(DFS) is a file system, distributed across multiple machines which
appear to the user as a centralized file system. The main goal of a DFS is to provide services to
clients such as file creation, read, modification and deletion.

##Consistency model
By replicating objects some consistency issues arise. For example, we have to keep data
consistency between multiple copies of the data. The strong consistency model, is a model
whichensures the strongest consistency among all the others models. With this model, whenever
a write operation completes any subsequent read will return the most updated written value.
While there are some other consistency models such as Sequential, Relaxed and Eventual
Consistency which are all weaker than strong consistency model, In this thesis, we are
concentrate in the strongest consistency model.
