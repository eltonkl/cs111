#include "schedos-kern.h"
#include "x86.h"
#include "lib.h"

/*****************************************************************************
 * schedos-kern
 *
 *   This is the schedos's kernel.
 *   It sets up process descriptors for the 4 applications, then runs
 *   them in some schedule.
 *
 *****************************************************************************/

// The program loader loads 4 processes, starting at PROC1_START, allocating
// 1 MB to each process.
// Each process's stack grows down from the top of its memory space.
// (But note that SchedOS processes, like MiniprocOS processes, are not fully
// isolated: any process could modify any part of memory.)

#define NPROCS		5
#define PROC1_START	0x200000
#define PROC_SIZE	0x100000

// +---------+-----------------------+--------+---------------------+---------/
// | Base    | Kernel         Kernel | Shared | App 0         App 0 | App 1
// | Memory  | Code + Data     Stack | Data   | Code + Data   Stack | Code ...
// +---------+-----------------------+--------+---------------------+---------/
// 0x0    0x100000               0x198000 0x200000              0x300000
//
// The program loader puts each application's starting instruction pointer
// at the very top of its stack.
//
// System-wide global variables shared among the kernel and the four
// applications are stored in memory from 0x198000 to 0x200000.  Currently
// there is just one variable there, 'cursorpos', which occupies the four
// bytes of memory 0x198000-0x198003.  You can add more variables by defining
// their addresses in schedos-symbols.ld; make sure they do not overlap!


// A process descriptor for each process.
// Note that proc_array[0] is never used.
// The first application process descriptor is proc_array[1].
static process_t proc_array[NPROCS];

// A pointer to the currently running process.
// This is kept up to date by the run() function, in mpos-x86.c.
process_t *current;

// The preferred scheduling algorithm.
int scheduling_algorithm;

// UNCOMMENT THESE LINES IF YOU DO EXERCISE 4.A
// Use these #defines to initialize your implementation.
// Changing one of these lines should change the initialization.
#define __PRIORITY_1__ 254
#define __PRIORITY_2__ 254
#define __PRIORITY_3__ 255
#define __PRIORITY_4__ 0

uint8_t current_priority;

// UNCOMMENT THESE LINES IF YOU DO EXERCISE 4.B
// Use these #defines to initialize your implementation.
// Changing one of these lines should change the initialization.
#define __SHARE_1__ 1
#define __SHARE_2__ 2
#define __SHARE_3__ 3
#define __SHARE_4__ 4

uint32_t used_shares;
uint32_t max_shares;

// Exercise 7 variables
// xorshift: credit to George Marsaglia
// http://www.jstatsoft.org/v08/i14/paper
uint32_t x, y, z, w;
uint32_t total_tickets;

// USE THESE VALUES FOR SETTING THE scheduling_algorithm VARIABLE.
#define __EXERCISE_1__   0  // the initial algorithm
#define __EXERCISE_2__   1  // strict priority scheduling (exercise 2)
#define __EXERCISE_4A__  2  // p_priority algorithm (exercise 4.a)
#define __EXERCISE_4B__  3  // p_share algorithm (exercise 4.b)
#define __EXERCISE_7__   4  // any algorithm for exercise 7


/*****************************************************************************
 * start
 *
 *   Initialize the hardware and process descriptors, then run
 *   the first process.
 *
 *****************************************************************************/

static inline uint32_t rdtsc()
{
    uint32_t low_bits;
    asm volatile ( "rdtsc\n"
                   : "=a" (low_bits)
                   : 
                   : "%edx", "memory" );
    return low_bits;
}

void
start(void)
{
	int i;

	// Set up hardware (schedos-x86.c)
	segments_init();
	interrupt_controller_init(1);
	console_clear();

	// Initialize process descriptors as empty
	memset(proc_array, 0, sizeof(proc_array));
	for (i = 0; i < NPROCS; i++) {
		proc_array[i].p_pid = i;
		proc_array[i].p_state = P_EMPTY;
	}

        // Initialization for lottery scheduling algorithm
        x = rdtsc();
        y = 312346069;
        // Since bochs is deterministic, make initial values non-deterministic
        // at compile time
        z = (__TIME__[0] + __TIME__[1] + __TIME__[2] + __TIME__[3] + __TIME__[4]
                + (__TIME__[5] * __TIME__[6]))
            * __TIME__[7] * __TIME__[0] * __TIME__[2]
            * __TIME__[5] * __TIME__[4] - (__TIME__[7] % __TIME__[0]);
        w = (98432998 * (uint32_t)__TIMESTAMP__ - (uint32_t)__TIMESTAMP__
            * __TIME__[0]) ^ (uint32_t)__TIME__[7];
        total_tickets = 0;

	// Set up process descriptors (the proc_array[])
	for (i = 1; i < NPROCS; i++) {
		process_t *proc = &proc_array[i];
		uint32_t stack_ptr = PROC1_START + i * PROC_SIZE;

		// Initialize the process descriptor
		special_registers_init(proc);

		// Set ESP
		proc->p_registers.reg_esp = stack_ptr;

		// Load process and set EIP, based on ELF image
		program_loader(i - 1, &proc->p_registers.reg_eip);

		// Mark the process as runnable!
		proc->p_state = P_RUNNABLE;
                // Initialize 4a priority levels
                proc->p_has_run = 0;
                // Initialize 4b shares
                proc->p_used_shares = 0;
                // Initialize 7 tickets
                proc->p_tickets = (xorshift128() % NPROCS) + 1;
                total_tickets += proc->p_tickets;
	}

        proc_array[1].p_priority = __PRIORITY_1__;
        proc_array[2].p_priority = __PRIORITY_2__;
        proc_array[3].p_priority = __PRIORITY_3__;
        proc_array[4].p_priority = __PRIORITY_4__;

        proc_array[1].p_share = __SHARE_1__;
        proc_array[2].p_share = __SHARE_2__;
        proc_array[3].p_share = __SHARE_3__;
        proc_array[4].p_share = __SHARE_4__;

	// Initialize the cursor-position shared variable to point to the
	// console's first character (the upper left).
	cursorpos = (uint16_t *) 0xB8000;
        // Initialize global lock
        lock = 0;

	// Initialize the scheduling algorithm.
	// USE THE FOLLOWING VALUES:
	//    0 = the initial algorithm
	//    1 = strict priority scheduling (exercise 2)
	//    2 = p_priority algorithm (exercise 4.a)
	//    3 = p_share algorithm (exercise 4.b)
	//    4 = any algorithm that you may implement for exercise 7
	scheduling_algorithm = 0;

        // Initialization for priority algorithm
        current_priority = 0;
        pid_t j;
        for (j = 1; j < NPROCS; j++)
            current_priority = current_priority < proc_array[j].p_priority ? current_priority : proc_array[j].p_priority;

        // Initialization for share algorithm
        used_shares = 0;
        max_shares = 0;
        for (j = 1; j < NPROCS; j++)
            max_shares += proc_array[j].p_share;

	// Switch to the first process.
	run(&proc_array[1]);

	// Should never get here!
	while (1)
		/* do nothing */;
}



/*****************************************************************************
 * interrupt
 *
 *   This is the weensy interrupt and system call handler.
 *   The current handler handles 4 different system calls (two of which
 *   do nothing), plus the clock interrupt.
 *
 *   Note that we will never receive clock interrupts while in the kernel.
 *
 *****************************************************************************/

void
interrupt(registers_t *reg)
{
	// Save the current process's register state
	// into its process descriptor
	current->p_registers = *reg;

	switch (reg->reg_intno) {

	case INT_SYS_YIELD:
		// The 'sys_yield' system call asks the kernel to schedule
		// the next process.
		schedule();

	case INT_SYS_EXIT:
		// 'sys_exit' exits the current process: it is marked as
		// non-runnable.
		// The application stored its exit status in the %eax register
		// before calling the system call.  The %eax register has
		// changed by now, but we can read the application's value
		// out of the 'reg' argument.
		// (This shows you how to transfer arguments to system calls!)
		current->p_state = P_ZOMBIE;
		current->p_exit_status = reg->reg_eax;
                if (scheduling_algorithm == __EXERCISE_4B__)
                {
                    max_shares -= current->p_share;
                    used_shares -= current->p_used_shares;
                }
                else if (scheduling_algorithm == __EXERCISE_7__)
                    total_tickets -= current->p_tickets;

                schedule();

	case INT_SYS_PRIORITY:
		// 'sys_user*' are provided for your convenience, in case you
		// want to add a system call.
		/* Your code here (if you want). */
                current->p_priority = (uint8_t)reg->reg_eax;
		if (current->p_priority < current_priority)
                    current_priority = current->p_priority;
                run(current);

	case INT_SYS_SHARE:
		/* Your code here (if you want). */
                if (reg->reg_eax > 0)
                {
                    uint8_t old_share = current->p_share;
                    current->p_share = reg->reg_eax;
                    max_shares = max_shares - old_share + current->p_share;
                    if (current->p_used_shares > current->p_share)
                    {
                        current->p_used_shares = current->p_share;
                        used_shares = used_shares - (current->p_used_shares - current->p_share);
                    }
                }
		run(current);

	case INT_CLOCK:
		// A clock interrupt occurred (so an application exhausted its
		// time quantum).
		// Switch to the next runnable process.
		schedule();

	default:
		while (1)
			/* do nothing */;

	}
}



/*****************************************************************************
 * schedule
 *
 *   This is the weensy process scheduler.
 *   It picks a runnable process, then context-switches to that process.
 *   If there are no runnable processes, it spins forever.
 *
 *   This function implements multiple scheduling algorithms, depending on
 *   the value of 'scheduling_algorithm'.  We've provided one; in the problem
 *   set you will provide at least one more.
 *
 *****************************************************************************/

void
schedule(void)
{
	pid_t pid = current->p_pid;

	if (scheduling_algorithm == __EXERCISE_1__)
		while (1) {
			pid = (pid + 1) % NPROCS;

			// Run the selected process, but skip
			// non-runnable processes.
			// Note that the 'run' function does not return.
			if (proc_array[pid].p_state == P_RUNNABLE)
				run(&proc_array[pid]);
		}
        else if (scheduling_algorithm == __EXERCISE_2__)
        {
            pid_t i = 1;
            while (1) {
                if (proc_array[i].p_state == P_RUNNABLE)
                    run(&proc_array[i]);
                i = (i + 1) % NPROCS;
            }
        }
        else if (scheduling_algorithm == __EXERCISE_4A__)
        {
            proc_array[pid].p_has_run = 1;
            
            pid_t i;
            bool_t more_same_priority = 0;
            unsigned int next_priority = 256;
            for (i = 1; i < NPROCS; i++)
            {
                if (proc_array[i].p_state == P_RUNNABLE)
                {
                    if (proc_array[i].p_priority == current_priority)
                    {
                        if (!proc_array[i].p_has_run)
                            run(&proc_array[i]);
                        else
                            more_same_priority = 1;
                    }
                    if (proc_array[i].p_priority > current_priority &&
                        proc_array[i].p_priority < next_priority)
                        next_priority = proc_array[i].p_priority;
                }
            }

            if (!more_same_priority)
            {
                if (next_priority < 256)
                    current_priority = (uint8_t)next_priority;
                else
                    while (1)
                        /* do nothing */;
            }

            // Need to mark all processes of the current priority as not having run
            pid_t first_runnable = NPROCS;
            for (i = 1; i < NPROCS; i++)
            {
                if (proc_array[i].p_state == P_RUNNABLE &&
                    proc_array[i].p_priority == current_priority)
                {
                    if (first_runnable == NPROCS)
                        first_runnable = i;
                    proc_array[i].p_has_run = 0;
                }
            }
            run(&proc_array[first_runnable]);
        }
        else if (scheduling_algorithm == __EXERCISE_4B__)
        {
            used_shares++;
            proc_array[pid].p_used_shares++;

            pid_t i;
            if (used_shares >= max_shares)
            {
                for (i = 1; i < NPROCS; i++)
                    proc_array[i].p_used_shares = 0;
                used_shares = 0;
            }

            for (i = 1; i < NPROCS; i++)
            {
                if (proc_array[i].p_state == P_RUNNABLE &&
                    proc_array[i].p_used_shares < proc_array[i].p_share)
                    run(&proc_array[i]);
            }

            while (1)
                /* do nothing */;
        }
        else if (scheduling_algorithm == __EXERCISE_7__)
        {
            if (total_tickets == 0)
                while (1)
                    /* do nothing */;

            uint32_t ticket = xorshift128() % total_tickets;
            pid_t i;
            for (i = 1; i < NPROCS; i++)
            {
                if (proc_array[i].p_state == P_RUNNABLE)
                {
                    if (ticket >= proc_array[i].p_tickets)
                        ticket -= proc_array[i].p_tickets;
                    else
                        run(&proc_array[i]);
                }
            }
        }

	// If we get here, we are running an unknown scheduling algorithm.
	cursorpos = console_printf(cursorpos, 0x100, "\nUnknown scheduling algorithm %d\n", scheduling_algorithm);
	while (1)
		/* do nothing */;
}

// xorshift: credit to George Marsaglia
// http://www.jstatsoft.org/v08/i14/paper
uint32_t xorshift128(void)
{
    uint32_t t;
    t=(x^(x<<11));x=y;y=z;z=w; return( w=(w^(w>>19)) ^ (t ^ (t>>8)) );
}
