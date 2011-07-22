#include "ser_parse.h"
#include "timer.h"
#include "board.h"
#include "uart.h"
#include "uartutil.h"
#include "slip.h"
#include "slip_rx.h"
#include "cmd.h"

// parser states
#define STATE_WAIT_FOR_DATA    0
#define STATE_IN_FIRST_DELAY   1
#define STATE_GET_MARKERS      2
#define STATE_IN_LAST_DELAY    3
#define STATE_COMMAND_MODE     4

// options
u08 ser_marker_char = '+'; // send DELAY +++ DELAY to enter command mode
u08 ser_marker_count = 3;
u16 ser_marker_wait_delay = 100; // 1000 ms
u16 ser_marker_wait_char = 50; // 500 ms
u16 ser_command_timeout = 500; // 5000 ms

// public values
u16 ser_read_errors = 0;

static u08 state = STATE_WAIT_FOR_DATA; // wait for a serial char
static u08 num_markers = 0;
static u08 data = 0;

#define MAX_CMD_LINE_LEN  16
static u08 cmd_line[MAX_CMD_LINE_LEN];
static u08 cmd_pos;

static u08 read_char(void)
{
  // a char arrived
  u08 status = slip_read(&data);
  if(status == SLIP_STATUS_END) {
    slip_rx_end();
  } else if(status == SLIP_STATUS_OK) {
    slip_rx_data(data);
  } else {
    ser_read_errors++;
  }
  // reset timer
  timer_10ms = 0;
  return status;
}

static void send_ok(void)
{
  uart_send_string("OK");
  uart_send_crlf();
}

static void send_bye(void)
{
  uart_send_string("BYE");
  uart_send_crlf();
}

static void send_huh(void)
{
  uart_send_string("??");
  uart_send_crlf();
}

static void handle_command_char(void)
{
  switch(data) {
    case '\r':
      uart_send(data);
      break;
    case '\n':
      uart_send(data);
      cmd_line[cmd_pos] = '\0';
      if(cmd_pos > 0) {
        u08 status = cmd_parse(cmd_pos, (const char *)cmd_line);
        if(status == CMD_EXIT) {
          send_bye();
          led_yellow_off();
          state = STATE_WAIT_FOR_DATA;
        } else if(status != CMD_OK) {
          send_huh();
        }
      } else {
        send_ok();
      }
      break;
    default:
      if(cmd_pos < (MAX_CMD_LINE_LEN-1)) {
        cmd_line[cmd_pos] = data;
        cmd_pos++;
        uart_send(data); // echo character
      }
      break;
  }  
}

u08 ser_parse_worker(void)
{
#define DEBUG_STATE
#ifdef DEBUG_STATE
  u08 old_state = state;
#endif
  
  switch(state) {
    // waiting for incoming data
    case STATE_WAIT_FOR_DATA:
      if(uart_read_data_available()) {
        // get char and go to HAVE_DATA or GOT_SLIP_END
        read_char();
      } else {
        // nothing arrived in delay time
        if(timer_10ms >= ser_marker_wait_delay) {
          timer_10ms = 0;
          state = STATE_IN_FIRST_DELAY;
        }
      }
      break;
    // first delay before marker passed
    case STATE_IN_FIRST_DELAY:
      // get a char and check for marker
      if(uart_read_data_available()) {
        if(read_char() == SLIP_STATUS_OK) {
          if(data == ser_marker_char) {
            num_markers = 1;
            state = STATE_GET_MARKERS;
          } else {
            state = STATE_WAIT_FOR_DATA;
          }
        } else {
          state = STATE_WAIT_FOR_DATA;
        }
      } 
      break;
    // waiting for marker chars after first delay
    case STATE_GET_MARKERS:
      // next marker char?
      if(uart_read_data_available()) {
        if(read_char() == SLIP_STATUS_OK) {
          if(data == ser_marker_char) {
            num_markers ++;
            if(num_markers == ser_marker_count) {
              state = STATE_IN_LAST_DELAY;
            } else {
              state = STATE_GET_MARKERS;
            }
          } else {
            state = STATE_WAIT_FOR_DATA;
          }
        }
      }
      // no marker char arrived in time
      else if(timer_10ms >= ser_marker_wait_char) {
        state = STATE_WAIT_FOR_DATA;
      }
      break;
    // last delay after markers
    case STATE_IN_LAST_DELAY:
      // if we get a char then abort
      if(uart_read_data_available()) {
        read_char();
        state = STATE_WAIT_FOR_DATA;
      } 
      // delay passed -> enter command mode
      else if(timer_10ms >= ser_marker_wait_delay) {
        state = STATE_COMMAND_MODE;

        // enter command mode
        led_yellow_on();
        send_ok();
        cmd_pos = 0;
      }
      break;
    // command mode
    case STATE_COMMAND_MODE:
      // get command chars
      if(uart_read_data_available()) {
        if(uart_read(&data)) {
          handle_command_char();
          // reset timer after command execution
          timer_10ms = 0;
        }
      }
      // no char entered in a long time? -> leave command mode
      else if(timer_10ms >= ser_command_timeout) {
        led_yellow_off();
        send_bye();
        state = STATE_WAIT_FOR_DATA;
      }
      break;
  }
  
#ifdef DEBUG_STATE
  if(state != old_state) {
    uart_send_hex_byte_spc(old_state);
    uart_send_hex_byte_crlf(state);
  }
#endif  

  // state change?
  return state != old_state;
}