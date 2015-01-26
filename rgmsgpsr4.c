#if ___STDC_VERSION__ >= 199901L
#include <stdint.h>
#else
#define SIZE_MAX ((size_t)-1) /* C89 doesn't have stdint.h or SIZE_MAX */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <winsock2.h>

#ifndef MEMBER_SIZE
#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define MEM_BLK_SIZE 128

typedef enum {
    PSR_MSG_NOT_BEGUN,
    PSR_ENTRY_NOT_BEGUN,
    PSR_ENTRY_BEGUN,
    PSR_ENTRY_ENDED,
    PSR_MSG_MIGHT_HAVE_ENDED
} RGMSG_PARSER_STATE;

typedef enum {
    MSG_INCOMPLETE,
    MSG_COMPLETE
} RGMSG_MESSAGE_STATE;

typedef struct rg_msg {
	unsigned char mode;
	unsigned short int  seq;
	unsigned short int  port;
	unsigned short int  len;  // in para
	unsigned short int  command;
	unsigned char  para[64];
} rg_msg;

struct rgmsg_parser {
    int pstate;                 /* parser state */
    unsigned char *entry;       /* buffer containing submited entry data */
    size_t entry_pos;           /* length of submited entry data */
    size_t entry_size;          /* buffer size */
    int status;                 /* error status */
    int options;                /* paser options */
    size_t blk_size;            /* memory block size for entry buffer */
    unsigned char delim_char;   /* entry delimiter character */
};

struct rgmsg_parser_counter {
    int mstate;                 /* parsed message status */
    int entry_counter;          /* counter for message entries */
    void *message;              /* message struct pointer */
    void *parser;               /* parser struct */
    unsigned char *text;        /* message in text string format */
    size_t text_pos;            /* length of text string */
    size_t text_size;           /* text string buffer size */
};

/* Error Codes */
#define PSR_SUCCESS          0
#define PSR_EPARSE           1   /* Parse error in strict mode */
#define PSR_ENOMEM           2   /* Out of memory while increasing buffer size */
#define PSR_ETOOBIG          3   /* Buffer larger than SIZE_MAX needed */
#define PSR_EINVALID         4   /* Invalid code,should never be received*/
#define PSR_EENTRY_LENGTH    5   /* Invalid entry length */
#define PSR_EENTRY_DATA      6   /* Invalid entry data */

#define CHAR_TAB             0x09
#define CHAR_SPACE           0x20
#define CHAR_CR              0x0d
#define CHAR_LF              0x0a
#define CHAR_VLINE           0x7c

#define PSR_ENTRY_APPEND_NULL    (1<<2)

#define PSR_ENTRIES          5   /* Number of entries in a message */

struct rgmsg_parser_entry_handler {
    size_t  offset;
    size_t  size;
    int (*handler)(void *, void *, size_t, void *);
};

struct rgmsg_parser_entry_handler rgmsg_entry_handlers [];

#define SUBMIT_ENTRY(p) \
	do { \
		if (p->options & PSR_ENTRY_APPEND_NULL) \
			((p)->entry[entry_pos]) = '\0'; \
		if (cb1 && entry_pos == 0) \
			pstate = cb1(NULL, entry_pos, data); \
		else if (cb1) \
			pstate = cb1(p->entry, entry_pos, data); \
		else \
			pstate = PSR_ENTRY_ENDED; \
		entry_pos = 0; \
	} while (0)

#define SUBMIT_MESSAGE(p, c) \
	do { \
		if (cb2) \
			pstate = cb2(c, data); \
		else \
			pstate = PSR_MSG_NOT_BEGUN; \
		entry_pos = 0; \
	} while (0)

#define SUBMIT_CHAR(p, c) ((p)->entry[entry_pos++] = (c))


bool rgmsg_mode_check(unsigned char m);


/**
 * @fn u_ascii_to_hex()
 * @brief Convert ASCII string [0-9a-fA-F] to hex chars, 2 bytes to 1 byte
 *
 * @param src Source string where ascii chars come from
 * @param dst Destination where hex chars to be stored
 * @param dstlen Length of hex chars wanted, consumes 2*dstlen chars from src
 * @return 0 - if ok, -1 for error, positive N - N bytes incomplete
 */
int u_ascii_to_hex(unsigned char *src, unsigned char *dst, int dstlen)
{
	unsigned char c;

	if (!src || !dst)
		return -1;

	for (; dstlen>0;) {
		c = *src;
		if(c>='0' && c<='9') {
			c = c - '0';
		}
		else if(c>='A' && c<='F') {
			c = c - 'A' + 0x0A;
		}
		else if(c>='a' && c<='f') {
			c = c - 'a' + 0x0A;
		}
		else {
			/*Invalid char*/
			break;
		}
		/*half byte high*/
		*dst = c<<4;

		c = *(++src);
		if(c>='0' && c<='9') {
			c = c - '0';
		}
		else if(c>='A' && c<='F') {
			c = c - 'A' + 0x0A;
		}
		else if(c>='a' && c<='f') {
			c = c - 'a' + 0x0A;
		}
		else {
			/*Invalid char*/
			break;
		}
		/*half byte low*/
		*dst = (*dst) | c;

		src++;
		dst++;
		dstlen--; /*1 hex char ok*/
	}

	return dstlen;
}

int rg_string_to_short(unsigned char *src, unsigned char *dst)
{
	int rv;
	rv = u_ascii_to_hex(src, dst, 2);
	/*revert bytes order*/
	*(short*)dst = ntohs(*(short*)dst);
	return rv;
}

static char *rgmsg_parser_errors[] = {
    "success",
    "error parsing data while strict checking enabled",
    "memory exhausted while increasing buffer size",
    "data size too large",
    "invalid status code"
};

int rgmsg_parser_error(struct rgmsg_parser *p)
{
    /* Return the current status of the parser */
    return p->status;
}

char *rgmsg_parser_strerror(int status)
{
    /* Return a textual description of status */
    if (status >= PSR_EINVALID || status < 0)
        return rgmsg_parser_errors[PSR_EINVALID];
    else
        return rgmsg_parser_errors[status];
}

int rgmsg_parser_get_opts(struct rgmsg_parser *p)
{
    /* Return the currently set options of parser */
    if (p == NULL)
        return -1;

    return p->options;
}

int rgmsg_parser_set_opts(struct rgmsg_parser *p, int options)
{
    /* Set the options */
    if (p == NULL)
        return -1;

    p->options = options;
    return 0;
}

int rgmsg_parser_init(struct rgmsg_parser *p, int options)
{
    if (p == NULL)
        return -1;

    p->entry       = NULL;
    p->pstate      = PSR_MSG_NOT_BEGUN;
    p->entry_pos   = 0;
    p->entry_size  = 0;
    p->status      = 0;
    p->options     = options;
    p->delim_char  = CHAR_VLINE;
    p->blk_size    = MEM_BLK_SIZE;

    return 0;
}

void rgmsg_parser_free(struct rgmsg_parser *p)
{
    /* Free the entry_buffer of csv_parser object */
    if (p == NULL)
        return;

    if (p->entry)
        free(p->entry);

    p->entry = NULL;
    p->entry_size = 0;

    return;
}

void rgmsg_parser_set_delim(struct rgmsg_parser *p, unsigned char c)
{
    /* Set the delimiter */
    if (p) p->delim_char = c;
}

unsigned char rgmsg_parser_get_delim(struct rgmsg_parser *p)
{
    /* Get the delimiter */
    return p->delim_char;
}

void rgmsg_parser_set_blk_size(struct rgmsg_parser *p, size_t size)
{
    /* Set the block size used to increment buffer size */
    if (p) p->blk_size = size;
}

size_t rgmsg_parser_get_buffer_size(struct rgmsg_parser *p)
{
    /* Get the size of the entry buffer */
    if (p)
        return p->entry_size;
    return 0;
}

static int rgmsg_parser_increase_buffer(struct rgmsg_parser *p)
{
    /* Increase the size of the entry buffer.  Attempt to increase size by
     * p->blk_size, if this is larger than SIZE_MAX try to increase current
     * buffer size to SIZE_MAX.  If allocation fails, try to allocate halve
     * the size and try again until successful or increment size is zero.
     */

    size_t to_add = p->blk_size;
    void *vp;

    if (p->entry_size >= SIZE_MAX - to_add)
        to_add = SIZE_MAX - p->entry_size;

    if (!to_add) {
        p->status = PSR_ETOOBIG;
        return -1;
    }

    while ((vp = realloc(p->entry, p->entry_size + to_add)) == NULL) {
        to_add /= 2;
        if (!to_add) {
            p->status = PSR_ENOMEM;
            return -1;
        }
    }

    /* Update entry buffer pointer and entry_size if successful */
    p->entry = vp;
    p->entry_size += to_add;
    return 0;
}

static int rgmsg_parser_counter_increase_buffer(struct rgmsg_parser_counter *c)
{
    /* Increase the size of the text buffer.  Attempt to increase size by
     * MEM_BLK_SIZE, if this is larger than SIZE_MAX try to increase current
     * buffer size to SIZE_MAX.  If allocation fails, try to allocate halve
     * the size and try again until successful or increment size is zero.
     */

    size_t to_add = MEM_BLK_SIZE;
    void *vp;

    if (c->text_size >= SIZE_MAX - to_add)
        to_add = SIZE_MAX - c->text_size;

    if (!to_add) {
        return -1;
    }

    while ((vp = realloc(c->text, c->text_size + to_add)) == NULL) {
        to_add /= 2;
        if (!to_add) {
            return -1;
        }
    }

    /* Update text buffer pointer and text_size if successful */
    c->text = vp;
    c->text_size += to_add;
    return 0;
}

bool rgmsg_is_valid_character(unsigned char c)
{
	fprintf(stdout, "rgmsg_is_valid_character()\n");
    if (c >= '0' && c <= '9' || c >= 'a' && c <= 'f' || c >= 'A' && c <= 'F')
        return true;
    return false;
}

size_t rgmsg_parse(struct rgmsg_parser *p,
                        const void *s,
                        size_t len,
                        int (*cb1)(void *, size_t, void *),
                        int (*cb2)(int c, void *),
                        void *data)
{
    unsigned const char *us = s;  /* Access input data as array of unsigned char */
    unsigned char c;              /* The character we are currently processing */
    size_t pos = 0;               /* The number of characters we have processed in this call */
    size_t ommitedchars = 0;
    struct rgmsg_parser_counter *ctr;

	fprintf(stdout, "rg_message_parse()\n");
    ctr = (struct rgmsg_parser_counter *)data;

    /* Store key entrys into local variables for performance */
    unsigned char delim = p->delim_char;
    int pstate = p->pstate;
    size_t entry_pos = p->entry_pos;

    if (!p->entry && pos < len) {
        /* Buffer hasn't been allocated yet and len > 0 */
        if (rgmsg_parser_increase_buffer(p) != 0) {
            p->pstate = pstate, p->entry_pos = entry_pos;
            return pos;
        }
    }

    while (pos < len) {
        /* Check memory usage, increase buffer if neccessary */
        if (entry_pos == ((p->options & PSR_ENTRY_APPEND_NULL) ? p->entry_size - 1 : p->entry_size)) {
            if (rgmsg_parser_increase_buffer(p) != 0) {
                p->pstate = pstate, p->entry_pos = entry_pos;
                return pos;
            }
        }

        c = us[pos++];

        switch (pstate) {
        case PSR_MSG_NOT_BEGUN:
        case PSR_ENTRY_NOT_BEGUN:
        	fprintf(stdout, "PSR_MSG_NOT_BEGUN | PSR_ENTRY_NOT_BEGUN\n");
        	//ctr->entry_counter = 0;
        	rgmsg_parser_counter_reset(ctr);
        	
            if (c == delim) { /* Vertical line */
                pstate = PSR_ENTRY_BEGUN;
            }
            else {
                ommitedchars++;
                fprintf(stdout, "ommit char: %02x\n", c);
            }
            break;
        case PSR_ENTRY_BEGUN:
        	fprintf(stdout, "PSR_ENTRY_BEGUN\n");
            if (c == delim) { /* 1st Vertical line */
                /**
                 * Submiting shall check length & content validity with current
                 * entry, if length/content is invalid, discard entry data and
                 * reset parser state to PSR_ENTRY_BEGUN, considering this delimier
                 * as start marker of a message. Otherwise it is a good data entry.
                 */
                SUBMIT_ENTRY(p);
                //pstate = PSR_ENTRY_ENDED;
            }
            else if (rgmsg_mode_check(c) || rgmsg_is_valid_character(c)) {
                SUBMIT_CHAR(p, c);
            }
            else { /* Anything else */
                /**
                 * Message content is not valid, just reset parser state
                 * to PSR_MSG_NOT_BEGUN.
                 */
                pstate = PSR_MSG_NOT_BEGUN;
                entry_pos = 0;
            }
            break;
        case PSR_ENTRY_ENDED:
        	fprintf(stdout, "PSR_ENTRY_ENDED\n");
            if (c == delim) { /* 2nd Vertical line */
                pstate = PSR_MSG_MIGHT_HAVE_ENDED;
            }
            else if (rgmsg_mode_check(c) || rgmsg_is_valid_character(c)) {
                /* Begin a consecutive follow entry */
                pstate = PSR_ENTRY_BEGUN;
                SUBMIT_CHAR(p, c);
            }
            else { /* Anything else */
                pstate = PSR_MSG_NOT_BEGUN;
                entry_pos = 0;
            }
            break;
        case PSR_MSG_MIGHT_HAVE_ENDED:
        	fprintf(stdout, "PSR_MSG_MIGHT_HAVE_ENDED\n");

            SUBMIT_MESSAGE(p, (unsigned char)c);
            break;
        default:
        	fprintf(stdout, "unknown branch\n");
            break;
        }
        
        fprintf(stdout, "[%c]\n", c);

    }
    p->pstate = pstate, p->entry_pos = entry_pos;
    return pos;
}

int rgmsg_entry_handle_uchar(void *data,
                             void *entry,
                             size_t entry_len,
                             void *dst)
{
    struct rgmsg_parser_counter *ctr;

	fprintf(stdout, "rgmsg_entry_handle_uchar()\n");
    ctr = (struct rgmsg_parser_counter *)data;

	if (entry_len != 1)
		return PSR_EENTRY_LENGTH;
	if (rgmsg_mode_check(*(unsigned char *)entry) == false)
		return PSR_EENTRY_DATA;
		
	/* drill a character */
    *(unsigned char *)dst = *(unsigned char *)entry;

	return PSR_SUCCESS;
}

int rgmsg_entry_handle_ushort(void *data,
                              void *entry,
                              size_t entry_len, 
                              void *dst)
{
    struct rgmsg_parser_counter *ctr;

	fprintf(stdout, "rgmsg_entry_handle_ushort()\n");
    ctr = (struct rgmsg_parser_counter *)data;

	if (entry_len != 4)
		return PSR_EENTRY_LENGTH;
	
	if (rg_string_to_short((char *)entry, (unsigned char*)dst) != 0)
		return PSR_EENTRY_DATA;
	
	return PSR_SUCCESS;
}

int rgmsg_entry_handle_string(void *data,
                              void *entry,
                              size_t entry_len,
                              void *dst)
{
    int rv;
    struct rgmsg_parser_counter *ctr;

	fprintf(stdout, "rgmsg_entry_handle_string(len %d)\n", entry_len);
    ctr = (struct rgmsg_parser_counter *)data;

    /* length check */
    if ((entry_len & 0x0000001) == 1 || entry_len > MEMBER_SIZE(rg_msg, para)*2)
        return PSR_EENTRY_LENGTH;

    rv = u_ascii_to_hex((unsigned char*)entry, (unsigned char*)dst, entry_len>>1);
    fprintf(stdout, "u_ascii_to_hex(%d)\n", rv);
    if (rv != 0)
        return PSR_EENTRY_DATA;

	return PSR_SUCCESS;
}

int rgmsg_parser_counter_append(struct rgmsg_parser_counter *ctr, void *data, size_t dlen)
{
	if (!ctr)
		return -1;
	if (!data || dlen == 0)
		return 0;
		
	/* Raw message text data buffer size check */
    if (!ctr->text_pos || ctr->text_pos >= ctr->text_size-1) {
        if (rgmsg_parser_counter_increase_buffer(ctr) != 0) {
            fprintf(stderr, "ETOOBIG\n");
            return -1;
        }
    }
    /* Save the raw message text data */
    if (dlen+ctr->text_pos < ctr->text_size) {
        memcpy(ctr->text+ctr->text_pos, data, dlen);
        ctr->text_pos += dlen;
        return 0;
    }
    return -1;
}

int rgmsg_parser_counter_reset(struct rgmsg_parser_counter *ctr)
{
	if (!ctr)
		return -1;
		
	ctr->mstate = MSG_INCOMPLETE;
	ctr->text_pos = 0;
	ctr->entry_counter = 0;
	if (ctr->text && ctr->text_size > 0) {
		ctr->text[0] = '\0';
	}
	
	return 0;
}

int rgmsg_parse_entry_callback(void *entry, size_t entry_len, void *data)
{
	int rv;
	struct rgmsg_parser_counter *ctr;
    struct rgmsg_parser_entry_handler *eh;
    struct rg_msg *msg;

	fprintf(stdout, "rgmsg_parse_entry_handler()\n");
    ctr = (struct rgmsg_parser_counter *)data;
    msg = (struct rg_msg *)ctr->message;
    
    do {
    	rv = PSR_EPARSE;

    	/* It is a valid mode field , we reset the message state */
    	if (entry_len == 1 && rgmsg_mode_check(*(char *)entry)) {
    		/* Short cut to a new entry */
    		//ctr->entry_counter = 0;
    		rgmsg_parser_counter_reset(ctr);
    	}

    	if (ctr->entry_counter < 0 || ctr->entry_counter > PSR_ENTRIES-1) {
    		/* Invalid/Too-many entries */
    		rv = PSR_EPARSE;
    		break;
		}
		
		/* run handlers for each entry */
		eh = &rgmsg_entry_handlers[ctr->entry_counter];
		rv = eh->handler(data, entry, entry_len, (void*)(((char*)msg) + eh->offset));
		
	} while (0);
	
	if (rv == PSR_SUCCESS) {
        /* entry good, goto next entry */
        fprintf(stdout, "string ok\n");
        /* Raw message text data buffer size check */
        rgmsg_parser_counter_append(ctr, "|", 1);
        rgmsg_parser_counter_append(ctr, entry, entry_len);
        
        ctr->entry_counter++;
        return PSR_ENTRY_ENDED;
	}
	else {
        /* reset message state */
        //ctr->mstate = MSG_INCOMPLETE;
        //ctr->entry_counter = 0;
        rgmsg_parser_counter_reset(ctr);
        //return PSR_MSG_NOT_BEGUN;
        return PSR_ENTRY_BEGUN;
	}
}

int rgmsg_parse_message_callback(int c, void *data)
{
	int pstate;
    struct rgmsg_parser *psr;
    struct rgmsg_parser_counter *ctr;

	fprintf(stdout, "rgmsg_message_handle()\n");
    ctr = (struct rgmsg_parser_counter *)data;
    psr = ctr->parser;

    /* members all collected */
    if (ctr->entry_counter == PSR_ENTRIES) {
        ctr->mstate = MSG_COMPLETE;
    }

    /* TODO: handle message here */
    if (ctr->mstate == MSG_COMPLETE) {
    	fprintf(stdout, "===message ok===\n");
        /* Raw message text data buffer size check */
        rgmsg_parser_counter_append(ctr, "||\n", 3);
        ctr->text[ctr->text_pos] = '\0';
        fprintf(stdout, "raw message: [%s]", ctr->text);
    }

    if (c == psr->delim_char) { /* 3rd Vertical line */
        /* No CRLF or Space between two messages */
        pstate = PSR_ENTRY_BEGUN;
        fprintf(stdout, "PSR_ENTRY_BEGUN\n");
    }
    else if (c == CHAR_CR || c == CHAR_LF || c == CHAR_SPACE) { /* Carriage Return or Line Feed or Space*/
        pstate = PSR_MSG_NOT_BEGUN;
        fprintf(stdout, "PSR_MSG_NOT_BEGUN\n");
    }
    else { /* Anything else */
        pstate = PSR_MSG_NOT_BEGUN;
        fprintf(stdout, "PSR_MSG_NOT_BEGUN\n");
    }

	return pstate;
}

struct rgmsg_parser_entry_handler rgmsg_entry_handlers [] = {
    {
        .offset  = offsetof(struct rg_msg, mode),
        .size    = MEMBER_SIZE(rg_msg, mode),
        .handler = rgmsg_entry_handle_uchar,
    },
    {
        .offset  = offsetof(struct rg_msg, seq),
        .size    = MEMBER_SIZE(rg_msg, seq),
        .handler = rgmsg_entry_handle_ushort,
    },
    {
        .offset  = offsetof(struct rg_msg, port),
        .size	 = MEMBER_SIZE(rg_msg, port),
        .handler = rgmsg_entry_handle_ushort,
    },
    {
        .offset  = offsetof(struct rg_msg, command),
        .size	 = MEMBER_SIZE(rg_msg, command),
        .handler = rgmsg_entry_handle_ushort,
    },
    {
        .offset  = offsetof(struct rg_msg, para),
        .size	 = MEMBER_SIZE(rg_msg, para),
        .handler = rgmsg_entry_handle_string,
    }
};

bool rgmsg_mode_check(unsigned char m)
{
    switch(m)
    {
    case 'R': /* for Master Request to remote */
    case 'W': /* for Slave report */
    case 'L': /* for Loop test */
    case 'S': /* for Voice Link or system status report */
    case 'T': /* for local TSI related */
    case 'M': /* for from Master Media control */
    case 'G': /* for Get System Info */
        return true;
        break;
    default:
        break;
    }
    return false;
}

int main(int argc, char *argv[])
{
	FILE *fp;
	struct rgmsg_parser psr;
	struct rgmsg_parser_counter ctr;
	struct rg_msg msg;
	char line[128];
	
	if (argc != 2) {
		fprintf(stdout, "Usage: %s <filename>", argv[0]);
		return 1;
	}
	
	fp = fopen(argv[1], "r");
	if (!fp) {
		fprintf(stdout, "Usage: %s <filename>", argv[0]);
		return 1;
	}
	
	memset(line, 0, sizeof(line));
	memset(&ctr, 0, sizeof(ctr));
	memset(&msg, 0, sizeof(msg));
	
	ctr.message = &msg;
	ctr.parser  = &psr;
	rgmsg_parser_init(&psr, PSR_ENTRY_APPEND_NULL);
	
	while (fgets(line, sizeof(line), fp)) {
		rgmsg_parse(&psr, line, strlen(line),
		            rgmsg_parse_entry_callback,
		            rgmsg_parse_message_callback,
		            &ctr);
		memset(line, 0, sizeof(line));
	}
	
	rgmsg_parser_free(&psr);
	
	return 0;
}
