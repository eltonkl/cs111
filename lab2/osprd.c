#include <linux/version.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>  /* printk() */
#include <linux/errno.h>   /* error codes */
#include <linux/types.h>   /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/wait.h>
#include <linux/file.h>
#include <linux/list.h>

#include "spinlock.h"
#include "osprd.h"

/* The size of an OSPRD sector. */
#define SECTOR_SIZE	512

/* This flag is added to an OSPRD file's f_flags to indicate that the file
 * is locked. */
#define F_OSPRD_LOCKED	0x80000

/* eprintk() prints messages to the console.
 * (If working on a real Linux machine, change KERN_NOTICE to KERN_ALERT or
 * KERN_EMERG so that you are sure to see the messages.  By default, the
 * kernel does not print all messages to the console.  Levels like KERN_ALERT
 * and KERN_EMERG will make sure that you will see messages.) */
#define eprintk(format, ...) printk(KERN_NOTICE format, ## __VA_ARGS__)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("CS 111 RAM Disk");
// EXERCISE: Pass your names into the kernel as the module's authors.
MODULE_AUTHOR("Elton Leong and James Wang");

#define OSPRD_MAJOR	222

/* This module parameter controls how big the disk will be.
 * You can specify module parameters when you load the module,
 * as an argument to insmod: "insmod osprd.ko nsectors=4096" */
static int nsectors = 32;
module_param(nsectors, int, 0);

typedef struct reader_list
{
    struct list_head list;
    pid_t pid;
} reader_list_t;

typedef struct ticket_list
{
    struct list_head list;
    int ticket_num;
} ticket_list_t;

/* The internal representation of our device. */
typedef struct osprd_info {
	uint8_t *data;                  // The data array. Its size is
	                                // (nsectors * SECTOR_SIZE) bytes.

	osp_spinlock_t mutex;           // Mutex for synchronizing access to
					// this block device

	unsigned ticket_head;		// Currently running ticket for
					// the device lock

	unsigned ticket_tail;		// Next available ticket for
					// the device lock

	wait_queue_head_t blockq;       // Wait queue for tasks blocked on
					// the device lock

        unsigned num_read_locks;        // Number of read locks

        unsigned is_write_locked;       // bool indicating if device is write
                                        // locked

        pid_t writer_pid;               // If device is write locked,
                                        // PID of the write lock owner

        reader_list_t readers;          // Linked list of reader PIDs

        ticket_list_t tickets;  // Linked list of valid tickets

	// The following elements are used internally; you don't need
	// to understand them.
	struct request_queue *queue;    // The device request queue.
	spinlock_t qlock;		// Used internally for mutual
	                                //   exclusion in the 'queue'.
	struct gendisk *gd;             // The generic disk.
} osprd_info_t;

#define NOSPRD 4
static osprd_info_t osprds[NOSPRD];


// Declare useful helper functions

/*
 * file2osprd(filp)
 *   Given an open file, check whether that file corresponds to an OSP ramdisk.
 *   If so, return a pointer to the ramdisk's osprd_info_t.
 *   If not, return NULL.
 */
static osprd_info_t *file2osprd(struct file *filp);

/*
 * for_each_open_file(task, callback, user_data)
 *   Given a task, call the function 'callback' once for each of 'task's open
 *   files.  'callback' is called as 'callback(filp, user_data)'; 'filp' is
 *   the open file, and 'user_data' is copied from for_each_open_file's third
 *   argument.
 */
static void for_each_open_file(struct task_struct *task,
			       void (*callback)(struct file *filp,
						osprd_info_t *user_data),
			       osprd_info_t *user_data);


/*
 * osprd_process_request(d, req)
 *   Called when the user reads or writes a sector.
 *   Should perform the read or write, as appropriate.
 */
static void osprd_process_request(osprd_info_t *d, struct request *req)
{
        unsigned long offset, nbytes;
	if (!blk_fs_request(req)) {
		end_request(req, 0);
		return;
	}

	// EXERCISE: Perform the read or write request by copying data between
	// our data array and the request's buffer.
	// Hint: The 'struct request' argument tells you what kind of request
	// this is, and which sectors are being read or written.
	// Read about 'struct request' in <linux/blkdev.h>.
	// Consider the 'req->sector', 'req->current_nr_sectors', and
	// 'req->buffer' members, and the rq_data_dir() function.

	// Your code here.
	//eprintk("Should process request...\n");
        offset = req->sector * SECTOR_SIZE;
        nbytes = req->current_nr_sectors * SECTOR_SIZE;
        if (offset + nbytes > nsectors * SECTOR_SIZE)
        {
            eprintk("Error: attempt to read past the bounds of the ramdisk\n");
            end_request(req, 0);
            return;
        }
        if (rq_data_dir(req) == READ)
        {
            //eprintk("Reading %u sectors from sector %lu\n", req->current_nr_sectors, (unsigned long)req->sector);
            memcpy(req->buffer, d->data + offset, nbytes);
        }
        else
        {
            //eprintk("Writing %u sectors to sector %lu\n", req->current_nr_sectors, (unsigned long)req->sector);
            memcpy(d->data + offset, req->buffer, nbytes);
        }
	end_request(req, 1);
}


// This function is called when a /dev/osprdX file is opened.
// You aren't likely to need to change this.
static int osprd_open(struct inode *inode, struct file *filp)
{
	// Always set the O_SYNC flag. That way, we will get writes immediately
	// instead of waiting for them to get through write-back caches.
	filp->f_flags |= O_SYNC;
	return 0;
}


// This function is called when a /dev/osprdX file is finally closed.
// (If the file descriptor was dup2ed, this function is called only when the
// last copy is closed.)
static int osprd_close_last(struct inode *inode, struct file *filp)
{
	if (filp) {
		osprd_info_t *d = file2osprd(filp);
		int filp_writable = filp->f_mode & FMODE_WRITE;

		// EXERCISE: If the user closes a ramdisk file that holds
		// a lock, release the lock.  Also wake up blocked processes
		// as appropriate.

		// Your code here.

                if (filp->f_flags & F_OSPRD_LOCKED)
                {
                    osp_spin_lock(&d->mutex);
                    //eprintk("close_last\n");
                    //Perform freeing if it was a write lock
                    if (filp_writable)
                    {
                        if (current->pid == d->writer_pid)
                        {
                            d->is_write_locked = 0;
                            d->writer_pid = -1;
                            filp->f_flags ^= F_OSPRD_LOCKED;
                        }
                        //eprintk("close_last write\n");
                    }
                    //Otherwise perform freeing if it was a read lock
                    else
                    {
                        struct list_head *pos, *q;
                        reader_list_t* tmp;
                        int removed = 0;
                        list_for_each_safe(pos, q, &d->readers.list) // Delete all read locks the process held
                        {
                            tmp = list_entry(pos, reader_list_t, list);
                            if (tmp->pid == current->pid)
                            {
                                d->num_read_locks--;
                                list_del(pos);
                                kfree(tmp);
                                removed = 1;
                            }
                        }
                        if (removed)
                            filp->f_flags ^= F_OSPRD_LOCKED;
                        //eprintk("close_last read\n");
                    }
                    osp_spin_unlock(&d->mutex);
                    wake_up_all(&d->blockq);
                }
        }
        return 0;
}


/*
 * osprd_lock
 */

/*
 * osprd_ioctl(inode, filp, cmd, arg)
 *   Called to perform an ioctl on the named file.
 */
int osprd_ioctl(struct inode *inode, struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	osprd_info_t *d = file2osprd(filp);	// device info
	int r = 0;			// return value: initially 0

	// is file open for writing?
	int filp_writable = (filp->f_mode & FMODE_WRITE) != 0;

	// Set 'r' to the ioctl's return value: 0 on success, negative on error

        ticket_list_t* ticket = NULL;
        reader_list_t* reader = NULL;
        if (cmd == OSPRDIOCACQUIRE) {

		// EXERCISE: Lock the ramdisk.
		//
		// If *filp is open for writing (filp_writable), then attempt
		// to write-lock the ramdisk; otherwise attempt to read-lock
		// the ramdisk.
		//
                // This lock request must block using 'd->blockq' until:
		// 1) no other process holds a write lock;
		// 2) either the request is for a read lock, or no other process
		//    holds a read lock; and
		// 3) lock requests should be serviced in order, so no process
		//    that blocked earlier is still blocked waiting for the
		//    lock.
		//
		// If a process acquires a lock, mark this fact by setting
		// 'filp->f_flags |= F_OSPRD_LOCKED'.  You also need to
		// keep track of how many read and write locks are held:
		// change the 'osprd_info_t' structure to do this.
		//
		// Also wake up processes waiting on 'd->blockq' as needed.
		//
		// If the lock request would cause a deadlock, return -EDEADLK.
		// If the lock request blocks and is awoken by a signal, then
		// return -ERESTARTSYS.
		// Otherwise, if we can grant the lock request, return 0.

		// 'd->ticket_head' and 'd->ticket_tail' should help you
		// service lock requests in order.  These implement a ticket
		// order: 'ticket_tail' is the next ticket, and 'ticket_head'
		// is the ticket currently being served.  You should set a local
		// variable to 'd->ticket_head' and increment 'd->ticket_head'.
		// Then, block at least until 'd->ticket_tail == local_ticket'.
		// (Some of these operations are in a critical section and must
		// be protected by a spinlock; which ones?)

                // Why C90? :(
                int res;
                int local_ticket;
                struct list_head *pos;
                ticket_list_t* tmp;
                int queue_empty;

                ticket = kmalloc(sizeof(ticket_list_t), GFP_ATOMIC);
                if (ticket == NULL)
                    return -ENOMEM;

                if (!filp_writable)
                {
                    reader = kmalloc(sizeof(reader_list_t), GFP_ATOMIC);
                    if (reader == NULL) // malloc failed, do not increment the number of read locks
                    {
                        r = -ENOMEM;
                        goto fail;
                    }
                }

                osp_spin_lock(&d->mutex);
                if (d->is_write_locked && d->writer_pid == current->pid) // Check if owner of write lock is attempting to acquire another lock
                {
                    osp_spin_unlock(&d->mutex);
                    r = -EDEADLK;
                    goto fail;
                }
                if (filp_writable) //Check if owner of read lock is attempting to acquire a write lock
                {
                    list_for_each(pos, &d->readers.list)
                    {
                        reader_list_t* cur;
                        cur = list_entry(pos, reader_list_t, list);
                        {
                            if (cur->pid == current->pid)
                            {
                                osp_spin_unlock(&d->mutex);
                                r = -EDEADLK;
                                goto fail;
                            }
                        }
                    }
                }
                // Create ticket, and insert the ticket into the ticket queue
                local_ticket = d->ticket_tail;
                d->ticket_tail++;
                //eprintk("acquire %d local %d head %d tail %d\n", filp_writable, local_ticket, d->ticket_head, d->ticket_tail);
                ticket->ticket_num = local_ticket;
                list_add_tail(&ticket->list, &d->tickets.list);
                osp_spin_unlock(&d->mutex);

                if (filp_writable) // If we are acquiring a write lock, wait until there are no readers or writers
		    res = wait_event_interruptible(d->blockq,
                            local_ticket == d->ticket_head &&
                            d->is_write_locked == 0 &&
                            d->num_read_locks == 0);
                else // If we are acquiring a read lock, wait until there are no writers
                    res = wait_event_interruptible(d->blockq,
                            local_ticket == d->ticket_head &&
                            d->is_write_locked == 0);
                if (res != 0) //Interrupted by signal!
                {
                    r = -ERESTARTSYS;
                    osp_spin_lock(&d->mutex);
                    //eprintk("interrupt local %d head %d tail %d\n", local_ticket, d->ticket_head, d->ticket_tail);
                }
                else //Continue like normal (no interruption)
                {
                    filp->f_flags |= F_OSPRD_LOCKED;
                    r = 0;
                    if (!filp_writable)
                        reader->pid = current->pid;
                    osp_spin_lock(&d->mutex);
                    if (filp_writable) //Write lock the ramdisk
                    { 
                        d->is_write_locked = 1;
                        d->writer_pid = current->pid;
                    }
                    else //if (!filp_writable) //Read lock the ramdisk
                    {
                        list_add_tail(&reader->list, &d->readers.list);
                        //increment number of read locks
                        d->num_read_locks++;
                    }
                    //eprintk("success local %d head %d tail %d\n", local_ticket, d->ticket_head, d->ticket_tail);
                }
                // Delete ticket from ticket queue
                list_for_each(pos, &d->tickets.list)
                {
                    tmp = list_entry(pos, ticket_list_t, list);
                    if (tmp->ticket_num == local_ticket)
                    {
                        list_del(pos);
                        kfree(tmp);
                        //eprintk("Deleted ticket %d from list\n", local_ticket);
                        break;
                    }
                }
                queue_empty = list_empty(&d->tickets.list);
                if (queue_empty) // No tickets in queue, set head to tail
                    d->ticket_head = d->ticket_tail;
                else // Serve the next ticket in the queue
                {
                    ticket_list_t* first = list_entry(d->tickets.list.next, ticket_list_t, list);
                    d->ticket_head = first->ticket_num;
                }
                //eprintk("New ticket num: %d\n", d->ticket_head);
                osp_spin_unlock(&d->mutex);
                if (res != 0)
                {
                    if (!filp_writable)
                    {
                        if (reader)
                            kfree(reader);
                    }
                }
                if (!queue_empty)
                    wake_up_all(&d->blockq);
	} else if (cmd == OSPRDIOCTRYACQUIRE) {

		// EXERCISE: ATTEMPT to lock the ramdisk.
		//
		// This is just like OSPRDIOCACQUIRE, except it should never
		// block.  If OSPRDIOCACQUIRE would block or return deadlock,
		// OSPRDIOCTRYACQUIRE should return -EBUSY.
		// Otherwise, if we can grant the lock request, return 0
                osp_spin_lock(&d->mutex);
                if (d->ticket_head != d->ticket_tail)
                {
                    osp_spin_unlock(&d->mutex);
                    return -EBUSY;
                }
                if (filp_writable)
                {
                        //Check to make sure there aren't read locks and there isn't already a write lock
                        if (d->num_read_locks > 0 || d->is_write_locked == 1)
                        {
                            osp_spin_unlock(&d->mutex);
                            return -EBUSY;
                        }
                        else
                        {
                            d->is_write_locked = 1;
                            d->writer_pid = current->pid;
                        }
                }
                else
                {
                        //Check to make sure there's no write lock
                        if (d->is_write_locked == 1)
                        {
                            osp_spin_unlock(&d->mutex);
                            return -EBUSY;
                        }
                        else
                        {
                            reader_list_t* entry = kmalloc(sizeof(reader_list_t), GFP_ATOMIC);
                            if (entry == NULL)
                            {
                                osp_spin_unlock(&d->mutex);
                                return -ENOMEM;
                            }
                            else
                            {
                                entry->pid = current->pid;
                                list_add_tail(&entry->list, &d->readers.list);
                                d->num_read_locks++;
                            }
                    }
                }
                
                //If we make it this far, then we have a lock
                d->ticket_head++;
                d->ticket_tail++;
                filp->f_flags |= F_OSPRD_LOCKED;
                osp_spin_unlock(&d->mutex);
                r = 0;
	} else if (cmd == OSPRDIOCRELEASE) {

		// EXERCISE: Unlock the ramdisk.
		//
		// If the file hasn't locked the ramdisk, return -EINVAL.
		// Otherwise, clear the lock from filp->f_flags, wake up
		// the wait queue, perform any additional accounting steps
		// you need, and return 0.

		// Your code here (instead of the next line).

		//If the file hasn't locked the ramdisk, return -EINVAL
		if ((filp->f_flags & F_OSPRD_LOCKED) == 0)
			return -EINVAL;

		//Lock to prepare for performing the free
		osp_spin_lock(&d->mutex);

		//Perform freeing if it was a write lock
		if (filp_writable)
		{
                    if (current->pid == d->writer_pid)
                    {
                        d->is_write_locked = 0;
                        d->writer_pid = -1;
                        filp->f_flags ^= F_OSPRD_LOCKED;
                    }
                    else
                    {
                        osp_spin_unlock(&d->mutex);
                        return -EINVAL;
                    }
		}
		//Otherwise perform freeing if it was a read lock
		else
		{
                    struct list_head *pos, *q;
                    reader_list_t* tmp;
                    int removed = 0;
                    int more_read_locks = 0;
                    d->num_read_locks--;
                    list_for_each_safe(pos, q, &d->readers.list)
                    {
                        tmp = list_entry(pos, reader_list_t, list);
                        if (!removed && tmp->pid == current->pid)
                        {
                            list_del(pos);
                            kfree(tmp);
                            removed = 1;
                            continue;
                        }
                        if (removed && tmp->pid == current->pid)
                        {
                            more_read_locks = 1;
                            break;
                        }
                    }

                    if (!removed)
                    {
                        osp_spin_unlock(&d->mutex);
                        return -EINVAL;
                    }
                    else //if (removed)
                    {
                        if (!more_read_locks)
                            filp->f_flags ^= F_OSPRD_LOCKED;
                    }
		}

		//Unlock, then wake up the wait queue
		osp_spin_unlock(&d->mutex);
		wake_up_all(&d->blockq);
		r = 0;
	} else
		r = -ENOTTY; /* unknown command */
	return r;
fail:
        if (ticket)
            kfree(ticket);
        if (reader)
            kfree(reader);
        return r;
}


// Initialize internal fields for an osprd_info_t.

static void osprd_setup(osprd_info_t *d)
{
	/* Initialize the wait queue. */
	init_waitqueue_head(&d->blockq);
	osp_spin_lock_init(&d->mutex);
	d->ticket_head = d->ticket_tail = 0;
	/* Add code here if you add fields to osprd_info_t. */
        d->num_read_locks = 0;
        d->is_write_locked = 0;
        INIT_LIST_HEAD(&d->readers.list);
        INIT_LIST_HEAD(&d->tickets.list);
}


/*****************************************************************************/
/*         THERE IS NO NEED TO UNDERSTAND ANY CODE BELOW THIS LINE!          */
/*                                                                           */
/*****************************************************************************/

// Process a list of requests for a osprd_info_t.
// Calls osprd_process_request for each element of the queue.

static void osprd_process_request_queue(request_queue_t *q)
{
	osprd_info_t *d = (osprd_info_t *) q->queuedata;
	struct request *req;

	while ((req = elv_next_request(q)) != NULL)
		osprd_process_request(d, req);
}


// Some particularly horrible stuff to get around some Linux issues:
// the Linux block device interface doesn't let a block device find out
// which file has been closed.  We need this information.

static struct file_operations osprd_blk_fops;
static int (*blkdev_release)(struct inode *, struct file *);

static int _osprd_release(struct inode *inode, struct file *filp)
{
	if (file2osprd(filp))
		osprd_close_last(inode, filp);
	return (*blkdev_release)(inode, filp);
}

static int _osprd_open(struct inode *inode, struct file *filp)
{
	if (!osprd_blk_fops.open) {
		memcpy(&osprd_blk_fops, filp->f_op, sizeof(osprd_blk_fops));
		blkdev_release = osprd_blk_fops.release;
		osprd_blk_fops.release = _osprd_release;
	}
	filp->f_op = &osprd_blk_fops;
	return osprd_open(inode, filp);
}


// The device operations structure.

static struct block_device_operations osprd_ops = {
	.owner = THIS_MODULE,
	.open = _osprd_open,
	// .release = osprd_release, // we must call our own release
	.ioctl = osprd_ioctl
};


// Given an open file, check whether that file corresponds to an OSP ramdisk.
// If so, return a pointer to the ramdisk's osprd_info_t.
// If not, return NULL.

static osprd_info_t *file2osprd(struct file *filp)
{
	if (filp) {
		struct inode *ino = filp->f_dentry->d_inode;
		if (ino->i_bdev
		    && ino->i_bdev->bd_disk
		    && ino->i_bdev->bd_disk->major == OSPRD_MAJOR
		    && ino->i_bdev->bd_disk->fops == &osprd_ops)
			return (osprd_info_t *) ino->i_bdev->bd_disk->private_data;
	}
	return NULL;
}


// Call the function 'callback' with data 'user_data' for each of 'task's
// open files.

static void for_each_open_file(struct task_struct *task,
		  void (*callback)(struct file *filp, osprd_info_t *user_data),
		  osprd_info_t *user_data)
{
	int fd;
	task_lock(task);
	spin_lock(&task->files->file_lock);
	{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 13)
		struct files_struct *f = task->files;
#else
		struct fdtable *f = task->files->fdt;
#endif
		for (fd = 0; fd < f->max_fds; fd++)
			if (f->fd[fd])
				(*callback)(f->fd[fd], user_data);
	}
	spin_unlock(&task->files->file_lock);
	task_unlock(task);
}


// Destroy a osprd_info_t.

static void cleanup_device(osprd_info_t *d)
{
	wake_up_all(&d->blockq);
	if (d->gd) {
		del_gendisk(d->gd);
		put_disk(d->gd);
	}
	if (d->queue)
		blk_cleanup_queue(d->queue);
	if (d->data)
		vfree(d->data);
}


// Initialize a osprd_info_t.

static int setup_device(osprd_info_t *d, int which)
{
	memset(d, 0, sizeof(osprd_info_t));

	/* Get memory to store the actual block data. */
	if (!(d->data = vmalloc(nsectors * SECTOR_SIZE)))
		return -1;
	memset(d->data, 0, nsectors * SECTOR_SIZE);

	/* Set up the I/O queue. */
	spin_lock_init(&d->qlock);
	if (!(d->queue = blk_init_queue(osprd_process_request_queue, &d->qlock)))
		return -1;
	blk_queue_hardsect_size(d->queue, SECTOR_SIZE);
	d->queue->queuedata = d;

	/* The gendisk structure. */
	if (!(d->gd = alloc_disk(1)))
		return -1;
	d->gd->major = OSPRD_MAJOR;
	d->gd->first_minor = which;
	d->gd->fops = &osprd_ops;
	d->gd->queue = d->queue;
	d->gd->private_data = d;
	snprintf(d->gd->disk_name, 32, "osprd%c", which + 'a');
	set_capacity(d->gd, nsectors);
	add_disk(d->gd);

	/* Call the setup function. */
	osprd_setup(d);

	return 0;
}

static void osprd_exit(void);


// The kernel calls this function when the module is loaded.
// It initializes the 4 osprd block devices.

static int __init osprd_init(void)
{
	int i, r;

	// shut up the compiler
	(void) for_each_open_file;
#ifndef osp_spin_lock
	(void) osp_spin_lock;
	(void) osp_spin_unlock;
#endif

	/* Register the block device name. */
	if (register_blkdev(OSPRD_MAJOR, "osprd") < 0) {
		printk(KERN_WARNING "osprd: unable to get major number\n");
		return -EBUSY;
	}

	/* Initialize the device structures. */
	for (i = r = 0; i < NOSPRD; i++)
		if (setup_device(&osprds[i], i) < 0)
			r = -EINVAL;

	if (r < 0) {
		printk(KERN_EMERG "osprd: can't set up device structures\n");
		osprd_exit();
		return -EBUSY;
	} else
		return 0;
}


// The kernel calls this function to unload the osprd module.
// It destroys the osprd devices.

static void osprd_exit(void)
{
	int i;
	for (i = 0; i < NOSPRD; i++)
		cleanup_device(&osprds[i]);
	unregister_blkdev(OSPRD_MAJOR, "osprd");
}


// Tell Linux to call those functions at init and exit time.
module_init(osprd_init);
module_exit(osprd_exit);
