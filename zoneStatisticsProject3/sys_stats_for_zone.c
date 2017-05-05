#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm_types.h>
#include <linux/mmzone.h>
#include <asm/pgtable.h>
#include <linux/page-flags.h>

asmlinkage int sys_stats_for_zone(void){
  struct zone* current_zone;
  struct page* cached_page;
  struct zone_lru current_lru;
  int i;
  int active_pages = 0;
  int inactive_pages = 0;
  int active_ref = 0;
  int inactive_ref = 0;
  int page_count;
  int count;
  for_each_zone(current_zone){
    for_each_lru(i) {
      count = 0;
      current_lru = current_zone->lru[i];
      page_count = atomic_long_read(&(current_zone->vm_stat[NR_ACTIVE_ANON]));
      list_for_each_entry(cached_page, &current_lru.list, lru) {
        if(i == LRU_ACTIVE_ANON || i == LRU_ACTIVE_FILE){
	  active_pages++;
	  if(PageReferenced(cached_page)){
	    active_ref++;
	  }  
        }else{
	  inactive_pages++;
	  if(PageReferenced(cached_page)){
	    inactive_ref++;
	  }
        }
	
	if (count == page_count)
	  break;
	else
	  count++;
      }
    }
    spin_unlock(&(current_zone->lru_lock));
	
  }
  printk(KERN_EMERG "Total Pages: %d \n",active_pages + inactive_pages);
  printk(KERN_EMERG "Active pages: %d \n Ref: %d \n", active_pages, active_ref);
  printk(KERN_EMERG "Inactive pages: %d \n Ref: %d", inactive_pages, inactive_ref);
  return 0;
}
