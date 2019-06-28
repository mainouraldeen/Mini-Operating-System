
#include <inc/lib.h>

// malloc()
//	This function use BEST FIT strategy to allocate space in heap
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



struct userHeapBlock
{
	uint32 startAddress;
	uint32 endAddress;
	uint32 size;
	int empty;
};

#define sizze ((USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE)
struct userHeapBlock userBlock[sizze];

struct userHeapBlock reservedPlaces[1000];
int reservedPlacesIndex = 0;
int initialize=1;

void* malloc(uint32 size)
{
	//TODO: [PROJECT 2019 - MS2 - [5] User Heap] malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	// Steps:
	//	1) Implement BEST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_allocateMem to invoke the Kernel for allocation
	// 	4) Return pointer containing the virtual address of allocated space,
	//
	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //
	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy
	//change this "return" according to your answer
	struct userHeapBlock availablePlaces;
	if(initialize==1){
		int index1=0;
		for(int i=USER_HEAP_START; i< USER_HEAP_MAX; i+= PAGE_SIZE)
		{
			userBlock[index1].startAddress = i;
			userBlock[index1].endAddress = i+PAGE_SIZE;
			userBlock[index1].empty=1;
			index1++;
		}
		initialize=0;
	}

	//saving the sizes and start address of every suitable compatable size
	availablePlaces.size = 0x7fffffff;
	uint32 numberOfPages=(size/PAGE_SIZE);
	if (size % PAGE_SIZE)
		numberOfPages++;

	uint32*ptr = NULL;
	int pageCounter=0;

	//bestfit
	if (sys_isUHeapPlacementStrategyBESTFIT())
	{
		//scan user heap
		uint32 addrs=USER_HEAP_START;
		int index2=0;
		for(uint32 i= USER_HEAP_START; i<USER_HEAP_MAX; i+=PAGE_SIZE)
		{
			uint32 check = (userBlock[index2].empty);
			if(check == 1)//checking for an empty page
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
			index2++;
		}


		if(pageCounter >= numberOfPages && availablePlaces.size > pageCounter)
		{
			availablePlaces.startAddress = USER_HEAP_MAX-(pageCounter*PAGE_SIZE);
			availablePlaces.size = pageCounter;
			availablePlaces.endAddress=availablePlaces.startAddress+pageCounter*PAGE_SIZE;
			//availablePlacesIndex++;
		}
		else if(availablePlaces.size == 0x7fffffff)// didn't find suitable space
		{
			return NULL;
		}


	}//if best fit

	//maping
	sys_allocateMem(availablePlaces.startAddress, numberOfPages);
	int indx = 0;
	uint32 tmpAddr = availablePlaces.startAddress;
	//to set the allocated space as not empty
	for(int i=USER_HEAP_START; i< USER_HEAP_MAX; i+= PAGE_SIZE)
	{
		if(userBlock[indx].startAddress == tmpAddr)
		{
			for(int j = 0; j < numberOfPages; j++)
			{
				userBlock[indx].empty = 0;
				indx++;
			}
			break;
		}
		indx++;
	}

	reservedPlaces[reservedPlacesIndex].startAddress = availablePlaces.startAddress;
	reservedPlaces[reservedPlacesIndex].size = numberOfPages;
	reservedPlaces[reservedPlacesIndex].endAddress = reservedPlaces[reservedPlacesIndex].startAddress+(numberOfPages*PAGE_SIZE);
	reservedPlacesIndex ++;


	return (void*)availablePlaces.startAddress;

}


void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//hal h3ml zay el malloc blzabt, mn 8er el sys_allocatemem ?????????????????
	//cprintf("at begin smalloc name: %s, size: %d, isWritable: %d\n\n",sharedVarName, size,isWritable);
		if(initialize==1)
		{
			int index1=0;
			for(int i=USER_HEAP_START; i< USER_HEAP_MAX; i+= PAGE_SIZE)
			{
				userBlock[index1].startAddress = i;
				userBlock[index1].endAddress = i+PAGE_SIZE;
				userBlock[index1].empty=1;
				index1++;
			}
			initialize=0;
		}

		//saving the sizes and start address of every suitable compatable size
		struct userHeapBlock availablePlaces;
		availablePlaces.size = 0x7fffffff;

		uint32 numberOfPages=(size/PAGE_SIZE);
		if (size % PAGE_SIZE)
			numberOfPages++;

		uint32*ptr = NULL;
		int pageCounter=0;

		//bestfit
		if (sys_isUHeapPlacementStrategyBESTFIT())
		{
			//scan user heap
			uint32 addrs = USER_HEAP_START;
			int index2 = 0;
			for(uint32 i= USER_HEAP_START; i<USER_HEAP_MAX; i+=PAGE_SIZE)
			{

				if(userBlock[index2].empty == 1)//checking for an empty page
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
				index2++;
			}


			if(pageCounter >= numberOfPages && availablePlaces.size > pageCounter)
			{
				availablePlaces.startAddress = USER_HEAP_MAX-(pageCounter*PAGE_SIZE);
				availablePlaces.size = pageCounter;
				availablePlaces.endAddress=availablePlaces.startAddress+pageCounter*PAGE_SIZE;
				//availablePlacesIndex++;
			}
			else if(availablePlaces.size == 0x7fffffff)// didn't find suitable space
			{
				return NULL;
			}


		}//if best fit


		int id = sys_createSharedObject(sharedVarName, size, isWritable, (void*)availablePlaces.startAddress);
			if(id == E_NO_SHARE || id == E_SHARED_MEM_EXISTS)
				return NULL;


		int indx = 0;
		uint32 tmpAddr = availablePlaces.startAddress;
		//to set the allocated space as not empty
		for(int i=USER_HEAP_START; i< USER_HEAP_MAX; i+= PAGE_SIZE)
		{
			if(userBlock[indx].startAddress == tmpAddr)
			{
				for(int j = 0; j < numberOfPages; j++)
				{
					userBlock[indx].empty = 0;
					indx++;
				}
				break;
			}
			indx++;
		}


		reservedPlaces[reservedPlacesIndex].startAddress = availablePlaces.startAddress;
		reservedPlaces[reservedPlacesIndex].size = numberOfPages;
		reservedPlaces[reservedPlacesIndex].endAddress = reservedPlaces[reservedPlacesIndex].startAddress+(numberOfPages*PAGE_SIZE);
		reservedPlacesIndex ++;


	return (void*)availablePlaces.startAddress;

}

void* sget(int32 ownerEnvID, char *sharedVarName)
{

	//TODO: [PROJECT 2019 - MS2 - [6] Shared Variables: Get] sget() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");

	// Steps:
	//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
	int sizeOfSharedVar = sys_getSizeOfSharedObject(ownerEnvID, sharedVarName);
	//	2) If not exists, return NULL
	if(sizeOfSharedVar == E_SHARED_MEM_NOT_EXISTS)
		return NULL;
//	cprintf("In fn sget: sharedVarName %s sizeOfSharedVar1 %d\n",sharedVarName,sizeOfSharedVar);
	//	3) Implement BEST FIT strategy to search the heap for suitable space
	//		to share the variable (should be on 4 KB BOUNDARY)
	//	4) if no suitable space found, return NULL
	//	 Else,

	if(initialize==1)
	{
		int index1=0;
		for(int i=USER_HEAP_START; i< USER_HEAP_MAX; i+= PAGE_SIZE)
		{
			userBlock[index1].startAddress = i;
			userBlock[index1].endAddress = i+PAGE_SIZE;
			userBlock[index1].empty=1;
			index1++;
		}
		initialize=0;
	}

	//saving the sizes and start address of every suitable compatable size
	struct userHeapBlock availablePlaces;
	availablePlaces.size = 0x7fffffff;

	int shareObjectSize = sys_getSizeOfSharedObject(ownerEnvID, sharedVarName);

	if(shareObjectSize == E_SHARED_MEM_NOT_EXISTS)
		return NULL;

	int numberOfPages = (shareObjectSize / PAGE_SIZE);
	if (shareObjectSize % PAGE_SIZE)
		numberOfPages++;

	int pageCounter=0;

	//bestfit
	if (sys_isUHeapPlacementStrategyBESTFIT())
	{
		//scan user heap
		uint32 addrs = USER_HEAP_START;
		int index2 = 0;
		for(uint32 i= USER_HEAP_START; i<USER_HEAP_MAX; i+=PAGE_SIZE)
		{
			if(userBlock[index2].empty == 1)//checking for an empty page
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
			index2++;
		}


		if(pageCounter >= numberOfPages && availablePlaces.size > pageCounter)
		{
			availablePlaces.startAddress = USER_HEAP_MAX-(pageCounter*PAGE_SIZE);
			availablePlaces.size = pageCounter;
			availablePlaces.endAddress=availablePlaces.startAddress+pageCounter*PAGE_SIZE;
			//availablePlacesIndex++;
		}
		else if(availablePlaces.size == 0x7fffffff)// didn't find suitable space
		{
			return NULL;
		}


	}//if best fit

//	cprintf("In fn sget: after best fit\n\n");

	//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
	//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
	//	6) If the Kernel successfully share the variable, return its virtual address
	//	   Else, return NULL

	// el fr2 benha w ben ely fo2: en dih bt-map el frames kman w bt-return el id

	int sharedId = sys_getSharedObject(ownerEnvID, sharedVarName, (void*)availablePlaces.startAddress);
//	cprintf("In fn sget: sharedId: %d\n", sharedId);
	if(sharedId == E_SHARED_MEM_NOT_EXISTS)
		return NULL;

//	cprintf("In fn sget: after sys_getSharedObject\n\n");
	//
	//This function should find the space for sharing the variable
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy

	//change this "return" according to your answer
	int indx = 0;
	uint32 tmpAddr = availablePlaces.startAddress;
	//to set the allocated space as not empty
	for(int i=USER_HEAP_START; i< USER_HEAP_MAX; i+= PAGE_SIZE)
	{
		if(userBlock[indx].startAddress == tmpAddr)
		{
			for(int j = 0; j < numberOfPages; j++)
			{
				userBlock[indx].empty = 0;
				indx++;
			}
			break;
		}
		indx++;
	}
//	cprintf("In fn sget: after setting allocated space as not empty\n\n");

	reservedPlaces[reservedPlacesIndex].startAddress = availablePlaces.startAddress;
	reservedPlaces[reservedPlacesIndex].size = numberOfPages;
	reservedPlaces[reservedPlacesIndex].endAddress = reservedPlaces[reservedPlacesIndex].startAddress+(numberOfPages*PAGE_SIZE);
	reservedPlacesIndex ++;

//	cprintf("In fn sget: allocated space at address %x\n ",availablePlaces.startAddress);

	return (void*)availablePlaces.startAddress;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	//TODO: [PROJECT 2019 - MS2 - [5] User Heap] free() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	//you should get the size of the given allocation using its address
	//you need to call sys_freeMem()
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
	//freeing from the pagefile and working set
	sys_freeMem(requiredStartAddress,requiredSize);

	//setting the empty bit by 1
	int indx = 0;
	uint32 tmpAddr = requiredStartAddress;
	//to set the allocated space as not empty
	for(int i=USER_HEAP_START; i< USER_HEAP_MAX; i+= PAGE_SIZE)
	{
		if(userBlock[indx].startAddress == tmpAddr)
		{
			for(int j = 0; j < requiredSize; j++)
			{
				userBlock[indx].empty = 1;
				indx++;
			}
			break;
		}
		indx++;
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



//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=============
// [1] sfree():
//=============
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT 2019 - BONUS4] Free Shared Variable [User Side]
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");

	//	1) you should find the ID of the shared variable at the given address
	//	2) you need to call sys_freeSharedObject()

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
	//TODO: [PROJECT 2019 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

}
