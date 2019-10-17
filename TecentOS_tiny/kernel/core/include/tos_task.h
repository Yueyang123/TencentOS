#ifndef _TOS_TASK_H_
#define  _TOS_TASK_H_

// task state is just a flag, indicating which manager list we are in.
//TOS�е�����״̬
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
 * ������ƿ�
 */
typedef struct k_task_st {
    k_stack_t          *sp;                 /**< ����ջָ��,�����л�������*/

#if TOS_CFG_OBJECT_VERIFY_EN > 0u
    knl_obj_t           knl_obj;            /**< ֻ��Ϊ����֤�����Ե�ǰ�����Ƿ������һ������*/
#endif

    char               *name;               /**< �������� */
    k_task_entry_t      entry;              /**< �������� */
    void               *arg;                /**< ���������β� */
    k_task_state_t      state;              /**< ����״̬ */
    k_prio_t            prio;               /**< �������ȼ� */

    k_stack_t          *stk_base;           /**< ����ջ����ַ */
    size_t              stk_size;           /**< ����ջ��С */

    k_tick_t            tick_expires;       /**< ����������ʱ�� */

    k_list_t            tick_list;          /**< ��ʱ�б� */
    k_list_t            pend_list;          /**< �������ȴ��б� */
//���ʹ���˻�����
#if TOS_CFG_MUTEX_EN > 0u
    k_list_t            mutex_own_list;     /**< ����ӵ�еĻ����� */
    k_prio_t            prio_pending;       /*< ���ڼ�¼���л������������ʼ���ȼ��������ȼ��̳���ʹ�� */
#endif

    pend_obj_t         *pending_obj;       /**< ��¼�����ʱ���ص����б� */
    pend_state_t        pend_state;         /**< �ȴ������ѵ�ԭ��״̬�� */
//���ʹ����ʱ��Ƭ�����㷨
#if TOS_CFG_ROUND_ROBIN_EN > 0u
    k_timeslice_t       timeslice_reload;   /**< ʱ��Ƭ��ʼֵ����װ��ֵ�� */
    k_timeslice_t       timeslice;          /**< ʣ��ʱ��Ƭ */
#endif
//���ʹ������Ϣ
#if TOS_CFG_MSG_EN > 0u
    void               *msg_addr;           /**< ������յ�����Ϣ */
    size_t              msg_size;			/**< ������յ�����Ϣ��С */
#endif
//���ʹ�����¼�
#if TOS_CFG_EVENT_EN > 0u
    k_opt_t             opt_event_pend;     /**< �ȴ��¼��ĵĲ������ͣ�TOS_OPT_EVENT_PEND_ANY �� TOS_OPT_EVENT_PEND_ALL */
    k_event_flag_t      flag_expect;        /**< �ڴ��������¼� */
    k_event_flag_t     *flag_match;         /**< �ȴ������¼� */
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
//���񴴽�����
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
 *ɾ��������
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
 *��������
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
 *ȡ����������
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
 *����������
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
 *�ָ�������
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
 *�ı��������ȼ�
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

//����״̬�жϺ���
//ͨ����tast_>stateȡ����

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

//�ı�����״̬����

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

