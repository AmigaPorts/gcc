/* simple-object-amiga.c -- routines to read Amiga HUNK objects.
   Copyright (C) 2026 Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "config.h"
#include "libiberty.h"
#include "simple-object.h"

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "simple-object-common.h"

#define HUNK_UNIT 999
#define HUNK_NAME 1000
#define HUNK_CODE 1001
#define HUNK_DATA 1002
#define HUNK_BSS 1003
#define HUNK_RELOC32 1004
#define HUNK_RELOC16 1005
#define HUNK_RELOC8 1006
#define HUNK_EXT 1007
#define HUNK_SYMBOL 1008
#define HUNK_DEBUG 1009
#define HUNK_END 1010
#define HUNK_DREL32 1015
#define HUNK_DREL16 1016
#define HUNK_DREL8 1017
#define HUNK_RELOC32SHORT 1020
#define HUNK_RELRELOC32 1021
#define HUNK_ABSRELOC16 1022
#define HUNK_RELRELOC26 1260

#define HUNK_VALUE(hunk_id) ((hunk_id) & 0x3fffffff)
#define AMIGA_LTO_DEBUG_MAGIC 0x4c544f31 /* "LTO1" */
#define AMIGA_LTO_DEBUG_HEADER_SIZE 8

#define EXT_SYMB 0
#define EXT_DEF 1
#define EXT_ABS 2
#define EXT_ABSREF32 129
#define EXT_COMMON 130
#define EXT_RELREF16 131
#define EXT_RELREF8 132
#define EXT_DEXT32 133
#define EXT_DEXT16 134
#define EXT_DEXT8 135
#define EXT_RELREF32 136
#define EXT_RELCOMMON 137
#define EXT_ABSREF16 138
#define EXT_ABSREF8 139
#define EXT_DEXT32COMMON 208
#define EXT_DEXT16COMMON 209
#define EXT_DEXT8COMMON 210
#define EXT_RELREF26 229

struct simple_object_amiga_section
{
  char *name;
  off_t offset;
  off_t length;
  unsigned int hunk_index;
  struct simple_object_amiga_record *records;
  struct simple_object_amiga_record **record_tail;
  struct simple_object_amiga_section *next;
};

/* A record associated with the preceding section hunk.  Relocations and
   symbols must follow copied early-debug sections into the output object.  */

struct simple_object_amiga_record
{
  unsigned long type;
  off_t offset;
  off_t length;
  struct simple_object_amiga_record *next;
};

struct simple_object_amiga_read
{
  struct simple_object_amiga_section *sections;
  struct simple_object_amiga_section **tail;
  unsigned int hunk_count;
};

struct simple_object_amiga_attributes
{
  int unused;
};

struct simple_object_amiga_write_record
{
  unsigned long type;
  unsigned char *data;
  size_t length;
  struct simple_object_amiga_write_record *next;
};

/* Records attached to a generic simple-object output section.  */

struct simple_object_amiga_write_section
{
  simple_object_write_section *section;
  struct simple_object_amiga_write_record *records;
  struct simple_object_amiga_write_record **record_tail;
  struct simple_object_amiga_write_section *next;
};

struct simple_object_amiga_write
{
  struct simple_object_amiga_write_section *sections;
  struct simple_object_amiga_write_section **tail;
};

static unsigned long
simple_object_amiga_fetch_32 (const unsigned char *buf)
{
  return (((unsigned long) buf[0] << 24)
	  | ((unsigned long) buf[1] << 16)
	  | ((unsigned long) buf[2] << 8)
	  | (unsigned long) buf[3]);
}

static unsigned int
simple_object_amiga_fetch_16 (const unsigned char *buf)
{
  return (((unsigned int) buf[0] << 8) | (unsigned int) buf[1]);
}

static void
simple_object_amiga_store_32 (unsigned char *buf, unsigned long value)
{
  buf[0] = (value >> 24) & 0xff;
  buf[1] = (value >> 16) & 0xff;
  buf[2] = (value >> 8) & 0xff;
  buf[3] = value & 0xff;
}

static void
simple_object_amiga_store_16 (unsigned char *buf, unsigned int value)
{
  buf[0] = (value >> 8) & 0xff;
  buf[1] = value & 0xff;
}

static int
simple_object_amiga_read_32 (int descriptor, off_t offset,
			     unsigned long *value,
			     const char **errmsg, int *err)
{
  unsigned char buf[4];

  if (!simple_object_internal_read (descriptor, offset, buf, sizeof (buf),
				    errmsg, err))
    return 0;

  *value = simple_object_amiga_fetch_32 (buf);
  return 1;
}

static int
simple_object_amiga_read_16 (int descriptor, off_t offset,
			     unsigned int *value,
			     const char **errmsg, int *err)
{
  unsigned char buf[2];

  if (!simple_object_internal_read (descriptor, offset, buf, sizeof (buf),
				    errmsg, err))
    return 0;

  *value = simple_object_amiga_fetch_16 (buf);
  return 1;
}

static int
simple_object_amiga_write_32 (int descriptor, off_t offset,
			      unsigned long value,
			      const char **errmsg, int *err)
{
  unsigned char buf[4];

  simple_object_amiga_store_32 (buf, value);

  return simple_object_internal_write (descriptor, offset, buf, sizeof (buf),
				       errmsg, err);
}

static const char *
simple_object_amiga_write_name (int descriptor, off_t *offset,
				const char *name, int *err)
{
  const char *errmsg;
  size_t len = strlen (name);
  size_t padded = (len + 3) & ~(size_t) 3;
  unsigned char zero[3] = { 0, 0, 0 };

  if (!simple_object_amiga_write_32 (descriptor, *offset, padded / 4,
				     &errmsg, err))
    return errmsg;
  *offset += 4;

  if (len != 0
      && !simple_object_internal_write (descriptor, *offset,
					(const unsigned char *) name, len,
					&errmsg, err))
    return errmsg;
  *offset += len;

  if (padded != len
      && !simple_object_internal_write (descriptor, *offset, zero,
					padded - len, &errmsg, err))
    return errmsg;
  *offset += padded - len;

  return NULL;
}

static struct simple_object_amiga_section *
simple_object_amiga_add_section (struct simple_object_amiga_read *amiga,
				 char *name, off_t offset, off_t length,
				 unsigned int hunk_index)
{
  struct simple_object_amiga_section *section;

  section = XNEW (struct simple_object_amiga_section);
  section->name = name;
  section->offset = offset;
  section->length = length;
  section->hunk_index = hunk_index;
  section->records = NULL;
  section->record_tail = &section->records;
  section->next = NULL;
  *amiga->tail = section;
  amiga->tail = &section->next;
  return section;
}

static void
simple_object_amiga_add_record (struct simple_object_amiga_section *section,
				unsigned long type, off_t offset,
				off_t length)
{
  struct simple_object_amiga_record *record;

  if (section == NULL)
    return;

  record = XNEW (struct simple_object_amiga_record);
  record->type = type;
  record->offset = offset;
  record->length = length;
  record->next = NULL;
  *section->record_tail = record;
  section->record_tail = &record->next;
}

static int
simple_object_amiga_is_lto_data_section (const char *name)
{
  return strncmp (name, ".gnu.lto", 8) == 0;
}

static int
simple_object_amiga_is_lto_debug_section (const char *name)
{
  return strncmp (name, ".gnu.debuglto", 13) == 0;
}

static int
simple_object_amiga_is_lto_section (const char *name)
{
  return (simple_object_amiga_is_lto_data_section (name)
	  || simple_object_amiga_is_lto_debug_section (name));
}

static int
simple_object_amiga_is_padded_lto_section (const char *name)
{
  if (!simple_object_amiga_is_lto_data_section (name))
    return 0;

  if (strcmp (name, ".gnu.lto_.opts") == 0)
    return 0;

  if (strncmp (name, ".gnu.lto_.symtab", 16) == 0
      || strncmp (name, ".gnu.lto_.ext_symtab", 20) == 0
      || strncmp (name, ".gnu.lto_.lto", 13) == 0)
    return 0;

  return 1;
}

static off_t
simple_object_amiga_trim_lto_padding (int descriptor, off_t offset,
				      off_t length,
				      const char **errmsg, int *err)
{
  unsigned char buf[3];
  size_t count;

  if (length <= 0)
    return length;

  count = length < 3 ? (size_t) length : 3;
  if (!simple_object_internal_read (descriptor, offset + length - count,
				    buf, count, errmsg, err))
    return length;

  while (count != 0 && buf[count - 1] == 0)
    {
      --count;
      --length;
    }

  return length;
}

static int
simple_object_amiga_read_lto_header (int descriptor, off_t *offset,
				     off_t *length,
				     const char **errmsg, int *err)
{
  unsigned char header[AMIGA_LTO_DEBUG_HEADER_SIZE];
  unsigned long magic;
  unsigned long payload_length;

  if (*length < AMIGA_LTO_DEBUG_HEADER_SIZE)
    return 0;

  if (!simple_object_internal_read (descriptor, *offset, header,
				    sizeof (header), errmsg, err))
    return -1;

  magic = simple_object_amiga_fetch_32 (header);
  if (magic != AMIGA_LTO_DEBUG_MAGIC)
    return 0;

  payload_length = simple_object_amiga_fetch_32 (header + 4);
  if ((off_t) payload_length > *length - AMIGA_LTO_DEBUG_HEADER_SIZE)
    {
      *errmsg = "invalid Amiga LTO debug hunk length";
      *err = 0;
      return -1;
    }

  *offset += AMIGA_LTO_DEBUG_HEADER_SIZE;
  *length = payload_length;
  return 1;
}

static char *
simple_object_amiga_read_name (int descriptor, off_t *offset,
			       const char **errmsg, int *err)
{
  unsigned long name_words;
  size_t name_len;
  char *name;

  if (!simple_object_amiga_read_32 (descriptor, *offset, &name_words,
				    errmsg, err))
    return NULL;
  *offset += 4;

  name_len = HUNK_VALUE (name_words) * 4;
  name = XNEWVEC (char, name_len + 1);
  if (name_len != 0
      && !simple_object_internal_read (descriptor, *offset,
				       (unsigned char *) name, name_len,
				       errmsg, err))
    {
      XDELETEVEC (name);
      return NULL;
    }

  name[name_len] = '\0';
  *offset += name_len;
  return name;
}

static int
simple_object_amiga_skip_ext (int descriptor, off_t *offset,
			      const char **errmsg, int *err)
{
  unsigned long entry;

  if (!simple_object_amiga_read_32 (descriptor, *offset, &entry, errmsg, err))
    return 0;
  *offset += 4;

  while (entry != 0)
    {
      unsigned long len = entry & 0x00ffffff;
      unsigned long type = (entry >> 24) & 0xff;
      unsigned long count;

      if (type < 200)
	type &= ~0x60;

      switch (type)
	{
	case EXT_SYMB:
	case EXT_DEF:
	case EXT_ABS:
	  *offset += (len + 1) * 4;
	  break;

	case EXT_ABSREF32:
	case EXT_RELREF16:
	case EXT_RELREF8:
	case EXT_DEXT32:
	case EXT_DEXT16:
	case EXT_DEXT8:
	case EXT_RELREF32:
	case EXT_ABSREF16:
	case EXT_ABSREF8:
	case EXT_RELREF26:
	  *offset += len * 4;
	  if (!simple_object_amiga_read_32 (descriptor, *offset, &count,
					    errmsg, err))
	    return 0;
	  *offset += 4 + count * 4;
	  break;

	case EXT_COMMON:
	case EXT_RELCOMMON:
	case EXT_DEXT32COMMON:
	case EXT_DEXT16COMMON:
	case EXT_DEXT8COMMON:
	  *offset += (len + 1) * 4;
	  if (!simple_object_amiga_read_32 (descriptor, *offset, &count,
					    errmsg, err))
	    return 0;
	  *offset += 4 + count * 4;
	  break;

	default:
	  return 0;
	}

      if (!simple_object_amiga_read_32 (descriptor, *offset, &entry,
					errmsg, err))
	return 0;
      *offset += 4;
    }

  return 1;
}

static int
simple_object_amiga_skip_relocs (int descriptor, off_t *offset,
				 const char **errmsg, int *err)
{
  unsigned long count;

  if (!simple_object_amiga_read_32 (descriptor, *offset, &count, errmsg, err))
    return 0;
  *offset += 4;

  while (count != 0)
    {
      *offset += 4 + count * 4;
      if (!simple_object_amiga_read_32 (descriptor, *offset, &count,
					errmsg, err))
	return 0;
      *offset += 4;
    }

  return 1;
}

static int
simple_object_amiga_skip_short_relocs (int descriptor, off_t *offset,
				       const char **errmsg, int *err)
{
  unsigned int count;

  if (!simple_object_amiga_read_16 (descriptor, *offset, &count, errmsg, err))
    return 0;
  *offset += 2;

  while (count != 0)
    {
      *offset += (count + 1) * 2;
      if (!simple_object_amiga_read_16 (descriptor, *offset, &count,
					errmsg, err))
	return 0;
      *offset += 2;
    }

  if ((*offset & 2) != 0)
    *offset += 2;

  return 1;
}

static int
simple_object_amiga_skip_symbols (int descriptor, off_t *offset,
				  const char **errmsg, int *err)
{
  unsigned long name_words;

  if (!simple_object_amiga_read_32 (descriptor, *offset, &name_words,
				    errmsg, err))
    return 0;
  *offset += 4;

  while (name_words != 0)
    {
      *offset += (name_words + 1) * 4;
      if (!simple_object_amiga_read_32 (descriptor, *offset, &name_words,
					errmsg, err))
	return 0;
      *offset += 4;
    }

  return 1;
}

static void
simple_object_amiga_release_sections (struct simple_object_amiga_read *amiga)
{
  struct simple_object_amiga_section *section = amiga->sections;

  while (section != NULL)
    {
      struct simple_object_amiga_section *next = section->next;
      struct simple_object_amiga_record *record = section->records;

      while (record != NULL)
	{
	  struct simple_object_amiga_record *next_record = record->next;
	  XDELETE (record);
	  record = next_record;
	}
      XDELETEVEC (section->name);
      XDELETE (section);
      section = next;
    }
}

static void *
simple_object_amiga_match (unsigned char header[SIMPLE_OBJECT_MATCH_HEADER_LEN],
			   int descriptor, off_t offset,
			   const char *segment_name ATTRIBUTE_UNUSED,
			   const char **errmsg, int *err)
{
  struct simple_object_amiga_read *amiga;
  struct simple_object_amiga_section *current_section;
  char *name;
  off_t pos;
  unsigned long hunk_type;

  if (simple_object_amiga_fetch_32 (header) != HUNK_UNIT)
    {
      *errmsg = NULL;
      *err = 0;
      return NULL;
    }

  amiga = XNEW (struct simple_object_amiga_read);
  amiga->sections = NULL;
  amiga->tail = &amiga->sections;
  amiga->hunk_count = 0;
  current_section = NULL;

  pos = offset + 4;
  name = simple_object_amiga_read_name (descriptor, &pos, errmsg, err);
  if (name == NULL)
    {
      XDELETE (amiga);
      return NULL;
    }
  XDELETEVEC (name);
  name = NULL;

  while (simple_object_amiga_read_32 (descriptor, pos, &hunk_type,
				      errmsg, err))
    {
      unsigned long hunk_value = HUNK_VALUE (hunk_type);
      unsigned long hunk_size;

      pos += 4;
      switch (hunk_value)
	{
	case HUNK_NAME:
	  if (name != NULL)
	    XDELETEVEC (name);
	  name = simple_object_amiga_read_name (descriptor, &pos, errmsg, err);
	  if (name == NULL)
	    goto fail;
	  break;

	case HUNK_CODE:
	case HUNK_DATA:
	case HUNK_DEBUG:
	  if (!simple_object_amiga_read_32 (descriptor, pos, &hunk_size,
					    errmsg, err))
	    goto fail;
	  pos += 4;
	  hunk_size = HUNK_VALUE (hunk_size) * 4;
	  current_section = NULL;
	  if (name != NULL && name[0] != '\0'
	      && (hunk_value == HUNK_DEBUG
		  || simple_object_amiga_is_lto_debug_section (name)))
	    {
	      off_t section_offset = pos;
	      off_t length = hunk_size;
	      int has_lto_header = 0;

	      if (hunk_value == HUNK_DEBUG
		  && simple_object_amiga_is_lto_section (name))
		{
		  has_lto_header =
		    simple_object_amiga_read_lto_header (descriptor,
							 &section_offset,
							 &length,
							 errmsg, err);
		  if (has_lto_header < 0)
		    goto fail;
		}
	      if (has_lto_header == 0
		  && simple_object_amiga_is_padded_lto_section (name))
		length =
		  simple_object_amiga_trim_lto_padding (descriptor,
							section_offset,
							length,
							errmsg, err);
	      current_section = simple_object_amiga_add_section (
		amiga, name, section_offset - offset, length,
		amiga->hunk_count);
	      name = NULL;
	    }
	  amiga->hunk_count++;
	  pos += hunk_size;
	  if (name != NULL)
	    {
	      XDELETEVEC (name);
	      name = NULL;
	    }
	  break;

	case HUNK_BSS:
	  pos += 4;
	  current_section = NULL;
	  amiga->hunk_count++;
	  if (name != NULL)
	    {
	      XDELETEVEC (name);
	      name = NULL;
	    }
	  break;

	case HUNK_RELOC32:
	case HUNK_RELOC16:
	case HUNK_RELOC8:
	case HUNK_DREL32:
	case HUNK_DREL16:
	case HUNK_DREL8:
	case HUNK_RELRELOC32:
	case HUNK_ABSRELOC16:
	case HUNK_RELRELOC26:
	  {
	    off_t record_offset = pos;
	    if (!simple_object_amiga_skip_relocs (descriptor, &pos,
						    errmsg, err))
	      goto fail;
	    simple_object_amiga_add_record (current_section, hunk_value,
					    record_offset - offset,
					    pos - record_offset);
	  }
	  break;

	case HUNK_RELOC32SHORT:
	  {
	    off_t record_offset = pos;
	    if (!simple_object_amiga_skip_short_relocs (descriptor, &pos,
							  errmsg, err))
	      goto fail;
	    simple_object_amiga_add_record (current_section, hunk_value,
					    record_offset - offset,
					    pos - record_offset);
	  }
	  break;

	case HUNK_SYMBOL:
	  {
	    off_t record_offset = pos;
	    if (!simple_object_amiga_skip_symbols (descriptor, &pos,
						     errmsg, err))
	      goto fail;
	    simple_object_amiga_add_record (current_section, hunk_value,
					    record_offset - offset,
					    pos - record_offset);
	  }
	  break;

	case HUNK_EXT:
	  {
	    off_t record_offset = pos;
	    if (!simple_object_amiga_skip_ext (descriptor, &pos, errmsg, err))
	      goto fail;
	    simple_object_amiga_add_record (current_section, hunk_value,
					    record_offset - offset,
					    pos - record_offset);
	  }
	  break;

	case HUNK_END:
	  current_section = NULL;
	  if (name != NULL)
	    {
	      XDELETEVEC (name);
	      name = NULL;
	    }
	  break;

	default:
	  goto done;
	}
    }

done:
  if (name != NULL)
    XDELETEVEC (name);
  *errmsg = NULL;
  *err = 0;
  return amiga;

fail:
  if (name != NULL)
    XDELETEVEC (name);
  simple_object_amiga_release_sections (amiga);
  XDELETE (amiga);
  return NULL;
}

static const char *
simple_object_amiga_find_sections (simple_object_read *sobj,
				   int (*pfn) (void *, const char *,
					       off_t offset, off_t length),
				   void *data, int *err ATTRIBUTE_UNUSED)
{
  struct simple_object_amiga_read *amiga =
    (struct simple_object_amiga_read *) sobj->data;
  struct simple_object_amiga_section *section;

  for (section = amiga->sections; section != NULL; section = section->next)
    if (!(*pfn) (data, section->name, section->offset, section->length))
      break;

  return NULL;
}

static void *
simple_object_amiga_fetch_attributes (simple_object_read *sobj ATTRIBUTE_UNUSED,
				      const char **errmsg ATTRIBUTE_UNUSED,
				      int *err ATTRIBUTE_UNUSED)
{
  return XNEW (struct simple_object_amiga_attributes);
}

static void
simple_object_amiga_release_read (void *data)
{
  struct simple_object_amiga_read *amiga =
    (struct simple_object_amiga_read *) data;

  simple_object_amiga_release_sections (amiga);
  XDELETE (amiga);
}

static const char *
simple_object_amiga_attributes_merge (void *todata ATTRIBUTE_UNUSED,
				      void *fromdata ATTRIBUTE_UNUSED,
				      int *err ATTRIBUTE_UNUSED)
{
  return NULL;
}

static void
simple_object_amiga_release_attributes (void *data)
{
  XDELETE (data);
}

static void *
simple_object_amiga_start_write (void *attributes_data ATTRIBUTE_UNUSED,
				 const char **errmsg ATTRIBUTE_UNUSED,
				 int *err ATTRIBUTE_UNUSED)
{
  struct simple_object_amiga_write *amiga;

  amiga = XNEW (struct simple_object_amiga_write);
  amiga->sections = NULL;
  amiga->tail = &amiga->sections;
  return amiga;
}

static const char *
simple_object_amiga_remap_relocs (unsigned long type, unsigned char *data,
				  size_t length, const unsigned int *map,
				  unsigned int map_count, int *err)
{
  size_t pos = 0;

  if (type == HUNK_RELOC32SHORT)
    {
      while (1)
	{
	  unsigned int count;
	  unsigned int target;

	  if (length - pos < 2)
	    goto invalid;
	  count = simple_object_amiga_fetch_16 (data + pos);
	  pos += 2;
	  if (count == 0)
	    break;
	  if (length - pos < 2 || count > (length - pos - 2) / 2)
	    goto invalid;
	  target = simple_object_amiga_fetch_16 (data + pos);
	  if (target >= map_count || map[target] >= map_count)
	    goto discarded;
	  simple_object_amiga_store_16 (data + pos, map[target]);
	  pos += (count + 1) * 2;
	}

      if (length - pos > 2)
	goto invalid;
      return NULL;
    }

  while (1)
    {
      unsigned long count;
      unsigned long target;

      if (length - pos < 4)
	goto invalid;
      count = simple_object_amiga_fetch_32 (data + pos);
      pos += 4;
      if (count == 0)
	break;
      if (length - pos < 4 || count > (length - pos - 4) / 4)
	goto invalid;
      target = simple_object_amiga_fetch_32 (data + pos);
      if (target >= map_count || map[target] >= map_count)
	goto discarded;
      simple_object_amiga_store_32 (data + pos, map[target]);
      pos += (count + 1) * 4;
    }

  if (pos != length)
    goto invalid;
  return NULL;

invalid:
  *err = 0;
  return "invalid Amiga relocation hunk";

discarded:
  *err = 0;
  return "Amiga LTO debug relocation references a discarded section";
}

/* Return nonzero for records whose section indexes need remapping when a
   subset of the input hunks is copied.  */

static int
simple_object_amiga_is_reloc_hunk (unsigned long type)
{
  switch (type)
    {
    case HUNK_RELOC32:
    case HUNK_RELOC16:
    case HUNK_RELOC8:
    case HUNK_DREL32:
    case HUNK_DREL16:
    case HUNK_DREL8:
    case HUNK_RELOC32SHORT:
    case HUNK_RELRELOC32:
    case HUNK_ABSRELOC16:
    case HUNK_RELRELOC26:
      return 1;

    default:
      return 0;
    }
}

static const char *
simple_object_amiga_copy_record (
  simple_object_read *sobj, struct simple_object_amiga_record *record,
  struct simple_object_amiga_write_section *write_section,
  const unsigned int *map, unsigned int map_count, int *err)
{
  struct simple_object_amiga_write_record *write_record;
  const char *errmsg;

  write_record = XNEW (struct simple_object_amiga_write_record);
  write_record->type = record->type;
  write_record->length = record->length;
  write_record->data = XNEWVEC (unsigned char, write_record->length);
  write_record->next = NULL;

  if (!simple_object_internal_read (sobj->descriptor,
				    sobj->offset + record->offset,
				    write_record->data,
				    write_record->length, &errmsg, err))
    {
      XDELETEVEC (write_record->data);
      XDELETE (write_record);
      return errmsg;
    }

  if (simple_object_amiga_is_reloc_hunk (record->type))
    {
      errmsg = simple_object_amiga_remap_relocs (
	record->type, write_record->data, write_record->length,
	map, map_count, err);
      if (errmsg != NULL)
	{
	  XDELETEVEC (write_record->data);
	  XDELETE (write_record);
	  return errmsg;
	}
    }

  *write_section->record_tail = write_record;
  write_section->record_tail = &write_record->next;
  return NULL;
}

static const char *
simple_object_amiga_copy_lto_debug_sections (simple_object_read *sobj,
					     simple_object_write *dobj,
					     char *(*pfn) (const char *),
					     int *err)
{
  struct simple_object_amiga_read *read_amiga =
    (struct simple_object_amiga_read *) sobj->data;
  struct simple_object_amiga_write *write_amiga =
    (struct simple_object_amiga_write *) dobj->data;
  struct simple_object_amiga_section *section;
  unsigned int *map;
  char **names;
  unsigned int i;
  unsigned int new_index = 0;
  const char *errmsg = NULL;

  map = XNEWVEC (unsigned int, read_amiga->hunk_count);
  names = XCNEWVEC (char *, read_amiga->hunk_count);
  for (i = 0; i < read_amiga->hunk_count; i++)
    map[i] = read_amiga->hunk_count;

  /* Ask the generic LTO callback which sections to keep, then build the
     old-to-new HUNK index map needed by their relocation records.  */
  for (section = read_amiga->sections; section != NULL;
	 section = section->next)
    {
      char *name = (*pfn) (section->name);

      if (name != NULL)
	{
	  map[section->hunk_index] = new_index++;
	  names[section->hunk_index] = name;
	}
    }

  for (section = read_amiga->sections; section != NULL;
	 section = section->next)
    if (names[section->hunk_index] != NULL)
      {
	struct simple_object_amiga_write_section *write_section;
	struct simple_object_amiga_record *record;
	simple_object_write_section *dest;
	unsigned char *data = NULL;

	dest = simple_object_write_create_section (
	  dobj, names[section->hunk_index], 2, &errmsg, err);
	free (names[section->hunk_index]);
	names[section->hunk_index] = NULL;
	if (dest == NULL)
	  goto done;

	if (section->length != 0)
	  {
	    data = XNEWVEC (unsigned char, section->length);
	    if (!simple_object_internal_read (sobj->descriptor,
					      sobj->offset + section->offset,
					      data, section->length,
					      &errmsg, err))
	      {
		XDELETEVEC (data);
		goto done;
	      }

	    errmsg = simple_object_write_add_data (dobj, dest, data,
						     section->length, 1, err);
	    XDELETEVEC (data);
	    if (errmsg != NULL)
	      goto done;
	  }

	write_section = XNEW (struct simple_object_amiga_write_section);
	write_section->section = dest;
	write_section->records = NULL;
	write_section->record_tail = &write_section->records;
	write_section->next = NULL;
	*write_amiga->tail = write_section;
	write_amiga->tail = &write_section->next;

	for (record = section->records; record != NULL;
	     record = record->next)
	  {
	    errmsg = simple_object_amiga_copy_record (
	      sobj, record, write_section, map,
	      read_amiga->hunk_count, err);
	    if (errmsg != NULL)
	      goto done;
	  }
      }

done:
  for (i = 0; i < read_amiga->hunk_count; i++)
    free (names[i]);
  XDELETEVEC (names);
  XDELETEVEC (map);
  return errmsg;
}

static const char *
simple_object_amiga_write_to_file (simple_object_write *sobj, int descriptor,
				   int *err)
{
  struct simple_object_amiga_write *amiga =
    (struct simple_object_amiga_write *) sobj->data;
  struct simple_object_amiga_write_section *write_section = amiga->sections;
  simple_object_write_section *section;
  const char *errmsg;
  off_t offset = 0;

  if (!simple_object_amiga_write_32 (descriptor, offset, HUNK_UNIT, &errmsg,
				     err))
    return errmsg;
  offset += 4;

  errmsg = simple_object_amiga_write_name (descriptor, &offset, "", err);
  if (errmsg != NULL)
    return errmsg;

  for (section = sobj->sections; section != NULL; section = section->next)
    {
      struct simple_object_write_section_buffer *buffer;
      size_t size = 0;
      size_t lto_header_size = 0;
      size_t hunk_size;
      size_t pad;
      unsigned char zero[3] = { 0, 0, 0 };

      for (buffer = section->buffers; buffer != NULL; buffer = buffer->next)
	size += buffer->size;
      if (simple_object_amiga_is_lto_section (section->name))
	lto_header_size = AMIGA_LTO_DEBUG_HEADER_SIZE;
      hunk_size = size + lto_header_size;

      if (!simple_object_amiga_write_32 (descriptor, offset, HUNK_NAME,
					 &errmsg, err))
	return errmsg;
      offset += 4;

      errmsg = simple_object_amiga_write_name (descriptor, &offset,
					       section->name, err);
      if (errmsg != NULL)
	return errmsg;

      if (!simple_object_amiga_write_32 (descriptor, offset, HUNK_DEBUG,
					 &errmsg, err))
	return errmsg;
      offset += 4;

      if (!simple_object_amiga_write_32 (descriptor, offset,
					 (hunk_size + 3) / 4,
					 &errmsg, err))
	return errmsg;
      offset += 4;

      if (lto_header_size != 0)
	{
	  if (!simple_object_amiga_write_32 (descriptor, offset,
					     AMIGA_LTO_DEBUG_MAGIC,
					     &errmsg, err))
	    return errmsg;
	  offset += 4;
	  if (!simple_object_amiga_write_32 (descriptor, offset, size,
					     &errmsg, err))
	    return errmsg;
	  offset += 4;
	}

      for (buffer = section->buffers; buffer != NULL; buffer = buffer->next)
	{
	  if (buffer->size != 0
	      && !simple_object_internal_write (descriptor, offset,
						(const unsigned char *)
						buffer->buffer,
						buffer->size, &errmsg, err))
	    return errmsg;
	  offset += buffer->size;
	}

      pad = ((hunk_size + 3) & ~(size_t) 3) - hunk_size;
      if (pad != 0
	  && !simple_object_internal_write (descriptor, offset, zero, pad,
					    &errmsg, err))
	return errmsg;
      offset += pad;

      if (write_section != NULL && write_section->section == section)
	{
	  struct simple_object_amiga_write_record *record;

	  for (record = write_section->records; record != NULL;
	       record = record->next)
	    {
	      if (!simple_object_amiga_write_32 (descriptor, offset,
						 record->type, &errmsg, err))
		return errmsg;
	      offset += 4;
	      if (record->length != 0
		  && !simple_object_internal_write (descriptor, offset,
						    record->data,
						    record->length,
						    &errmsg, err))
		return errmsg;
	      offset += record->length;
	    }
	  write_section = write_section->next;
	}

      if (!simple_object_amiga_write_32 (descriptor, offset, HUNK_END,
					 &errmsg, err))
	return errmsg;
      offset += 4;
    }

  return NULL;
}

static void
simple_object_amiga_release_write (void *data)
{
  struct simple_object_amiga_write *amiga =
    (struct simple_object_amiga_write *) data;
  struct simple_object_amiga_write_section *section = amiga->sections;

  while (section != NULL)
    {
      struct simple_object_amiga_write_section *next_section = section->next;
      struct simple_object_amiga_write_record *record = section->records;

      while (record != NULL)
	{
	  struct simple_object_amiga_write_record *next_record = record->next;
	  XDELETEVEC (record->data);
	  XDELETE (record);
	  record = next_record;
	}
      XDELETE (section);
      section = next_section;
    }
  XDELETE (amiga);
}

const struct simple_object_functions simple_object_amiga_functions =
{
  simple_object_amiga_match,
  simple_object_amiga_find_sections,
  simple_object_amiga_fetch_attributes,
  simple_object_amiga_release_read,
  simple_object_amiga_attributes_merge,
  simple_object_amiga_release_attributes,
  simple_object_amiga_start_write,
  simple_object_amiga_write_to_file,
  simple_object_amiga_release_write,
  simple_object_amiga_copy_lto_debug_sections
};
