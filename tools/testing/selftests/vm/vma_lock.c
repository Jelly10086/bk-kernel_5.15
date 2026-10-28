// SPDX-License-Identifier: GPL-2.0
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <setjmp.h>
#include <sched.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <linux/magic.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../kselftest.h"

/* Run on a regular filesystem: shmem does not exercise filemap_fault(). */
#define PATTERN 0x5a
#define READERS 2
#define PAGES 256
#define DEFAULT_ROUNDS 128

enum operation {
	PROTECT,
	REPLACE,
	MOVE,
	FORK_COW,
	INTERRUPT,
};

struct workload {
	unsigned char *base;
	size_t length;
	size_t page_size;
	atomic_bool stop;
	atomic_bool corrupt;
	atomic_ulong probes;
	atomic_ulong faults;
	atomic_int ready;
};

/* sig_atomic_t is shared with the handler, and faulting loads must not vanish. */
static __thread sigjmp_buf fault_env;
static __thread volatile sig_atomic_t probing;
static unsigned int rounds = DEFAULT_ROUNDS;
static size_t page_size;
static size_t length;
static int backing_fd;

static void fault_handler(int sig)
{
	if (probing) {
		probing = 0;
		siglongjmp(fault_env, 1);
	}
	_exit(128 + sig);
}

static void interrupt_handler(int sig)
{
	(void)sig;
}

static void *read_pages(void *arg)
{
	struct workload *work = arg;
	size_t offset;

	atomic_fetch_add(&work->ready, 1);
	while (!atomic_load(&work->stop)) {
		for (offset = 0; offset < work->length; offset += work->page_size) {
			if (sigsetjmp(fault_env, 1)) {
				atomic_fetch_add(&work->faults, 1);
				continue;
			}
			probing = 1;
			if (*(volatile unsigned char *)(work->base + offset) != PATTERN)
				atomic_store(&work->corrupt, true);
			probing = 0;
			atomic_fetch_add(&work->probes, 1);
		}
	}
	return NULL;
}

static int child_status(pid_t pid)
{
	int status;
	pid_t waited;

	do {
		waited = waitpid(pid, &status, 0);
	} while (waited < 0 && errno == EINTR);
	if (waited != pid || !WIFEXITED(status))
		return KSFT_FAIL;
	return WEXITSTATUS(status);
}

static int fork_cow(unsigned char *base)
{
	pid_t pid = fork();
	size_t offset;

	if (pid < 0)
		return KSFT_FAIL;
	if (!pid) {
		/* Only async-signal-safe operations after a multithreaded fork. */
		alarm(10);
		for (offset = 0; offset < length; offset += page_size) {
			if (base[offset] != PATTERN)
				_exit(KSFT_FAIL);
			base[offset] = PATTERN + 1;
		}
		for (offset = 0; offset < length; offset += page_size)
			if (base[offset] != PATTERN + 1)
				_exit(KSFT_FAIL);
		_exit(KSFT_PASS);
	}
	return child_status(pid);
}

static int run_concurrent(enum operation operation)
{
	struct sigaction action = { .sa_handler = fault_handler };
	struct workload work = { 0 };
	pthread_t readers[READERS];
	unsigned char *area, *base, *destination;
	unsigned int iteration, started = 0;
	size_t offset;
	int result = KSFT_FAIL;

	sigemptyset(&action.sa_mask);
	if (sigaction(SIGSEGV, &action, NULL) || sigaction(SIGBUS, &action, NULL))
		return KSFT_FAIL;
	action.sa_handler = interrupt_handler;
	if (sigaction(SIGUSR1, &action, NULL))
		return KSFT_FAIL;

	area = mmap(NULL, length * 2, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (area == MAP_FAILED)
		return KSFT_FAIL;
	base = mmap(area, length, PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_FIXED, backing_fd, 0);
	if (base == MAP_FAILED)
		goto out;
	work.base = base;
	work.length = length;
	work.page_size = page_size;
	for (started = 0; started < READERS; started++)
		if (pthread_create(&readers[started], NULL, read_pages, &work))
			goto join;
	while (atomic_load(&work.ready) < READERS)
		sched_yield();
	while (!atomic_load(&work.probes))
		sched_yield();

	for (iteration = 0; iteration < rounds; iteration++) {
		switch (operation) {
		case PROTECT:
			/* Split, change permissions, then merge without denying reads. */
			if (mprotect(base + page_size, length - 2 * page_size, PROT_READ) ||
			    mprotect(base, length, PROT_READ | PROT_WRITE))
				goto join;
			break;
		case REPLACE:
			if (munmap(base, length))
				goto join;
			if (mmap(base, length, PROT_READ | PROT_WRITE,
				 MAP_PRIVATE | MAP_FIXED, backing_fd, 0) != base)
				goto join;
			break;
		case MOVE:
			destination = base == area ? area + length : area;
			if (mremap(base, length, length,
				   MREMAP_MAYMOVE | MREMAP_FIXED, destination) == MAP_FAILED)
				goto join;
			base = destination;
			break;
		case FORK_COW:
			if (fork_cow(base) != KSFT_PASS)
				goto join;
			break;
		case INTERRUPT:
			for (offset = 0; offset < READERS; offset++)
				if (pthread_kill(readers[offset], SIGUSR1))
					goto join;
			break;
		}
		if (madvise(base, length, MADV_DONTNEED))
			goto join;
		if (atomic_load(&work.corrupt))
			goto join;
	}
	result = KSFT_PASS;
join:
	atomic_store(&work.stop, true);
	while (started)
		if (pthread_join(readers[--started], NULL))
			result = KSFT_FAIL;
	if (!atomic_load(&work.probes) || atomic_load(&work.corrupt))
		result = KSFT_FAIL;
	if (operation != REPLACE && operation != MOVE && atomic_load(&work.faults))
		result = KSFT_FAIL;
	if (result == KSFT_PASS)
		for (offset = 0; offset < length; offset += page_size)
			if (base[offset] != PATTERN)
				result = KSFT_FAIL;
out:
	munmap(area, length * 2);
	return result;
}

static int run_swap(void)
{
	unsigned char *base, *resident;
	size_t bytes = 32UL << 20, offset, count = 0;

	base = mmap(NULL, bytes, PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (base == MAP_FAILED)
		return KSFT_FAIL;
	resident = calloc(bytes / page_size, 1);
	if (!resident) {
		munmap(base, bytes);
		return KSFT_FAIL;
	}
	for (offset = 0; offset < bytes; offset += page_size)
		base[offset] = PATTERN;
	if (madvise(base, bytes, MADV_PAGEOUT) || mincore(base, bytes, resident)) {
		free(resident);
		munmap(base, bytes);
		return KSFT_SKIP;
	}
	for (offset = 0; offset < bytes / page_size; offset++)
		count += !(resident[offset] & 1);
	free(resident);
	if (!count) {
		munmap(base, bytes);
		return KSFT_SKIP;
	}
	for (offset = 0; offset < bytes; offset += page_size)
		if (base[offset] != PATTERN) {
			munmap(base, bytes);
			return KSFT_FAIL;
		}
	munmap(base, bytes);
	return KSFT_PASS;
}

static int create_backing(const char *directory)
{
	char path[PATH_MAX];
	struct statfs filesystem;
	unsigned char *buffer;
	size_t offset;
	ssize_t written;

	if (snprintf(path, sizeof(path), "%s/vma-lock-XXXXXX", directory) >= sizeof(path))
		return -1;
	backing_fd = mkstemp(path);
	if (backing_fd < 0)
		return -1;
	unlink(path);
	if (fstatfs(backing_fd, &filesystem) || filesystem.f_type == TMPFS_MAGIC) {
		close(backing_fd);
		errno = EOPNOTSUPP;
		return -1;
	}
	buffer = malloc(length);
	if (!buffer) {
		close(backing_fd);
		return -1;
	}
	memset(buffer, PATTERN, length);
	offset = 0;
	while (offset < length) {
		written = write(backing_fd, buffer + offset, length - offset);
		if (written < 0 && errno == EINTR)
			continue;
		if (written <= 0)
			break;
		offset += written;
	}
	free(buffer);
	if (offset != length || fsync(backing_fd)) {
		close(backing_fd);
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	const char *directory = getenv("TMPDIR");
	static const char * const names[] = {
		"file faults concurrent with VMA split, mprotect and merge",
		"file faults concurrent with munmap and MAP_FIXED",
		"file faults concurrent with page-table relocation",
		"parent faults concurrent with fork and child COW",
		"signals during repeated file faults",
		"anonymous swap-out and refault data integrity",
	};
	unsigned long value;
	unsigned int test;
	char *end;
	pid_t pid;
	int option, result;
	long size = sysconf(_SC_PAGESIZE);

	if (size <= 0)
		return KSFT_FAIL;
	page_size = size;
	length = page_size * PAGES;
	if (!directory)
		directory = "/tmp";
	while ((option = getopt(argc, argv, "d:n:h")) != -1) {
		switch (option) {
		case 'd':
			directory = optarg;
			break;
		case 'n':
			errno = 0;
			value = strtoul(optarg, &end, 10);
			if (errno || !*optarg || *end || !value || value > 10000)
				return KSFT_FAIL;
			rounds = value;
			break;
		default:
			printf("Usage: %s [-d regular-filesystem-directory] [-n rounds]\n",
			       argv[0]);
			return option == 'h' ? KSFT_PASS : KSFT_FAIL;
		}
	}
	if (optind != argc)
		return KSFT_FAIL;
	ksft_print_header();
	ksft_set_plan(ARRAY_SIZE(names));
	if (create_backing(directory))
		ksft_exit_skip("cannot create backing file in %s: %s\n",
			       directory, strerror(errno));
	for (test = 0; test < ARRAY_SIZE(names); test++) {
		/* A deadlocked fault path fails the case instead of hanging the suite. */
		pid = fork();
		if (pid == 0) {
			alarm(60);
			_exit(test <= INTERRUPT ? run_concurrent(test) : run_swap());
		}
		result = pid < 0 ? KSFT_FAIL : child_status(pid);
		if (result == KSFT_SKIP)
			ksft_test_result_skip("%s (no pages could be swapped out)\n", names[test]);
		else
			ksft_test_result(result == KSFT_PASS, "%s\n", names[test]);
	}
	close(backing_fd);
	ksft_exit(ksft_cnt.ksft_fail == 0);
}
