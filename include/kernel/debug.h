#ifndef __DEBUG_H__
#define __DEBUG_H__

void print_context(unsigned int *regs);
void print_kernel_status(unsigned int *sp, unsigned int lr, unsigned int psr);
void print_user_status(unsigned int *sp);
void print_task_status(struct task *task);

#endif /* __DEBUG_H__ */
