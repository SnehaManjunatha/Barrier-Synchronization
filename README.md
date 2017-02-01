 Implementing a  Barrier Mechanism in Linux Kernel 
 
 Barrier synchronization has been applied widely in parallel computing to synchronize the execution of parallel loops. In this assignment we have implemented  
 barrier functonalities as  system call.
 
 	The barrier structure contains the following members:
		unsigned int m_count;     					//count the number of threads waiting onthe barrier
		unsigned int m_barrier_id;					// barrier id
		spinlock_t m_Lock;							// lock for barrier each structure 
		pid_t m_common_id;							// the pid of the process to distindiush between the address space
		unsigned int m_num_of_threads;				// total number of threads to wait on the barrier 
		wait_queue_head_t my_queue;					// wait_queue to enqueue the threads on the barrier
		struct list_head barrier_entry;				// to maintain a list of barrier structure 
 


The system calls are:

 	syscall(352, th_args->bar_id) for  sys_barrier_wait(unsigned int barrier_id)
	 syscall(351,buf,u_bar_id1)    for  sys_barrier_init(unsigned int count, unsigned int *barrier_id)
	 syscall(353,bar_id1)		   for  sys_barrier_destroy(unsigned int barrier_id)
 
 Process to run the test program:
 
 	1) Applying the patch:
		patch -p1 < path/to/patch/barrier.patch
	 2)Then perform the following steps:
		Include the cross-compilation tools in your PATH:
			export PATH=path_to_sdk/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux:$PATH
		Cross-compile the kernel
			ARCH=x86 LOCALVERSION= CROSS_COMPILE=i586-poky-linux- make -j4
		Build and extract the kernel modules from the build to a target directory (e.g ../galileo-install)
			ARCH=x86 LOCALVERSION= INSTALL_MOD_PATH=../galileo-install CROSS_COMPILE=i586-poky-linux- make modules_install
		Extract the kernel image (bzImage) from the build to a target directory (e.g ../galileo-install)
			cp arch/x86/boot/bzImage ../galileo-install/
		Install the new kernel and modules from the target directory (e.g ../galileo-install) to your micro SD card
			- Replace the bzImage found in the first partition (ESP) of your micro SD card with the one from your target 				  directory (backup the bzImage on the micro SD card e.g. rename it to bzImage.old)
			- Copy the kernel modules from the target directory to the /lib/modules/ directory found in the second partition 			   of your micro SD card.
		Reboot into your new kernel
		
	 3) Run the Makefile to obtain the executable :
       		make all
 	 4) After the command "make all", b_test.o object file is generated
 	 5) Run that file on the galileo serial termial by running the following command
       		sudo screen /dev/ttyUSB0 115200
	 6) Now we can run our application by :
		chmod 777 b_test
       		./b_test
 

The output is displayed in the following:

1) To differentiate between the two child processes we have used the following format:
	
	(a) Single tab for dispaying the threads belonging to child1 along with pid and tgid.
	
	(b) Double tab for dispaying the threads belonging to child2  along with pid and tgid.
	
	eg: 100 round of synchonization : Thread entering : pid is 223 my thread id is 230
	        100 round of synchonization : Thread entering : pid is 224 my thread id is 232
					
The patch "barrier.patch" and the "b_test" file are present in the excecutables folder.
We have also uploaded the file "barrier_drv.c" which containes the barrier system call implementation.
