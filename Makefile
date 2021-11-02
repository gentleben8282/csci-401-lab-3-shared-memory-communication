shm_proc: shm_processes.c
	gcc shm_processes.c -D_SVID_SOURCE -D_GNU_SOURCE -Wall -std=c99 -Werror=cpp -pedantic -o shm_proc

shm_proc2: shm_processes2.c
	gcc shm_processes2.c -D_SVID_SOURCE -D_GNU_SOURCE -Wall -std=c99 -Werror=cpp -pedantic -o shm_proc2
