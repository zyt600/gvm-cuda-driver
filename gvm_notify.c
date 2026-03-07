#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "gvm_notify.h"
#include "utils.h"

static gvm_notice_fn g_handler;
static volatile bool g_active;
static pthread_t     g_notify_thread;

static void *notify_thread_fn(void *arg) {
	(void)arg;
	UVM_WAIT_NOTICE_PARAMS params;

	while (!try_init_uvmfd()) {
		printf("Waiting for uvmfd...\n");
		sleep(1);
	}

	while (g_active) {
		memset(&params, 0, sizeof(params));
		if (ioctl(g_uvmfd, UVM_WAIT_NOTICE, &params) != 0) {
			if (errno == EINTR)
				continue;
			break;
		}
		g_handler(&params);
	}
	return NULL;
}

int gvm_register_notify(gvm_notice_fn handler) {
	g_handler = handler;
	g_active = true;

	if (pthread_create(&g_notify_thread, NULL, notify_thread_fn, NULL) != 0) {
		g_active = false;
		perror("gvm_register_notify: thread create failed");
		return -1;
	}

	return 0;
}

void gvm_unregister_notify(void) {
	g_active = false;
	pthread_cancel(g_notify_thread);
	pthread_join(g_notify_thread, NULL);
}
