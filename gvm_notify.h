#ifndef GVM_NOTIFY_H
#define GVM_NOTIFY_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#include <cuda.h>

typedef enum
{
	GVM_NOTICE_EVICTION     = 0,
	GVM_NOTICE_AVAILABILITY = 1,
} gvm_notice_type_t;

#define UVM_WAIT_NOTICE 82
typedef struct
{
	CUuuid            uuid;
	gvm_notice_type_t type;
	union {
		struct {
			uint64_t target_memory;
			uint64_t current_memory;
		} eviction;
		struct {
			uint64_t available_memory;
		} availability;
	};
	int rmStatus;
} UVM_WAIT_NOTICE_PARAMS;

typedef void (*gvm_notice_fn)(const UVM_WAIT_NOTICE_PARAMS *params);

int  gvm_register_notify(gvm_notice_fn handler);
void gvm_unregister_notify(void);

#endif
