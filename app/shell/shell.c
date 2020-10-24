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

static void task1_entry(const struct app_descriptor *app, void *args)
{
	int i=0;

	while(1)
	{		
		// printf("task 1 %d\n", i++);
		//thread_sleep(1000);		
	}
}

static void task2_entry(const struct app_descriptor *app, void *args)
{
	int i=0;

	while(1)
	{		
		// printf("task 2 %d\n", i++);
		//thread_sleep(1000);		
	}
}

APP_START(task1)
  .entry = task1_entry,
APP_END

APP_START(task2)
  .entry = task2_entry,
APP_END