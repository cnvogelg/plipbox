#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <string.h>
#include <stdio.h>

#include "param.h"

/* convert string with 1..4 chars to tag ULONG */
int param_parse_tag(const char *str, ULONG *tag)
{
  int n = strlen(str);
  if((n < 1) || (n>4)) {
    return PARAM_WRONG_TAG_SIZE;
  }

  int bits = 24;
  ULONG res_tag = 0;
  while(*str) {
    res_tag |= (ULONG)*str << bits;
    bits -= 8;
  }

  *tag = res_tag;
  return PARAM_OK;
}

#define MAX_NUM_BUF   32

static int parse_digit(char ch, int base, ULONG *res)
{
  BYTE num = 0;
  if((ch >= '0') && (ch <= '9')) {
    num = ch - '0';
  }
  else if((ch >= 'a') && (ch <= 'f')) {
    num = ch + 10 - 'a';
  }
  else if((ch >= 'A') && (ch <= 'F')) {
    num = ch + 10 - 'A';
  }
  else {
    return PARAM_NO_DIGIT_CHAR;
  }

  if(num >= base) {
    return PARAM_DIGIT_NOT_IN_BASE;
  }

  *res = num;
  return PARAM_OK;
}

static int parse_number(const char *str, int base, ULONG *num, int *consumed)
{
  char buffer[MAX_NUM_BUF];
  int len = 0;

  // force hex?
  if(*str == '$') {
    base = 16;
    str++;
    len++;
  }
  else if(*str == '!') {
    base = 10;
    str++;
    len++;
  }
  else if(*str == '%') {
    base = 2;
    str++;
    len++;
  }

  ULONG result = 0;
  while(*str) {
    ULONG digit;
    int res = parse_digit(*str, base, &digit);
    if(res != PARAM_OK) {
      return res;
    }
    result *= base;
    result |= digit;
    len++;
    str++;

    // is a valid terminator for arrays?
    if((*str == '.') || (*str == ':')) {
      break;
    }
  }
  *num = result;
  if(consumed != NULL) {
    *consumed = len;
  }
  return PARAM_OK;
}

static int get_base(s2pb_param_def_t *def)
{
  int base = 10;
  if((def->format & S2PB_PARAM_FORMAT_HEX) == S2PB_PARAM_FORMAT_HEX) {
    base = 16;
  }
  else if((def->format & S2PB_PARAM_FORMAT_BIN) == S2PB_PARAM_FORMAT_BIN) {
    base = 2;
  }
  return base;
}

static void set_value(UBYTE *data, ULONG number, int value_bytes)
{
  if(value_bytes == 1) {
    *data = (UBYTE)number;
  }
  else if(value_bytes == 2) {
    UWORD *ptr = (UWORD *)data;
    *ptr = (UWORD)number;
  }
  else if(value_bytes == 4) {
    ULONG *ptr = (ULONG *)data;
    *ptr = number;
  }
}

static int parse_scalar(const char *str, s2pb_param_def_t *def, UBYTE *data,
                         UWORD value_bytes)
{
  if(def->size != value_bytes) {
    return PARAM_WRONG_DATA_SIZE;
  }

  // get default base for numbers
  int base = get_base(def);

  ULONG number = 0;
  int res = parse_number(str, base, &number, NULL);
  if(res != PARAM_OK) {
    return res;
  }

  // copy number to buffer
  set_value(data, number, value_bytes);

  return PARAM_OK;
}

static int get_num_elements(s2pb_param_def_t *def, UWORD value_bytes)
{
  int max_count = def->size;
  if(value_bytes == 2) {
    max_count >>= 1;
  }
  else if(value_bytes == 4) {
    max_count >>= 2;
  }
  return max_count;
}

static int parse_array(const char *str, s2pb_param_def_t *def, UBYTE *data,
                        UWORD value_bytes)
{
  if((def->size % value_bytes) != 0) {
    return PARAM_WRONG_DATA_SIZE;
  }

  // parse as string?
  if((def->format & S2PB_PARAM_FORMAT_STR) == S2PB_PARAM_FORMAT_STR) {
    int n = strlen(str);
    if(n >= def->size) {
      return PARAM_DATA_TOO_LONG;
    }
    if(n == 0) {
      return PARAM_DATA_TOO_SHORT;
    }
    strncpy(data, str, def->size);
    return PARAM_OK;
  }

  // get default base for numbers
  int base = get_base(def);

  // get number of elements
  int max_count = get_num_elements(def, value_bytes);

  // parse up to size values
  int count = 0;
  UBYTE *ptr = data;
  while(*str) {
    int consumed = 0;
    ULONG number = 0;
    int res = parse_number(str, base, &number, &consumed);
    if(res != PARAM_OK) {
      return res;
    }
    str += consumed;

    // store value
    count++;
    if(count == def->size) {
      return PARAM_DATA_TOO_LONG;
    }

    set_value(ptr, number, value_bytes);
    ptr += value_bytes;
  }

  if(count != max_count) {
    return PARAM_DATA_TOO_SHORT;
  }

  return PARAM_OK;
}

int param_parse_val(const char *str, s2pb_param_def_t *def, UBYTE *data)
{
  switch(def->type) {
    case S2PB_PARAM_TYPE_BYTE:
      return parse_scalar(str, def, data, 1);
    case S2PB_PARAM_TYPE_WORD:
      return parse_scalar(str, def, data, 2);
    case S2PB_PARAM_TYPE_LONG:
      return parse_scalar(str, def, data, 4);
    case S2PB_PARAM_TYPE_BYTE_ARRAY:
      return parse_array(str, def, data, 1);
    case S2PB_PARAM_TYPE_WORD_ARRAY:
      return parse_array(str, def, data, 2);
    case S2PB_PARAM_TYPE_LONG_ARRAY:
      return parse_array(str, def, data, 4);
    default:
      return PARAM_WRONG_TYPE;
  }
}

// ----- print -----

static void print_binary(char *str, ULONG num, int value_bytes)
{
  int digits = value_bytes * 8;
  int shift = digits - 1;
  ULONG mask = 1 << shift;

  *str++ = '%';

  for(int i=0;i<digits;i++) {
    char ch = ((num & mask) == mask) ? '1' : '0';
    *str++;
    mask >>= 1;
  }
}

static int print_number(char *str, int base, ULONG num, int value_bytes, int *consumed)
{
  int len = 0;
  if(base == 16) {
    if(value_bytes == 1) {
      len = sprintf(str, "$%02lx", num);
    }
    else if(value_bytes == 2) {
      len = sprintf(str, "$%04lx", num);
    }
    else if(value_bytes == 4) {
      len = sprintf(str, "$%08lx", num);
    }
  }
  else if(base == 10) {
    len = sprintf(str, "%lu", num);
  }
  else if(base == 2) {
    print_binary(str, num, value_bytes);
    len = value_bytes * 8 + 1;
  }
  else {
    return PARAM_WRONG_BASE;
  }

  if(consumed != NULL) {
    *consumed = len;
  }

  return PARAM_OK;
}

static int print_def(char *str, s2pb_param_def_t *def)
{
  return sprintf(str, "#%03lu %-4s [%4lu]  ", (ULONG)def->index, (char *)&def->tag, (ULONG)def->size);
}

static ULONG get_val(const UBYTE *data, int value_bytes)
{
  if(value_bytes == 1) {
    return (ULONG)*data;
  }
  else if(value_bytes == 2) {
    UWORD *wptr = (UWORD *)data;
    return *wptr;
  }
  else if(value_bytes == 4) {
    ULONG *lptr = (ULONG *)data;
    return *lptr;
  }
  return 0;
}

static int print_scalar(char *str, s2pb_param_def_t *def, const UBYTE *data, int value_bytes)
{
  if(def->size != value_bytes) {
    return PARAM_WRONG_DATA_SIZE;
  }

  print_def(str, def);

  int base = get_base(def);
  ULONG number = get_val(data, value_bytes);
  int res = print_number(str, base, number, value_bytes, NULL);

  return res;
}

static char get_sep(int base)
{
  if(base == 16)
    return ':';
  else
    return '.';
}

static int print_array(char *str, s2pb_param_def_t *def, const UBYTE *data, int value_bytes)
{
  if((def->size % value_bytes) != 0) {
    return PARAM_WRONG_DATA_SIZE;
  }

  int len = print_def(str, def);
  str += len;

  // print as string?
  if((def->format & S2PB_PARAM_FORMAT_STR) == S2PB_PARAM_FORMAT_STR) {
    strcpy(str, data);
    return PARAM_OK;
  }

  // print as numbers
  int base = get_base(def);
  int elements = get_num_elements(def, value_bytes);
  char sep = get_sep(base);
  const UBYTE *ptr = data;
  for(int i=0;i<elements;i++) {
    // add seperator char
    if(i > 0) {
      *str = sep;
      str++;
    }

    // print number
    int consumed = 0;
    ULONG number = get_val(ptr, value_bytes);
    int res = print_number(str, base, number, value_bytes, &consumed);
    if(res != PARAM_OK) {
      return res;
    }

    str += consumed;
    ptr += value_bytes;
  }

  return PARAM_OK;
}

int param_print_val(char *str, s2pb_param_def_t *def, const UBYTE *data)
{
  switch(def->type) {
    case S2PB_PARAM_TYPE_BYTE:
      return print_scalar(str, def, data, 1);
    case S2PB_PARAM_TYPE_WORD:
      return print_scalar(str, def, data, 2);
    case S2PB_PARAM_TYPE_LONG:
      return print_scalar(str, def, data, 4);
    case S2PB_PARAM_TYPE_BYTE_ARRAY:
      return print_array(str, def, data, 1);
    case S2PB_PARAM_TYPE_WORD_ARRAY:
      return print_array(str, def, data, 2);
    case S2PB_PARAM_TYPE_LONG_ARRAY:
      return print_array(str, def, data, 4);
  }
}

char *param_perror(int res)
{
  switch(res) {
  case PARAM_OK: return "OK";
  case PARAM_WRONG_TAG_SIZE: return "Wrong tag size!";
  case PARAM_NO_DIGIT_CHAR: return "No digit char!";
  case PARAM_DIGIT_NOT_IN_BASE: return "Digit not in base!";
  case PARAM_WRONG_DATA_SIZE: return "Wrong data size!";
  case PARAM_WRONG_TYPE: return "Wrong type!";
  case PARAM_DATA_TOO_LONG: return "Data too long";
  case PARAM_DATA_TOO_SHORT: return "Data too short";
  case PARAM_WRONG_BASE: return "Wrong base!";
  default: return "???";
  }
}

