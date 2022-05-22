#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

// 2022: NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

// kernel heap structure
const int NUM_OF_PAGES = (KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE;
struct kernelHeap
{
	uint32 address;
	bool isFree;
	int headIndex;
	int tailIndex;
} kernelHeapPages[(KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE];

struct kernelRange
{
	int start;
	int end;
};

int nextAccssedPageIndex = 0;
bool isKHeapInitialized = 0;

void initializeKHeap()
{
	int i = 0;
	for (uint32 address = KERNEL_HEAP_START; address < KERNEL_HEAP_MAX; address += PAGE_SIZE)
	{
		kernelHeapPages[i].address = address;
		kernelHeapPages[i].isFree = 1;
		i++;
	}
}

void initializeKHeapOnce()
{
	if (isKHeapInitialized == 1)
		return;
	initializeKHeap();
	isKHeapInitialized = 1;
}

struct kernelRange nextFit(int pagesNumber)
{
	int pageFlag = 0;
	int j = nextAccssedPageIndex;
	bool currentCondition = j < NUM_OF_PAGES;
	int sizeCounter = 0;
	int startIndex = -1;
	int endIndex = -1;
	for (; currentCondition; j++)
	{
		if (kernelHeapPages[j].isFree == 1)
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
			if (nextAccssedPageIndex >= NUM_OF_PAGES - 1)
				nextAccssedPageIndex = 0;
			break;
		}

		// cprintf("j : %d \t", j);
		sizeCounter++;

		if (sizeCounter >= NUM_OF_PAGES)
		{
			struct kernelRange range = {-1, -1};
			return range;
		}

		if (j == NUM_OF_PAGES - 1)
		{
			j = 0;
			currentCondition = j < nextAccssedPageIndex;
			pageFlag = 0;
			startIndex = -1;
		}
	}

	struct kernelRange range = {startIndex, endIndex};

	return range;
}

struct kernelRange bestFit(int pagesNumber)
{
	int pageFlag = 0;
	int sizeCounter = 0;
	int startIndex = -1;
	int endIndex = -1;
	int rangeIndex = 0;

	struct kernelRange range[(KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE];

	for (int i = 0; i < NUM_OF_PAGES; i++)
	{

		if (kernelHeapPages[i].isFree == 1)
		{
			if (pageFlag == 0)
			{
				startIndex = i;
			}


			pageFlag++;
		}
		else
		{
			// struct kernelRange tempRange = {startIndex, endIndex};
			// range[rangeIndex] = tempRange;
			rangeIndex++;
			pageFlag = 0;
			startIndex = -1;
		}
	}

	cprintf("rangeIndex : %d\n", rangeIndex);

	// int smallestRange = 0;
	// for (int i = 0; i < rangeIndex; i++)
	// {
	// 	if (range[i].end - range[i].start < range[smallestRange].end - range[smallestRange].start)
	// 	{
	// 		smallestRange = i;
	// 	}
	// }

	// struct kernelRange bestRange = {range[smallestRange].start, range[smallestRange].end};
	struct kernelRange bestRange = {1, 5};
	return bestRange;
}

void *kmalloc(unsigned int size)
{

	initializeKHeapOnce();
	size = ROUNDUP(size, PAGE_SIZE);

	int pagesNumber = size / PAGE_SIZE;
	int startIndex = -1;
	int endIndex = -1;

	if (KERNEL_HEAP_MAX - kernelHeapPages[nextAccssedPageIndex].address < size)
	{
		return NULL;
	}
	struct kernelRange range;

	if (isKHeapPlacementStrategyNEXTFIT())
		range = nextFit(pagesNumber);
	else if (isKHeapPlacementStrategyBESTFIT())
		range = bestFit(pagesNumber);

	startIndex = range.start;
	endIndex = range.end;

	if (startIndex == -1 && endIndex == -1)
	{
		return NULL;
	}
	//
	for (int j = startIndex; j <= endIndex; j++)
	{
		kernelHeapPages[j].isFree = 0;
		kernelHeapPages[j].headIndex = startIndex;
		kernelHeapPages[j].tailIndex = endIndex;

		struct Frame_Info *currentFrame;

		int ret = allocate_frame(&currentFrame);
		if (ret == E_NO_MEM)
			return NULL;

		ret = map_frame(ptr_page_directory, currentFrame, (void *)kernelHeapPages[j].address, PERM_PRESENT | PERM_WRITEABLE);
		if (ret == E_NO_MEM)
			return NULL;
	}


	return (void *)kernelHeapPages[startIndex].address;
}

int findPageIndexByVA(void *virtual_address)
{
	int va = (int)ROUNDDOWN(virtual_address, PAGE_SIZE);

	for (int i = 0; i < NUM_OF_PAGES; i++)
	{
		if (kernelHeapPages[i].address == va)
			return i;
	}

	return -1;
}

void kfree(void *virtual_address)
{
	int index = findPageIndexByVA(virtual_address);
	if (index == -1)
		return;

	struct kernelHeap currentPage = kernelHeapPages[index];

	for (int i = currentPage.headIndex; i <= currentPage.tailIndex; i++)
	{
		unmap_frame(ptr_page_directory, (void *)kernelHeapPages[i].address);
		kernelHeapPages[i].isFree = 1;
		kernelHeapPages[i].headIndex = -1;
		kernelHeapPages[i].tailIndex = -1;
	}

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{

	struct Frame_Info *currentFrame;
	uint32 *pageTable;

	for (uint32 i = KERNEL_HEAP_START; i <= kernelHeapPages[nextAccssedPageIndex].address; i += PAGE_SIZE)
	{
		currentFrame = get_frame_info(ptr_page_directory, (void *)i, &pageTable);
		if (currentFrame != NULL && to_physical_address(currentFrame) == physical_address)
			return i;
		
	}

	return 0;
}

uint32 *getPageTable(uint32 logicalAddress)
{
	uint32 *pageTable = NULL;
	get_page_table(ptr_page_directory, (void *)logicalAddress, &pageTable);

	return pageTable;
}

uint32 getFrame(unsigned int logicalAddress)
{
	uint32 *pageTable = getPageTable(logicalAddress);

	if (pageTable == NULL)
		return -1;

	uint32 PageIndex = PTX(logicalAddress);
	uint32 pageTableEntry = pageTable[PageIndex];

	uint32 frameNumber = (pageTableEntry >> 12);

	return frameNumber;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	return getFrame(virtual_address) * PAGE_SIZE;
}
