#include <string.h>
#include <libxml/xmlwriter.h>
#include <wchar.h>
#include <gmime/gmime.h>
#include <OpenEBTS.h>
#include <OpenEBTSErrors.h>

void
add_meta (GMimeMultipart * message, const char *pData)
{
  GMimeDataWrapper *content;
  GMimePart *part;
  GMimeStream *fstream;

  part = g_mime_part_new_with_type ("text", "xml");

  fstream = g_mime_stream_mem_new ();
  g_mime_stream_write_string (fstream, pData);
  g_mime_stream_reset (fstream);
  content =
    g_mime_data_wrapper_new_with_stream (fstream,
					 GMIME_CONTENT_ENCODING_DEFAULT);
  g_object_unref (fstream);
  g_mime_part_set_filename (part, "meta.xml");

  g_mime_part_set_content_object (part, content);
  g_mime_multipart_add (message, (GMimeObject *) part);
  g_object_unref (part);
  g_object_unref (content);
}

void
add_image (GMimeMultipart * message, const void *pData, int cbSize,
	   char *name)
{
  GMimeDataWrapper *content;
  GMimePart *part;
  GMimeStream *fstream;

  part = g_mime_part_new_with_type ("application", "octet-stream");

  fstream = g_mime_stream_mem_new ();
  g_mime_stream_write (fstream, pData, cbSize);
  g_mime_stream_reset (fstream);
  content =
    g_mime_data_wrapper_new_with_stream (fstream,
					 GMIME_CONTENT_ENCODING_DEFAULT);
  g_object_unref (fstream);
  g_mime_part_set_filename (part, name);

  g_mime_part_set_content_object (part, content);
  g_mime_multipart_add (message, (GMimeObject *) part);
  g_object_unref (part);
  g_object_unref (content);
}

int
ebts_mime (char *src, char *dest)
{
  int iRet;
  int nRecs;
  CIWTransaction *pTrans = NULL;
  CIWVerification *pVer = NULL;

  iRet = IWRead (src, pVer, &pTrans);
  if (iRet != IW_SUCCESS)
    {
      printf ("Failed to read file");
      return 1;
    }
  xmlBufferPtr buf;
  xmlTextWriterPtr writer;
  // TODO check for null buf
  buf = xmlBufferCreate ();
  // TODO check for null writer
  writer = xmlNewTextWriterMemory (buf, 0);
  xmlTextWriterStartDocument (writer, NULL, "UTF-8", NULL);
  xmlTextWriterStartElement (writer, "ebts");
  GMimeStream *stream;

  GMimeMultipart *message;
  message = g_mime_multipart_new ();

  int i;
  for (i = 1; i < 100; i++)
    {
      iRet = IWGetRecordTypeCount (pTrans, i, &nRecs);
      if (iRet == IW_SUCCESS)
	{
	  if (nRecs > 0)
	    {
	      char type[10];
	      snprintf (type, 10, "%d", i);
	      xmlTextWriterStartElement (writer, "type");
	      xmlTextWriterWriteAttribute (writer, "num", type);
	    }
	  // Loop over the records
	  int j;
	  for (j = 1; j <= nRecs; j++)
	    {
	      char record[10];
	      snprintf (record, 10, "%d", j);
	      xmlTextWriterStartElement (writer, "record");
	      xmlTextWriterWriteAttribute (writer, "num", record);
	      // Write out any images
	      int cbSize;
	      const TCHAR *szFormat;
	      const void *pData;
	      char name[50];
	      iRet = IWGetImage (pTrans, i, j, &szFormat, &cbSize, &pData);
	      if (iRet == IW_SUCCESS)
		{
		  snprintf (name, 50, "image-%d-%d.%s", i, j, szFormat);
		  add_image (message, pData, cbSize, name);
		}

	      int nFields;
	      iRet = IWGetNumFields (pTrans, i, j, &nFields);
	      if (iRet == IW_SUCCESS)
		{
		  // Loop over the fields
		  int k;
		  int x = 0;
		  for (k = 1; k <= nFields; k++)
		    {
		      iRet = IWGetNextField (pTrans, i, j, x, &x);
		      if (iRet == IW_SUCCESS)
			{
			  char field[10];
			  snprintf (field, 10, "%d", x);
			  xmlTextWriterStartElement (writer, "field");
			  xmlTextWriterWriteAttribute (writer, "num", field);
			  int nSubFields;
			  iRet =
			    IWNumSubfields (pTrans, i, j, x, &nSubFields);
			  if (iRet == IW_SUCCESS)
			    {
			      // Loop over the subfields
			      int l;
			      for (l = 1; l <= nSubFields; l++)
				{
				  if (nSubFields > 1)
				    {
				      char subfield[10];
				      snprintf (subfield, 10, "%d", l);
				      xmlTextWriterStartElement (writer,
								 "subfield");
				      xmlTextWriterWriteAttribute (writer,
								   "num",
								   subfield);
				    }
				  int nCount;
				  iRet =
				    IWNumItems (pTrans, i, j, x, l, &nCount);
				  if (iRet == IW_SUCCESS)
				    {
				      // Loop over the values
				      int m;
				      for (m = 1; m <= nCount; m++)
					{
					  const char *szData;
					  iRet =
					    IWFindItem (pTrans, i, j, x, l, m,
							&szData);
					  if (iRet == IW_SUCCESS)
					    {
					      int len =
						2 *
						wcslen ((const wchar_t *)
							szData);
					      char *value;
					      value = malloc (len);
					      snprintf (value, len, "%S",
							szData);

					      xmlTextWriterWriteElement
						(writer, "value", value);
					      free (value);
					    }
					}
				    }
				  // End subfield
				  if (nSubFields > 1)
				    {
				      xmlTextWriterEndElement (writer);
				    }
				}
			    }

			  // End field
			  xmlTextWriterEndElement (writer);
			}
		    }
		}
	      // End Record
	      xmlTextWriterEndElement (writer);
	    }

	  if (nRecs > 0)
	    {
	      // End Type
	      xmlTextWriterEndElement (writer);
	    }
	}
    }

  if (pTrans != NULL)
    {
      IWClose (&pTrans);
      pTrans = NULL;
    }

  if (pVer != NULL)
    {
      IWCloseVerification (&pVer);
      pVer = NULL;
    }
  // close all open tags and finish the document
  xmlTextWriterEndDocument (writer);
  xmlFreeTextWriter (writer);
  add_meta (message, buf->content);
  xmlBufferFree (buf);

  FILE *fd;
  fd = fopen (dest, "w");
  if (fd == NULL)
    {
      printf ("File didn't open\n");
    }
  stream = g_mime_stream_file_new (fd);

  g_mime_object_write_to_stream ((GMimeObject *) message, stream);

  g_mime_stream_flush (stream);

  g_object_unref (stream);
  g_object_unref (message);

  return 0;
}

void
ebts_init ()
{
  g_mime_init (0);
}

void
ebts_shutdown ()
{
  g_mime_shutdown ();
}
