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
int nextAccssedPageIndex = 0;

// x --> x + 5

bool isKHeapInitialized = 0;
void initializeKHeap()
{
	if (isKHeapInitialized == 1)
		return;

	int i = 0;
	for (uint32 address = KERNEL_HEAP_START; address < KERNEL_HEAP_MAX; address += PAGE_SIZE)
	{
		kernelHeapPages[i].address = address;
		kernelHeapPages[i].isFree = 1;
		i++;
	}
	isKHeapInitialized = 1;
}

void *kmalloc(unsigned int size)
{

	initializeKHeap();
	size = ROUNDUP(size, PAGE_SIZE);

	int pagesNumber = size / PAGE_SIZE;
	int pageFlag = 0;
	int startIndex = -1;
	int endIndex = -1;

	// cprintf("last : %x \n", kernelHeapPages[nextAccssedPageIndex].address);
	if (KERNEL_HEAP_MAX - kernelHeapPages[nextAccssedPageIndex].address < size)
	{
		return NULL;
	}
	for (int j = nextAccssedPageIndex; j < NUM_OF_PAGES; j++)
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
			break;
		}
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

	cprintf("\nsize : %d \n", pagesNumber);
	cprintf("Start Address Index: %d\naddress: %x\thead: %x\ttail: %x\n",
			startIndex, kernelHeapPages[startIndex].address,
			kernelHeapPages[startIndex].headIndex, kernelHeapPages[startIndex].tailIndex);
	cprintf("End Address Index: %d\naddress: %x\thead: %x\ttail: %x\n\n",
			endIndex, kernelHeapPages[endIndex].address,
			kernelHeapPages[endIndex].headIndex, kernelHeapPages[endIndex].tailIndex);

	return (void *)kernelHeapPages[startIndex].address;

	// TODO: [PROJECT 2022 - BONUS1] Implement a Kernel allocation strategy
	//  Instead of the Next allocation/deallocation, implement
	//  BEST FIT strategy
	//  use "isKHeapPlacementStrategyBESTFIT() ..."
	//  and "isKHeapPlacementStrategyNEXTFIT() ..."
	// functions to check the current strategy
	// change this "return" according to your answer
}

int findPageIndexByVA(void *virtual_address)
{
	int va = (int)virtual_address;

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
	// for (int i = 0; i < NUM_OF_PAGES; i+=PAGE_SIZE)
	// {
	// 	if (kheap_physical_address(kernelHeapPages[i].address) == physical_address)
	// 		return (unsigned int)kernelHeapPages[i].address;
	// }

	// return 0;

	struct Frame_Info *currentFrame;
	uint32 *ptr_page_table;

	for (uint32 i = KERNEL_HEAP_START; i <= nextAccssedPageIndex; i += PAGE_SIZE){
		currentFrame = NULL;
		currentFrame = get_frame_info(ptr_page_directory, (void*) i, &ptr_page_table);
		if(currentFrame != NULL && to_physical_address(currentFrame) == physical_address){
			return i;
		}
	}

	return 0;
}

uint32 OFFSET(uint32 logicalAddress)
{
	return (logicalAddress << 20) >> 20;
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

	if ((pageTableEntry & PERM_PRESENT) <= 0)
		return -1;

	uint32 frameNumber = pageTableEntry >> 12;

	return frameNumber;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{

	uint32 frameNumber = getFrame(virtual_address);

	if (frameNumber == -1)
		return -1;

	// uint32 offset = OFFSET(virtual_address);

	uint32 physicalAddress = frameNumber * PAGE_SIZE;

		if (physicalAddress == 0)
		return -1;

	return physicalAddress;
}
