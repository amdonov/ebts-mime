#include <sys/inotify.h>
#include "fswatch.h"
#include "ebts.h"

struct appdata
{
  char *srcdir;
  char *destdir;
};

void
process_file (void *filename, void *data)
{
  struct appdata *d = (struct appdata *) data;
  gchar *srcfile = g_build_filename (d->srcdir, (char *) filename, NULL);
  gchar *destfile = g_build_filename (d->destdir, (char *) filename, NULL);
  g_free (filename);

  g_message("Processing %s", srcfile);
  ebts_mime(srcfile, destfile);

  g_free (srcfile);
  g_free (destfile);
}

int
main (int argc, char **argv)
{
  struct appdata data;
  data.srcdir = NULL;
  data.destdir = NULL;

  GOptionEntry entries[] = {
    {"srcdir", 's', 0, G_OPTION_ARG_FILENAME, &data.srcdir,
     "Directory monitored for EBTS files", NULL},
    {"destdir", 'd', 0, G_OPTION_ARG_FILENAME, &data.destdir,
     "Output directory MIME files", NULL},
    {NULL}
  };

  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new ("- monitor a directory for new EBTS files");
  g_option_context_add_main_entries (context, entries, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_critical ("Option parsing failed: %s", error->message);
      return 1;
    }
  g_option_context_free (context);
  if (data.srcdir == NULL)
    {
      g_critical ("Source directory not specified. Try --help.");
      return 1;
    }
  if (data.destdir == NULL)
    {
      g_critical ("Destination directory not specified. Try --help.");
      g_free(data.srcdir);
      return 1;
    }

  g_mime_init (0);
  fswatch_dir (data.srcdir, IN_MOVED_TO | IN_CLOSE_WRITE, process_file,
	       &data);
  g_free (data.srcdir);
  g_mime_shutdown ();
}
