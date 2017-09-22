/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <rte_common.h>

#include "rte_cfgfile.h"

struct rte_cfgfile_section {
	char name[CFG_NAME_LEN];
	int num_entries;
	int allocated_entries;
	struct rte_cfgfile_entry *entries;
};

struct rte_cfgfile {
	int flags;
	int num_sections;
	int allocated_sections;
	struct rte_cfgfile_section *sections;
};

/** when we resize a file structure, how many extra entries
 * for new sections do we add in */
#define CFG_ALLOC_SECTION_BATCH 8
/** when we resize a section structure, how many extra entries
 * for new entries do we add in */
#define CFG_ALLOC_ENTRY_BATCH 16

/**
 * Default cfgfile load parameters.
 */
static const struct rte_cfgfile_parameters default_cfgfile_params = {
	.comment_character = CFG_DEFAULT_COMMENT_CHARACTER,
};

/**
 * Defines the list of acceptable comment characters supported by this
 * library.
 */
static const char valid_comment_chars[] = {
	'!',
	'#',
	'%',
	';',
	'@'
};

static unsigned
_strip(char *str, unsigned len)
{
	int newlen = len;
	if (len == 0)
		return 0;

	if (isspace(str[len-1])) {
		/* strip trailing whitespace */
		while (newlen > 0 && isspace(str[newlen - 1]))
			str[--newlen] = '\0';
	}

	if (isspace(str[0])) {
		/* strip leading whitespace */
		int i, start = 1;
		while (isspace(str[start]) && start < newlen)
			start++
			; /* do nothing */
		newlen -= start;
		for (i = 0; i < newlen; i++)
			str[i] = str[i+start];
		str[i] = '\0';
	}
	return newlen;
}

static struct rte_cfgfile_section *
_get_section(struct rte_cfgfile *cfg, const char *sectionname)
{
	int i;

	for (i = 0; i < cfg->num_sections; i++) {
		if (strncmp(cfg->sections[i].name, sectionname,
				sizeof(cfg->sections[0].name)) == 0)
			return &cfg->sections[i];
	}
	return NULL;
}

static int
rte_cfgfile_check_params(const struct rte_cfgfile_parameters *params)
{
	unsigned int valid_comment;
	unsigned int i;

	if (!params) {
		printf("Error - missing cfgfile parameters\n");
		return -EINVAL;
	}

	valid_comment = 0;
	for (i = 0; i < RTE_DIM(valid_comment_chars); i++) {
		if (params->comment_character == valid_comment_chars[i]) {
			valid_comment = 1;
			break;
		}
	}

	if (valid_comment == 0)	{
		printf("Error - invalid comment characters %c\n",
		       params->comment_character);
		return -ENOTSUP;
	}

	return 0;
}

struct rte_cfgfile *
rte_cfgfile_load(const char *filename, int flags)
{
	return rte_cfgfile_load_with_params(filename, flags,
					    &default_cfgfile_params);
}

struct rte_cfgfile *
rte_cfgfile_load_with_params(const char *filename, int flags,
			     const struct rte_cfgfile_parameters *params)
{
	int allocated_sections = CFG_ALLOC_SECTION_BATCH;
	int allocated_entries = 0;
	int curr_section = -1;
	int curr_entry = -1;
	char buffer[CFG_NAME_LEN + CFG_VALUE_LEN + 4] = {0};
	int lineno = 0;
	struct rte_cfgfile *cfg = NULL;

	if (rte_cfgfile_check_params(params))
		return NULL;

	FILE *f = fopen(filename, "r");
	if (f == NULL)
		return NULL;

	cfg = malloc(sizeof(*cfg) + sizeof(cfg->sections[0]) *
		allocated_sections);
	if (cfg == NULL)
		goto error2;

	memset(cfg->sections, 0, sizeof(cfg->sections[0]) * allocated_sections);

	if (flags & CFG_FLAG_GLOBAL_SECTION) {
		curr_section = 0;
		allocated_entries = CFG_ALLOC_ENTRY_BATCH;
		cfg->sections = malloc(
			sizeof(cfg->sections[0]) +
			sizeof(cfg->sections[0].entries) *
			allocated_entries);
		if (cfg->sections == NULL) {
			printf("Error - no memory for global section\n");
			goto error1;
		}

		snprintf(cfg->sections[curr_section].name,
				 sizeof(cfg->sections[0].name), "GLOBAL");
	}

	while (fgets(buffer, sizeof(buffer), f) != NULL) {
		char *pos = NULL;
		size_t len = strnlen(buffer, sizeof(buffer));
		lineno++;
		if ((len >= sizeof(buffer) - 1) && (buffer[len-1] != '\n')) {
			printf("Error line %d - no \\n found on string. "
					"Check if line too long\n", lineno);
			goto error1;
		}
		pos = memchr(buffer, params->comment_character, len);
		if (pos != NULL) {
			*pos = '\0';
			len = pos -  buffer;
		}

		len = _strip(buffer, len);
		if (buffer[0] != '[' && memchr(buffer, '=', len) == NULL)
			continue;

		if (buffer[0] == '[') {
			/* section heading line */
			char *end = memchr(buffer, ']', len);
			if (end == NULL) {
				printf("Error line %d - no terminating '['"
					"character found\n", lineno);
				goto error1;
			}
			*end = '\0';
			_strip(&buffer[1], end - &buffer[1]);

			/* close off old section and add start new one */
			if (curr_section >= 0)
				cfg->sections[curr_section].num_entries =
					curr_entry + 1;
			curr_section++;

			/* resize overall struct if we don't have room for more
			sections */
			if (curr_section == allocated_sections) {
				allocated_sections += CFG_ALLOC_SECTION_BATCH;
				struct rte_cfgfile *n_cfg = realloc(cfg,
					sizeof(*cfg) + sizeof(cfg->sections[0])
					* allocated_sections);
				if (n_cfg == NULL) {
					curr_section--;
					printf("Error - no more memory\n");
					goto error1;
				}
				cfg = n_cfg;
			}

			/* allocate space for new section */
			allocated_entries = CFG_ALLOC_ENTRY_BATCH;
			curr_entry = -1;
			cfg->sections = malloc(
				sizeof(cfg->sections[0]) +
				sizeof(cfg->sections[0].entries) *
				allocated_entries);
			if (cfg->sections == NULL) {
				printf("Error - no more memory\n");
				goto error1;
			}

			snprintf(cfg->sections[curr_section].name,
					sizeof(cfg->sections[0].name),
					"%s", &buffer[1]);
		} else {
			/* value line */
			if (curr_section < 0) {
				printf("Error line %d - value outside of"
					"section\n", lineno);
				goto error1;
			}

			struct rte_cfgfile_section *sect = cfg->sections;

			char *split[2] = {NULL};
			split[0] = buffer;
			split[1] = memchr(buffer, '=', len);

			/* when delimeter not found */
			if (split[1] == NULL) {
				printf("Error at line %d - cannot "
					"split string\n", lineno);
				goto error1;
			} else {
				/* when delimeter found */
				*split[1] = '\0';
				split[1]++;

				if (!(flags & CFG_FLAG_EMPTY_VALUES) &&
						(*split[1] == '\0')) {
					printf("Error at line %d - cannot "
						"split string\n", lineno);
					goto error1;
				}
			}

			curr_entry++;
			if (curr_entry == allocated_entries) {
				allocated_entries += CFG_ALLOC_ENTRY_BATCH;
				struct rte_cfgfile_section *n_sect = realloc(
					sect, sizeof(*sect) +
					sizeof(sect->entries[0]) *
					allocated_entries);
				if (n_sect == NULL) {
					curr_entry--;
					printf("Error - no more memory\n");
					goto error1;
				}
				sect = cfg->sections = n_sect;
			}

			sect->entries = malloc(
				sizeof(sect->entries[0]));
			if (sect->entries == NULL) {
				printf("Error - no more memory\n");
				goto error1;
			}

			struct rte_cfgfile_entry *entry = sect->entries;
			snprintf(entry->name, sizeof(entry->name), "%s",
				split[0]);
			snprintf(entry->value, sizeof(entry->value), "%s",
				 split[1] ? split[1] : "");
			_strip(entry->name, strnlen(entry->name,
				sizeof(entry->name)));
			_strip(entry->value, strnlen(entry->value,
				sizeof(entry->value)));
		}
	}
	fclose(f);
	cfg->flags = flags;
	cfg->num_sections = curr_section + 1;
	/* curr_section will still be -1 if we have an empty file */
	if (curr_section >= 0)
		cfg->sections[curr_section].num_entries = curr_entry + 1;
	return cfg;

error1:
	cfg->num_sections = curr_section + 1;
	if (curr_section >= 0)
		cfg->sections[curr_section].num_entries = curr_entry + 1;
	rte_cfgfile_close(cfg);
error2:
	fclose(f);
	return NULL;
}

int rte_cfgfile_close(struct rte_cfgfile *cfg)
{
	int i;

	if (cfg == NULL)
		return -1;

	if (cfg->sections != NULL) {
		for (i = 0; i < cfg->allocated_sections; i++) {
			if (cfg->sections[i].entries != NULL) {
				free(cfg->sections[i].entries);
				cfg->sections[i].entries = NULL;
			}
		}
		free(cfg->sections);
		cfg->sections = NULL;
	}
	free(cfg);
	cfg = NULL;

	return 0;
}

int
rte_cfgfile_num_sections(struct rte_cfgfile *cfg, const char *sectionname,
size_t length)
{
	int i;
	int num_sections = 0;
	for (i = 0; i < cfg->num_sections; i++) {
		if (strncmp(cfg->sections[i].name, sectionname, length) == 0)
			num_sections++;
	}
	return num_sections;
}

int
rte_cfgfile_sections(struct rte_cfgfile *cfg, char *sections[],
	int max_sections)
{
	int i;

	for (i = 0; i < cfg->num_sections && i < max_sections; i++)
		snprintf(sections[i], CFG_NAME_LEN, "%s",
		cfg->sections[i].name);

	return i;
}

int
rte_cfgfile_has_section(struct rte_cfgfile *cfg, const char *sectionname)
{
	return _get_section(cfg, sectionname) != NULL;
}

int
rte_cfgfile_section_num_entries(struct rte_cfgfile *cfg,
	const char *sectionname)
{
	const struct rte_cfgfile_section *s = _get_section(cfg, sectionname);
	if (s == NULL)
		return -1;
	return s->num_entries;
}

int
rte_cfgfile_section_num_entries_by_index(struct rte_cfgfile *cfg,
	char *sectionname, int index)
{
	if (index < 0 || index >= cfg->num_sections)
		return -1;

	const struct rte_cfgfile_section *sect = &(cfg->sections[index]);

	snprintf(sectionname, CFG_NAME_LEN, "%s", sect->name);
	return sect->num_entries;
}
int
rte_cfgfile_section_entries(struct rte_cfgfile *cfg, const char *sectionname,
		struct rte_cfgfile_entry *entries, int max_entries)
{
	int i;
	const struct rte_cfgfile_section *sect = _get_section(cfg, sectionname);
	if (sect == NULL)
		return -1;
	for (i = 0; i < max_entries && i < sect->num_entries; i++)
		entries[i] = sect->entries[i];
	return i;
}

int
rte_cfgfile_section_entries_by_index(struct rte_cfgfile *cfg, int index,
		char *sectionname,
		struct rte_cfgfile_entry *entries, int max_entries)
{
	int i;
	const struct rte_cfgfile_section *sect;

	if (index < 0 || index >= cfg->num_sections)
		return -1;
	sect = &cfg->sections[index];
	snprintf(sectionname, CFG_NAME_LEN, "%s", sect->name);
	for (i = 0; i < max_entries && i < sect->num_entries; i++)
		entries[i] = sect->entries[i];
	return i;
}

const char *
rte_cfgfile_get_entry(struct rte_cfgfile *cfg, const char *sectionname,
		const char *entryname)
{
	int i;
	const struct rte_cfgfile_section *sect = _get_section(cfg, sectionname);
	if (sect == NULL)
		return NULL;
	for (i = 0; i < sect->num_entries; i++)
		if (strncmp(sect->entries[i].name, entryname, CFG_NAME_LEN)
									== 0)
			return sect->entries[i].value;
	return NULL;
}

int
rte_cfgfile_has_entry(struct rte_cfgfile *cfg, const char *sectionname,
		const char *entryname)
{
	return rte_cfgfile_get_entry(cfg, sectionname, entryname) != NULL;
}
