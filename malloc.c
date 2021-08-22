//Name= Avijeet Adhikari UT Arlington

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *freeList = NULL; /* Free list to track the _blocks available */
struct _block *nextfit_tracker = NULL; //tracks last used block for Next Fit
/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
*/

struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = freeList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
    
    struct _block *new=NULL; //to save the best block
    struct _block *tempptr=NULL;//helps to compare current and next block
    while (curr)
    {
        if(curr->free && curr->size >= size)// CUrrent block should be free and its size should be greater or equal than the requested size
        {
            if((curr->size)-size == 0)//When block size and requested size are equal then get that block which is the best case.
            {
                break;//break the loop cause we will already have the block that fits best.
            }
            else //If block size is greater than the requested size that not equal.
            {
                if(tempptr!=NULL)
                {
                    if(tempptr && tempptr->free)
                    {
                        if(tempptr->size < curr->size)//update Best Block if next Block is less than current Block.
                        {
                            new = tempptr;
                        }
                        else
                        {
                            new = curr;
                        }
                    }
                }
            }
        }
        tempptr = curr;
        curr  = curr->next;
   }
    if(curr && new==NULL && (curr->size)-size == 0) //If exact same sized block is found loop hits the break, return that Block.
    {
        *last = curr;
        curr = curr->next;
    }
    else //returns best possible block matched.
    {
        if(new!=NULL)
        {
            *last = new->prev;
            curr = new;
        }
    }
   
#endif

#if defined WORST && WORST == 0
    /*Worst Fit has opposite algorithm compared to Best Fit.*/
    struct _block *new=NULL;
    struct _block *tempptr=NULL;
    while (curr)
    {
        if(curr->free && curr->size >= size)//CUrr Block should be free and greater or equal to th requested size.
        {
            if(tempptr!=NULL)
            {
                if(tempptr && tempptr->free)
                {
                    if(tempptr->size >= curr->size) //If next Block is greater than current block update Worst fit block. Also take care for equal sized block.
                    {
                        new = tempptr;
                    }
                    else
                    {
                        new = curr;
                    }
                }
            }
        }
        tempptr = curr;
        curr  = curr->next;
    }
    if(new!=NULL)
    {
        *last = new->prev;
        curr = new;
    }

#endif

#if defined NEXT && NEXT == 0
    //First find the block which it fits first
    while (curr && !(curr->free && curr->size >= size))
    {
        *last = curr;
        curr  = curr->next;
    }
    
    nextfit_tracker = *last; //starts to track from the first fit block
    
    if(nextfit_tracker)
    {
        curr = nextfit_tracker->next;
        //starts from  the first fit block and goes upto null to find the next block to fit
        while (curr && !(curr->free && curr->size >= size))
        {
            *last = curr;
            curr  = curr->next;
        }
    }
    else //If nextfit_tracker reach the last block, restart the process from first of the block list.
    {
        while (curr && !(curr->free && curr->size >= size))
        {
            *last = curr;
            curr  = curr->next;
        }
        nextfit_tracker = *last;
    }
#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update freeList if not set */
   if (freeList == NULL) 
   {
      freeList = curr;
   }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
    max_heap+=size;
    num_grows++;
    num_blocks++;
    return curr;
}
/*
 As stated by profesor in class,we are saving the product of size of two arguments
 as product and malloc it and save as new Pointer
 memset the value of Cal_ptr as 0.
 */
void *calloc(size_t nmemb, size_t size)
{
    size_t product= nmemb*size;
    if(product==0)
    {
        return NULL;
    }
    void *Cal_ptr=malloc(product);
    memset(Cal_ptr,0,product);
    return Cal_ptr;
}

/*As stated by professor in class, we malloc the argument size and store in re_ptr,
 copy all the adress from ptr to new pinter re_ptr and return it*/
void *realloc(void *ptr, size_t size)
{
    void *re_ptr=malloc(size);
    memcpy(re_ptr,ptr,size);
    return re_ptr;
    
}
/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = freeList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO: Split free _block if possible */
    //Each time free block used num_reuses increases.
    if(next!=NULL &&(next->size) >= (size + sizeof(struct _block)))
    {
        num_reuses++;
    }
    if(next!=NULL &&(next->size) > (size + sizeof(struct _block)))
    {
        struct _block *prev_block = next;//saving the original block to retrieve later
        //do the pinter airthmetic by casting the prev_block to char* and the shift it by the total of size while
        //taking the account of the size of block so that we can start the splitted new block form there
        //typecasting it back to the struct _block pointer for the actual address and specific pointer type.
        struct _block *Next_block = (struct _block *)((char*)prev_block + size + sizeof(struct _block));
        Next_block->next =prev_block;
        Next_block->size =(next->size)-sizeof(struct _block)-size;
        Next_block->free=true;
        num_splits++;
        num_blocks++;
    }
   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;
    num_requested+=size;
    num_mallocs++;
   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
    
    /* TODO: Coalesce free _blocks if needed */
    
    struct _block *templist = freeList;
    //linking the current block's previous block from the freeList so that
    //we can check and coalesce
    if(templist)
    {
        while(templist)
        {
            if(templist == curr)
            {
                curr->prev =templist;
            }
            templist=templist->next;
        }
    }
    
    size_t prev_total;//To store the sum of two merged Block
    struct _block *tempprev = curr -> prev;
     if(tempprev && tempprev->free)//If previous pointer is not null/Free
     {
         prev_total=(curr->size + tempprev->size + sizeof(struct _block));//add current block size, previous block size and size of Block
         tempprev->size=prev_total;//Give new value to previous Block
         tempprev->next=curr->next;//join previous block and next block skipping the current block.
         num_coalesces++;
     }
    else//If previous block is not free then look for next block to comn=bine.
    {
        size_t total;//To store the sum of two merged Block
        struct _block *tempnext = curr -> next;
    
        if(tempnext && tempnext->free)//If the next block is free.
        {
            total=(curr->size + tempnext->size + sizeof(struct _block));//add current block size, next block size and size of Block
            curr->size=total;//Give new value to curr size.
            curr->next=tempnext->next;//Current added sized block skips next pointer and joins with the next->next pointer.
            num_coalesces++;
        }
    }
    num_frees++;
     
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
