#include <string.h>
#include <sys/inotify.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "fswatch.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

void
termination_handler (int signum)
{
  g_message ("Shutting down...");
}

void
catch_signals ()
{
  /* The following traps system signals.
   * Although the termination_handler does not do anything,
   * it causes the blocking read from inotify to stop gracefully.
   * Then, graceful shutdown proceeds */
  struct sigaction new_action, old_action;
  new_action.sa_handler = termination_handler;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;
  sigaction (SIGINT, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGINT, &new_action, NULL);
  sigaction (SIGHUP, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGHUP, &new_action, NULL);
  sigaction (SIGTERM, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
    sigaction (SIGTERM, &new_action, NULL);
}

int
fswatch_dir (const char *dir, uint32_t mask, GFunc func, void *user_data)
{
  catch_signals ();
  char buffer[EVENT_BUF_LEN];
  GThreadPool *thread_pool;
  g_thread_init (NULL);
  thread_pool = g_thread_pool_new (func, user_data, 5, FALSE, NULL);
  int fd;
  fd = inotify_init ();
  int wd;
  wd = inotify_add_watch (fd, dir, mask);

  int length;
  for (length = read (fd, buffer, EVENT_BUF_LEN);
       length >= 0; length = read (fd, buffer, EVENT_BUF_LEN))
    {
      int j = 0;

      /*actually read return the list of change events happens. Here, read the change event one by one and process it accordingly. */
      while (j < length)
	{
	  struct inotify_event *event = (struct inotify_event *) &buffer[j];
	  if (event->len)
	    {
	      char *name = g_strdup (event->name);
	      g_thread_pool_push (thread_pool, name, NULL);
	    }
	  j += EVENT_SIZE + event->len;
	}
    }

  inotify_rm_watch (fd, wd);
  close (fd);
  g_thread_pool_free (thread_pool, FALSE, TRUE);
  g_message ("Exiting.");
  return 0;
}
