#include "slip.h"
#include "uart.h"

#define SLIP_END      0300
#define SLIP_ESC      0333
#define SLIP_ESC_END  0334
#define SLIP_ESC_ESC  0335

static slip_data_func_t the_data_func = 0;
static slip_end_func_t the_end_func = 0;
static u08 state = 0; // 0=normal 1=escape

void slip_push_init(slip_data_func_t data_func, slip_end_func_t end_func)
{
  the_data_func = data_func;
  the_end_func = end_func;
  state = 0;
}

void slip_push(u08 data)
{
  if(state == 0) {
    if(data == SLIP_END) {
      the_end_func();
    } else if(data == SLIP_ESC) {
      state = 1;
    } else {
      the_data_func(data);
    }
  } else {
    if(data == SLIP_ESC_END) {
      the_data_func(SLIP_END);
    } else if(data == SLIP_ESC_ESC) {
      the_data_func(SLIP_ESC);
    } else {
      // invalid escaping
    }
    state = 0;
  }
}

u08 slip_read(u08 *data)
{
  u08 d;
  u08 ok = uart_read(&d);
  if(!ok) {
    return 0;
  }
  // real END
  if(d == SLIP_END) {
    return 2;
  }
  // quoted data
  if(d == SLIP_ESC) {
    ok = uart_read(&d);
    if(!ok) {
      return 0;
    }
    // quoted END
    if(d == SLIP_ESC_END) {
      *data = SLIP_END;
    } 
    // quoted ESC
    else if(d == SLIP_ESC_ESC) {
      *data = SLIP_ESC;
    } 
    // invalid quoting
    else {
      return 0;
    }
  } 
  // normal data
  else {
    *data = d;
  }
  return 1;
}

u08 slip_send(u08 data)
{
  u08 ok = 0;
  // quote END
  if(data == SLIP_END) {
    ok = uart_send(SLIP_ESC);
    if(ok) {
      ok = uart_send(SLIP_ESC_END);
    }
  }
  // quote ESC
  else if(data == SLIP_ESC) {
    ok = uart_send(SLIP_ESC);
    if(ok) {
      ok = uart_send(SLIP_ESC_ESC);
    }
  }
  // normal char
  else {
    ok = uart_send(data);
  }
  return ok;
}

u08 slip_send_end(void)
{
  return uart_send(SLIP_END);
}
