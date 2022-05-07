	#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//2022: NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
const int NUM_OF_PAGES =(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE;
bool isCalled=0;
struct kernelHeap{
	uint32 address;
	bool isFree;
}kernelHeapPages[(KERNEL_HEAP_MAX-KERNEL_HEAP_START)/PAGE_SIZE];
int lastAccssedPage=0;
void intialize()
{
	if(isCalled==1)
	{
		return;
	}
	int i=0;
	for(uint32 address=KERNEL_HEAP_START;address<KERNEL_HEAP_MAX;address+=PAGE_SIZE)
		{
			kernelHeapPages[i].address=address;
//			cprintf("index of :%d = %x \n",i,kernelHeapPages[i].address);
			kernelHeapPages[i].isFree=1;
			i++;
		}
	isCalled=1;

}
void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT 2022 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code

	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	intialize();
	size=ROUNDUP(size,PAGE_SIZE);

	int pagesNumber=size/PAGE_SIZE;
	int pageFlag=0;
	int startIndex=-1;
	int endIndex=-1;
//	if(pagesNumber>=NUM_OF_PAGES)
//	{
//		return NULL;
//	}
	cprintf("last : %x \n",kernelHeapPages[lastAccssedPage].address);
	if(KERNEL_HEAP_MAX-kernelHeapPages[lastAccssedPage].address<size)
	{
		return NULL;
	}
	for(int j=lastAccssedPage;j<NUM_OF_PAGES;j++)
	{
		if(kernelHeapPages[j].isFree==1)
		{
			if(pageFlag==0){
				startIndex=j;

			}

			pageFlag++;
		}
		else{
			pageFlag=0;
			startIndex=-1;
		}
		if(pageFlag==pagesNumber)
		{
			endIndex=j;
			lastAccssedPage=j;
			break;
		}

	}
	for(int j=startIndex;j<=endIndex;j++)
	{
		kernelHeapPages[j].isFree=0;
		struct Frame_Info * currentFrame;
		int ret=allocate_frame(&currentFrame);
		if(ret==E_NO_MEM)
		{
			break;
		}

		int ret1=map_frame(ptr_page_directory,currentFrame,(void*)kernelHeapPages[j].address,PERM_PRESENT|PERM_WRITEABLE);
	}

cprintf("size : %d \n",pagesNumber);
cprintf("Start Address :  %x \n",startIndex);
cprintf("End Adrress : %x \n",endIndex);

return (void*)kernelHeapPages[startIndex].address;
	//NOTE: Allocation using NEXTFIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details



	//TODO: [PROJECT 2022 - BONUS1] Implement a Kernel allocation strategy
	// Instead of the Next allocation/deallocation, implement
	// BEST FIT strategy
	// use "isKHeapPlacementStrategyBESTFIT() ..."
	// and "isKHeapPlacementStrategyNEXTFIT() ..."
	//functions to check the current strategy
	//change this "return" according to your answer

	return NULL;


}


void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2022 - [2] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2022 - [3] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer

	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2022 - [4] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer
	return 0;
}

