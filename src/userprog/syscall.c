#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "lib/kernel/console.h"
#include "lib/user/syscall.h"
#include "userprog/process.h"
#include "devices/shutdown.h"
//-----------------------------
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
// i think we need the string lib. I still dont completely get the 
//      difference with this and c vs c++
#include "string.h"
//-----------------------------


static void syscall_handler (struct intr_frame *);
struct list_elem *get_list_elem(int fd);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
//syscall_handler (struct intr_frame *f UNUSED)
syscall_handler (struct intr_frame *f) 
{
  /* just for clarity sake */
  int *esp = (int*)f->esp;

  check_addr_valid(esp);  

  int *sys_call_number = (int *) f->esp;
  /*
    Remem:
    system call number is passed int he %eax register (32 bit)
        This is to distinguish which syscall to invoke
    alltrap() saves it along with all other registers (dont understand alltrap???)
  */
  switch(*sys_call_number)
  {
    /*
      Terminates Pintos by calling shutdown_power_off() 
      (declared in ‘devices/shutdown.h’). This should be seldom used, 
      because you lose some information about possible deadlock 
      situations, etc.
    */
    case SYS_HALT: 
    {
      shutdown_power_off();
      break;
    }

    /*
      Terminates the current user program, returning status to the kernel. If the process’s
      parent waits for it (see below), this is the status that will be returned. Conventionally,
      a status of 0 indicates success and nonzero values indicate errors.
    */
    case SYS_EXIT: 
    {
      check_addr_valid(esp+1);
      int exit_code = (int) *(esp+1);
      int retval = exit_code;
      f->eax = retval;
      exit(retval);
      break;
    }

    /*
      Runs the executable whose name is given in cmd line, passing any given arguments,
      and returns the new process’s program id (pid). Must return pid -1, which otherwise
      should not be a valid pid, if the program cannot load or run for any reason. Thus,
      the parent process cannot return from the exec until it knows whether the child
      process successfully loaded its executable. You must use appropriate synchronization
      to ensure this.
    */
    case SYS_EXEC: 
    {
      check_addr_valid(esp+1);
      check_addr_valid(*(esp+1));
      // set our raw to the char at our stack pointer
      const char *raw = (char *) *(esp+1);
      /* Starts a new thread running a user program loaded from
       FILENAME.  The new thread may be scheduled (and may even exit)
       before process_execute() returns.  Returns the new process's
       thread id, or TID_ERROR if the thread cannot be created. */
      
      f->eax = process_execute(raw);
      

      break;
    }

    /*
      Waits for a child process pid and retrieves the child’s exit status.
      If pid is still alive, waits until it terminates. Then, returns the status that pid passed
      to exit. If pid did not call exit(), but was terminated by the kernel (e.g. killed due
      to an exception), wait(pid) must return -1. It is perfectly legal for a parent process
      to wait for child processes that have already terminated by the time the parent calls
      wait, but the kernel must still allow the parent to retrieve its child’s exit status, or
      learn that the child was terminated by the kernel.
      wait must fail and return -1 immediately if any of the following conditions is true:
          
          • pid does not refer to a direct child of the calling process. pid is a direct child
          of the calling process if and only if the calling process received pid as a return
          value from a successful call to exec.
          Note that children are not inherited: if A spawns child B and B spawns child
          process C, then A cannot wait for C, even if B is dead. A call to wait(C) by
          process A must fail. Similarly, orphaned processes are not assigned to a new
          parent if their parent process exits before they do.

          • The process that calls wait has already called wait on pid. That is, a process
          may wait for any given child at most once.
          
      Processes may spawn any number of children, wait for them in any order, and may
      even exit without having waited for some or all of their children. Your design should
      consider all the ways in which waits can occur. All of a process’s resources, including
      its struct thread, must be freed whether its parent ever waits for it or not, and
      regardless of whether the child exits before or after its parent.
      You must ensure that Pintos does not terminate until the initial process exits.
      The supplied Pintos code tries to do this by calling process_wait() (in
      ‘userprog/process.c’) from main() (in ‘threads/init.c’). We suggest that you
      implement process_wait() according to the comment at the top of the function
      and then implement the wait system call in terms of process_wait().
      Implementing this system call requires considerably more work than any of the rest.
    */
    case SYS_WAIT: 
    {
      check_addr_valid(esp+1);
      pid_t wait_pid = ((pid_t )*(esp+1));
      f->eax = process_wait(wait_pid);
      break;
    }

    /*
      Creates a new file called file initially initial size bytes in size. Returns true if suc-
      cessful, false otherwise. Creating a new file does not open it: opening the new file is
      a separate operation which would require a open system call.
    */
    case SYS_CREATE: 
    {
      /* check validity of address */
      check_addr_valid(esp+1);
      check_addr_valid(*(esp+1));
      check_addr_valid(esp+2);

      const char *file_name = (char*)*(esp + 1);

      /* get initial size */
      off_t initial_size = (int32_t)*(esp+2);

      /* create new file */
      
      f->eax = (bool)filesys_create(file_name, initial_size);
      

    }

    /*
      Deletes the file called file. Returns true if successful, false otherwise. A file may be
      removed regardless of whether it is open or closed, and removing an open file does
      not close it. See [Removing an Open File], page 35, for details.
    */
    case SYS_REMOVE: 
    {
      /* check validity of address */
      check_addr_valid(esp+1);
      check_addr_valid(*(esp+1));
      const char *file_name = (char*)*(esp + 1);

      /* remove file from directory */
      
      f->eax = filesys_remove(file_name);
      

      break;
    }

    /*
      Opens the file called file. Returns a nonnegative integer handle called a “file descrip-
      tor” (fd), or -1 if the file could not be opened.
      File descriptors numbered 0 and 1 are reserved for the console: fd 0 (STDIN_FILENO) is
      standard input, fd 1 (STDOUT_FILENO) is standard output. The open system call will
      never return either of these file descriptors, which are valid as system call arguments
      only as explicitly described below.
      
      Each process has an independent set of file descriptors. File descriptors are not
      inherited by child processes.
      
      When a single file is opened more than once, whether by a single process or different
      processes, each open returns a new file descriptor. Different file descriptors for a single
      file are closed independently in separate calls to close and they do not share a file
      position.
    */
    case SYS_OPEN: 
    {
      check_addr_valid(esp+1);
      check_addr_valid(*(esp+1));
      const char *file_name = (char*)*(esp+1);

      /* get current running thread */
      struct thread *t = thread_current();
      int retval;

      /* open file */
      
      struct file *file = filesys_open(file_name);
      
      struct file_map fm;
      if(file == NULL)
        retval = -1;
      else
      {
        fm.fd = ++t->next_fd;
        fm.file = file;
        list_push_back(&t->files, &fm.file_elem);
        retval = fm.fd;
      }
      f->eax = retval;
      break; 
    }

    /*
      Returns the size, in bytes, of the file open as fd.
    */
    case SYS_FILESIZE: 
    {
      check_addr_valid(esp+1);
      int fd = (int)*(esp+1);
      int filesize = 0;
      int retval = 0;
      struct list_elem *e = get_list_elem(fd);
      if(e != NULL)
      {
        
        struct file_map *temp = list_entry(e, struct file_map, file_elem);
        retval = (int)file_length(temp->file);
        
      }
      f->eax = retval;
      break;
    }

    /*
      Reads size bytes from the file open as fd into buffer. Returns the number of bytes
      actually read (0 at end of file), or -1 if the file could not be read (due to a condition
      other than end of file). Fd 0 reads from the keyboard using input_getc().
    */
    case SYS_READ: 
    {
      check_addr_valid(esp+1);
      check_addr_valid(esp+2);
      check_addr_valid(esp+3);
      check_addr_valid(*(esp+2));
      int fd = (int)*(esp + 1);
      void * buffer = *(esp+2);
      unsigned size = (unsigned)*(esp+3);
      int retval = -1;

      if(fd == 0)
      {

      }
      else if(fd == 1)
        retval = -1;
      else
      {
        struct list_elem *e = get_list_elem(fd);
        if(e != NULL)
        {
          
          struct file_map *temp = list_entry(e, struct file_map, file_elem);
          retval = file_read(temp->file, buffer, size);
          
        }
      }
      f->eax = retval;
      break;
    }

    /*
      Writes size bytes from buffer to the open file fd. Returns the number of bytes actually
      written, which may be less than size if some bytes could not be written.
      Writing past end-of-file would normally extend the file, but file growth is not imple-
      mented by the basic file system. The expected behavior is to write as many bytes as
      possible up to end-of-file and return the actual number written, or 0 if no bytes could
      be written at all.
    */
    case SYS_WRITE: 
    {
      /* check validity of our stack addresses */
      check_addr_valid(esp+1);
      check_addr_valid(esp+2);
      check_addr_valid(esp+3);
      check_addr_valid(*(esp+2));
      int fd = (int)*(esp + 1);
      const void* buffer = *(esp+2);
      unsigned size = (unsigned)*(esp+3);
      int retval = -1; /* in case of failure */

      if(fd == 0)
        retval = -1;
      /* write to console 100 bytes at a time */
      else if (fd == 1)
      {
        
        putbuf(buffer, size);
        
        retval = size;
      }
      /* write to file */
      else
      {
        struct list_elem *e = get_list_elem(fd);
        if(e != NULL) 
        {
          struct file_map *temp = list_entry(e, struct file_map, file_elem);
          
          retval = file_write(temp->file, buffer, size);
                }
        }
      f->eax = retval;
      break;
    }

    /*

      Changes the next byte to be read or written in open file fd to position, expressed in
      bytes from the beginning of the file. (Thus, a position of 0 is the file’s start.)
      A seek past the current end of a file is not an error. A later read obtains 0 bytes,
      indicating end of file. A later write extends the file, filling any unwritten gap with
      zeros. (However, in Pintos files have a fixed length until project 4 is complete, so
      writes past end of file will return an error.) These semantics are implemented in the
      file system and do not require any special effort in system call implementation.
    */
    case SYS_SEEK: 
    {
      check_addr_valid(esp+1);
      check_addr_valid(esp+2);
      int fd = (int)*(esp+1);
      unsigned position = (unsigned)*(esp+2);

      /* get list element */
      struct list_elem *e = get_list_elem(fd);
      if(e != NULL)
      {
        struct file_map *fmp = list_entry(e, struct file_map, file_elem);
        
        file_seek(fmp->file, position);
        
      }
      break;
    }

    /*
      Returns the position of the next byte to be read or written in open file fd, expressed
      in bytes from the beginning of the file.
    */
    case SYS_TELL: 
    {
      check_addr_valid(esp+1);

      int fd = (int)*(esp+1);

      struct list_elem *e = get_list_elem(fd);
      if(e != NULL)
      {
        struct file_map *fmp = list_entry(e, struct file_map, file_elem);
        
        f->eax = file_tell(fmp->file);
        
      }
      else
        f->eax = 0;
      break;
    }
    /*
      Closes file descriptor fd. Exiting or terminating a process implicitly closes all its open
      file descriptors, as if by calling this function for each one.
    */
    case SYS_CLOSE: 
    {
      check_addr_valid(esp+1);

      int fd = (int) *(esp+1);

      /* get list element */
      struct list_elem* e = get_list_elem(fd);
      if(e != NULL)
      {
        struct file_map* fmp = list_entry(e, struct file_map, file_elem);
        
        file_close(fmp->file);
        list_remove(e);
        
      }
      break;    
    }

    default: 
    {
      /* bad system call */
      break;
    }
  } /* end of switch statement */
}

void 
exit(int exit_code)
{
  struct thread *t = thread_current();
  t->parent_share->exit_code = exit_code;
  //t->parent_share->reference_count -= 1;
  //t->parent_share->ref_count -= 1;
  char *thr_name = thread_name();
  printf("%s: exit(%d)\n", thr_name, exit_code);
  
  sema_up(&thread_current()->parent_share->dead_sema);
  thread_exit();
}

/* 
  Checks whether a given addr is in the current processes user space. 
  If not should return exit(-1) to signal the error
*/
void 
check_addr_valid(void *addr)
{
  for(int i = 0; i < 4; i++)
  {
    if((addr+i) == NULL || !is_user_vaddr(addr+i) ||
      pagedir_get_page(thread_current()->pagedir,(addr+i)) == NULL)
    {
      exit(-1);
    }
  }
}

/*
  Returns a struct file_map * object for given fd. Returns null
  if fd failed to be found
*/
struct list_elem
*get_list_elem(int fd)
{
  struct thread *t = thread_current();
  struct list_elem *e;
  for (e = list_begin (&t->files); e != list_end (&t->files);
      e = list_next (e))
  {
    struct file_map *fmp = list_entry (e, struct file_map, file_elem);
    if(fmp->fd == fd)
      return e;
  }
  return NULL;
}