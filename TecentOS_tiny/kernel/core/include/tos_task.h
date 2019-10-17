#ifndef _TOS_TASK_H_
#define  _TOS_TASK_H_

// task state is just a flag, indicating which manager list we are in.
//TOS中的任务状态
// ready to schedule
// a task's pend_list is in readyqueue
#define K_TASK_STATE_READY                (k_task_state_t)0x0000

// delayed, or pend for a timeout
// a task's tick_list is in k_tick_list
#define K_TASK_STATE_SLEEP                (k_task_state_t)0x0001

// pend for something
// a task's pend_list is in some pend object's list
#define K_TASK_STATE_PEND                 (k_task_state_t)0x0002

// suspended
#define K_TASK_STATE_SUSPENDED            (k_task_state_t)0x0004

// deleted
#define K_TASK_STATE_DELETED              (k_task_state_t)0x0008

// actually we don't really need those TASK_STATE below, if you understand the task state deeply, the code can be much more elegant.

// we are pending, also we are waitting for a timeout(eg. tos_sem_pend with a valid timeout, not TOS_TIME_FOREVER)
// both a task's tick_list and pend_list is not empty
#define K_TASK_STATE_PENDTIMEOUT                      (k_task_state_t)(K_TASK_STATE_PEND | K_TASK_STATE_SLEEP)

// suspended when sleeping
#define K_TASK_STATE_SLEEP_SUSPENDED                  (k_task_state_t)(K_TASK_STATE_SLEEP | K_TASK_STATE_SUSPENDED)

// suspened when pending
#define K_TASK_STATE_PEND_SUSPENDED                   (k_task_state_t)(K_TASK_STATE_PEND | K_TASK_STATE_SUSPENDED)

// suspended when pendtimeout
#define K_TASK_STATE_PENDTIMEOUT_SUSPENDED            (k_task_state_t)(K_TASK_STATE_PENDTIMEOUT | K_TASK_STATE_SUSPENDED)


// if you configure TOS_CFG_TASK_PRIO_MAX as 10, means the priority for kernel is (0 ... 9]
// the priority 9(TOS_CFG_TASK_PRIO_MAX - 1) is only for idle, so avaliable priority for you is (0 ... 8]
#define K_TASK_PRIO_IDLE                                (k_prio_t)(TOS_CFG_TASK_PRIO_MAX - (k_prio_t)1u)
#define K_TASK_PRIO_INVALID                             (k_prio_t)(TOS_CFG_TASK_PRIO_MAX)

typedef void (*k_task_entry_t)(void *arg);

/**
 * task control block
 * 任务控制块
 */
typedef struct k_task_st {
    k_stack_t          *sp;                 /**< 任务栈指针,用于切换上下文*/

#if TOS_CFG_OBJECT_VERIFY_EN > 0u
    knl_obj_t           knl_obj;            /**< 只是为了验证，测试当前对象是否真的是一项任务。*/
#endif

    char               *name;               /**< 任务名称 */
    k_task_entry_t      entry;              /**< 任务主体 */
    void               *arg;                /**< 任务主体形参 */
    k_task_state_t      state;              /**< 任务状态 */
    k_prio_t            prio;               /**< 任务优先级 */

    k_stack_t          *stk_base;           /**< 任务栈基地址 */
    size_t              stk_size;           /**< 任务栈大小 */

    k_tick_t            tick_expires;       /**< 任务阻塞的时间 */

    k_list_t            tick_list;          /**< 延时列表 */
    k_list_t            pend_list;          /**< 就绪、等待列表 */
//如果使能了互斥量
#if TOS_CFG_MUTEX_EN > 0u
    k_list_t            mutex_own_list;     /**< 任务拥有的互斥量 */
    k_prio_t            prio_pending;       /*< 用于记录持有互斥量的任务初始优先级，在优先级继承中使用 */
#endif

    pend_obj_t         *pending_obj;       /**< 记录任务此时挂载到的列表 */
    pend_state_t        pend_state;         /**< 等待被唤醒的原因（状态） */
//如果使能了时间片调度算法
#if TOS_CFG_ROUND_ROBIN_EN > 0u
    k_timeslice_t       timeslice_reload;   /**< 时间片初始值（重装载值） */
    k_timeslice_t       timeslice;          /**< 剩余时间片 */
#endif
//如果使能了消息
#if TOS_CFG_MSG_EN > 0u
    void               *msg_addr;           /**< 保存接收到的消息 */
    size_t              msg_size;			/**< 保存接收到的消息大小 */
#endif
//如果使能了事件
#if TOS_CFG_EVENT_EN > 0u
    k_opt_t             opt_event_pend;     /**< 等待事件的的操作类型：TOS_OPT_EVENT_PEND_ANY 、 TOS_OPT_EVENT_PEND_ALL */
    k_event_flag_t      flag_expect;        /**< 期待发生的事件 */
    k_event_flag_t     *flag_match;         /**< 等待到的事件 */
#endif
} k_task_t;

/**
 * @brief Create a task.
 * create a task.
 *
 * @attention None
 *
 * @param[in]   task        pointer to the handler of the task.
 * @param[in]   name        name of the task.
 * @param[in]   entry       running entry of the task.
 * @param[in]   arg         argument for the entry of the task.
 * @param[in]   prio        priority of the task.
 * @param[in]   stk_base    stack base address of the task.
 * @param[in]   stk_size    stack size of the task.
 * @param[in]   timeslice   time slice of the task.
 * @param[in]   opt         option for the function call.
 *
 * @return  errcode
 * @retval  #K_ERR_TASK_STK_SIZE_INVALID    stack size is invalid.
 * @retval  #K_ERR_TASK_PRIO_INVALID        priority is invalid.
 * @retval  #K_ERR_NONE                     return successfully.
 */
//任务创建函数
__API__ k_err_t tos_task_create(k_task_t *task,
                                            char *name,
                                            k_task_entry_t entry,
                                            void *arg,
                                            k_prio_t prio,
                                            k_stack_t *stk_base,
                                            size_t stk_size,
                                            k_timeslice_t timeslice);

/**
 * @brief Destroy a task.
 * delete a task.
 *删除任务函数
 * @attention None
 *
 * @param[in]   task        pointer to the handler of the task to be deleted.
 *
 * @return  errcode
 * @retval  #K_ERR_TASK_DESTROY_IDLE    attempt to destroy idle task.
 * @retval  #K_ERR_NONE                 return successfully.
 */
__API__ k_err_t tos_task_destroy(k_task_t *task);

/**
 * @brief Delay current task for ticks.
 * Delay for a specified amount of ticks.
 *延期任务
 * @attention None
 *
 * @param[in]   delay       amount of ticks to delay.
 *
 * @return  errcode
 * @retval  #K_ERR_DELAY_ZERO     delay is zero.
 * @retval  #K_ERR_NONE           return successfully.
 */
__API__ k_err_t tos_task_delay(k_tick_t delay);

/**
 * @brief Resume task from delay.
 * Resume a delayed task from delay.
 *取消延期任务
 * @attention None
 *
 * @param[in]   task        the pointer to the handler of the task.
 *
 * @return  errcode
 * @retval  #K_ERR_TASK_NOT_DELAY         task is not delayed.
 * @retval  #K_ERR_TASK_SUSPENDED         task is suspended.
 * @retval  #K_ERR_NONE                   return successfully.
 */
__API__ k_err_t tos_task_delay_abort(k_task_t *task);

/**
 * @brief Suspend a task.
 * Bring a task to sleep.
 *挂起任务函数
 * @attention None
 *
 * @param[in]   task        pointer to the handler of the task to be resume.
 *
 * @return  errcode
 * @retval  #K_ERR_TASK_SUSPEND_IDLE  attempt to suspend idle task.
 * @retval  #K_ERR_NONE               return successfully.
 */
__API__ k_err_t tos_task_suspend(k_task_t *task);

/**
 * @brief Resume a task.
 * Bring a task to run.
 *恢复任务函数
 * @attention None
 *
 * @param[in]   task        pointer to the handler of the task to be resume.
 *
 * @return  errcode
 * @retval  #K_ERR_TASK_RESUME_SELF   attempt to resume self-task.
 * @retval  #K_ERR_NONE               return successfully.
 */
__API__ k_err_t tos_task_resume(k_task_t *task);

/**
 * @brief Change task priority.
 * Change a priority of the task.
 *改变任务优先级
 * @attention None
 *
 * @param[in]   task        pointer to the handler of the task to be resume.
 * @param[in]   prio_new    new priority.
 *
 * @return  errcode
 * @retval  #K_ERR_TASK_PRIO_INVALID  new priority is invalid.
 * @retval  #K_ERR_NONE               return successfully.
 */
__API__ k_err_t tos_task_prio_change(k_task_t *task, k_prio_t prio_new);

/**
 * @brief Quit schedule this time.
 * Quit the cpu this time.
 *
 * @attention None
 *
 * @param   None
 *
 * @return  None
 */
__API__ void    tos_task_yield(void);


#if TOS_CFG_TASK_STACK_DRAUGHT_DEPTH_DETACT_EN > 0u

/**
 * @brief Get the maximum stack draught depth of a task.
 *
 * @attention None
 *
 * @param[in]   task        pointer to the handler of the task.
 * @param[out]  depth       task stack draught depth.
 *
 * @return  errcode
 * @retval  #K_ERR_NONE                 get depth successfully.
 * @retval  #K_ERR_TASK_STK_OVERFLOW    task stack is overflow.
 */
__API__ k_err_t tos_task_stack_draught_depth(k_task_t *task, int *depth);

#endif

//任务状态判断函数
//通过与tast_>state取与获得

__KERNEL__ __STATIC_INLINE__ int task_state_is_ready(k_task_t *task)
{
    return task->state == K_TASK_STATE_READY;
}

__KERNEL__ __STATIC_INLINE__ int task_state_is_sleeping(k_task_t *task)
{
    return task->state & K_TASK_STATE_SLEEP;
}

__KERNEL__ __STATIC_INLINE__ int task_state_is_pending(k_task_t *task)
{
    return task->state & K_TASK_STATE_PEND;
}

__KERNEL__ __STATIC_INLINE__ int task_state_is_suspended(k_task_t *task)
{
    return task->state & K_TASK_STATE_SUSPENDED;
}

//改变任务状态函数

__KERNEL__ __STATIC_INLINE__ void task_state_reset_pending(k_task_t *task)
{
    task->state &= ~K_TASK_STATE_PEND;
}

__KERNEL__ __STATIC_INLINE__ void task_state_reset_sleeping(k_task_t *task)
{
    task->state &= ~K_TASK_STATE_SLEEP;
}

__KERNEL__ __STATIC_INLINE__ void task_state_reset_suspended(k_task_t *task)
{
    task->state &= ~K_TASK_STATE_SUSPENDED;
}

__KERNEL__ __STATIC_INLINE__ void task_state_set_suspended(k_task_t *task)
{
    task->state |= K_TASK_STATE_SUSPENDED;
}

__KERNEL__ __STATIC_INLINE__ void task_state_set_pend(k_task_t *task)
{
    task->state |= K_TASK_STATE_PEND;
}

__KERNEL__ __STATIC_INLINE__ void task_state_set_ready(k_task_t *task)
{
    task->state = K_TASK_STATE_READY;
}

__KERNEL__ __STATIC_INLINE__ void task_state_set_deleted(k_task_t *task)
{
    task->state = K_TASK_STATE_DELETED;
}

__KERNEL__ __STATIC_INLINE__ void task_state_set_sleeping(k_task_t *task)
{
    task->state |= K_TASK_STATE_SLEEP;
}

#endif /* _TOS_TASK_H_ */

