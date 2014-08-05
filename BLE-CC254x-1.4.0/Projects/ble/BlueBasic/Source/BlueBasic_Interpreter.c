////////////////////////////////////////////////////////////////////////////////
// BlueBasic
////////////////////////////////////////////////////////////////////////////////
//
// Authors:
//      Tim Wilkinson <tim.j.wilkinson@gmail.com>
//
//      Original TinyBasicPlus
//        Mike Field <hamster@snap.net.nz>
//	      Scott Lawrence <yorgle@gmail.com>
//

#define kVersion "v0.2"

// v0.2: 2014-08-01
//      Seems like a good place to declare 0.2. Work is complete enough to have a pre-flashed device which can
//      be connected to over BLE and then program, including setting up services and characteristics, and attaching
//      these to the general i/o on the chip.
//
// v0.1: 2014-07-10
//      Start of Blue Basic. Heavily modified from Tiny Basic Plus (https://github.com/BleuLlama/TinyBasicPlus).
//      The goal is to put a Basic interpreter onto the TI CC254x Bluetooth LE chip.

#include "os.h"


////////////////////////////////////////////////////////////////////////////////

#ifndef BLUEBASIC_MEM
#define kRamSize   2048 /* arbitrary */
#else // BLUEBASIC_MEM
#define kRamSize   (BLUEBASIC_MEM)
#endif // BLUEBASIC_MEM

////////////////////////////////////////////////////////////////////////////////
// ASCII Characters
#define CR	'\r'
#define NL	'\n'
#define WS_TAB	'\t'
#define WS_SPACE   ' '
#define SQUOTE  '\''
#define DQUOTE  '\"'
#define CTRLC	0x03
#define CTRLH	0x08

typedef short unsigned LINENUM;

static unsigned char *txtpos;
static unsigned char *list_line;

enum
{
  ERROR_OK = 0,
  ERROR_GENERAL,
  ERROR_EXPRESSION,
  ERROR_DIV0,
  ERROR_OOM,
  ERROR_TOOBIG,
  ERROR_BADPIN,
};
static unsigned char error_num;

static const char* const error_msgs[] =
{
  "OK",
  "Error",
  "Bad expression",
  "Divide by zero",
  "Out of memory",
  "Too big",
  "Bad pin",
};

static const char initmsg[]           = "BlueBasic " kVersion;
static const char memorymsg[]         = " bytes free.";

/***********************************************************/
// Keyword table and constants - the last character has 0x80 added to it
static const unsigned char keywords[] =
{
  'L','I','S','T'+0x80,
  'L','O','A','D'+0x80,
  'N','E','W'+0x80,
  'R','U','N'+0x80,
  'S','A','V','E'+0x80,
  'N','E','X','T'+0x80,
  'L','E','T'+0x80,
  'I','F'+0x80,
  'E','L','S','E'+0x80,
  'E','L','I','F'+0x80,
  'G','O','T','O'+0x80,
  'G','O','S','U','B'+0x80,
  'R','E','T','U','R','N'+0x80,
  'R','E','M'+0x80,
  'F','O','R'+0x80,
  'T','O'+0x80,
  'S','T','E','P'+0x80,
  'P','R','I','N','T'+0x80,
  'S','T','O','P'+0x80,
  'R','E','B','O','O','T'+0x80,
  'M','E','M'+0x80,
  'D','I','M'+0x80,
  'E','N','D'+0x80,
  'T','I','M','E','R'+0x80,
  'R','E','P','E','A','T'+0x80,
  'D','E','L','A','Y'+0x80,
  'A','B','S','('+0x80,
  'R','N','D','('+0x80,
  'M','I','L','L','I','S','('+0x80,
  'B','A','T','T','E','R','Y','('+0x80,
  'A','U','T','O','R','U','N'+0x80,
  '>','='+0x80,
  '<','>'+0x80,
  '>'+0x80,
  '=','='+0x80,
  '='+0x80,
  '<','='+0x80,
  '<'+0x80,
  '!','='+0x80,
  '0','X'+0x80,
  '+'+0x80,
  '-'+0x80,
  '*'+0x80,
  '/'+0x80,
  '%'+0x80,
  '&'+0x80,
  '|'+0x80,
  '^'+0x80,
  'D','S','A','V','E'+0x80,
  'D','L','O','A','D'+0x80,
  // I/O pin support
  'P','I','N','M','O','D','E'+0x80,
  'A','T','T','A','C','H','I','N','T','E','R','R','U','P','T'+0x80,
  'D','E','T','A','C','H','I','N','T','E','R','R','U','P','T'+0x80,
  'P','U','L','L','U','P'+0x80,
  'P','U','L','L','D','O','W','N'+0x80,
  'I','N','P','U','T'+0x80,
  'A','D','C'+0x80,
  'O','U','T','P','U','T'+0x80,
  'R','I','S','I','N','G'+0x80,
  'F','A','L','L','I','N','G'+0x80,
  'P','0','('+0x80,
  'P','1','('+0x80,
  'P','2','('+0x80,
  'O','N','R','E','A','D'+0x80,
  'O','N','W','R','I','T','E'+0x80,
  'O','N','C','O','N','N','E','C','T'+0x80,
  // Constants BEGIN - keep them together
  'T','R','U','E'+0x80,
  'F','A','L','S','E'+0x80,
  'O','N'+0x80,
  'O','F','F'+0x80,
  // Constants for GAPRole_SetParameter
  'A','D','V','E','R','T','_','E','N','A','B','L','E','D'+0x80,
  'M','I','N','_','C','O','N','N','_','I','N','T','E','R','V','A','L'+0x80,
  'M','A','X','_','C','O','N','N','_','I','N','T','E','R','V','A','L'+0x80,
  'S','L','A','V','E','_','L','A','T','E','N','C','Y'+0x80,
  'T','I','M','E','O','U','T','_','M','U','L','T','I','P','L','I','E','R'+0x80,
  // Constants for GAP_SetParamValue
  'L','I','M','_','D','I','S','C','_','A','D','V','_','I','N','T','_','M','I','N'+0x80,
  'L','I','M','_','D','I','S','C','_','A','D','V','_','I','N','T','_','M','A','X'+0x80,
  'G','E','N','_','D','I','S','C','_','A','D','V','_','I','N','T','_','M','I','N'+0x80,
  'G','E','N','_','D','I','S','C','_','A','D','V','_','I','N','T','_','M','A','X'+0x80,
  // Constants END
  // BLE support
  'G','A','T','T'+0x80,
  'S','E','R','V','I','C','E'+0x80,
  'C','H','A','R','A','C','T','E','R','I','S','T','I','C'+0x80,
  'R','E','A','D'+0x80,
  'W','R','I','T','E','N','O','R','S','P'+0x80,
  'W','R','I','T','E'+0x80,
  'N','O','T','I','F','Y'+0x80,
  'I','N','D','I','C','A','T','E'+0x80,
  'A','D','V','E','R','T'+0x80,
  'S','C','A','N'+0x80,
  'G','E','N','E','R','A','L'+0x80,
  'L','I','M','I','T','E','D'+0x80,
  'M','O','R','E'+0x80,
  'N','A','M','E'+0x80,
  'C','U','S','T','O','M'+0x80,
  'G','A','P','R','O','L','E','('+0x80,
  'G','A','P','R','O','L','E'+0x80,
  'G','A','P','('+0x80,
  'G','A','P'+0x80,
  // SPI support
  'S','P','I'+0x80,
  'T','R','A','N','S','F','E','R'+0x80,
  'M','S','B'+0x80,
  'L','S','B'+0x80,
  0
};

// Start enum at 128 so we get instant token values.
// By moving the command list to an enum, we can easily remove sections
// above and below simultaneously to selectively obliterate functionality.
enum {
  KW_LIST = 0x80,
  KW_LOAD,
  KW_NEW,
  KW_RUN,
  KW_SAVE,
  KW_NEXT,
  KW_LET,
  KW_IF,
  KW_ELSE,
  KW_ELIF,
  KW_GOTO,
  KW_GOSUB,
  KW_RETURN,
  KW_REM,
  KW_FOR,
  KW_TO,
  KW_STEP,
  KW_PRINT,
  KW_STOP,
  KW_REBOOT,
  KW_MEM,
  KW_DIM,
  KW_END,
  KW_TIMER,
  KW_REPEAT,
  KW_DELAY,
  FUNC_ABS,
  FUNC_RND,
  FUNC_MILLIS,
  FUNC_BATTERY,
  KW_AUTORUN,
  RELOP_GE,
  RELOP_NE,
  RELOP_GT,
  RELOP_EQEQ,
  RELOP_EQ,
  RELOP_LE,
  RELOP_LT,
  RELOP_NE_BANG,
  FUNC_HEX,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_REM,
  OP_AND,
  OP_OR,
  OP_XOR,
  KW_DSAVE,
  KW_DLOAD,
  KW_PINMODE,
  KW_ATTACHINTERRUPT,
  KW_DETACHINTERRUPT,
  PM_PULLUP,
  PM_PULLDOWN,
  PM_INPUT,
  PM_ADC,
  PM_OUTPUT,
  PM_RISING,
  PM_FALLING,
  PIN_P0,
  PIN_P1,
  PIN_P2,
  BLE_ONREAD,
  BLE_ONWRITE,
  BLE_ONCONNECT,
  CO_TRUE,
  CO_FALSE,
  CO_ON,
  CO_OFF,
  CO_ADVERT_ENABLED,
  CO_MIN_CONN_INTERVAL,
  CO_MAX_CONN_INTERVAL,
  CO_SLAVE_LATENCY,
  CO_TIMEOUT_MULTIPLIER,
  CO_LIM_DISC_INT_MIN,
  CO_LIM_DISC_INT_MAX,
  CO_GEN_DISC_INT_MIN,
  CO_GEN_DISC_INT_MAX,
  BLE_GATT,
  BLE_SERVICE,
  BLE_CHARACTERISTIC,
  BLE_READ,
  BLE_WRITENORSP,
  BLE_WRITE,
  BLE_NOTIFY,
  BLE_INDICATE,
  BLE_ADVERT,
  BLE_SCAN,
  BLE_GENERAL,
  BLE_LIMITED,
  BLE_MORE,
  BLE_NAME,
  BLE_CUSTOM,
  BLE_FUNC_GAPROLE,
  BLE_GAPROLE,
  BLE_FUNC_GAP,
  BLE_GAP,
  KW_SPI,
  KW_TRANSFER,
  KW_MSB,
  KW_LSB,
};

// Interpreter exit statuses
enum {
  IX_BYE,
  IX_PROMPT,
  IX_OUTOFMEMORY
};

#define VAR_TYPE    long int
#define VAR_SIZE    (4)

typedef struct
{
  char frame_type;
  unsigned short frame_size;
} stack_header;

typedef struct stack_for_frame
{
  stack_header header;
  char for_var;
  VAR_TYPE terminal;
  VAR_TYPE step;
  unsigned char *current_line;
  unsigned char *txtpos;
} stack_for_frame;

typedef struct stack_gosub_frame
{
  stack_header header;
  unsigned char *current_line;
  unsigned char *txtpos;
} stack_gosub_frame;

typedef struct stack_event_frame
{
  stack_header header;
} stack_event_frame;

typedef struct stack_variable_frame {
  stack_header header;
  char type;
  char name;
  char oflags;
  struct gatt_variable_ref* ble;
  // .... bytes ...
} stack_variable_frame;

enum {
  STACK_GOSUB_FLAG = 'G',
  STACK_FOR_FLAG = 'F',
  STACK_VARIABLE_FLAG = 'V',
  STACK_EVENT_FLAG = 'E',
  STACK_SERVICE_FLAG = 'S',
  STACK_VARIABLE_NORMAL = 'N'
};

static unsigned char *program_start;
static unsigned char *program_end;
static unsigned char *variables_begin;
static unsigned char *current_line;
static unsigned char *sp;
static unsigned char table_index;
static LINENUM linenum;


enum {
  VAR_NORMAL   = 0x00,
  VAR_VARIABLE = 0x01,
  // These flags are used inside the special struct
  VAR_INT      = 0x02,
  VAR_DIM_BYTE = 0x04,
};
static unsigned char vname;
#define VARIABLE_INT_ADDR(F)    (((VAR_TYPE*)variables_begin) + ((F) - 'A'))
#define VARIABLE_INT_GET(F)     (*VARIABLE_INT_ADDR(F))
#define VARIABLE_INT_SET(F,V)   (*VARIABLE_INT_ADDR(F) = (V))
#define VARIABLE_FLAGS_GET(F)   (vname = (F) - 'A', (((*(variables_begin + 26 * VAR_SIZE + vname / 8)) >> (vname % 8)) & 0x01))
#define VARIABLE_FLAGS_SET(F,V) do { vname = (F) - 'A'; unsigned char* v = variables_begin + 26 * VAR_SIZE + vname / 8; *v = (*v & (255 - (1 << (vname % 8)))) | ((V) << (vname % 8)); } while(0)

// Constant map (so far all constants are <= 16 bits)
static const unsigned short constantmap[] =
{
  1, // TRUE
  0, // FALSE
  1, // ON
  0, // OFF
  GAPROLE_ADVERT_ENABLED,
  GAPROLE_MIN_CONN_INTERVAL,
  GAPROLE_MAX_CONN_INTERVAL,
  GAPROLE_SLAVE_LATENCY,
  GAPROLE_TIMEOUT_MULTIPLIER,
  TGAP_LIM_DISC_ADV_INT_MIN,
  TGAP_LIM_DISC_ADV_INT_MAX,
  TGAP_GEN_DISC_ADV_INT_MIN,
  TGAP_GEN_DISC_ADV_INT_MAX,
};

#ifdef SIMULATE_PINS
static unsigned char P0DIR, P1DIR, P2DIR;
static unsigned char P0SEL, P1SEL, P2SEL;
static unsigned char P0INP, P1INP, P2INP;
static unsigned char P0, P1, P2;
static unsigned char APCFG, ADCH, ADCL, ADCCON1, ADCCON3;
static unsigned char PICTL, P0IEN, P1IEN, P2IEN;
static unsigned char IEN1, IEN2;
static unsigned char U0BAUD, U0GCR, U0CSR, U0DBUF;
static unsigned char U1BAUD, U1GCR, U1CSR, U1DBUF;
static unsigned char PERCFG;
#endif
static unsigned char spiChannel;

static void pin_write(unsigned char pin, unsigned char val);
static VAR_TYPE pin_read(unsigned char pin);
static unsigned char pin_parse(void);
#define PIN_MAJOR(P)  ((P) >> 4)
#define PIN_MINOR(P)  ((P) & 15)

static VAR_TYPE expression(void);

unsigned char  ble_adbuf[31];
unsigned char* ble_adptr;
unsigned char  ble_isadvert;

typedef struct stack_service_frame
{
  stack_header header;
  gattAttribute_t* attrs;
  LINENUM connect;
} stack_service_frame;

typedef struct gatt_variable_ref
{
  unsigned char var;
  gattAttribute_t* attrs;
  unsigned char* cfg;
  LINENUM read;
  LINENUM write;
} gatt_variable_ref;

#define INVALID_CONNHANDLE 0xFFFF

static short find_quoted_string(void);
static char ble_build_service(void);
static char ble_get_uuid(void);
static unsigned char ble_read_callback(unsigned short handle, gattAttribute_t* attr, unsigned char* value, unsigned char* len, unsigned short offset, unsigned char maxlen);
static unsigned char ble_write_callback(unsigned short handle, gattAttribute_t* attr, unsigned char* value, unsigned char len, unsigned short offset);
static void ble_notify_assign(gatt_variable_ref* vref);

static const unsigned char ble_primary_service_uuid[] = { 0x00, 0x28 };
static const unsigned char ble_characteristic_uuid[] = { 0x03, 0x28 };
static const unsigned char ble_characteristic_description_uuid[] = { 0x01, 0x29 };
static const unsigned char ble_client_characteristic_config_uuid[] = { 0x02, 0x29 };

static LINENUM servicestart;
static unsigned short servicecount;
static unsigned char ble_uuid[16];
static unsigned char ble_uuid_len;
static const gattServiceCBs_t ble_service_callbacks =
{
  ble_read_callback,
  ble_write_callback,
  NULL
};

//
// Skip whitespace
//
static void ignore_blanks(void)
{
  while (*txtpos == WS_SPACE)
  {
    txtpos++;
  }
}

//
// Tokenize the human readable command line into something easier, smaller and faster.
//  Note. The tokenized form must always be smaller than the human form otherwise this
//  will break because it overwrites the buffer as it goes along.
//
static void tokenize(void)
{
  const unsigned char* table = keywords;
  unsigned char token = 0x80;
  unsigned char quote = 0;
  unsigned char lastch = 0;
  unsigned char* writepos = txtpos;
  unsigned char* scanpos = txtpos;
  unsigned char* readpos = scanpos;
  for (;;)
  {
    unsigned char ch = *readpos++;
    if (ch == NL && scanpos == readpos - 1)
    {
      // Remove any trailing white space. There can only be 0 or 1 since we already reduced them
      if (writepos > txtpos && writepos[-1] == WS_SPACE)
      {
        writepos--;
      }
      *writepos = NL;
      return;
    }
    else if (quote)
    {
      if (ch == quote)
      {
        quote = 0;
        scanpos = readpos;
      }
      *writepos++ = ch;
    }
    // Keep quotes "as is"
    else if (quote == 0 && (ch == SQUOTE || ch == DQUOTE))
    {
      quote = ch;
      *writepos++ = ch;
    }
    // Reduce whitespaces down to a single whitespace
    else if ((ch == WS_SPACE || ch == WS_TAB) && lastch == WS_SPACE)
    {
      // Skip extra whitespace
      scanpos = readpos;
    }
    else
    {
      unsigned char tch = *table;
      if (tch == 0)
      {
        // No matching keyword. Advance to next possible keyword start
        token = 0x80;
        table = keywords;
        *writepos++ = ch == WS_TAB ? WS_SPACE : ch;
        if ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_')
        {
          for (ch = *readpos; (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '(' || ch == '_'; ch = *++readpos)
          {
            *writepos++ = ch;
          }
          ch = 0;
        }
        scanpos = readpos;
      }
      // Token is matching
      else if (ch == tch)
      {
        table++;
      }
      // Matching token found.
      else if ((ch | 0x80) == tch)
      {
        // Remove any whitespace proceeding the token
        if (writepos > txtpos && writepos[-1] == WS_SPACE)
        {
          writepos--;
        }
        *writepos++ = token;
        // Skip any whitespace that follows
        for (ch = *readpos; ch == WS_SPACE || ch == WS_TAB; ch = *++readpos)
          ;
        token = 0x80;
        table = keywords;
        scanpos = readpos;
      }
      else
      {
        // No match - advance to the next token and reset
        while (*table++ < 0x80)
          ;
        readpos = scanpos;
        ch = 0;
        token++;
      }
    }
    lastch = ch == WS_TAB ? WS_SPACE : ch;
  }
}

void printnum(VAR_TYPE num)
{
  VAR_TYPE size = 10;

  if (num < 0)
  {
    OS_putchar('-');
    num = -num;
  }
  for (; size <= num; size *= 10)
    ;
  for (size /= 10; size != 0; size /= 10)
  {
    OS_putchar('0' + num / size % 10);
  }
}

static unsigned short testnum(void)
{
  LINENUM num = 0;
  ignore_blanks();

  while (*txtpos >= '0' && *txtpos <= '9')
  {
    // Trap overflows
    if(num >= 0xFFFF/10)
    {
      num = 0xFFFF;
      break;
    }

    num = num * 10 + *txtpos - '0';
    txtpos++;
  }
  return num;
}

//
// Find the beginning and limit of a quoted string.
//  Returns the length, of -1 is invalid.
//
static short find_quoted_string(void)
{
  short i = 0;
  unsigned char delim = *txtpos;
  if (delim != '"' && delim != '\'')
  {
    return -1;
  }
  txtpos++;
  for(; txtpos[i] != delim; i++)
  {
    if (txtpos[i] == NL)
    {
      return -1;
    }
  }
  return i;
}

static unsigned char print_quoted_string(void)
{
  short i = find_quoted_string();
  if (i == -1)
  {
    return 0;
  }
  else
  {
    // Print the characters
    for (; i > 0; i--)
    {
      OS_putchar(*txtpos++);
    }
    txtpos++;

    return 1;
  }
}

void printmsg(const char *msg)
{
  while (*msg != 0)
  {
    OS_putchar(*msg++);
  }
  OS_putchar(NL);
}

static unsigned char* findline(void)
{
  unsigned char *line;
  for (line = program_start; ; line += line[sizeof(LINENUM)])
  {
    if(line == program_end)
    {
      return line;
    }

    if(((LINENUM *)line)[0] >= linenum)
    {
      return line;
    }
  }
}

//
// Print the current BASIC line. The line is tokenized so
// it is expanded as it's printed, including the addition of whitespace
// to make it more readable.
//
static void printline(unsigned char indent)
{
  LINENUM line_num;
  unsigned char lc = WS_SPACE;

  line_num = *(LINENUM *)list_line;
  list_line += sizeof(LINENUM) + sizeof(char);

  // Output the line */
  printnum(line_num);
  while (indent-- != 0)
  {
    OS_putchar(WS_SPACE);
  }
  for (unsigned char c = *list_line++; c != NL; c = *list_line++)
  {
    if (c < 0x80)
    {
      OS_putchar(c);
    }
    else
    {
      const unsigned char* table = keywords;
      for (; c > 0x80; c--)
      {
        while (*table++ < 0x80)
          ;
      }
      if (lc != WS_SPACE)
      {
        OS_putchar(WS_SPACE);
      }
      for (c = *table++; c < 0x80; c = *table++)
      {
        OS_putchar(c);
      }
      c &= 0x7F;
      OS_putchar(c);
      if (c != '(')
      {
        OS_putchar(WS_SPACE);
        c = WS_SPACE;
      }
    }
    lc = c;
  }
  OS_putchar(NL);
}

//
// Parse a string into an integer.
// We limit the number of characters scanned and specific a base (upto 16)
//
static VAR_TYPE parse_int(unsigned char maxlen, unsigned char base)
{
  VAR_TYPE v = 0;

  while (maxlen-- > 0)
  {
    char ch = *txtpos++;
    if (ch >= '0' && ch <= '9')
    {
      ch -= '0';
    }
    else if (ch >= 'A' && ch <= 'F')
    {
      ch -= 'A' - 10;
    }
    else if (ch >= 'a' && ch <= 'f')
    {
      ch -= 'a' - 10;
    }
    else
    {
    error:
      error_num = ERROR_EXPRESSION;
      txtpos--;
      break;
    }
    if (ch >= base)
    {
      goto error;
    }
    v = v * base + ch;
  }
  return v;
}

//
// Find the information on the stack about the given variable. Return the info in
// the passed argument, and return the address of the variable.
// This method handles the complexites of finding variables which are on the stack (DIMs)
// as well as the simple, preset, variables. Regardless, we return appropriate information
// so the caller doesn't need to worry about where the variable is stored.
//
static unsigned char* get_variable_frame(char name, stack_variable_frame** frame)
{
  unsigned char* tsp;

  if (VARIABLE_FLAGS_GET(name) == VAR_VARIABLE)
  {
    for (tsp = sp; tsp < variables_begin; tsp += ((stack_header*)tsp)->frame_size)
    {
      if (((stack_header*)tsp)->frame_type == STACK_VARIABLE_FLAG && ((stack_variable_frame*)tsp)->name == name)
      {
        if (frame)
        {
          *frame = (stack_variable_frame*)tsp;
        }
        if (((stack_variable_frame*)tsp)->type == VAR_INT)
        {
          return (unsigned char*)VARIABLE_INT_ADDR(name);
        }
        else
        {
          return tsp + sizeof(stack_variable_frame);
        }
      }
    }
  }
  if (frame)
  {
    static stack_variable_frame normal;
    normal.header.frame_type = STACK_VARIABLE_NORMAL;
    normal.type = VAR_INT;
    normal.name = name;
    normal.ble = NULL;
    *frame = &normal;
  }
  return (unsigned char*)VARIABLE_INT_ADDR(name);
}

// -------------------------------------------------------------------------------------------
//
// Expression evaluator
//
// -------------------------------------------------------------------------------------------

static VAR_TYPE expr4(void)
{
  unsigned char ch;
  VAR_TYPE a;
  unsigned char* ptr;

  ch = *txtpos++;

  if (ch == OP_SUB)
  {
    return -expr4();
  }

  else if (ch >= '0' && ch <= '9')
  {
    txtpos--;
    a = parse_int(255, 10);
    error_num = ERROR_OK;
    return a;
  }

  // Is it a function?
  else if (ch >= 0x80)
  {
    // Is it an input pin?
    if (ch >= PIN_P0 && ch <= PIN_P2)
    {
      txtpos--;
      a = pin_parse();
      if (error_num)
      {
        goto expr4_error;
      }
      return pin_read(a);
    }

    // 0x... hex literal
    else if (ch == FUNC_HEX)
    {
      a = parse_int(8, 16);
      error_num = ERROR_OK; // Okay to stop parsing early
      return a;
    }
    
    // Is it a constant?
    else if (ch >= CO_TRUE && ch < CO_TRUE + sizeof(constantmap) / VAR_SIZE)
    {
      return constantmap[ch - CO_TRUE];
    }

    // Is it a no-arg function?
    else if (*txtpos == ')')
    {
      txtpos++;
      switch (ch)
      {
        case FUNC_MILLIS:
          return OS_millis();

        case FUNC_BATTERY:
          ADCCON3 = 0x0F | 0x10 | 0x00; // VDD/3, 10-bit, internal voltage reference
#ifdef SIMULATE_PINS
          ADCCON1 = 0x80;
#endif
          while ((ADCCON1 & 0x80) == 0)
            ;
          a = ADCL;
          a |= ADCH << 8;
          a = a >> 6;
          // VDD can be in the range 2v to 3.6v. Internal reference voltage is 1.24v (per datasheet)
          // So we're measuring VDD/3 against 1.24v giving us (VDD x 511) / 3.72
          // or VDD = (ADC * 3.72 / 511). x 1000 to get result in mV.
          return (a * 3720L) / 511L;
          
#if 0 // Calibration seems problematic - leave this out until we find a fix.
        case FUNC_TEMP:
          ADCCON3 = 0x0E | 0x30 | 0x00; // Temp, 14-bit, internal voltage reference
#ifdef SIMULATE_PINS
          ADCCON1 = 0x80;
#endif
          while ((ADCCON1 & 0x80) == 0)
            ;
          a = ADCL;
          a |= ADCH << 8;
          a = a >> 2;
          // ADC value @ 24C is 1225. ADC value changes @ 4.5/1C. So ADC at 0C is 1117.
          // So temp in C is (ADC - 1117) / 4.5 or (2xADC - 2234) / 9.
          return (2 * a - 2234) / 9;
#endif

        default:
          goto expr4_error;
      }
    }

    // Assume it's a function with an argument
    a = expression();
    if (*txtpos != ')' || error_num)
    {
      goto expr4_error;
    }
    txtpos++;

    switch(ch)
    {
      default:
        goto expr4_error;
        
      case FUNC_ABS:
        return a < 0 ? -a : a;
        
      case FUNC_RND:
        return OS_rand() % a;

      case BLE_FUNC_GAPROLE:
        {
          unsigned short v = 0;
          GAPRole_GetParameter((unsigned short)a, &v);
          return v;
        }
      case BLE_FUNC_GAP:
        return GAP_GetParamValue((unsigned short)a);
    }
  }

  // Is it a variable reference?
  else if (ch >= 'A' && ch <= 'Z')
  {
    stack_variable_frame* frame;
    ptr = get_variable_frame(ch, &frame);

    if (frame->type == VAR_DIM_BYTE)
    {
      if (*txtpos != '(')
      {
        goto expr4_error;
      }
      txtpos++;
      a = expression();
      if(*txtpos != ')' || error_num)
      {
        goto expr4_error;
      }
      txtpos++;
      
      // Find the array and get value
      if(a < 0 || a >= frame->header.frame_size - sizeof(stack_variable_frame))
      {
        goto expr4_error;
      }
      return ptr[a];
    }
    else
    {
      if (*txtpos == '(')
      {
        goto expr4_error;
      }
      return *(VAR_TYPE*)ptr;
    }
  }
  else if (ch == '(')
  {
    a = expression();
    if(*txtpos != ')' || error_num)
    {
      goto expr4_error;
    }

    txtpos++;
    return a;
  }

expr4_error:
  error_num = ERROR_EXPRESSION;
  return 0;

}

static VAR_TYPE expr3(void)
{
  VAR_TYPE a, b;

  a = expr4();

  for (;;)
  {
    if (error_num)
      return a;
    switch (*txtpos++)
    {
      default:
        txtpos--;
        return a;
      case OP_MUL:
        a *= expr4();
        break;
      case OP_DIV:
        b = expr4();
        if (b == 0)
          error_num = ERROR_DIV0;
        else
          a /= b;
        break;
      case OP_REM:
        b = expr4();
        if (b == 0)
          error_num = ERROR_DIV0;
        else
          a %= b;
        break;
    }
  }
}

static VAR_TYPE expr2(void)
{
  VAR_TYPE a;

  if(*txtpos == OP_SUB || *txtpos == OP_ADD)
    a = 0;
  else
    a = expr3();

  for (;;)
  {
    if (error_num)
      return a;
    switch (*txtpos++)
    {
      default:
        txtpos--;
        return a;
      case OP_SUB:
        a -= expr3();
        break;
      case OP_ADD:
        a += expr3();
        break;
    }
  }
}

static VAR_TYPE expr1(void)
{
  VAR_TYPE a;

  a = expr2();

  for (;;)
  {
    if (error_num)
      return a;
    switch(*txtpos++)
    {
      default:
        txtpos--;
        return a;
      case RELOP_GE:
         a = a >= expr2();
        break;
      case RELOP_NE:
      case RELOP_NE_BANG:
        a = a != expr2();
        break;
      case RELOP_GT:
        a = a > expr2();
        break;
      case RELOP_EQ:
      case RELOP_EQEQ:
        a = a == expr2();
        break;
      case RELOP_LE:
        a = a <= expr2();
        break;
      case RELOP_LT:
        a = a < expr2();
        break;
    }
  }
}

static VAR_TYPE expression(void)
{
  VAR_TYPE a;

  ignore_blanks();

  a = expr1();

  for (;;)
  {
    if (error_num)
      return a;
    switch (*txtpos++)
    {
      default:
        txtpos--;
        return a;
      case OP_AND:
        a &= expr1();
        break;
      case OP_OR:
        a |= expr1();
        break;
      case OP_XOR:
        a ^= expr1();
        break;
    }
  }
}

// -------------------------------------------------------------------------------------------
//
// The main interpreter
//
// -------------------------------------------------------------------------------------------


void interpreter_init()
{
  program_start = OS_malloc(kRamSize);
  OS_memset(program_start, 0, kRamSize);
  program_end = program_start;
  variables_begin = program_start + kRamSize - 26 * VAR_SIZE - 4; // 4 bytes = 32 bits of flags
  sp = variables_begin;
  interpreter_banner();
}

void interpreter_banner(void)
{
  printmsg(initmsg);
  printnum((VAR_TYPE)(sp - program_end));
  printmsg(memorymsg);
  printmsg(error_msgs[ERROR_OK]);
}


//
// Setup the interpreter.
//  Initialize everything we need to run the interpreter, and process any
//  'autorun' which may have been set.
//
void interpreter_setup(void)
{
  OS_init();
  interpreter_init();
  OS_prompt_buffer(program_end + sizeof(LINENUM), sp);
  
  if (OS_autorun_get())
  {
    short len = OS_file_open(0, 'r');
    if (len > 0)
    {
      program_end = program_start + len;
      OS_file_read(program_start, len);
    }
    OS_file_close();
    if (program_end > program_start)
    {
      OS_timer_start(DELAY_TIMER, OS_AUTORUN_TIMEOUT, 0, *(LINENUM*)program_start);
    }
    current_line = program_end;
    OS_prompt_buffer(program_end + sizeof(LINENUM), sp);
  }
}

//
// Run the main interpreter while we have new commands to execute
//
void interpreter_loop(void)
{
  while (OS_prompt_available())
  {
    interpreter_run(0, 0);
    OS_prompt_buffer(program_end + sizeof(LINENUM), sp);
  }
}

//
// The main interpreter engine
//
unsigned char interpreter_run(LINENUM gofrom, unsigned char canreturn)
{
  unsigned char *start;
  unsigned char linelen;
  VAR_TYPE val;
  
  error_num = ERROR_OK;

  if (gofrom)
  {
    stack_event_frame *f;

    linenum = gofrom;
    if (canreturn)
    {
      if(sp - sizeof(stack_event_frame) < program_end)
        goto qoom;
      
      sp -= sizeof(stack_event_frame);
      f = (stack_event_frame *)sp;
      f->header.frame_type = STACK_EVENT_FLAG;
      f->header.frame_size = sizeof(stack_event_frame);
    }
    current_line = findline();
    goto execline;
  }

  txtpos = program_end + sizeof(LINENUM);
  tokenize();

  // Move it to the end of program_memory
  // Find the end of the freshly entered line
  for(linelen = 0; txtpos[linelen++] != NL;)
    ;
  OS_rmemcpy(sp - linelen, txtpos, linelen);
  txtpos = sp - linelen;

  // Now see if we have a line number
  linenum = testnum();
  ignore_blanks();
  if(linenum == 0)
    goto direct;

  if(linenum == 0xFFFF)
    goto qwhat;

  // Allow space for line header
  txtpos -= sizeof(LINENUM) + sizeof(char);

  // Calculate the length of what is left, including the (yet-to-be-populated) line header
  linelen = sp - txtpos;

  // Now we have the number, add the line header.
  *((LINENUM*)txtpos) = linenum;
  txtpos[sizeof(LINENUM)] = linelen;

  // Merge it into the rest of the program
  start = findline();

  // If a line with that number exists, then remove it
  if(start != program_end && *((LINENUM *)start) == linenum)
  {
    val = start[sizeof(LINENUM)];
    program_end -= val;
    OS_memcpy(start, start + val, program_end - start);
  }

  if(txtpos[sizeof(LINENUM) + sizeof(char)] == NL) // If the line has no txt, it was just a delete
    return IX_PROMPT;

  // Make room for new line if we can
  if ((unsigned short)(txtpos - program_end) < linelen)
    return IX_OUTOFMEMORY;

  // Move program up to make space
  OS_rmemcpy(start + linelen, start, program_end - start);
  // Insert new line
  OS_memcpy(start, txtpos, linelen);
  program_end += linelen;

  return IX_PROMPT;

// -- Commands ---------------------------------------------------------------

run_next_statement:
  ignore_blanks();
  if(*txtpos != NL)
  {
    goto qwhat;
  }
  goto execnextline;

direct:
  current_line = NULL;
  txtpos = program_end + sizeof(LINENUM);
  if(*txtpos == NL)
  {
    return IX_PROMPT;
  }

interperate:
  table_index = *txtpos++;
  if (table_index < 0x80)
  {
    txtpos--;
    goto assignment;
  }
  if (table_index >= PIN_P0 && table_index <= PIN_P2)
  {
    txtpos--;
    goto assignment;
  }
  else switch(table_index)
  {
  default:
    goto qwhat;
  case KW_LIST:
    goto list;
  case KW_LOAD:
    goto load;
  case KW_SAVE:
    goto save;
  case KW_DLOAD:
    goto cmd_dload;
  case KW_DSAVE:
    goto cmd_dsave;
  case KW_MEM:
    goto mem;
  case KW_NEW:
    if(txtpos[0] != NL)
    {
      goto qwhat;
    }
    program_end = program_start;
    // Fall through
  case KW_RUN:
    while (sp < variables_begin)
    {
      if (((stack_header*)sp)->frame_type == STACK_SERVICE_FLAG)
      {
        gattAttribute_t* attr;
        GATTServApp_DeregisterService(((stack_service_frame*)sp)->attrs[0].handle, &attr);
      }
      sp += ((stack_header*)sp)->frame_size;
    }
    current_line = program_start;
    goto execline;
  case KW_NEXT:
    goto next;
  case KW_LET:
    goto assignment;
  case KW_IF:
    if (*txtpos == KW_END)
    {
      goto execnextline;
    }
    // Fall through
  case KW_ELIF:
    goto cmd_elif;
  case KW_ELSE:
    goto cmd_else;
  case KW_GOTO:
    linenum = expression();
    if(error_num || *txtpos != NL)
    {
      goto qwhat;
    }
    current_line = findline();
    goto execline;
  case KW_GOSUB:
    goto gosub;
  case KW_RETURN:
    goto gosub_return;
  case KW_REM:
    goto execnextline;	// Ignore line completely
  case KW_FOR:
    goto forloop;
  case KW_PRINT:
    goto print;
  case KW_REBOOT:
    OS_reboot();
  case KW_END:
  case KW_STOP:
    if(txtpos[0] != NL)
    {
      goto qwhat;
    }
    current_line = program_end;
    goto execline;
  case KW_DIM:
    goto cmd_dim;
  case KW_TIMER:
    goto cmd_timer;
  case KW_DELAY:
    goto cmd_delay;
  case KW_AUTORUN:
    goto cmd_autorun;
  case BLE_GATT:
    goto ble_gatt;
  case BLE_ADVERT:
    ble_isadvert = 1;
    goto ble_advert_scan;
  case BLE_SCAN:
    ble_isadvert = 0;
    goto ble_advert_scan;
  case BLE_GAPROLE:
    goto ble_gaprole;
  case BLE_GAP:
    goto ble_gap;
  case KW_PINMODE:
    goto cmd_pinmode;
  case KW_ATTACHINTERRUPT:
    goto cmd_attachint;
  case KW_DETACHINTERRUPT:
    goto cmd_detachint;
  case KW_SPI:
    goto cmd_spi;
  case KW_TRANSFER:
    goto qwhat;
  }

// -- Errors -----------------------------------------------------------------
  
qwhat:
  if (!error_num)
  {
    error_num = ERROR_GENERAL;
  }
  goto execnextline;
  
qoom:
  error_num = ERROR_OOM;
  goto execnextline;
  
qtoobig:
  error_num = ERROR_TOOBIG;
  goto execnextline;
  
qbadpin:
  error_num = ERROR_BADPIN;
  goto execnextline;

// ---------------------------------------------------------------------------

execnextline:
  if (error_num)
  {
    printmsg(error_msgs[error_num]);
    if (current_line != NULL)
    {
      list_line = current_line;
      OS_putchar('>');
      OS_putchar('>');
      OS_putchar(WS_SPACE);
      printline(1);
      OS_putchar(NL);
    }
    return error_num == ERROR_OOM ? IX_OUTOFMEMORY : IX_PROMPT;
  }
  else if (current_line == NULL) // Processed direct command
  {
    printmsg(error_msgs[ERROR_OK]);
    return IX_PROMPT;
  }
  else
  {
    current_line += current_line[sizeof(LINENUM)];
execline:
    if(current_line == program_end) // Out of lines to run
    {
      printmsg(error_msgs[ERROR_OK]);
      return IX_PROMPT;
    }
    txtpos = current_line + sizeof(LINENUM) + sizeof(char);
    goto interperate;
  }

// --- Commands --------------------------------------------------------------

cmd_elif:
  
    if (current_line == NULL)
    {
      return IX_PROMPT;
    }
    val = expression();
    if(error_num || *txtpos != NL)
    {
      goto qwhat;
    }
    if (val)
    {
      goto execnextline;
    }
    else
    {
      unsigned char nest = 0;
      for (;;)
      {
        current_line +=	current_line[sizeof(LINENUM)];
        if(current_line == program_end)
        {
          printmsg(error_msgs[ERROR_OK]);
          return IX_PROMPT;
        }
        txtpos = current_line+sizeof(LINENUM)+sizeof(char);
        switch (*txtpos)
        {
          case KW_IF:
            if (txtpos[1] == KW_END)
            {
              if (!nest)
              {
                goto execnextline;
              }
              nest--;
            }
            else
            {
              nest++;
            }
            break;
          case KW_ELSE:
            if (!nest)
            {
              goto execnextline;
            }
            break;
          case KW_ELIF:
            if (!nest)
            {
              goto interperate;
            }
            break;
          default:
            break;
        }
      }
    }
 
cmd_else:
  {
    unsigned char nest = 0;
    for (;;)
    {
      current_line +=	current_line[sizeof(LINENUM)];
      if(current_line == program_end)
      {
        printmsg(error_msgs[ERROR_OK]);
        return IX_PROMPT;
      }
      txtpos = current_line+sizeof(LINENUM)+sizeof(char);
      if (txtpos[0] == KW_IF)
      {
        if (txtpos[1] == KW_END)
        {
          if (!nest)
          {
            goto execnextline;
          }
          nest--;
        }
        else
        {
          nest++;
        }
      }
    }
   }

forloop:
  {
    unsigned char var;
    VAR_TYPE initial, step, terminal;
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qwhat;
    var = *txtpos;
    txtpos++;
    ignore_blanks();
    if(*txtpos != RELOP_EQ)
      goto qwhat;
    txtpos++;

    initial = expression();
    if (error_num)
    {
      goto qwhat;
    }

    if (*txtpos++ != KW_TO)
    {
      goto qwhat;
    }

    terminal = expression();
    if (error_num)
    {
      goto qwhat;
    }

    if (*txtpos++ == KW_STEP)
    {
      step = expression();
      if (error_num)
      {
        goto qwhat;
      }
    }
    else
    {
      txtpos--;
      step = 1;
    }
    ignore_blanks();
    if(*txtpos != NL)
    {
      goto qwhat;
    }

    if(!error_num && *txtpos == NL)
    {
      stack_for_frame *f;
      if (sp - sizeof(stack_for_frame) < program_end)
      {
        goto qoom;
      }

      sp -= sizeof(stack_for_frame);
      f = (stack_for_frame *)sp;
      VARIABLE_INT_SET(var, initial);
      f->header.frame_type = STACK_FOR_FLAG;
      f->header.frame_size = sizeof(stack_for_frame);
      f->for_var = var;
      f->terminal = terminal;
      f->step = step;
      f->txtpos = txtpos;
      f->current_line = current_line;
      goto run_next_statement;
    }
  }
  goto qwhat;

gosub:
  linenum = expression();
  if (!error_num && *txtpos == NL)
  {
    stack_gosub_frame *f;
    if (sp + sizeof(stack_gosub_frame) < program_end)
    {
      goto qoom;
    }

    sp -= sizeof(stack_gosub_frame);
    f = (stack_gosub_frame *)sp;
    f->header.frame_type = STACK_GOSUB_FLAG;
    f->header.frame_size = sizeof(stack_gosub_frame);
    f->txtpos = txtpos;
    f->current_line = current_line;
    current_line = findline();
    goto execline;
  }
  goto qwhat;
  
next:
  // Find the variable name
  if (*txtpos < 'A' || *txtpos > 'Z')
  {
    goto qwhat;
  }
  txtpos++;
  ignore_blanks();
  if (*txtpos != NL)
  {
    goto qwhat;
  }

gosub_return:
  // Now walk up the stack frames and find the frame we want, if present
  while (sp < variables_begin)
  {
    switch (((stack_header*)sp)->frame_type)
    {
      case STACK_GOSUB_FLAG:
        if (table_index == KW_RETURN)
        {
          stack_gosub_frame *f = (stack_gosub_frame *)sp;
          current_line = f->current_line;
          txtpos = f->txtpos;
          sp += f->header.frame_size;
          goto run_next_statement;
        }
        // This is not the loop you are looking for... so walk back up the stack
        break;
      case STACK_EVENT_FLAG:
        if (table_index == KW_RETURN)
        {
          sp += ((stack_header*)sp)->frame_size;
          return IX_PROMPT;
        }
        break;
      case STACK_FOR_FLAG:
        // Flag, Var, Final, Step
        if (table_index == KW_NEXT)
        {
          stack_for_frame *f = (stack_for_frame *)sp;
          // Is the the variable we are looking for?
          if (txtpos[-1] == f->for_var)
          {
            VAR_TYPE v = VARIABLE_INT_GET(f->for_var) + f->step;
            VARIABLE_INT_SET(f->for_var, v);
            // Use a different test depending on the sign of the step increment
            if ((f->step > 0 && v <= f->terminal) || (f->step < 0 && v >= f->terminal))
            {
              // We have to loop so don't pop the stack
              txtpos = f->txtpos;
              current_line = f->current_line;
            }
            else
            {
              // We've run to the end of the loop. drop out of the loop, popping the stack
              sp += f->header.frame_size;
            }
            goto run_next_statement;
          }
        }
        // This is not the loop you are looking for... so Walk back up the stack
        break;
      case STACK_VARIABLE_FLAG:
        {
          stack_variable_frame* f = (stack_variable_frame*)sp;
          VARIABLE_FLAGS_SET(f->name, f->oflags);
        }
        break;
      case STACK_SERVICE_FLAG:
        {
          gattAttribute_t* attr;
          GATTServApp_DeregisterService(((stack_service_frame*)sp)->attrs[0].handle, &attr);
        }
        break;
      default:
        goto qoom;
    }
    sp += ((stack_header*)sp)->frame_size;
  }
  // Didn't find the variable we've been looking for
  goto qwhat;

//
// X = Y
// Assign a value to a variable or pin.
//
assignment:
  {
    unsigned char var;
    stack_variable_frame* frame;
    unsigned char* ptr;
    VAR_TYPE idx;

    // Are we setting an output pin?
    var = *txtpos;
    if (var >= PIN_P0 && var <= PIN_P2)
    {
      unsigned char pin;
      
      pin = pin_parse();
      if (error_num)
      {
        goto qwhat;
      }
      ignore_blanks();
      if (*txtpos != RELOP_EQ)
      {
        goto qwhat;
      }
      txtpos++;
      val = expression();
      if (error_num)
      {
        goto qwhat;
      }
      pin_write(pin, val);

      goto run_next_statement;
    }

    if (*txtpos < 'A' || *txtpos > 'Z')
    {
      goto qwhat;
    }
    var = *txtpos;
    ptr = get_variable_frame(var, &frame);
    
    txtpos++;
    ignore_blanks();

    switch (frame->type)
    {
      case VAR_INT:
        if (*txtpos != RELOP_EQ)
        {
          goto qwhat;
        }
        txtpos++;
        val = expression();
        if(error_num || *txtpos != NL)
        {
          goto qwhat;
        }
        *(VAR_TYPE*)ptr = val;
        goto var_done;

      case VAR_DIM_BYTE:
        if (*txtpos != '(')
        {
          goto qwhat;
        }
        txtpos++;
        idx = expression();
        if (error_num)
        {
          goto qwhat;
        }
        ignore_blanks();
        if (*txtpos != ')')
        {
          goto qwhat;
        }
        txtpos++;
        ignore_blanks();
        if (idx < 0 || idx >= frame->header.frame_size - sizeof(stack_variable_frame) || *txtpos != RELOP_EQ)
        {
          goto qwhat;
        }
        txtpos++;
        val = expression();
        if (error_num || *txtpos != NL)
        {
          goto qwhat;
        }
        ptr[idx] = (unsigned char)val;
var_done:
        if (frame->ble)
        {
          ble_notify_assign(frame->ble);
        }
        goto run_next_statement;

      default:
        goto qwhat;
    }
  }

//
// LIST [<linenr>]
// List the current program starting at the optional line number.
//
list:
  {
    unsigned char indent = 1;

    linenum = testnum(); // Returns 0 if no line found.

    // Should be EOL
    if (*txtpos != NL)
    {
      goto qwhat;
    }

    // Find the line
    list_line = findline();
    while(list_line != program_end)
    {
      switch (list_line[sizeof(LINENUM) + sizeof(char)])
      {
        case KW_IF:
          if (list_line[sizeof(LINENUM) + sizeof(char) + 1] == KW_END)
          {
            goto indent_next;
          }
          else
          {
            goto indent_for;
          }
        case KW_ELSE:
        case KW_ELIF:
          indent--;
          // Fall through
        case KW_FOR:
indent_for:
          printline(indent);
          indent++;
          break;
        case KW_NEXT:
indent_next:
          indent--;
          // Fall through
        default:
          printline(indent);
          break;
      }
    }
  }
  goto run_next_statement;

print:
  for (;;)
  {
    ignore_blanks();
    if (*txtpos == NL)
    {
      break;
    }
    else if (!print_quoted_string())
    {
      if(*txtpos == '"' || *txtpos == '\'')
      {
        goto qwhat;
      }
      else
      {
        VAR_TYPE e;
        e = expression();
        if (error_num || (*txtpos != NL && *txtpos != WS_SPACE))
        {
          goto qwhat;
        }
        printnum(e);
      }
    }
  }
  OS_putchar(NL);
  goto run_next_statement;

//
// MEM
// Print the current free memory.
//
mem:
  printnum((VAR_TYPE)(sp - program_end));
  printmsg(memorymsg);
  goto run_next_statement;

//
// LOAD
// Load any program in the persistent store back into memory.
//
load:
  {
    short len;
    // clear the program
    program_end = program_start;
    
    // load from a file into memory
    len = OS_file_open(0, 'r');
    if (len > 0)
    {
      program_end = program_start + len;
      OS_file_read(program_start, len);
    }
    OS_file_close();
    current_line = program_end;
  }
  goto execline;

//
// SAVE
// Save the current program to persistent store.
// 
save:
  OS_file_open(0, 'w');
  OS_file_write(program_start, program_end - program_start);
  OS_file_close();
  current_line = program_end;
  goto execline;

//
// DLOAD <1-15> <var>
// Load the value in the numbered store into <var> (which may be a DIM).
// The store persists across reboots/power-on/off events.
//
cmd_dload:
  {
    unsigned char chan;
    stack_variable_frame* vframe;
    unsigned char var;
    unsigned char* ptr;
    
    chan = expression();
    if (error_num || chan == 0)
    {
      goto qwhat;
    }

    ignore_blanks();
    var = *txtpos++;
    if (var < 'A' || var > 'Z')
    {
      txtpos--;
      goto dsave_error;
    }
    if (*txtpos != WS_SPACE && *txtpos != NL)
    {
      goto dsave_error;
    }

    if (OS_file_open(chan, 'r') < 0)
    {
      goto qwhat;
    }
    ptr = get_variable_frame(var, &vframe);
    OS_file_read(ptr, vframe->type == VAR_DIM_BYTE ? vframe->header.frame_size - sizeof(stack_variable_frame) : VAR_SIZE);
    OS_file_close();

    goto run_next_statement;
  }

//
// DSAVE <1-15> <var>
// Save the value of <var> (which may be a DIM) to the numbered store. This
// store persists across reboots and power-off/on.
//
cmd_dsave:
  {
    unsigned char chan;
    stack_variable_frame* vframe;
    unsigned char var;
    unsigned char* ptr;

    chan = expression();
    if (error_num || chan == 0)
    {
      goto qwhat;
    }

    ignore_blanks();
    var = *txtpos++;
    if (var < 'A' || var > 'Z')
    {
      txtpos--;
      goto dsave_error;
    }
    if (*txtpos != WS_SPACE && *txtpos != NL)
    {
      goto dsave_error;
    }

    if (OS_file_open(chan, 'w') < 0)
    {
      goto qwhat;
    }
    ptr = get_variable_frame(var, &vframe);
    OS_file_write(ptr, vframe->type == VAR_DIM_BYTE ? vframe->header.frame_size - sizeof(stack_variable_frame) : VAR_SIZE);
    OS_file_close();

    goto run_next_statement;
dsave_error:
    OS_file_close();
    goto qwhat;
  }

//
// DIM <var>(<size>)
// Converts a variable into an array of bytes of the given size.
//
cmd_dim:
  {
    VAR_TYPE size;
    unsigned char name;

    if(*txtpos < 'A' || *txtpos > 'Z')
    {
      goto qwhat;
    }
    name = *txtpos;
    txtpos++;
    ignore_blanks();
    if(*txtpos != '(')
    {
      goto qwhat;
    }
    txtpos++;
    ignore_blanks();
    size = expression();
    if (error_num)
    {
      goto qwhat;
    }
    ignore_blanks();
    if (*txtpos != ')')
    {
      goto qwhat;
    }
    txtpos++;
    // Check that we are at the end of the statement
    if (*txtpos != NL)
    {
      goto qwhat;
    }
    if(sp - sizeof(stack_variable_frame) - size < program_end)
    {
      goto qoom;
    }
    sp -= sizeof(stack_variable_frame) + size;
    stack_variable_frame *f = (stack_variable_frame *)sp;
    f->header.frame_type = STACK_VARIABLE_FLAG;
    f->header.frame_size = sizeof(stack_variable_frame) + size;
    f->type = VAR_DIM_BYTE;
    f->name = name;
    f->oflags = VARIABLE_FLAGS_GET(name);
    f->ble = NULL;
    VARIABLE_FLAGS_SET(name, VAR_VARIABLE);
    OS_memset(sp + sizeof(stack_variable_frame), 0, size);
  }
  goto run_next_statement;

//
// TIMER <timer number> <timeout ms> [REPEAT] GOSUB <linenum>
// Creates an optionally repeating timer which will call a specific subroutine everytime it fires.
//
cmd_timer:
  {
    unsigned char timer;
    VAR_TYPE timeout;
    unsigned char repeat = 0;
    LINENUM subline;
    
    timer = (unsigned char)expression();
    if (error_num)
    {
      goto qwhat;
    }
    ignore_blanks();
    if (*txtpos == KW_STOP)
    {
      txtpos++;
      OS_timer_stop(timer);
    }
    else
    {
      timeout = expression();
      if (error_num)
      {
        goto qwhat;
      }
      ignore_blanks();
      if (*txtpos == KW_REPEAT)
      {
        txtpos++;
        repeat = 1;
      }
      if (*txtpos != KW_GOSUB)
      {
        goto qwhat;
      }
      txtpos++;
      subline = (LINENUM)expression();
      if (error_num)
      {
        goto qwhat;
      }
      if (!OS_timer_start(timer, timeout, repeat, subline))
      {
        goto qwhat;
      }
    }
  }
  goto run_next_statement;

//
// DELAY <timeout>
// Pauses execution for timeout-ms 
//
cmd_delay:
  {
    VAR_TYPE timeout;
  
    timeout = expression();
    if (error_num)
    {
      goto qwhat;
    }
    current_line += current_line[sizeof(LINENUM)];
    OS_timer_start(DELAY_TIMER, timeout, 0, *(LINENUM*)current_line);
  }
  return IX_PROMPT;

//
// AUTORUN ON|OFF
// When on, will load the save program at boot time and execute it.
//
cmd_autorun:
  {
    VAR_TYPE v = expression();
    if (error_num)
    {
      goto qwhat;
    }
    OS_autorun_set(v ? 1 : 0);
  }
  goto run_next_statement;

cmd_pinmode:
  {
    unsigned char pin;
    unsigned char pinMode;
    unsigned char dbit;
    
    pin = pin_parse();
    if (error_num)
    {
      goto qwhat;
    }

    dbit = 1 << PIN_MINOR(pin);

    ignore_blanks();
    pinMode = *txtpos++;
    if (pinMode == PM_INPUT)
    {
      ignore_blanks();
      pinMode = *txtpos++;
      if (pinMode == NL)
      {
        txtpos--;
        pinMode = PM_INPUT;
      }
      else if (pinMode != PM_PULLUP && pinMode != PM_PULLDOWN)
      {
        txtpos--;
        goto qwhat;
      }
    }
    else if (pinMode != PM_OUTPUT)
    {
      txtpos--;
      goto qwhat;
    }
    
    if (PIN_MAJOR(pin) == 0) // PIN_P0
    {
      P0SEL &= ~dbit;
      APCFG &= ~dbit;
      P0DIR = (P0DIR & ~dbit) | (pinMode == PM_OUTPUT ? dbit : 0);
      switch (pinMode)
      {
        case PM_INPUT:
          P0INP |= dbit;
          break;
        case PM_PULLUP:
          P0INP &= ~dbit;
          P2INP &= ~(1 << 5);
          break;
        case PM_PULLDOWN:
          P0INP &= ~dbit;
          P2INP |= 1 << 5;
          break;
        case PM_ADC:
          APCFG |= dbit;
          break;
      }
    }
    else if (PIN_MAJOR(pin) == 1) // PIN_P1
    {
      P1SEL &= ~dbit;
      P1DIR = (P1DIR & ~dbit) | (pinMode == PM_OUTPUT ? dbit : 0);
      switch (pinMode)
      {
        case PM_INPUT:
          P1INP |= dbit;
          break;
        case PM_PULLUP:
          P1INP &= ~dbit;
          P2INP &= ~(1 << 6);
          break;
        case PM_PULLDOWN:
          P1INP &= ~dbit;
          P2INP |= 1 << 6;
          break;
        case PM_ADC:
          goto qwhat;
      }
    }
    else
    {
      P2SEL &= ~dbit;
      P2DIR = (P2DIR & ~dbit) | (pinMode == PM_OUTPUT ? dbit : 0);
      switch (pinMode)
      {
        case PM_INPUT:
          P2INP |= dbit;
          break;
        case PM_PULLUP:
          P2INP &= ~dbit;
          P2INP &= ~(1 << 7);
          break;
        case PM_PULLDOWN:
          P2INP &= ~dbit;
          P2INP |= 1 << 7;
          break;
        case PM_ADC:
          goto qwhat;
      }
    }
  }
  goto run_next_statement;

//
// ATTACHINTERRUPT <pin> RISING|FALLING GOSUB <linenr>
// Attach an interrupt handler to the given pin. When the pin either falls or rises, the specified
// subroutine will be called.
//
cmd_attachint:
  {
    unsigned short pin;
    LINENUM line;
    unsigned char falling = 0;
    unsigned char i;
    
    pin = pin_parse();
    if (error_num)
    {
      goto qwhat;
    }
    if (PIN_MAJOR(pin) == 2 && PIN_MINOR(pin) >= 4)
    {
      goto qbadpin;
    }
    ignore_blanks();
    switch (*txtpos++)
    {
      case PM_RISING:
        falling = 0;
        break;
      case PM_FALLING:
        falling = 1;
        break;
      default:
        txtpos--;
        goto qwhat;
    }
    if (*txtpos++ != KW_GOSUB)
    {
      txtpos--;
      goto qwhat;
    }
    line = expression();
    if (error_num)
    {
      goto qwhat;
    }
    
    if (!OS_interrupt_attach(pin, line))
    {
      goto qbadpin;
    }

    i = 1 << PIN_MINOR(pin);
    if (PIN_MAJOR(pin) == 0)
    {
      PICTL = (PICTL & 0x01) | falling;
      P0IEN |= i;
      IEN1 |= 1 << 5;
    }
    else if (PIN_MAJOR(pin) == 1)
    {
      if (PIN_MINOR(pin) < 4)
      {
        PICTL = (PICTL & 0x02) | (falling << 1);
        P1IEN |= i;
        IEN2 |= 1 << 4;
      }
      else
      {
        PICTL = (PICTL & 0x04) | (falling << 2);
        P1IEN |= i;
        IEN2 |= 1 << 4;
      }
    }
    else
    {
      PICTL = (PICTL & 0x08) | (falling << 3);
      P2IEN |= i;
      IEN2 |= 1 << 1;
    }
  }
  goto run_next_statement;

//
// DETACHINTERRUPT <pin>
// Detach the interrupt handler from the pin.
//
cmd_detachint:
  {
    unsigned short pin;
    unsigned char i;
    
    pin = pin_parse();
    if (error_num)
    {
      goto qwhat;
    }
    
    i = ~(1 << PIN_MINOR(pin));
    if (PIN_MAJOR(pin) == 0)
    {
      P0IEN &= i;
      if (P0IEN == 0)
      {
        IEN1 &= ~(1 << 5);
      }
    }
    else if (PIN_MAJOR(pin) == 1)
    {
      P1IEN &= i;
      if (P1IEN == 0)
      {
        IEN2 &= ~(1 << 4);
      }
    }
    else
    {
      P2IEN &= i;
      if (P2IEN == 0)
      {
        IEN2 &= ~(1 << 1);
      }
    }

    if (!OS_interrupt_detach(pin))
    {
      goto qbadpin;
    }
    goto run_next_statement;
  }

  unsigned char ch;
ble_gatt:
  switch(*txtpos++)
  {
    case BLE_SERVICE:
      servicestart = *(LINENUM*)current_line;
      servicecount = 1;
      break;
    case BLE_READ:
    case BLE_WRITE:
    case BLE_WRITENORSP:
    case BLE_NOTIFY:
    case BLE_INDICATE:
      {
        servicecount++;
        for (;;)
        {
          switch (*txtpos++)
          {
            case BLE_NOTIFY:
            case BLE_INDICATE:
              servicecount++;
              break;
            case BLE_READ:
            case BLE_WRITE:
            case BLE_WRITENORSP:
              break;
            default:
              goto value_done;
          }
        }
value_done:
        ch = *--txtpos;
        if (ch >= 'A' && ch <= 'Z')
        {
          stack_variable_frame* vframe;
          get_variable_frame(ch, &vframe);
          if (vframe->type == STACK_VARIABLE_NORMAL)
          {
            if(sp - sizeof(stack_variable_frame) < program_end)
            {
              goto qoom;
            }
            sp -= sizeof(stack_variable_frame);
            vframe = (stack_variable_frame*)sp;
            vframe->header.frame_type = STACK_VARIABLE_FLAG;
            vframe->header.frame_size = sizeof(stack_variable_frame);
            vframe->type = VAR_INT;
            vframe->name = ch;
            vframe->oflags = VARIABLE_FLAGS_GET(ch);
            vframe->ble = NULL;
            VARIABLE_FLAGS_SET(ch, VAR_VARIABLE);
          }
        }
        else
        {
          goto qwhat;
        }
      }
      break;
    case BLE_CHARACTERISTIC:
      servicecount += 2; // NB. We include an extra for the description. Optimize later!
      break;
    case KW_END:
      switch (ble_build_service())
      {
        default:
          break;
        case 1:
          goto qwhat;
        case 2:
          goto qoom;
      }
      break;
    default:
      goto qwhat;
  }
  goto execnextline;

ble_advert_scan:
  if (!ble_adptr)
  {
    ble_adptr = ble_adbuf;
  }
  ch = *txtpos;
  switch (ch)
  {
    case BLE_LIMITED:
      if (ble_adptr + 3 > ble_adbuf + sizeof(ble_adbuf))
      {
        goto qtoobig;
      }
      *ble_adptr++ = 2;
      *ble_adptr++ = GAP_ADTYPE_FLAGS;
      *ble_adptr++ = GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED;
      break;
    case BLE_GENERAL:
      if (ble_adptr + 3 > ble_adbuf + sizeof(ble_adbuf))
      {
        goto qtoobig;
      }
      *ble_adptr++ = 2;
      *ble_adptr++ = GAP_ADTYPE_FLAGS;
      *ble_adptr++ = GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED;
      break;
    case BLE_NAME:
      {
        ch = *++txtpos;
        if (ch != SQUOTE && ch != DQUOTE)
        {
          goto qwhat;
        }
        else
        {
          short len;
          short alen;

          len = find_quoted_string();
          if (len == -1)
          {
            goto qwhat;
          }
          if (ble_adptr + 3 > ble_adbuf + sizeof(ble_adbuf))
          {
            goto qtoobig;
          }
          else
          {
            alen = ble_adbuf + sizeof(ble_adbuf) - ble_adptr - 2;
            if (alen > len)
            {
              *ble_adptr++ = len + 1;
              *ble_adptr++ = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
              alen = len;
            }
            else
            {
              *ble_adptr++ = alen + 1;
              *ble_adptr++ = GAP_ADTYPE_LOCAL_NAME_SHORT;
            }
            OS_memcpy(ble_adptr, txtpos, alen);
            ble_adptr += alen;
            GGS_SetParameter(GGS_DEVICE_NAME_ATT, len < GAP_DEVICE_NAME_LEN ? len : GAP_DEVICE_NAME_LEN, txtpos);
            txtpos += len + 1;
          }
        }
      }
      break;
    case BLE_CUSTOM:
      {
        unsigned char* lenptr;
        if (ble_adptr == NULL)
        {
          goto qwhat;
        }
        if (ble_adptr + 1 > ble_adbuf + sizeof(ble_adbuf))
        {
          goto qtoobig;
        }
        lenptr = ble_adptr++;

        for (;;)
        {
          ch = *++txtpos;
          if (ch == NL)
          {
            *lenptr = (unsigned char)(ble_adptr - lenptr - 1);
            break;
          }
          else if (ch == SQUOTE || ch == DQUOTE)
          {
            unsigned short v;
            txtpos++;
            for (;;)
            {
              ignore_blanks();
              v = parse_int(2, 16);
              if (error_num)
              {
                if (*txtpos == ch)
                {
                  error_num = ERROR_OK;
                  break;
                }
                goto qwhat;
              }
              if (ble_adptr + 1 > ble_adbuf + sizeof(ble_adbuf))
              {
                goto qtoobig;
              }
              *ble_adptr++ = (unsigned char)v;
            }
          }
          else if (ch >= 'A' && ch <= 'Z')
          {
            stack_variable_frame* frame;
            unsigned char* ptr;
            
            if (txtpos[1] != NL && txtpos[1] != WS_SPACE)
            {
              goto qwhat;
            }

            ptr = get_variable_frame(ch, &frame);
            if (frame->type != VAR_DIM_BYTE)
            {
              goto qwhat;
            }
            if (ble_adptr + frame->header.frame_size - sizeof(stack_variable_frame) > ble_adbuf + sizeof(ble_adbuf))
            {
              goto qtoobig;
            }
            for (ch = sizeof(stack_variable_frame); ch < frame->header.frame_size; ch++)
            {
              *ble_adptr++ = *ptr++;
            }
          }
          else if (ch != WS_SPACE)
          {
            goto qwhat;
          }
        }
      }
      break;
    case KW_END:
      if (ble_adptr == ble_adbuf)
      {
        ble_adptr = NULL;
        goto qwhat;
      }
      if (ble_isadvert)
      {
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, ble_adptr - ble_adbuf, ble_adbuf);
        ch = 1;
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(ch), &ch);
      }
      else
      {
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, ble_adptr - ble_adbuf, ble_adbuf);
      }
      ble_adptr = NULL;
      break;
    default:
      if (ch != SQUOTE && ch != DQUOTE)
      {
        goto qwhat;
      }
      else
      {
        ch = ble_get_uuid();
        if (!ch)
        {
          goto qwhat;
        }
        if (ble_adptr + 2 + ble_uuid_len > ble_adbuf + sizeof(ble_adbuf))
        {
          goto qtoobig;
        }
        ignore_blanks();
        *ble_adptr++ = ble_uuid_len + 1;
        switch (ble_uuid_len)
        {
          case 2:
            *ble_adptr++ = GAP_ADTYPE_16BIT_COMPLETE;
            break;
          case 4:
            *ble_adptr++ = GAP_ADTYPE_32BIT_COMPLETE;
            break;
          case 16:
            *ble_adptr++ = GAP_ADTYPE_128BIT_COMPLETE;
            break;
          default:
            goto qwhat;
        }
        OS_memcpy(ble_adptr, ble_uuid, ble_uuid_len);
        if (*txtpos == BLE_MORE)
        {
          ble_adptr[-1] &= 0xFE;
        }
        ble_adptr += ble_uuid_len;
      }
      break;
  }
  goto execnextline;

ble_gaprole:
  {
    unsigned short param;
    unsigned short val;

    param = (unsigned short)expression();
    if (error_num)
    {
      goto qwhat;
    }
    ignore_blanks();
    val = (unsigned short)expression();
    if (error_num)
    {
      goto qwhat;
    }
    switch (param)
    {
      case GAPROLE_ADVERT_ENABLED:
        ch = GAPRole_SetParameter(param, sizeof(char), &val);
        break;
      default:
        ch = GAPRole_SetParameter(param, sizeof(short), &val);
        break;
    }
    if (ch != SUCCESS)
    {
      goto qwhat;
    }
  }
  goto execnextline;

ble_gap:
  {
    unsigned short param;
    unsigned short val;
    
    param = (unsigned short)expression();
    if (error_num)
    {
      goto qwhat;
    }
    ignore_blanks();
    val = (unsigned short)expression();
    if (error_num)
    {
      goto qwhat;
    }
    if (GAP_SetParamValue(param, val) != SUCCESS)
    {
      goto qwhat;
    }

  }
  goto execnextline;

//
// SPI <port 0|1|2|3> <mode 0|1|2|3> LSB|MSB <speed>
//  or
// SPI TRANSFER Px(y) <array>
//
cmd_spi:
  {
    if (*txtpos != KW_TRANSFER)
    {
      // Setup
      unsigned char port;
      unsigned char mode;
      unsigned char msblsb;
      unsigned char speed;

      port = expression();
      if (error_num || port > 3)
      {
        goto qwhat;
      }
      mode = expression();
      if (error_num || mode > 3)
      {
        goto qwhat;
      }
      msblsb = *txtpos;
      if (msblsb != KW_MSB && msblsb != KW_LSB)
      {
        goto qwhat;
      }
      txtpos++;
      speed = expression();
      if (error_num)
      {
        goto qwhat;
      }

      // Arduino compatible
      if (mode < 2)
      {
        mode = 1 - mode;
      }
      mode = (mode << 6) | (msblsb == KW_MSB ? 0x20 : 0x00);
      switch (speed)
      {
        case 1: // E == 15, M = 0
          mode |= 15;
          break;
        case 2: // E = 16
          mode |= 16;
          break;
        case 4: // E = 17
          mode |= 17;
          break;
        default:
          goto qwhat;
      }
      
      if ((port & 2) == 0)
      {
        spiChannel = 0;
        PERCFG = (PERCFG & 0xFE) | (port & 1);
        U0BAUD = 0;
        U0GCR = mode;
        U0CSR = 0xE0; // SPI mode, recv enable, SPI master
      }
      else
      {
        spiChannel = 1;
        PERCFG = (PERCFG & 0xFD) | ((port & 1) << 1);
        U1BAUD = 0;
        U1GCR = mode;
        U1CSR = 0xE0;
      }
    }
    else
    {
      // Transfer
      unsigned char pin;
      stack_variable_frame* vframe;
      unsigned char* ptr;
      unsigned short len;
      
      txtpos++;
      pin = pin_parse();
      if (error_num)
      {
        goto qwhat;
      }
      ignore_blanks();
      ch = *txtpos;
      if (ch < 'A' || ch > 'Z')
      {
        goto qwhat;
      }
      ptr = get_variable_frame(ch, &vframe);
      if (vframe->type != VAR_DIM_BYTE)
      {
        goto qwhat;
      }

      // .. transfer ..
      pin_write(pin, 0);
      if (spiChannel == 0)
      {
        for (len = vframe->header.frame_size - sizeof(stack_variable_frame); len > 0; len--, ptr++)
        {
          U0DBUF = *ptr;
#ifdef SIMULATE_PINS
          U0CSR |= 0x02;
#endif
          while ((U0CSR & 0x02) == 0)
            ;
          *ptr = U0DBUF;
        }
      }
      else
      {
        for (len = vframe->header.frame_size - sizeof(stack_variable_frame); len > 0; len--, ptr++)
        {
          U1DBUF = *ptr;
#ifdef SIMULATE_PINS
          U1CSR |= 0x02;
#endif
          while ((U1CSR & 0x02) == 0)
            ;
          *ptr = U1DBUF;
        }
      }
      pin_write(pin, 1);
    }
  }
  goto execnextline;
}

//
// Parse the immediate text into a PIN value.
// Pins are of the form PX(Y) and are returned as 0xXY
//
static unsigned char pin_parse(void)
{
  unsigned char major;
  unsigned char minor;
  
  major = *txtpos++;
  if (major < PIN_P0 || major > PIN_P2)
  {
    txtpos--;
    goto badpin;
  }
  minor = expression();
  if (error_num || minor > 7 || *txtpos != ')')
  {
    goto badpin;
  }
  txtpos++;
#ifdef ENABLE_DEBUG_INTERFACE
  if (major == PIN_P2 && (minor == 1 || minor == 2))
  {
    goto badpin;
  }
#endif
#ifdef OS_UART_PORT
#if OS_UART_PORT == HAL_UART_PORT_1
  if (major == PIN_P1 && minor >= 4)
  {
    goto badpin;
  }
#endif
#endif
  return ((major - PIN_P0) << 4) | minor;
badpin:
  error_num = ERROR_BADPIN;
  return 0;
}

//
// Write pin
//
void pin_write(unsigned char pin, unsigned char val)
{
  unsigned char bit = 1 << PIN_MINOR(pin);

  switch (PIN_MAJOR(pin))
  {
    case 0:
      P0 = (P0 & ~bit) | (val ? bit : 0);
      break;
    case 1:
      P1 = (P1 & ~bit) | (val ? bit : 0);
      break;
    default:
      P2 = (P2 & ~bit) | (val ? bit : 0);
      break;
  }
}

//
// Read pin
//
VAR_TYPE pin_read(unsigned char pin)
{
  VAR_TYPE minor = PIN_MINOR(pin);

  switch (PIN_MAJOR(pin))
  {
    case 0:
      if (APCFG & (1 << minor))
      {
        ADCCON3 = minor | 0x30 | 0x00; // 14-bit, internal voltage reference
#ifdef SIMULATE_PINS
        ADCCON1 = 0x80;
#endif
        while ((ADCCON1 & 0x80) == 0)
          ;
        minor = ADCL;
        minor |= ADCH << 8;
        return minor >> 2;
      }
      return (P0 >> minor) & 1;
    case 1:
      return (P1 >> minor) & 1;
    default:
      return (P2 >> minor) & 1;
  }
}

//
// Build a new BLE service and register it with the system
//
static char ble_build_service(void)
{
  unsigned char* line;
  unsigned char* origsp;
  gattAttribute_t* attributes;
  unsigned char ch;
  unsigned char cmd;
  short val;
  short count = 0;
  unsigned char* chardesc = NULL;
  unsigned char chardesclen = 0;
  stack_service_frame* frame;
  LINENUM onconnect = 0;

  linenum = servicestart;
  line = findline();

  origsp = sp;
  // Allocate space on the stack for the service attributes
  sp -= sizeof(gattAttribute_t) * servicecount + sizeof(unsigned short);
  if(sp < program_end)
  {
    goto oom;
  }
  attributes = (gattAttribute_t*)(sp + sizeof(unsigned short));
  
  for (count = 0; count < servicecount; count++)
  {
    txtpos = line + sizeof(LINENUM) + sizeof(char);

    // Ignore all GATT commands (we did them already)
    if (*txtpos++ != BLE_GATT)
    {
      continue;
    }
    cmd = *txtpos++;

    if (cmd == KW_END)
    {
      break;
    }
    
    // Some defaults
    attributes[count].type.uuid = NULL;
    attributes[count].type.len = 2;
    attributes[count].permissions = GATT_PERMIT_READ;
    attributes[count].handle = 0;
    *(unsigned char**)&attributes[count].pValue = NULL;

    if (cmd == BLE_SERVICE || cmd == BLE_CHARACTERISTIC)
    {
      ch = *txtpos;
      if (ch == '"' || ch == '\'')
      {
        ch = ble_get_uuid();
        if (!ch)
        {
          goto error;
        }
        if (cmd == BLE_SERVICE)
        {
          attributes[count].type.uuid = ble_primary_service_uuid;
          sp -= ble_uuid_len + sizeof(gattAttrType_t);
          if(sp < program_end)
            goto oom;
          OS_memcpy(sp + sizeof(gattAttrType_t), ble_uuid, ble_uuid_len);
          ((gattAttrType_t*)sp)->len = ble_uuid_len;
          ((gattAttrType_t*)sp)->uuid = sp + sizeof(gattAttrType_t);
          *(unsigned char**)&attributes[count].pValue = sp;
          
          ignore_blanks();
          if (*txtpos == BLE_ONCONNECT)
          {
            if (txtpos[1] == KW_GOSUB)
            {
              txtpos += 2;
              onconnect = expression();
              if (error_num)
              {
                goto error;
              }
            }
            else
            {
              goto error;
            }
          }
        }

        if (cmd == BLE_CHARACTERISTIC)
        {
          attributes[count].type.uuid = ble_characteristic_uuid;

          ignore_blanks();
          
          // Description
          val = find_quoted_string();
          if (val != -1)
          {
            chardesc = txtpos;
            chardesclen = (unsigned char)val;
            txtpos += val + 1;
          }
          else
          {
            chardesc = NULL;
            chardesclen = 0;
          }
        }
      }
      else
      {
        goto error;
      }
    }
    else if (cmd >= BLE_READ && cmd <= BLE_INDICATE)
    {
      txtpos--;
      ch = 0;
      for (;;)
      {
        switch (*txtpos++)
        {
          case BLE_READ:
            ch |= GATT_PROP_READ;
            break;
          case BLE_WRITE:
            ch |= GATT_PROP_WRITE;
            break;
          case BLE_NOTIFY:
            ch |= GATT_PROP_NOTIFY;
            break;
          case BLE_INDICATE:
            ch |= GATT_PROP_INDICATE;
            break;
          case BLE_WRITENORSP:
            ch |= GATT_PROP_WRITE_NO_RSP;
            break;
          default:
            txtpos--;
            goto done;
        }
      }
done:
      sp--;
      if(sp < program_end)
      {
        goto oom;
      }
      sp[0] = ch;
      *(unsigned char**)&attributes[count - 1].pValue = sp;
      
      ch = *txtpos;
      if (ch < 'A' || ch > 'Z')
      {
        goto error;
      }

      gatt_variable_ref* vref;
      stack_variable_frame* vframe;
      
      txtpos++;
      if (*txtpos != WS_SPACE && *txtpos != NL && *txtpos < 0x80)
        goto error;
      
      sp -= ble_uuid_len + sizeof(gatt_variable_ref);
      if(sp < program_end)
        goto oom;
      vref = (gatt_variable_ref*)(sp + ble_uuid_len);
      vref->read = 0;
      vref->write = 0;
      vref->var = ch;
      vref->attrs = attributes;
      vref->cfg = NULL;
      
      OS_memcpy(sp, ble_uuid, ble_uuid_len);
      *(unsigned char**)&attributes[count].pValue = (unsigned char*)vref;
      attributes[count].permissions = GATT_PERMIT_READ | GATT_PERMIT_WRITE;
      attributes[count].type.uuid = sp;
      attributes[count].type.len = ble_uuid_len;

      for (;;)
      {
        ignore_blanks();
        ch = *txtpos;
        if (ch == BLE_ONREAD || ch == BLE_ONWRITE)
        {
          txtpos++;
          if (*txtpos != KW_GOSUB)
          {
            goto error;
          }
          txtpos++;
          ignore_blanks();
          linenum = expression();
          if (error_num)
          {
            goto error;
          }
          if (ch == BLE_ONREAD)
          {
            vref->read = linenum;
          }
          else
          {
            vref->write = linenum;
          }
        }
        else if (ch == NL)
        {
          break;
        }
        else
        {
          goto error;
        }
      }

      if ((attributes[count - 1].pValue[0] & (GATT_PROP_INDICATE|GATT_PROP_NOTIFY)) != 0)
      {
        sp -= sizeof(gattCharCfg_t) * GATT_MAX_NUM_CONN;
        if(sp < program_end)
        {
          goto oom;
        }
        GATTServApp_InitCharCfg(INVALID_CONNHANDLE, (gattCharCfg_t*)sp);

        count++;
        attributes[count].type.uuid = ble_client_characteristic_config_uuid;
        attributes[count].type.len = 2;
        attributes[count].permissions = GATT_PERMIT_READ | GATT_PERMIT_WRITE;
        attributes[count].handle = 0;
        *(unsigned char**)&attributes[count].pValue = sp;
        vref->cfg = sp;

        sp -= sizeof(stack_service_frame);
        if(sp < program_end)
          goto oom;
        frame = (stack_service_frame*)sp;
        frame->header.frame_type = STACK_SERVICE_FLAG;
        frame->header.frame_size = origsp - sp;
        get_variable_frame(vref->var, &vframe);
        vframe->ble = vref;
        sp += sizeof(stack_service_frame);
      }
 
      if (chardesclen > 0)
      {
        count++;
        attributes[count].type.uuid = ble_characteristic_description_uuid;
        attributes[count].type.len = 2;
        attributes[count].permissions = GATT_PERMIT_READ;
        attributes[count].handle = 0;

        sp -= chardesclen + 1;
        if(sp < program_end)
          goto oom;
        OS_memcpy(sp, chardesc, chardesclen);
        sp[chardesclen] = 0;
        *(unsigned char**)&attributes[count].pValue = sp;
        chardesclen = 0;
      }
    }
    else
    {
      goto error;
    }

    line += line[sizeof(LINENUM)];
  }

  // Build a stack header for this service
  sp -= sizeof(stack_service_frame);
  if(sp < program_end)
    goto oom;
  frame = (stack_service_frame*)sp;
  frame->header.frame_type = STACK_SERVICE_FLAG;
  frame->header.frame_size = origsp - sp;
  frame->attrs = attributes;
  frame->connect = onconnect;
  ((unsigned short*)attributes)[-1] = count;

  // Register the service
  ch = GATTServApp_RegisterService((gattAttribute_t*)attributes, count, &ble_service_callbacks);
  if (ch != SUCCESS)
  {
    goto error;
  }

  return 0;

error:
  sp = origsp;
  current_line = line;
  return 1;
oom:
  sp = origsp;
  current_line = line;
  return 2;
}

//
// Parse a string into a UUID
//  This is fairly lax in what it considers valid.
//
static char ble_get_uuid(void)
{
  VAR_TYPE v = 0;
  unsigned char* end;
  
  end = txtpos + find_quoted_string();
  if (end < txtpos)
  {
    return 0;
  }

  ble_uuid_len = 0;
  while (txtpos < end)
  {
    if (ble_uuid_len >= sizeof(ble_uuid))
    {
      return 0;
    }
    else if (*txtpos == '-')
    {
      txtpos++;
    }
    else
    {
      v = parse_int(2, 16);
      if (error_num)
      {
        return 0;
      }
      ble_uuid[ble_uuid_len++] = (unsigned char)v;
    }
  }
#ifdef TARGET_CC254X
  // Reverse
  for (v = 0; v < ble_uuid_len / 2; v++)
  {
    unsigned char c = ble_uuid[v];
    ble_uuid[v] = ble_uuid[ble_uuid_len - v - 1];
    ble_uuid[ble_uuid_len - v - 1] = c;
  }
#endif
  txtpos++;
  return ble_uuid_len > 0;
}

//
// Calculate the maximum read/write offset for the specified variable
//
static unsigned short ble_max_offset(gatt_variable_ref* vref, unsigned short offset, unsigned char maxlen)
{
  stack_variable_frame* frame;
  unsigned char moffset = offset + maxlen;

  get_variable_frame(vref->var, &frame);

  if (frame->type == VAR_DIM_BYTE)
  {
    if (moffset > frame->header.frame_size - sizeof(stack_variable_frame))
    {
      moffset = frame->header.frame_size - sizeof(stack_variable_frame);
    }
  }
  else
  {
    if (moffset > VAR_SIZE)
    {
      moffset = VAR_SIZE;
    }
  }

  return moffset;
}

//
// When a BLE characteristic is read, we process the incoming request from
// the appropriate BASIC variable. If an ONREAD event is specified we notify the user
// of the read request *before* we do the actual read.
//
static unsigned char ble_read_callback(unsigned short handle, gattAttribute_t* attr, unsigned char* value, unsigned char* len, unsigned short offset, unsigned char maxlen)
{
  gatt_variable_ref* vref;
  unsigned char moffset;
  unsigned char* v;
  stack_variable_frame* frame;
  
  vref = (gatt_variable_ref*)attr->pValue;
  moffset = ble_max_offset(vref, offset, maxlen);
  if (!moffset)
  {
    return FAILURE;
  }

  if (vref->read)
  {
    interpreter_run(vref->read, 1);
  }

  v = get_variable_frame(vref->var, &frame);
#ifdef TARGET_CC254X
  if (frame->type == VAR_DIM_BYTE)
  {
    OS_memcpy(value, v + offset, moffset - offset);
  }
  else
  {
    for (unsigned char i = offset; i < moffset; i++)
    {
      value[i - offset] = v[moffset - i - 1];
    }
  }
#else
  OS_memcpy(value, v + offset, moffset - offset);
#endif

  *len = moffset - offset;
  return SUCCESS;
}

//
// When a BLE characteristic is written, we process the incomign data and update
// the appropriate BASIC variable. If an ONWRITE event is specified we notify the user
// of the change *after* the write.
//
static unsigned char ble_write_callback(unsigned short handle, gattAttribute_t* attr, unsigned char* value, unsigned char len, unsigned short offset)
{
  gatt_variable_ref* vref;
  unsigned char moffset;
  unsigned char* v;
  stack_variable_frame* frame;
  
  if (attr->type.uuid == ble_client_characteristic_config_uuid)
  {
    return GATTServApp_ProcessCCCWriteReq(handle, attr, value, len, offset, GATT_CLIENT_CFG_NOTIFY);
  }
 
  vref = (gatt_variable_ref*)attr->pValue;
  moffset = ble_max_offset(vref, offset, len);
  if (moffset != offset + len)
  {
    return FAILURE;
  }
  
  v = get_variable_frame(vref->var, &frame);

#ifdef TARGET_CC254X
  if (frame->type == VAR_DIM_BYTE)
  {
    OS_memcpy(v + offset, value, moffset - offset);
  }
  else
  {
    for (unsigned char i = offset; i < moffset; i++)
    {
      v[moffset - i - 1] = value[i - offset];
    }
  }
#else
  OS_memcpy(v + offset, value, moffset - offset);
#endif

  if (vref->write)
  {
    interpreter_run(vref->write, 1);
  }

  if (attr[1].type.uuid == ble_client_characteristic_config_uuid)
  {
    ble_notify_assign(vref);
  }
  
  return SUCCESS;
}

//
// Send a BLE NOTIFY event
//
static void ble_notify_assign(gatt_variable_ref* vref)
{
  GATTServApp_ProcessCharCfg((gattCharCfg_t*)vref->cfg, (unsigned char*)vref, 0, (gattAttribute_t*)vref->attrs, ((unsigned short*)vref->attrs)[-1], INVALID_TASK_ID);
}

//
// BLE connection management. If the ONCONNECT event was specified when a service
// was created, we forward any connection changes up to the user code.
//
void ble_connection_status(unsigned short connHandle, unsigned char changeType)
{
  unsigned char* ptr;
  stack_service_frame* vframe;
  unsigned char j;
  unsigned char f;
  short i;

  for (ptr = sp; ptr < variables_begin; ptr += ((stack_header*)ptr)->frame_size)
  {
    vframe = (stack_service_frame*)ptr;
    if (vframe->header.frame_type == STACK_SERVICE_FLAG)
    {
      if (vframe->connect)
      {
        VARIABLE_INT_SET('H', connHandle);
        VARIABLE_INT_SET('S', changeType);
        if (changeType == LINKDB_STATUS_UPDATE_STATEFLAGS)
        {
          f = 0;
          for (j = 0x01; j < 0x20; j <<= 1) // No direct way to read flag bits!
          {
            if (linkDB_State(connHandle, j))
            {
              f |= j;
            }
          }
          VARIABLE_INT_SET('F', f);
        }
        interpreter_run(vframe->connect, 1);
      }
      if (changeType == LINKDB_STATUS_UPDATE_REMOVED || (changeType == LINKDB_STATUS_UPDATE_STATEFLAGS && !linkDB_Up(connHandle)))
      {
        for (i = ((short*)vframe->attrs)[-1]; i > 0; i--)
        {
          if (vframe->attrs[i].type.uuid == ble_client_characteristic_config_uuid)
          {
            GATTServApp_InitCharCfg(connHandle, (gattCharCfg_t*)vframe->attrs[i].pValue);
          }
        }
      }
    }
  }
}
