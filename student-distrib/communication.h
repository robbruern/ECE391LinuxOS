/* macro used to read a byte from a port */
#define IN_BYTE(port,val)                                                  \
do {                                                                   \
    asm volatile ("                                                    \
      movl $0, %%eax                                                  ;\
      movw %w1, %%dx                                                  ;\
      inb (%%dx),%%al                                                 ;\
      movl %%eax, %0                                                   \
     ": "=r" (val)                                                     \
      : "r" (port)                                                     \
      : "memory", "cc", "%eax","edx");                                 \
} while (0)

/* macro used to read a word from a port */
#define IN_WORD(port,val)                                                  \
do {                                                                   \
    asm volatile ("                                                    \
      movl $0, %%eax                                                  ;\
      movw %w1, %%dx                                                  ;\
      inw (%%dx),%%al                                                 ;\
      movl %%eax, %0                                                   \
     ": "=r" (val)                                                     \
      : "r" (port)                                                     \
      : "memory", "cc", "%eax","edx");                                 \
} while (0)

/* macro used to write a word to a port */
#define OUT_WORD(port,val)                                                  \
do {                                                                    \
    asm volatile ("                                                     \
        outb %w1,(%w0)                                                  \
    " : /* no outputs */                                                \
      : "d" ((port)), "a" ((val))                                       \
      : "memory", "cc");                                                \
} while (0)

/* macro used to write a byte to a port */
#define OUT_BYTE(port,val)                                                  \
do {                                                                    \
    asm volatile ("                                                     \
        outb %b1,(%w0)                                                  \
    " : /* no outputs */                                                \
      : "d" ((port)), "a" ((val))                                       \
      : "memory", "cc");                                                \
} while (0)

