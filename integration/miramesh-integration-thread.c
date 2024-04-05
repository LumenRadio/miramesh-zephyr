#include <miramesh.h>
#include <zephyr/kernel.h>

#include <zephyr/random/rand32.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel/thread.h>
#include <zephyr/logging/log.h>

#include "miramesh-integration-thread.h"
#include "swi-callback-handler.h"
#define MIRAMESH_INTEGRATION_NOTIFY_EVENT 0xFFFF

LOG_MODULE_DECLARE(miramesh_integration, CONFIG_MIRAMESH_LOG_LEVEL);
K_MUTEX_DEFINE(miramesh_integration_api_lock);

extern const k_tid_t miramesh_integration_thread_id;

static K_SEM_DEFINE(miramesh_integration_sem, 1, 1);

static uint16_t random_number_get(void)
{
    union
    {
        uint16_t u16;
        uint8_t u8[2];
    } buf;
    sys_csrand_get(buf.u8, 2);
    return buf.u16;
}

static void miramesh_integration_lock()
{
    k_mutex_lock(&miramesh_integration_api_lock, K_FOREVER);
}

static void miramesh_integration_unlock()
{
    if (k_mutex_unlock(&miramesh_integration_api_lock) != 0) {
        LOG_ERR("Illegal mutex operation");
    }
}

static void miramesh_integration_notification_callback(void)
{
    k_sem_give(&miramesh_integration_sem);
}

static void miramesh_integration_wakeup_from_irq(void)
{
    invoke_swi_callback(miramesh_integration_notification_callback);
}

static void miramesh_integration_wakeup_from_app(void)
{
    k_sem_give(&miramesh_integration_sem);
}

static void miramesh_integration_thread(void)
{
    do {
        k_sem_take(&miramesh_integration_sem, K_FOREVER);
        /* Not really necessary as the thread is already an cooperative thread */
        k_sched_lock();
        miramesh_run_once();
        k_sched_unlock();
    } while (true);
}

K_THREAD_DEFINE(miramesh_integration_thread_id,
                CONFIG_MIRAMESH_STACK_SIZE,
                miramesh_integration_thread,
                NULL,
                NULL,
                NULL,
                CONFIG_MIRAMESH_THREAD_PRIO,
                0,
                0);

void miramesh_integration_thread_init(miramesh_config_t* config)
{
    k_mutex_init(&miramesh_integration_api_lock);
    config->callback.api_lock = miramesh_integration_lock;
    config->callback.api_unlock = miramesh_integration_unlock;
    config->callback.wakeup_from_irq_callback = miramesh_integration_wakeup_from_irq;
    config->callback.wakeup_from_app_callback = miramesh_integration_wakeup_from_app;
    config->rng.rng_gen_u16 = random_number_get;
    int ret = register_swi_callback(miramesh_integration_notification_callback);
    __ASSERT(ret >= 0, "Failed to register SWI callback");
}
