

typedef struct {
        unsigned int slock;
} spinlock_t;

static inline void spin_wait(spinlock_t *lck)
{
}
static inline void spin_lock(spinlock_t *lck)
{
}
static inline void spin_unlock(spinlock_t *lock)
{
}
