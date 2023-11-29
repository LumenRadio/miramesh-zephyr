#include "miramesh-integration-memory.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <miramesh.h>

LOG_MODULE_DECLARE(miramesh_integration, CONFIG_MIRAMESH_LOG_LEVEL);

static void* miramesh_integration_memory_malloc(mira_size_t size, void* storage)
{
    (void)storage;
    void* buffer = k_malloc(size);
    if (buffer == NULL) {
        LOG_ERR("Memory allocation failed");
        while (1)
            ;
    }

    return buffer;
}

void miramesh_integration_memory_init(void)
{
    /* Allocate heap for mira on the same heap as zephyr uses */
    mira_mem_set_alloc_callback(miramesh_integration_memory_malloc, NULL);
}