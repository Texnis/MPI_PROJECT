# MPI_PROJECT
This program implements parallel computation of basic statistical measures (Mean, Maximum, Variance) .It uses the Message Passing Interface (MPI) to distribute the work among multiple processes in a distributed memory system. The program is written in C and uses ONLY basic functions MPI_Send and MPI_Recv for communication.

## Full Documentation

For detailed information regarding the implementation, methodology, and project results, please refer to the comprehensive report:

[**Project Documentation (PDF)**](Documentation.pdf)

Note: While the **source code (`MPI.c`) includes comments and variable names primarily in English**, the comprehensive project documentation (found in `Documentation.pdf`) is written **entirely in Greek**.

---

### Requirements

To compile and run this program, you need:

* **GCC Compiler**
* An **MPI Library** (e.g., Open MPI or MPICH)

### Compilation Instructions

Use the MPI wrapper compiler (`mpicc`) and link the math library (`-lm`):

```bash
mpicc MPI.c -o mpi -lm
