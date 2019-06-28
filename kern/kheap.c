#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>


struct Blocks
{
	uint32 startAddress;
	uint32 endAddress;
	uint32 size;
};


struct Blocks availablePlaces;
struct Blocks reservedPlaces[1000];
int availablePlacesIndex = 0;
int reservedPlacesIndex = 0;
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kmalloc()
void* kmalloc(unsigned int size)
{
	availablePlaces.size = 0x7fffffff;
	//saving the sizes and start address of every suitable compatable size
	uint32 numberOfPages=(size/PAGE_SIZE);
	if (size % PAGE_SIZE)
		numberOfPages++;
	uint32*ptr = NULL;
	int pageCounter=0;

//first fit
	//setKHeapPlacementStrategyFIRSTFIT();//<== UNCOMMENT THIS LINE FOR TESTING BONUS OF FIRST FIT
	if(isKHeapPlacementStrategyFIRSTFIT())
	{
	//cprintf("first fit case\n");
		//scan kernel heap
		uint32 addrs=KERNEL_HEAP_START;
		for(uint32 i= KERNEL_HEAP_START; i<KERNEL_HEAP_MAX; i+=PAGE_SIZE)
		{
			uint32 retPageTable = get_page_table(ptr_page_directory, (void*)i, &ptr);
			uint32 check = (ptr[PTX(i)]&PERM_PRESENT);
			if(check == 0)//checking for an empty page
			{
				pageCounter++;
			}
			else//not empty page
			{
				//if found enough contiguous pages, save there start add and their count
				if(pageCounter >= numberOfPages && availablePlaces.size > pageCounter)
				{
					availablePlaces.startAddress = (i-(pageCounter*PAGE_SIZE));
					availablePlaces.size = pageCounter;
					availablePlaces.endAddress=availablePlaces.startAddress+pageCounter*PAGE_SIZE;
					pageCounter=0;
					break;
				}
				pageCounter=0;
			}
		}
		if(pageCounter >= numberOfPages && availablePlaces.size > pageCounter)
		{
			availablePlaces.startAddress = KERNEL_HEAP_MAX-(pageCounter*PAGE_SIZE);
			availablePlaces.size = pageCounter;
			availablePlaces.endAddress=availablePlaces.startAddress+pageCounter*PAGE_SIZE;
		}
		else if(availablePlaces.size == 0x7fffffff)// didn't find suitable space
		{
			return NULL;
		}
	}

//bestfit
	else if (isKHeapPlacementStrategyBESTFIT())
	{
		//scan kernel heap
		//cprintf("i'm the best\n");
		uint32 addrs=KERNEL_HEAP_START;
		for(uint32 i= KERNEL_HEAP_START; i<KERNEL_HEAP_MAX; i+=PAGE_SIZE)
		{
			uint32 retPageTable = get_page_table(ptr_page_directory, (void*)i, &ptr);
			uint32 check = (ptr[PTX(i)]&PERM_PRESENT);
			if(check == 0)//checking for an empty page
			{
				pageCounter++;
			}
			else//not empty page
			{
				//if found enough contiguous pages, save there start add and their count
				if(pageCounter >= numberOfPages && availablePlaces.size > pageCounter)
				{
					availablePlaces.startAddress = (i-(pageCounter*PAGE_SIZE));
					availablePlaces.size = pageCounter;
					availablePlaces.endAddress=availablePlaces.startAddress+pageCounter*PAGE_SIZE;
				}
				pageCounter=0;
			}
		}


		if(pageCounter >= numberOfPages && availablePlaces.size > pageCounter)
		{
			availablePlaces.startAddress = KERNEL_HEAP_MAX-(pageCounter*PAGE_SIZE);
			availablePlaces.size = pageCounter;
			availablePlaces.endAddress=availablePlaces.startAddress+pageCounter*PAGE_SIZE;
			//availablePlacesIndex++;
		}
		else if(availablePlaces.size == 0x7fffffff)// didn't find suitable space
		{
			return NULL;
		}


	}
	uint32 tmpAddress=0;
	uint32 endAddress=0;
	uint32 minSize = 0x7fffffff;
	int requiredIndex=0;

	tmpAddress = availablePlaces.startAddress;

	for(int i=0; i < numberOfPages; i++)
	{
		struct Frame_Info *ptr_frame_info = NULL;
		int ret = allocate_frame(&ptr_frame_info);
		int mappingFrameRet=0;
		if (ret != E_NO_MEM)
		{
			mappingFrameRet = map_frame(ptr_page_directory,ptr_frame_info,(void*)tmpAddress,PERM_PRESENT|PERM_WRITEABLE|PERM_USER);

			if(mappingFrameRet==E_NO_MEM)
			{
				for(int j=i;j>0;j--)
				{
					tmpAddress -= PAGE_SIZE;
					unmap_frame(ptr_page_directory, (void*)tmpAddress);
				}
				return NULL;
			}

		}
		else
		{
			//cprintf("nooooooooo");
			for(int j = i; j > 0; j--)
			{
				tmpAddress-=PAGE_SIZE;
				unmap_frame(ptr_page_directory, (void*)tmpAddress);
			}
			return NULL;
		}
		tmpAddress += PAGE_SIZE;
	}
	reservedPlaces[reservedPlacesIndex].startAddress = availablePlaces.startAddress;
	reservedPlaces[reservedPlacesIndex].size = numberOfPages;
	reservedPlaces[reservedPlacesIndex].endAddress = reservedPlaces[reservedPlacesIndex].startAddress+(numberOfPages*PAGE_SIZE);
	reservedPlacesIndex ++;

	return (void*)availablePlaces.startAddress;
}
	//TODO: [PROJECT 2019 - BONUS1] Implement the FIRST FIT strategy for Kernel allocation
	// Beside the BEST FIT
	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy


void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

	uint32 requiredSize = 0;
	uint32 requiredStartAddress = 0;
	uint32 requiredEndAddress = 0;
	uint32 requiredIndex=0;

	//search for given virtual address in reserved array
	for(int i = 0; i < reservedPlacesIndex; i ++)
	{
		if(reservedPlaces[i].startAddress == (uint32)virtual_address)
		{
			requiredSize = reservedPlaces[i].size;
			requiredStartAddress=reservedPlaces[i].startAddress;
			requiredEndAddress=reservedPlaces[i].endAddress;
			requiredIndex=i;
			break;
		}
	}
	for(int i=0; i< requiredSize; i++)
	{

		unmap_frame(ptr_page_directory, (void*)virtual_address);
		virtual_address += PAGE_SIZE;
	}
	//we are shifting the reserved when freed
	for(int j = requiredIndex+1; j < reservedPlacesIndex; j++)
	{
		reservedPlaces[j-1].startAddress=reservedPlaces[j].startAddress;
		reservedPlaces[j-1].size=reservedPlaces[j].size;
		reservedPlaces[j-1].endAddress=reservedPlaces[j].endAddress;
	}
		reservedPlacesIndex-=1;

}


unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	int frameNumber = physical_address / PAGE_SIZE;
	uint32 *ptr_page_table = NULL;
	for(int i=KERNEL_HEAP_START; i<KERNEL_HEAP_MAX; i+=PAGE_SIZE)
	{
		get_page_table(ptr_page_directory, (void*)i, &ptr_page_table);
		unsigned int frameno=(ptr_page_table[PTX(i)]>>12);
		if(frameno == frameNumber)
			return i;
	}


	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details
	//change this "return" according to your answer

	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2019 - MS1 - [1] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details
	uint32 *ptr_page_table = NULL;
	get_page_table(ptr_page_directory, (void*)virtual_address, &ptr_page_table);
	unsigned int frameno=(ptr_page_table[PTX(virtual_address)]>>12);
	unsigned int physicalAddress = frameno*PAGE_SIZE;
	physicalAddress += virtual_address & 0x00000fff;

	//change this "return" according to your answer
	return physicalAddress;
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2019 - BONUS2] Kernel Heap Realloc
	// Write your code here, remove the panic and write your code

	return NULL;
	panic("krealloc() is not implemented yet...!!");

}
