/*
 * Copyright (c) 2009 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <app.h>
#include <lk/debug.h>
#include <lib/console.h>
// #include <kernel/timer.h>
#include <kernel/thread.h>

static void shell_entry(const struct app_descriptor *app, void *args) {
    console_start();
}

APP_START(shell)
.entry = shell_entry,
APP_END

static void nbiot_init2(const struct app_descriptor *app)
{
	//printf("\r\n%s",__func__);	
}
static void nbiot_entry2(const struct app_descriptor *app, void *args)
{
	int i=0;
	// lk_time_t t;
	//UartPuts("n");
	while(1)
	{		
		printf("\nt2 %d",i++);

		thread_sleep(1000);		
	}
}
void exit_nbiot2(void)
{	
}
APP_START(nbiot2)
  .init = nbiot_init2,
  .entry = nbiot_entry2,
APP_END