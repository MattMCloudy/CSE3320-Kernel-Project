#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <linux/mm_types.h>

asmlinkage int sys_by_process_vma(unsigned long mem, int pid)
{
        struct task_struct* current_task = find_task_by_vpid(pid);
        int data = 0;
        int reference = 0;
        int dirty_bits = 0;
        struct mm_struct* current_mem = current_task->active_mm;
        
	//returning each section of the page table
	pgd_t* global_page = pgd_offset(current_mem, mem);
        pud_t* upper_page = pud_offset(global_page, mem);
        pmd_t* middle_page = pmd_offset(upper_page, mem);
        pte_t* table_entry = pte_offset_kernel(middle_page, mem);
        
	data = pte_present(*table_entry);
        reference = pte_young(*table_entry);
        dirty_bits =  pte_dirty(*table_entry);
       	
	printk(KERN_EMERG "Present: %i\n", data?1:0);
        printk(KERN_EMERG "Referenced: %i\n", reference?1:0);
        printk(KERN_EMERG "Dirty: %i\n", dirty_bits?1:0);
        return 0;
}
