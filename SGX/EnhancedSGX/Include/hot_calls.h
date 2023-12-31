// ----------------------------------------
// HotCalls
// Copyright 2017 The Regents of the University of Michigan
// Ofir Weisse, Valeria Bertacco and Todd Austin

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ---------------------------------------------

//Author: Ofir Weisse, www.OfirWeisse.com, email: oweisse (at) umich (dot) edu
//Based on ISCA 2017 "HotCalls" paper. 
//Link to the paper can be found at http://www.ofirweisse.com/previous_work.html
//If you make nay use of this code for academic purpose, please cite the paper. 



#ifndef __FAST_SGX_CALLS_H
#define __FAST_SGX_CALLS_H

//#include <stdio.h>
#include <stdlib.h>
#include <sgx_spinlock.h>
#include <stdbool.h>
// #include "utils.h"

#include "ut_spinlock.h"
#pragma GCC diagnostic ignored "-Wunused-function"

typedef unsigned long int pthread_t;

typedef struct {
    pthread_t       responderThread;
    sgx_spinlock_t  spinlock;

    void*           data;
    //long*           counterArr;
    //void*	    size;
    void*	    rei;
    int*	    intArray;
    double*	    doubleArray;
    float*          floatArray;
    char*           charArray;
    long*           longArray;
    char*           byteArray;

    int             intTail; 
    int 	    doubleTail;
    int             floatTail;
    int             charTail;    
    int             longTail;
    int 	    byteTail;

    char*           uuid;
    char*           ouuid;
    char*           cuuid;

    uint16_t        callID;
    bool            keepPolling;
    bool            runFunction;
    bool            isDone;
    bool            busy;
} HotCall;

typedef struct 
{
    uint16_t numEntries;
    void (**callbacks)(void*,void*,int*,int,double*,int,float*,int,char*,int,long*,int,char*,int,char*,char*,char*);
    void (**callback)(void*,char*,char*);
    void (**callbac)(void*,void*,int*,double*,float*,char*,long*,char*,char*);
} HotCallTable;

#define HOTCALL_INITIALIZER  {0, SGX_SPINLOCK_INITIALIZER, NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,0,0,0,0,0,0, true, false, false, false }

static void HotCall_init( HotCall* hotCall )
{
    hotCall->responderThread    = 0;
    hotCall->spinlock           = SGX_SPINLOCK_INITIALIZER;
    hotCall->data               = NULL; 
    //hotCall->counterArr           = NULL;
    //hotCall->size               = 0;
    hotCall->rei                 = NULL; 
    hotCall->intArray           = NULL;
    hotCall->doubleArray        = NULL;
    hotCall->floatArray         = NULL;
    hotCall->charArray          = NULL;
    hotCall->longArray          = NULL;
    hotCall->byteArray          = NULL;
    hotCall->uuid               = NULL;
    hotCall->ouuid               = NULL;
    hotCall->cuuid               = NULL;

    hotCall->intTail            = 0;
    hotCall->doubleTail            = 0;
    hotCall->floatTail            = 0;
    hotCall->charTail            = 0;
    hotCall->longTail            = 0;
    hotCall->byteTail            = 0;

    hotCall->callID             = 0;
    hotCall->keepPolling        = true;
    hotCall->runFunction        = false;
    hotCall->isDone             = false;
    hotCall->busy               = false;
}

static inline void _mm_pause(void) __attribute__((always_inline));



static inline int HotCall_requestCall( HotCall* hotCall, uint16_t callID, void *data,int *intArray,int intTail,double *doubleArray,int doubleTail,float *floatArray,int floatTail,char *charArray,int charTail,long *longArray,int longTail,char *byteArray,int byteTail,char *uuid,char* ouuid,char* cuuid,void *rei)
{
    int i = 0;
    const uint32_t MAX_RETRIES = 10;
    uint32_t numRetries = 0;
    //REquest call
    while( true ) {
        sgx_spin_lock( &hotCall->spinlock );
        if( hotCall->busy == false ) {
            hotCall->busy        = true;
            hotCall->isDone      = false;
            hotCall->callID      = callID;

            hotCall->data        = data;
	    //hotCall->counterArr  = counterArr;
            //hotCall->size        = size;
       	    hotCall->intArray    = intArray;
            hotCall->doubleArray = doubleArray;
   	    hotCall->floatArray  = floatArray;
    	    hotCall->charArray   = charArray;
    	    hotCall->longArray   = longArray;
    	    hotCall->byteArray   = byteArray;

            hotCall->intTail       = intTail;
   	    hotCall->doubleTail        = doubleTail;
    	    hotCall->floatTail        = floatTail;
    	    hotCall->charTail         = charTail;
   	    hotCall->longTail        = longTail;
   	    hotCall->byteTail       = byteTail;

	    hotCall->uuid   = uuid;
	    hotCall->ouuid   = ouuid;
            hotCall->cuuid   = cuuid;
    	    hotCall->rei   = rei;

	    hotCall->runFunction = true;
            sgx_spin_unlock( &hotCall->spinlock );
            break;
        }
        //else:
        sgx_spin_unlock( &hotCall->spinlock );

        //numRetries++;
       // if( numRetries > MAX_RETRIES )
        //    return -1;

        for( i = 0; i<3; ++i)
            _mm_pause();
    }

    //wait for answer
    while( true )
    {
        sgx_spin_lock( &hotCall->spinlock );
        if( hotCall->isDone == true ){
            hotCall->busy = false;
            sgx_spin_unlock( &hotCall->spinlock );
            break;
        }

        sgx_spin_unlock( &hotCall->spinlock );
        for( i = 0; i<3; ++i)
            _mm_pause();
    }
    //void* s = hotCall -> re;
    //int *x = (int*)s;
    return 1;
}

static inline void HotCall_waitForCall( HotCall *hotCall, HotCallTable* callTable )  __attribute__((always_inline));
static inline void HotCall_waitForCall( HotCall *hotCall, HotCallTable* callTable ) 
{
    static int i;
    // volatile void *data;
    //ocall_print_string("function HotCall_waitForCall\n");
    while( true )
    {
        sgx_spin_lock( &hotCall->spinlock );
        if( hotCall->keepPolling != true ) {
            sgx_spin_unlock( &hotCall->spinlock );
            break;
        }

        if( hotCall->runFunction )
        {
            //printf("runFunction\n");
            volatile uint16_t callID = hotCall->callID;
            //void *data = hotCall->data;
            //int *x = (int*)data; 
            //printf("counter=%d\n",*x);
	    //int* intArray = hotCall->intArray;
    	    //double* doubleArray = hotCall->doubleArray;
            //float* floatArray = hotCall->floatArray;
            //char* charArray = hotCall->charArray;
    	    //long* longArray = hotCall->longArray;
   	   // char* byteArray = hotCall->byteArray;
            sgx_spin_unlock( &hotCall->spinlock );
            //if( callID < callTable->numEntries ) {
                //printf( "Calling callback %d\n", callID );

                if(callID == 0){//branch
                	callTable->callbacks[ callID ]( hotCall->data,hotCall->rei,hotCall->intArray,hotCall->intTail,hotCall->doubleArray,hotCall->doubleTail,hotCall->floatArray,hotCall->floatTail,hotCall->charArray,hotCall->charTail,hotCall->longArray,hotCall->longTail,hotCall->byteArray,hotCall->byteTail,hotCall->uuid,hotCall->ouuid,hotCall->cuuid);
	   	}
		if(callID == 1 || callID == 2){
			callTable->callback[ callID ]( hotCall->data,hotCall->uuid,hotCall->charArray);
		}
		
		if(callID == 3){  //update
			callTable->callbacks[ callID ]( hotCall->data,hotCall->rei,hotCall->intArray,hotCall->intTail,hotCall->doubleArray,hotCall->doubleTail,hotCall->floatArray,hotCall->floatTail,hotCall->charArray,hotCall->charTail,hotCall->longArray,hotCall->longTail,hotCall->byteArray,hotCall->byteTail,hotCall->uuid,hotCall->ouuid,hotCall->cuuid);
		}

		if(callID == 4){  //get
			callTable->callbacks[ callID ]( hotCall->data,hotCall->rei,hotCall->intArray,hotCall->intTail,hotCall->doubleArray,hotCall->doubleTail,hotCall->floatArray,hotCall->floatTail,hotCall->charArray,hotCall->charTail,hotCall->longArray,hotCall->longTail,hotCall->byteArray,hotCall->byteTail,hotCall->uuid,hotCall->ouuid,hotCall->cuuid);
		}


           // }
           // else {
                // printf( "Invalid callID\n" );
                // exit(42);
           // }
            // DoWork( hotCall->data );
            // data = (int*)hotCall->data;
            // printf( "Enclave: Data is at %p\n", data );
            // *data += 1;
            sgx_spin_lock( &hotCall->spinlock );
            hotCall->isDone      = true;
            hotCall->runFunction = false;
        }
        
        sgx_spin_unlock( &hotCall->spinlock );
        for( i = 0; i<3; ++i)
            _mm_pause();
        
        // _mm_pause();
        //     _mm_pause();
        // _mm_pause();
    }

}
static inline void StopResponder( HotCall *hotCall );
static inline void StopResponder( HotCall *hotCall )
{
    sgx_spin_lock( &hotCall->spinlock );
    //void* s = hotCall -> re;
    //int *x = (int*)s; 
    hotCall->keepPolling = false;
    sgx_spin_unlock( &hotCall->spinlock );
    //return *x;
}



#endif
