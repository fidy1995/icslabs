#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

/*******************************************
 Name: Jiang Jiheng Student No.: 5130379033

 Lab 8 Part A: Cache Simulator
*******************************************/

/*******************************************
 Macros:
	Validbit: show cache taken
	Dirtybit: show recently used
	Used    : calculate usage
	Address : the real address
*******************************************/
#define VALIDBIT ((long long)1<<49)
#define DIRTYBIT ((long long)3<<50)
#define USED ((long long)1<<50)
#define ADDRESS (power2(49)-1)

/*******************************************
 Global Variables:
	h/m/e Counts: calculate h/m/e
	s/l/b Counts: getargs of s, E, b
	s/b Masks   : to get set and offset
	setSize     : lines of a set(=E)
	helpStatus  : -v, for debugging
	dataFile    : name of traces
	fp	    : to opem a trace
	c           : to record addresses
*******************************************/

int hitCount = 0, missCount = 0, evCount = 0;
int setCount, lineCount, blockCount;
int setMask, blockMask;
int setSize;
int helpStatus = 0;
char* dataFile;
FILE* fp;
long long *c;

/*******************************************
 To calculate pow(2, p), for masks
*******************************************/
long long power2(int p)
{	
	int i;
	long long rtn = 1;
	for (i = 0; i < p; i++)
		rtn *= 2;
	return rtn;
}

/*******************************************
 To calculate if it is recently used
 Using 2 bits to caculate
*******************************************/
void calLRU(int setNo, int latest)
{
	int startPos = setNo * setSize;
	int i;	
	long long dirt = (*(c + startPos + latest) & DIRTYBIT);
	
	long long current;
	
	// if the dirtybit is bigger than the recently used
	// it should be decrease by one to make the order right
	for (i = 0; i < setSize; i++) {
		current = *(c + startPos + i);
		if ((current & DIRTYBIT) > dirt) {
			current -= USED;
			*(c + startPos + i) = current;
		}
	}
	*(c + startPos + latest) |= DIRTYBIT;
}

/*******************************************
 Search in the particular set to see 
 if it hits the cache
*******************************************/
int cacheHit(long long addr, int setNo)
{
	int startPos = setNo * setSize;
	int i;
	long long current;
	
	// to find in the cache
	for (i = 0; i < lineCount; i++) {
		current = *(c + startPos + i);
		// if hit, recaculate dirtybits
		if ((current & ADDRESS) == addr 
			&& (current & VALIDBIT)) {
			calLRU(setNo, i);
			return 1;
		}
	}
	return 0;
}

/*******************************************
 To find out if the set has an empty line
*******************************************/
int cacheVacant(int setNo)
{
	int startPos = setNo * setSize;
	int i;
	
	// if there is, return the line number
	for (i = 0; i < lineCount; i++) {
		if (!(*(c + startPos + i) & VALIDBIT))
			return i; 
	}
	return -1;
}

/*******************************************
 If cache miss, the new address should
 either take an empty line or change
 a LRU line.
*******************************************/
void change(long long addr, int setNo)
{
	int cgLine;
	int startPos = setNo * setSize;
	// if empty line taken, recaculate dirtybits
	if ((cgLine = cacheVacant(setNo)) != -1) {
		*(c + startPos + cgLine) = addr | VALIDBIT;	 
		calLRU(setNo, cgLine);
		return;
	}
	// if there is no empty line do this
	evCount++;
	long long current;
	int i;
	for (i = 0; i < lineCount; i++) {
		current = *(c + startPos + i);
		// if the line is LRU, 
		// replace it and recalculate dirtybits
		if (((current & DIRTYBIT) >> 50) == (4 - lineCount)) {
			current = addr | VALIDBIT;
			*(c + startPos + i) = current;
			calLRU(setNo, i);
			if (helpStatus) printf("evc ");
			break;
		}
	}
}

/*******************************************
 Search the whole cache with the address
*******************************************/
void searchCache(long long addr)
{
	int offSet = addr & blockMask;
	int setNo = (addr >> blockCount) & setMask;
	long long location = addr - offSet;
	
	if (cacheHit(location, setNo)) {
		hitCount++;
		if (helpStatus) printf("hit ");
	}
	else {
		missCount++;
		if (helpStatus) printf("mis ");
		change(location, setNo);
	}		
}

/*******************************************
 To deal with cache instructions
*******************************************/
void calculate(char buffer, long long addr)
{
	switch (buffer) {
	// when it comes to 'M', that means
	// no matter the first time miss/hit
	// it hits on the second time
	// so just find one time
	case 'M':
		hitCount++;
		if (helpStatus) printf("hit ");
	case 'L': case 'S':
		searchCache(addr);
	default:
		break;
	}
}

/*******************************************
 Start the program
*******************************************/
void run()
{
	long long addr;
	int size;
	char buffer;
	char waste[] = "hahahahahahaahah";
	// read the file and get away 'I's
	while ((buffer = fgetc(fp)) != EOF) {
		if (buffer != ' ')
			fgets(waste, 100, fp);
		else {
			fscanf(fp, "%c %llx,%d", &buffer, &addr, &size);
			calculate(buffer, addr);
			if (helpStatus) printf("%c %llx\n", buffer, addr);
			fgets(waste, 100, fp);
		}
	}
	printSummary(hitCount, missCount, evCount);
}

int main(int argc, char **argv)
{
	char ch;
	while ((ch = getopt(argc, argv, "s:E:b:t:v")) != -1) {
		switch (ch) {
		case 's':
			setCount = *optarg - 48;
			break;
		case 'E':
			lineCount = *optarg - 48;
			break;
		case 'b':
			blockCount = *optarg - 48;
			break;
		case 't':
			dataFile = optarg;
			break;
		case 'v':
			helpStatus = 1;
		default:
			break;
		}
	}
	
	c = (long long*)malloc(power2(setCount) 
		* lineCount * sizeof(long long));

	setMask = power2(setCount) - 1;
	blockMask = power2(blockCount) - 1;

	setSize = lineCount;
	
	fp = fopen(dataFile, "r");
	// let's rock!
	run();
	fclose(fp);
	return 0;
}
