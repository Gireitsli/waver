#ifndef MY_CPU_INFO_REQUEST
#define MY_CPU_INFO_REQUEST

//******************************************************************************
// Course:    MPC
// Author:    M. Thaler, ZHAW
// File:      cpuinfo.h
// Revision:  7/2012
// Purpose:   get number of CPUs, Cores, Threads
// Functions: int getCPUs(void)       -> get number of available cpus (sysconf)
//                                       (same as getNumCPUs(void))
//            int getNumCPUs(void)    -> number of cpus = cores * threads
//            int getNumThreads(void) -> number of threads
//            int getNumCores(void)   -> number of cores
//******************************************************************************
//
//
//
//******************************************************************************
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define CPU_INFO "/tmp/MY_CPU_INFO_DATA"
#define THE_CPU_INFO_STRING "lscpu | \
        awk '/^CPU\\(/ {print $2}; \
             /Core/   {print $4}; \
             /Thread/ {print $4};' >  "CPU_INFO""

//******************************************************************************

int readInfo(int which) {
    int data, i;
    system(THE_CPU_INFO_STRING);
    FILE *theFile = fopen(CPU_INFO, "r");
    if ((which < 1) || (which > 3))
        return -1;
    for (i = 0; i < which; i++)
        fscanf(theFile, "%d", &data);
    fclose(theFile);
    unlink(CPU_INFO);
    return data;
}

//******************************************************************************
// get number of CPUS

#define getCPUs(X) sysconf(_SC_NPROCESSORS_ONLN)

int getNumCPUs(void) {
    return readInfo(1);
}

//------------------------------------------------------------------------------
// get number of hardware threads per CPU (?-way hyperthreading)

int getNumThreads(void) {
    return readInfo(2);
}

//------------------------------------------------------------------------------
// get number ob cores

int getNumCores(void) {
    return readInfo(3);
}

//******************************************************************************
#endif

