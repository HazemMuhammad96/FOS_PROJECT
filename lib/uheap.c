
#include <inc/lib.h>

// malloc()
//	This function use NEXT FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
const int NUM_OF_USER_PAGES = (USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE;

struct userHeap
{
	uint32 address;
	bool isFree;
	int headIndex;
	int tailIndex;
	uint32 size;
} userHeapPages[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE];

struct userRange
{
	int start;
	int end;
};

int nextAccssedPageIndex = 0;
bool isUHeapInitialized = 0;

void initializeUHeap()
{
	if (isUHeapInitialized == 1)
		return;

	int i = 0;
	for (uint32 address = USER_HEAP_START; address < USER_HEAP_MAX; address += PAGE_SIZE)
	{
		userHeapPages[i].address = address;
		userHeapPages[i].isFree = 1;
		i++;
	}
	isUHeapInitialized = 1;
}

struct userRange nextFit(int pagesNumber)
{
	int pageFlag = 0;
	int j = nextAccssedPageIndex;
	bool currentCondition = j < NUM_OF_USER_PAGES;
	int sizeCounter = 0;
	int startIndex = -1;
	int endIndex = -1;
	for (; currentCondition; j++)
	{
		if (userHeapPages[j].isFree == 1)
		{
			if (pageFlag == 0)
			{
				startIndex = j;
			}

			pageFlag++;
		}
		else
		{
			pageFlag = 0;
			startIndex = -1;
		}
		if (pageFlag == pagesNumber)
		{
			endIndex = j;
			nextAccssedPageIndex = j + 1;
			if (nextAccssedPageIndex >= NUM_OF_USER_PAGES - 1)
				nextAccssedPageIndex = 0;
			break;
		}

		// cprintf("j : %d \t", j);
		sizeCounter++;

		if (sizeCounter >= NUM_OF_USER_PAGES)
		{
			struct userRange range = {-1, -1};
			return range;
		}

		if (j == NUM_OF_USER_PAGES - 1)
		{
			j = 0;
			currentCondition = j < nextAccssedPageIndex;
			pageFlag = 0;
			startIndex = -1;
		}
	}

	struct userRange range = {startIndex, endIndex};

	return range;
}

void *malloc(uint32 size)
{

	initializeUHeap();
	size = ROUNDUP(size, PAGE_SIZE);
	int pagesNumber = size / PAGE_SIZE;
	int startIndex = -1;
	int endIndex = -1;

	if (USER_HEAP_MAX - userHeapPages[nextAccssedPageIndex].address < size)
	{
		return NULL;
	}

	struct userRange range;

	if (sys_isUHeapPlacementStrategyNEXTFIT())
		range = nextFit(pagesNumber);
	// else if (sys_isUHeapPlacementStrategyBESTFIT())
	// 	range = bestFit(pagesNumber);

	startIndex = range.start;
	endIndex = range.end;

	if (startIndex == -1 && endIndex == -1)
	{
		return NULL;
	}

	for (int j = startIndex; j <= endIndex; j++)
	{
		userHeapPages[j].isFree = 0;
		userHeapPages[j].headIndex = startIndex;
		userHeapPages[j].tailIndex = endIndex;
		userHeapPages[j].size = size;
	}
	sys_allocateMem(userHeapPages[startIndex].address, pagesNumber);

	return (void *)userHeapPages[startIndex].address;
}

void *smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	panic("smalloc() is not required ..!!");
	return NULL;
}

void *sget(int32 ownerEnvID, char *sharedVarName)
{
	panic("sget() is not required ..!!");
	return 0;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it
int findPageIndexByVA(void *virtual_address)
{
	// int va = (int)virtual_address;
	int va = (int)ROUNDDOWN(virtual_address, PAGE_SIZE);

	for (int i = 0; i < NUM_OF_USER_PAGES; i++)
	{
		if (userHeapPages[i].address == va)
			return i;
	}

	return -1;
}

void free(void *virtual_address)
{
	virtual_address = ROUNDDOWN(virtual_address, PAGE_SIZE);

	int index = findPageIndexByVA(virtual_address);
	if (index == -1)
		return;

	struct userHeap currentPage = userHeapPages[index];
	for (int i = currentPage.headIndex; i <= currentPage.tailIndex; i++)
	{
		userHeapPages[i].isFree = 1;
		userHeapPages[i].headIndex = -1;
		userHeapPages[i].tailIndex = -1;
	}
	sys_freeMem((uint32)virtual_address, userHeapPages[index].size);
}

void sfree(void *virtual_address)
{
	panic("sfree() is not requried ..!!");
}

//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	// TODO: [PROJECT 2022 - BONUS3] User Heap Realloc [User Side]
	//  Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

	return NULL;
}
