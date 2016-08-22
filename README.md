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
which ensures the strongest consistency among all the others models. With this model, whenever
a write operation completes any subsequent read will return the most updated written value.
While there are some other consistency models such as Sequential, Relaxed and Eventual
Consistency which are all weaker than strong consistency model, In this thesis, we are
concentrate in the strongest consistency model.

##LDR algorithm
The layered Data Replication Algorithm(LDR) is suitable especially for large-scale read/write
data object such as files and at the same time ensure atomic data consistency combining low
latency cost. The main idea of the algorithm is to store copies of data objects separately from
metadata about the location of up-to-date copies.

LDR’s performance is based on the fact that, in a typical application requiring replication, such
as a file system, the size of the objects being replicated is usually much larger than the size of the
metadata used by the algorithm to point or tag the actual objects. Therefore, it is more efficient to
perform operations on the metadata instead of operating directly on the actual data while all read
and write operations are guaranteed to appear to happen atomically. As a result, all the metadata
for the up-to-date of data objects are store in directories servers and the actual data object are
store on replica servers.

##How LDR works
For a read operation, the client read from a quorum of directories to find the most up-to-date
replicas that hold the specific object. In this step, the client algorithm looking for the max tag
which is associated with the data. Max tag indicate the latest value written to the object.Because
the writer inform a quorum of directories in write operation it is quarantines at least one directory
from the quorumalways know the latest data object. Now, the client have to update a quorum of
directories before return the set of up-to-date replicas. This step is necessary for the algorithm for
consistency issues. Further, client have to communicate with a replica from the up-to-date replica
set and retrieve the data object.

For a write operation, the client readsfrom a quorum of directories to find the latest tag for the
specific object. Consequently, the client increase the max tag that found and write the data object
to a set of replica with the associated tag. Further, writes to a quorum of directories, to indicate
the set of replicas that are the most up-to-date. Finally, it sends to each replica a secure message
to inform them that at least a quorum of directories know about the most up-to-date replica. So,
replicas can garbage collect old values of the object.

##Implementation
In this thesis, I have implemented a distributed file system from scratch in c programming language. Furthermore, I have designed and implemented a friendly interface, in order for the DFS to be used and debug easily. Although there are plenty of distributed file systems, none of them, do guarantee strong consistency. As a consequence, it’s the first DFS which guarantee strong consistency. In other words, if you have multiple users modify the same file, the current system should always give the latest version of the file to the user. As a result, we have built this system on recent research on atomic read/write register. One part of the implementation is based on the theoretical LDR algorithm. Technically speaking, the whole distributed file system was implemented in Linux environment with C programming language. We used C sockets (TCP/IP) since the system require reliable communication links. For storing metadata about the files, we use advanced data structures such as HashTable. As we wanted to transfer efficient large data files, I have implemented my own transfer layer using TCP/IP protocol. More precisely, I designed and implemented my own layer in order to transfer only necessary information with the actual data. Additionally, I developed bit-compression techniques to efficient transmit data over the network So, the transfer of the large files became extremely efficient. For serving the clients I used threads. In this way, we can serve all clients efficiently and concurrently and at the same time to take advantages of multiprocessors.
