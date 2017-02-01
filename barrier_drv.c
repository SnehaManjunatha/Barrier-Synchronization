/* Barrier System Call Implementation
 --------------------------------------*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include<linux/init.h>
#include<linux/moduleparam.h>

int errno;
/* barrier structure*/
typedef struct 
{
	unsigned int m_count;
	unsigned int m_barrier_id;
	spinlock_t m_Lock;
	pid_t m_common_id;
	unsigned int m_num_of_threads;
	wait_queue_head_t my_queue;
	struct list_head barrier_entry;
			
}barrier_struct, *pbarrier_struct;

static LIST_HEAD(barrier_list);
static bool wait_flag = 0;
static int wait_count =0;

/*barrier destroy system call*/ 
asmlinkage long sys_barrier_destroy(unsigned int barrier_id)
{
	int list_flag = 0;
	struct list_head *pos, *q;
	pbarrier_struct del_pbarrier; 

	if(wait_count != 0)
		return -EBUSY;
	list_for_each_safe(pos, q ,&barrier_list)
	{
		 
		del_pbarrier= list_entry(pos, barrier_struct, barrier_entry);
		if(del_pbarrier->m_barrier_id == barrier_id && del_pbarrier->m_common_id == task_tgid_vnr(current) )
		{
			list_flag = 1;
	 		 printk("The barrier id %d with tgid %d is destroyed \n",del_pbarrier->m_barrier_id,task_tgid_vnr(current));
			list_del(pos);

			kfree(del_pbarrier);
		}
	
 	
	}
	if(list_flag == 0)
	{

		return -EINVAL;
	}

	return 0;	
}

asmlinkage long sys_barrier_wait(unsigned int barrier_id)
{
	int list_flag = 0;

	pbarrier_struct temp_pbarrier;
    
   	int barrierID;
		
	barrierID = barrier_id;
  	
	list_for_each_entry(temp_pbarrier, &barrier_list, barrier_entry)
	{
		if(temp_pbarrier->m_common_id == task_tgid_vnr(current)  && temp_pbarrier->m_barrier_id == barrierID)	
		{
			list_flag = 1;
			break;
		}
	}
	if(list_flag == 0)
	{
		return -EINVAL;
	}
   
   spin_lock(&temp_pbarrier->m_Lock);
	if(temp_pbarrier->m_count+1 < temp_pbarrier->m_num_of_threads)
	{
		temp_pbarrier->m_count++;
		printk("tgid = %d, pid = %d enqueued\n", task_tgid_vnr(current), task_pid_nr(current));
		wait_count++;
   spin_unlock(&temp_pbarrier->m_Lock);	
	
		wait_event_interruptible(temp_pbarrier->my_queue,wait_flag);  
		printk(KERN_INFO "tgid = %d, pid = %d WOKEN UP\n",task_tgid_vnr(current), task_pid_nr(current));

   spin_lock(&temp_pbarrier->m_Lock);
		if(wait_count == 0)		
		{
			wait_flag = 0;
		}
		else
			wait_count--;
   spin_unlock(&temp_pbarrier->m_Lock);
	}
	
	else	
	{
		wait_flag =1;
		wake_up_all(&temp_pbarrier->my_queue);
		temp_pbarrier->m_count = 0;
   spin_unlock(&temp_pbarrier->m_Lock);
	}
	return 0;
}
/*barrier init system call*/ 
asmlinkage long sys_barrier_init(unsigned int count, unsigned int *barrier_id)
{	
	pbarrier_struct pbarrier;
	pbarrier_struct temp_pbarrier;
	int list_flag = 0;
	pbarrier = kmalloc(sizeof(barrier_struct), GFP_KERNEL);
	if (!pbarrier) 
	{
		printk("Bad pbarrier Kmalloc\n");
	}
	memset(pbarrier, 0, sizeof( barrier_struct));
	
	if(list_empty(&barrier_list)==0)   
	{
		list_for_each_entry(temp_pbarrier, &barrier_list, barrier_entry)
		{
			if(temp_pbarrier->m_common_id == task_tgid_vnr(current))	
			{	
				list_flag = 1;
				pbarrier->m_barrier_id = temp_pbarrier->m_barrier_id + 1;
				
			}
		}
		if(list_flag == 0)
			pbarrier->m_barrier_id = 1;
	}
	else
	pbarrier->m_barrier_id = 1;
	pbarrier->m_num_of_threads = count;
	init_waitqueue_head(&pbarrier->my_queue);
	spin_lock_init(&pbarrier->m_Lock);
	pbarrier->m_count = 0;
	pbarrier->m_common_id = task_tgid_vnr(current);               
	
	INIT_LIST_HEAD(&pbarrier->barrier_entry) ;
	list_add(&pbarrier->barrier_entry, &barrier_list );
	
	printk("The barrier id created for tgid =  %d, pid = %d is %d\n",task_tgid_vnr(current), task_pid_nr(current), pbarrier->m_barrier_id);
	
	*barrier_id = pbarrier->m_barrier_id ;
	return 0;
}


