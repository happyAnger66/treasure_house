#include "coroutine.h"
#include "wait.h"
#include "signal.h"
#include "co_errno.h"

void init_wait_entry(wait_queue_t *wait, int flags)
{
    wait->flags = flags;
    wait->private = coroutine_current();
    wait->func = autoremove_wake_function;
    INIT_LIST_HEAD(&wait->task_list);
}

long prepare_to_wait_event(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry, int state)
{
    long ret = 0
    co_spin_lock(&wq_head->spin_lock);

    if (signal_pending_state(state, coroutine_current()))
    {
        list_del_init(&wq_entry->spin_lock);
        ret = ERESTARTSYS;
    }
    else
    {
        if (list_empty(&wq_entry->task_list))
        {
            if (wq_entry->flags & WQ_FLAG_EXCLUSIVE)
                list_add_tail(&wq_head->task_list, &wq_entry->task_list);
            else
                list_add(&wq_head->task_list, &wq_entry->task_list);
        }
        coroutine_set_current_state(state);
    }

    co_spin_unlock(&wq_head->spin_lock);
    return ret;
}

void prepare_to_wait(wait_queue_head_t *q, wait_queue_t *wait, int state)
{
    co_spin_lock(&q->spin_lock);
    if (list_empty(&wait->task_list))
        _add_wait_queue(q, wait);

    coroutine_set_current_state(state);
    co_spin_unlock(&q->spin_lock);
}

void finish_wait(wait_queue_head_t *q, wait_queue_t *wait)
{
    co_spin_lock(&q->lock);
    if (!list_empty(&wait->task_list))
        list_del(&wait->task_list);
    co_spin_unlock(&q->lock);
}

static void __wake_up_common_lock(struct wait_queue_head *wq_head, unsigned int mode,
            int nr_exclusive, int wake_flags, void *key)
{
    unsigned long flags;
    co_spin_lock(&wq_head->lock);
    nr_exclusive = __wake_up_common(wq_head, mode, nr_exclusive, wake_flags, key);
    co_spin_unlock(&wq_head->lock);
}

void __wake_up(struct wait_queue_head *wq_head, unsigned int mode,
    int nr_exclusive, void *key)
{
    __wake_up_common_lock(wq_head, mode, nr_exclusive, 0, key);
}