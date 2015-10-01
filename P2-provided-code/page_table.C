#include "page_table.H"
#include "paging_low.H"

FramePool     * PageTable::kernel_mem_pool;    /* Frame pool for the kernel memory */
FramePool     * PageTable::process_mem_pool;   /* Frame pool for the process memory */
unsigned long   PageTable::shared_size;        /* size of shared address space */
PageTable     * PageTable::current_page_table;

void PageTable::init_paging(FramePool * _kernel_mem_pool,
                          FramePool * _process_mem_pool,
                          const unsigned long _shared_size){
  kernel_mem_pool  = _kernel_mem_pool;
  process_mem_pool = _process_mem_pool;
  shared_size      = _shared_size;  
}
  /* Set the global parameters for the paging subsystem. */

PageTable::PageTable(){
  page_directory = (unsigned long*)(kernel_mem_pool->get_frame()*PAGE_SIZE);
  unsigned long* page_table = (unsigned long*)(kernel_mem_pool->get_frame()*PAGE_SIZE);
  
  unsigned long frame_addr = 0;
  for (unsigned int i = 0; i < ENTRIES_PER_PAGE; i++){
    page_table[i] = frame_addr | 3;
    frame_addr += PAGE_SIZE;
  }

  page_directory[0] = (unsigned long)page_table;
  page_directory[0] = page_directory[0] | 3;

  for (unsigned int i = 1; i < ENTRIES_PER_PAGE; i++)
  {
    page_directory[i] = 0 | 2;
  }

}
  /* Initializes a page table with a given location for the directory and the
     page table proper.
     NOTE: The PageTable object still needs to be stored somewhere! Probably it is best
           to have it on the stack, as there is no memory manager yet...
     NOTE2: It may also be simpler to create the first page table *before* paging
           has been enabled.
  */

void PageTable::load(){
  current_page_table = this;
}
  /* Makes the given page table the current table. This must be done once during
     system startup and whenever the address space is switched (e.g. during
     process switching). */

void PageTable::enable_paging(){
  write_cr3((unsigned long)(current_page_table->page_directory)); // put that page directory address into CR3
  write_cr0(read_cr0() | 0x80000000); // set the paging bit in CR0 to 1
}
  /* Enable paging on the CPU. Typically, a CPU start with paging disabled, and
     memory is accessed by addressing physical memory directly. After paging is
     enabled, memory is addressed logically. */

void PageTable::handle_fault(REGS * _r){
  unsigned long faultAddress = read_cr2();

  if (((_r -> err_code)&1) == 0){
    unsigned long* page_table;
    unsigned long page_dir_index = faultAddress >> 22;
    unsigned long page_table_index = (faultAddress >> 12) & 0x003FF;

    unsigned long* page_dir = current_page_table -> page_directory;
    unsigned long* physicalAddr;

    page_table = (unsigned long*)page_dir[page_table_index];

//??????
    if ((page_dir[page_table_index]&1) ==0){
      page_table = (unsigned long *) (kernel_mem_pool->get_frame() * PAGE_SIZE);

      for (int i = 0; i < ENTRIES_PER_PAGE; i++){
        physicalAddr = (unsigned long*) process_mem_pool->get_frame();
        page_table[i] = (unsigned long)physicalAddr | 3;
      }

      page_dir[page_dir_index] = (unsigned long) (page_table);
      page_dir[page_dir_index] |= 3;
    }else{
      physicalAddr = (unsigned long *) (process_mem_pool->get_frame() * PAGE_SIZE);
        page_table[page_table_index] = (unsigned long)(physicalAddr);
        page_table[page_table_index] |= 7;
    }

  }

}























