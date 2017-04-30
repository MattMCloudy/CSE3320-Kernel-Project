#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/fs.h>
#include <asm/page.h>
#include <asm/mman.h>

asmlinkage int sys_statistics_vma(int pid)
{
        struct task_struct *beginning_task;
        int vma_count;
        unsigned long protected = 0;
        unsigned long beginning = 0;
        int count = 0;
        int whole_size = 0;
        char *name;
        struct mm_struct* beginning_mem;
        struct vm_area_struct* current_vma;
	
	//we'll start by finding the task we're at rn
	if((beginning_task = find_task_by_vpid(pid)) == NULL) {
		printk(KERN_EMERG "This task does not exist\n");
		return 0;
	}

        beginning_mem = beginning_task->active_mm;
        vma_count = beginning_mem->map_count;
        current_vma = beginning_mem->mmap;
        beginning = current_vma->vm_start;

        printk(KERN_EMERG "\nNumber of VMAs: %i\n", vma_count);
        for (; count < vma_count; current_vma = current_vma->vm_next, count++) {	
                protected = current_vma->vm_flags;
                printk(KERN_EMERG "Number of current VMA: %i\n", count+1);
                printk(KERN_EMERG "Start: 0x%lx\n", current_vma->vm_start);
                printk(KERN_EMERG "End: 0x%lx\n", current_vma->vm_end);
                printk(KERN_EMERG "Total size: %lu\n", current_vma->vm_end - current_vma->vm_start);
                printk(KERN_EMERG "\nPermissions\n");
                printk(KERN_EMERG "Read: %s \n", protected&PROT_READ?"yes":"no"); 
                printk(KERN_EMERG "Write: %s \n", protected&PROT_WRITE?"yes":"no"); 
                printk(KERN_EMERG "Execute: %s \n", protected&PROT_EXEC?"yes":"no"); 

                if (current_vma->vm_file != NULL) {
                        name = d_path(&current_vma->vm_file->f_path, (char*)__get_free_page(GFP_TEMPORARY), PAGE_SIZE);
                        printk(KERN_EMERG "Filename: %s\n", name);
                }


                whole_size += current_vma->vm_end - current_vma->vm_start;
        }
        printk(KERN_EMERG "\nSize of all VMAs: %i\n", whole_size);

        return 1;
}
