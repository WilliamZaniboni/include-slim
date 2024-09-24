
# Include-Slim: Variants of similarity retrieval operators in a Metric Access Method

## Introduction

Include-Slim is a fixed-sized memory page MAM designed to perform the following variants of similarity retrieval operators: 

- Performing conjunctive conditions on complex and scalar data types in the Range and kNN operators.

- A kNN operator with the ability to disregard the query center as part of the answer.

- A kNN operator with the ability to return all the elements tied in the k^{th} position.

- A kNN operator with the ability to untie elements in k^{th} position using scalar data as tiebreakers.

- A novel similarity `exists' query.

## Development

The implementation of the Include-Slim and the sequential scan operators are based on the [Arboretum](http://www.gbdi.icmc.usp.br/) library, which is implemented in C++ and compiled using GNU /Linux.

The steps for building the tree and performing the queries:

```shell
cd /deepLesion-Jaccard
make clean
make
./DeepLesion
```
