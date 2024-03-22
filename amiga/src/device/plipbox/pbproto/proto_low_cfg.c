#define __NOLIBBASE__
#include <proto/exec.h>

#include "compiler.h"
#include "debug.h"

#include "pario.h"
#include "pario_port.h"

void proto_low_config_port(struct pario_port *port)
{
  /* control: clk=sel,cflg=out(1) rak=in*/
  *port->ctrl_ddr |= port->sel_mask;                       // clk
  *port->ctrl_ddr &= ~(port->pout_mask | port->busy_mask); // rak + busy
  *port->ctrl_port |= port->all_mask;

  /* data: port=0, ddr=0xff (OUT) */
  *port->data_port = 0;
  *port->data_ddr = 0xff;
}
