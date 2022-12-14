

/*
 * Loader Implementation
 *
 * 2022, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <math.h>

#include "exec_parser.h"

static so_exec_t *exec;
static struct sigaction default_action;
static int file;

typedef struct segment_data {
	int *pages;
	int pages_number;
}segment_data;


int MIN(int a, int b) {
	if (a < b)
		return a;
	return b;
}
/**
 * Verify in which segment is the page-fault
*/
int find_segment_with_segv(void * segv_adr) {

	for (int i = 0; i < exec->segments_no; i++) {
		void * start = (void *)exec->segments[i].vaddr;
		void * end = (void *)exec->segments[i].vaddr + exec->segments[i].mem_size;
		if (start <= segv_adr && segv_adr <= end)
			return i;
	}

	return -1;
}

/**
 *  Handler for SIGSEGV 
 * */
static void segv_handler(int signum, siginfo_t *info, void *context)
{

	/* TODO - actual loader implementation */

	/* we verify if our signal is SIGSEGV */
	if (signum != SIGSEGV) {
		default_action.sa_sigaction(signum, info, context);
		return;
	}

	/* the address of out signal */
	void *segv_adr = info->si_addr;

	/* the segment where the signal ocurred */
	int index_segment = find_segment_with_segv(segv_adr);

	if (index_segment == -1) {

		/* invalid acces memory */
		default_action.sa_sigaction(signum, info, context);
		return;
	} else {

		so_seg_t *segment = &exec->segments[index_segment];

		int page_size = getpagesize();

		/* the page where the signal ocurred */
		int index_page = (int)(segv_adr - segment->vaddr) / page_size;


		if (((segment_data *)segment->data)->pages[index_page] == 1) {
			
			/* unpermitted acces memory */
			default_action.sa_sigaction(signum, info, context);
			return;
		} else {
			void *map_address = mmap((void *)segment->vaddr + index_page * page_size, 
									page_size, 
									PROT_WRITE, 
									MAP_SHARED | MAP_FIXED | MAP_ANONYMOUS,
									-1, 
									0);

			if (map_address == MAP_FAILED) {
				perror("Error allocate memory for map");
				exit(1);
			}
			((segment_data *)segment->data)->pages[index_page] = 1;
			int r;

			if ((int)segment->file_size > index_page * page_size) {
				lseek(file, segment->offset + index_page * page_size, SEEK_SET);
				r = read(file, map_address, MIN((int)segment->file_size - index_page * page_size, page_size));

				if (r == -1) {
					perror("Error read");
					exit(1);
				}
			}

			r = mprotect(map_address, page_size, segment->perm);

			if (r == -1) {
				perror("Couldn't set permissions");
				exit(1);
			}
		}


	}
	

}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));


	sa.sa_sigaction = segv_handler;

	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	/* parse an executable specified in path */
	exec = so_parse_exec(path);
	if (!exec) {
		return -1;
	}

	/* we open file to copy information later */
	file = open(path, O_RDONLY);

	if (!file) {
		perror("Couldn't open file!");
		return -1;
	}

	/* we allocate the structure for segament data */
	for(int i = 0; i < exec->segments_no; i++) {

		segment_data *data = (segment_data *) malloc(sizeof(segment_data));

		if (data == NULL) {
			perror("Error memory allocation for segment!");
			return -1;
		}

		int page_size = getpagesize();
		data->pages_number = exec->segments[i].mem_size / page_size;
		data->pages = (int *) calloc(data->pages_number, sizeof(int));

		if (data->pages == NULL) {
			perror("Error memory allocation for pages!");
			return -1;
		}

		exec->segments[i].data = data;
	}

	/* we run the first instruction of our executable */

	
	so_start_exec(exec, argv);
    close(file);
	return 0;
}
