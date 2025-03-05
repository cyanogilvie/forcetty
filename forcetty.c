#define _POSIX_C_SOURCE 200908L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>
#include <stdatomic.h>
#include <errno.h>
#include <pty.h>
#include <sys/types.h>
#include <sys/wait.h>

#define bufsize 4096
static uint8_t		g_buf[bufsize];
static atomic_int	g_running = 1;
static atomic_int	g_exitstatus = EXIT_SUCCESS;
static pid_t		child_pid;

static void child_handler(int sig, siginfo_t*const restrict info, [[maybe_unused]] void*const ucontextPtr)
{
	//struct ucontext_t*const restrict	ucontext = (struct ucontext_t*)ucontextPtr;
	switch (info->si_code) {
		case CLD_EXITED:	g_exitstatus = info->si_status;		g_running = 0;	break;
		case CLD_KILLED:	fprintf(stderr, "Child killed\n");	g_running = 0;	break;
		case CLD_DUMPED:	fprintf(stderr, "Child dumped\n");	g_running = 0;	break;
		default:																break;
	}
}

int main(int argc, char* argv[argc+1])
{
	int				master_fd;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s program ?arg ...?\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = child_handler;
	if (-1 == sigaction(SIGCHLD, &sa, NULL)) {
		perror("sigaction SIGCHLD");
		return EXIT_FAILURE;
	}

	const int		rc = forkpty(&master_fd, NULL, NULL, NULL);
	switch (rc) {
		case -1:
			perror("forkpty");
			return EXIT_FAILURE;

		case 0:
			// child
			if (-1 == execvp(argv[1], argv+1)) {
				perror("execvp");
				exit(EXIT_FAILURE);
			}

		default:
			// parent: rc is the child pid
			child_pid = rc;
			break;
	}

	for (;g_running;) {
		const ssize_t	got = read(master_fd, g_buf, bufsize);
		if (got == -1) {
			switch (errno) {
				case EINTR:	continue;
				case EIO:	if (!g_running) break;
				default:
					perror("read");
					exit(EXIT_FAILURE);
			}
		} else {
			uint8_t*	buf = g_buf;
			ssize_t		remain = got;
			for (;remain;) {
				const ssize_t wrote = write(1, buf, remain);
				if (wrote == -1) {
					if (errno == EINTR) continue;
					perror("write");
					exit(EXIT_FAILURE);
				}
				remain -= wrote;
				buf += wrote;
			}
		}
	}

	// Reap child
	for (;;) {
		int		wstatus;
		if (-1 == waitpid(child_pid, &wstatus, 0)) {
			if (errno == EINTR) continue;
			perror("waitpid");
			return EXIT_FAILURE;
		}
		break;
	}

	return g_exitstatus;
}
