#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <cuda.h>

#define UVM_IS_INITIALIZED 80
typedef struct
{
	CUuuid					uuid;			// IN
	bool					initialized;    // OUT
	int						rmStatus;		// OUT
} UVM_IS_INITIALIZED_PARAMS;

typedef enum
{
	UVM_SUBMIT_KERNEL_EVENT = 0,
	UVM_END_KERNEL_EVENT,

	UVM_EVENT_TYPE_COUNT
} UVM_EVENT_TYPE;

typedef enum {
	UVM_ADD_EVENT_COUNT = 0,
	UVM_SET_EVENT_COUNT,

	UVM_UPDATE_EVENT_COUNT_TYPE_COUNT
} UVM_UPDATE_EVENT_COUNT_TYPE;

#define UVM_UPDATE_EVENT_COUNT 81
typedef struct
{
	CUuuid uuid;
	UVM_EVENT_TYPE type;
	UVM_UPDATE_EVENT_COUNT_TYPE op;
	uint64_t value;

	int rmStatus;
} UVM_UPDATE_EVENT_COUNT_PARAMS;

int find_initialized_uvm(CUuuid uuid);
int update_event_count(int uvmfd, CUuuid uuid, UVM_EVENT_TYPE type, UVM_UPDATE_EVENT_COUNT_TYPE op, uint64_t value);
size_t gettime_ms(void);

extern int g_uvmfd;
bool try_init_uvmfd(void);

#endif
