/* A Merge Sort Implementation
*  Written by Will Sumner in C++
*  Created May 2016
*  Modified February 16th 2017
*/
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <cstdlib>
#include <pthread.h>

struct tInfo // everything you need to start a sort on a thread
{
	int *oarray;
	int *tarray;
	int start;
	int end;
	int length;
	long tid;
};

// Random Number Generation Functions
int prettyGoodNumberGen (unsigned int value) //This is a pseudo random number generator, the results are predictable
// It's predictability is helpful in bug testing as the same output can be recovered by using the same input
{
	static unsigned int seed = value; //Static declares this number only ONCE, makes it a seed
	static long m = 4294967296; //Declare this number once as well, for efficiency
	seed = (71*seed) + 22695477; //Bordland LCG, basic form of this is x = (ax)+y
	int randNum = seed % m; // return the modulo of the number
	if (randNum < 0)
		randNum *= -1;
	return (randNum % 100); //the modulo here outputs numbers 0 - 999
}

//Array Functions
void randomArray (int *newArray, int size,int numSeed) //Fills an array with random number
{
	int num = 0;
	for (int i = 0; i < size; i++)
	{
		num = prettyGoodNumberGen(numSeed); // Generate a random number
		newArray[i] = num; // Add it to the list
	}
}
void emptyArray (int *array, int start, int end) // Fills an array with 0's for debugging
{
	for (int i = start; i < end; i++)
	{
		array[i] = 0;
	}
}
void printArray (int *array,int start, int end) // Prints an array
{
	if (end-start > 30){
		int length = end-start;
		int step = 20;
		int overlap = length % step;
		int div = (int) length/step;

		for (int i=start;i<div;i++)
		{
			for (int j = 0; j < step; j++)
			{
				if (array[(i*step)+j] < 10) { std::cout << "0"; }
				std::cout << array[(i*step)+j]<<" ";
			}
			std::cout <<"\n";
		}

		for (int i=end-overlap;i<=end;i++)
		{
			if (array[i] < 10) { std::cout << "0"; }
			std::cout << array[i] << " ";
		}
		std::cout << "\n";
	}

	else
	{
	for (int i=start;i<end;i++)
		std::cout << array[i] << " ";
	std::cout << array[end] << "\n";
	}
}

//Sorting Functions

void mergeStep (int *array,int *tempList,int start, int lengthOne, int lengthTwo) //the merge step of a merge sort
{
	// Only pointers to arrays can be passed, so the indices are set to the start of each list
	int i = start;
	int j = i+lengthOne;
	int k = 0; // index for the entire templist

	while (k < lengthOne+lengthTwo) // a C++ while loop
	{
		if (i - start == lengthOne)
		{ //list one exhausted
			for (int n = 0; n+j < lengthTwo+lengthOne+start;n++ ) //add the rest
			{
				tempList[k++] = array[j+n];
			}
			break;
		}

		if (j-(lengthOne+lengthTwo)-start == 0)
		{//list two exhausted
			for (int n = i; n < start+lengthOne;n++ ) //add the rest
			{
				tempList[k++] = array[n];
			}
			break;
		}

		if (array[i] > array[j]) // figure out which variable should go first
		{
			tempList[k] = array[j++];
		}

		else
		{
			tempList[k] = array[i++];
		}
		k++;
	}

	for (int s = 0; s < lengthOne+lengthTwo;s++) // add the templist into the original
	{
		array[start+s] = tempList[s];
	}
}

void sortStep (int *array, int length, int start, int end,int *tempList)
{
	if (length < 3)
	{
		if (length > 1)
		{
			if (array[start] > array[end])
			{
				int temp = 0;
				temp = array[start];
				array[start] = array[end];
				array[end] = temp;
			}
		}
	}
	else
	{
		int mid = (int)(start+end)/2;//find mid
		sortStep(array,mid-start+1,start,mid,tempList); //left sort
		sortStep(array,end-mid,mid+1,end, tempList); // right sort
		mergeStep(array,tempList,start,mid-start+1,end-mid); // merge
	}
}

void *threadSort(void *threadDat) // Function initially called by thread
{
	tInfo *threadInf = (tInfo*)threadDat;
	//printf("Thread %d \nStart %d End %d Length %d \n",threadInf->tid, threadInf->start,threadInf->end, threadInf->length);
	sortStep(threadInf->oarray,threadInf->length,threadInf->start,threadInf->end,threadInf->tarray);//call all the important biz
	return (threadDat);
}

// Main Program
using namespace std;
int main()
{
	int threadnum = 1;
	int size = 2;
	int listSize = 100;

	std::cout << "Multi-threaded Merge Sort\n";
	while (size > 1)
	{
		int status = 3;
		int tsize = 0;

		// Getting thread number and list size from user
		printf("Please enter the number of threads to use: ");
		std::cin >> threadnum;
		if (threadnum < 1 || threadnum > 8)
		{
			printf("\nExiting....");
			return(0);
		}
		printf("Please the length of the list or -1 to exit: ");
		std:: cin >> size;
		if (size <= 1)
		{
			std::cout << "\nExiting...";
			return (0); //Anything less than 1 will end the program (also to protect it)
		}


		// Dividing up list for threads
		std::cout << "Organizing " << threadnum << " threads... \n";
		tsize = (int)(size / threadnum);
		int *tlist= new int[threadnum*2]; //list of starts and ends for the threads
		tInfo *tInfoList = new tInfo[threadnum]; // array parameter structs passed to the threads
		long i;
		int count = 0;
		for (i=0;i<threadnum*2;i++){ //start end indices for threads
			tlist[i] = count;
			if (i%2 ==0) { count +=tsize; }
			else { count+=1; }
		}
		tlist[threadnum*2-1] = size-1;

		// Setting up the threads and arrays
		pthread_t *threads = new pthread_t[threadnum]; //allocating an array for the threads
		std::cout << "Allocating array memory... \n";
		int *testArray = new int[size]; //original list
		int *tempList = new int[size]; //sorting list
		std::cout << "Generating random numbers... \n";
		randomArray(testArray,size,5538); //fill list with random numbers

		if (size<=listSize)
		{
			std::cout << "Original list: \n";
			printArray(testArray,0,size-1);
		}

		std::cout << "Sorting... \n\n";

		float x = clock(); // timing
		for (i=0;i<threadnum;i++){ // for each thread
			tInfo *thready = &tInfoList[i]; // a simple struct
			thready->start = tlist[i*2];
			thready->end = tlist[i*2+1];
			thready->length = tlist[i*2+1]-tlist[i*2];
			thready->oarray = testArray;
			thready->tarray = tempList;
			thready->tid = i;
			tInfo *tptr = thready;
			status = pthread_create(&threads[i], NULL, threadSort, (void *)tptr);
			if (status) { std::cout << "####################### \nError with thread "<< i <<"\n#######################\n";}
		}

		int someThing = 0; //throwaway holder for join
		void *noptr = &someThing; // pointer to throwaway
		for (i=0;i<threadnum;i++){ //wait for all the threads to finish
			pthread_join(threads[i],&noptr);
		}

		// FINAL MERGE
		int lStart = 0;
		int lEnd = 0;
		for (i=0; i < threadnum*2-2; i+=2){ //merge all sublists
			lStart = tlist[i+1]-tlist[i]+1+lStart;
			lEnd = tlist[i+3]-tlist[i+2]+1;
			mergeStep(testArray,tempList,0,lStart,lEnd);
		}

		float y = clock() - x; //timing
		std::cout << "It took " << y/(CLOCKS_PER_SEC*threadnum) << " seconds to sort the list of " << size << " items, and " << y << " clocks. \n";
		if (size<=listSize)
		{
			std::cout << "Sorted list: \n";
			printArray(testArray,0,size-1);
		}
		std::cout << "\n";

		// Memory Cleanup
		delete[] testArray;
		testArray = 0;
		delete[] tempList;
		tempList = 0;
		delete[] tlist;
		tlist = 0;
		delete[] tInfoList;
		tInfoList = 0;
		delete[] threads;
		threads = 0;
	}
	return (0);
}
