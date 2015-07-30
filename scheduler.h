#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
#define PRIV_TEST
extern unsigned uartlock;
extern void lock_init(unsigned *lock);
extern unsigned lock_acquire(unsigned *lock);
extern void lock_release(unsigned *lock);

extern int get_currThread(void);
extern void yield(void);
extern unsigned lock(unsigned *threadlock);
extern void unlock(unsigned *threadlock);
extern void unlock_force(unsigned *threadlock);

extern void reg_save(int *state);
extern void reg_restore(int *state);
#endif // _SCHEDULER_H_
