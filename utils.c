#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <limits.h>

#include <cuda.h>

#include "utils.h"

int g_uvmfd = -1;

int find_initialized_uvm(CUuuid uuid)
{
	pid_t pid = getpid();
	char fd_dir[64];
	const char *target_path = "/dev/nvidia-uvm";
	int ret = -1;

	snprintf(fd_dir, sizeof(fd_dir), "/proc/%d/fd", (int)pid);

	DIR *dir = opendir(fd_dir);
	if (!dir) {
		fprintf(stderr, "Failed to open %s: %s\n", fd_dir, strerror(errno));
		return ret;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		char link_path[PATH_MAX];
		char fd_path[PATH_MAX];
		ssize_t len;
		int fd;

		if (entry->d_name[0] == '.' &&
			(entry->d_name[1] == '\0' ||
			 (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
			continue;
		}

		snprintf(link_path, sizeof(link_path), "%s/%s", fd_dir, entry->d_name);

		len = readlink(link_path, fd_path, sizeof(fd_path) - 1);
		if (len < 0)
			continue;
		fd_path[len] = '\0';

		if (strcmp(fd_path, target_path) != 0)
			continue;

		fd = atoi(entry->d_name);
		if (fd < 0) {
			fprintf(stderr, "Invalid file descriptor: %d\n", fd);
			continue;
		}

		UVM_IS_INITIALIZED_PARAMS params = {
			.uuid = uuid,
			.initialized = 0,
			.rmStatus = 0
		};

		if (ioctl(fd, UVM_IS_INITIALIZED, &params) == 0 && params.rmStatus == 0) {
			if (params.initialized) {
				ret = fd;
				break;
			}
		} else {
			fprintf(stderr, "Failed to check UVM initialization on fd %d: %s\n",
					fd, strerror(errno));
		}
	}

	closedir(dir);
	return ret;
}

int update_event_count(int uvmfd, CUuuid uuid, UVM_EVENT_TYPE type, UVM_UPDATE_EVENT_COUNT_TYPE op, uint64_t value)
{
	int ret = 0;

	UVM_UPDATE_EVENT_COUNT_PARAMS params = {
		.uuid = uuid,
		.type = type,
		.op = op,
		.value = value,
		.rmStatus = 0
	};

	if ((ret = ioctl(uvmfd, UVM_UPDATE_EVENT_COUNT, &params)) != 0)
		return ret;

	if (params.rmStatus != 0)
		fprintf(stderr, "uvm_ioctl: error code %d\n", params.rmStatus);

	return 0;
}

size_t gettime_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (size_t)(ts.tv_sec) * 1000 + (ts.tv_nsec) / 1000000;
}

bool try_init_uvmfd(void)
{
	if (g_uvmfd >= 0)
		return true;

	CUdevice device;
	CUuuid uuid;
	CUresult ret;

	ret = cuCtxGetDevice_IMPL(&device);
	if (ret != CUDA_SUCCESS) {
		fprintf(stderr, "cuCtxGetDevice: error code %d\n", ret);
		return false;
	}
	ret = cuDeviceGetUuid_IMPL(&uuid, device);
	if (ret != CUDA_SUCCESS) {
		fprintf(stderr, "cuDeviceGetUuid: error code %d\n", ret);
		return false;
	}
	g_uvmfd = find_initialized_uvm(uuid);
	printf("Find uvmfd at %d\n", g_uvmfd);
	return true;
}
