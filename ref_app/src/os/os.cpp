///////////////////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2007 - 2013.
//  Distributed under the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <algorithm>
#include <mcal_irq.h>
#include <os/os_task_control_block.h>
#include <util/utility/util_bit_mask.h>

namespace os
{
  os::task_list_type task_list = OS_TASK_LIST;
}

void os::start_os()
{
  // Initialize each task once.
  std::for_each(task_list.cbegin(),
                task_list.cend(),
                [](const task_control_block& tcb)
                {
                  tcb.initialize();
                });

  // Initialize the idle task.
  OS_IDLE_TASK_INIT();

  // Enter the endless loop of the multitasking scheduler.
  for(;;)
  {
    // Find the next ready task using a priority-based search algorithm.
    const task_list_type::const_iterator it_ready_task = std::find_if(task_list.cbegin(),
                                                                      task_list.cend(),
                                                                      [](const task_control_block& tcb) -> bool
                                                                      {
                                                                        return tcb.execute();
                                                                      });

    // If no ready-task was found, then service the idle task.
    if(it_ready_task == task_list.end())
    {
      // Set the task index to one higher than the highest
      // task index in order to represent the idle task.
      task_control_block::task_global_index = task_control_block::index_type(task_id_end);

      // Set the bit in the task trace belonging to the idle task.
      task_control_block::task_global_trace |=
        util::bit_mask_single_bit_value<task_control_block::trace_type,
                                        static_cast<unsigned>(task_id_end)>::value;

      // Check if all of the tasks have checked in via checking the task trace value.
      // If all the tasks have checked in, then service the idle task.
      if(task_control_block::task_global_trace == task_control_block::task_idle_mask)
      {
        OS_IDLE_TASK_FUNC(true);

        // Reset the task trace value to zero.
        task_control_block::task_global_trace = task_control_block::trace_type(0U);
      }
      else
      {
        OS_IDLE_TASK_FUNC(false);
      }
    }
  }
}

void os::set_event(const task_id_type task_id, const event_type& event_to_set)
{
  if(task_id < task_id_end)
  {
    // Get a pointer to the control block corresponding to
    // the task ID that has been supplied to this subroutine.
    task_control_block* control_block_of_the_task_id = &task_list[task_list_type::size_type(task_id)];

    mcal::irq::disable_all();
    control_block_of_the_task_id->event |= event_to_set;
    mcal::irq::enable_all();
  }
}

void os::get_event(event_type& event_to_get)
{
  // Get a pointer to the control block of the task that is running.
  const task_control_block* control_block_of_the_running_task = task_control_block::get_running_task_pointer();

  if(control_block_of_the_running_task != nullptr)
  {
    mcal::irq::disable_all();
    const event_type my_event = control_block_of_the_running_task->event;
    mcal::irq::enable_all();

    event_to_get = my_event;
  }
  else
  {
    event_to_get = event_type(0U);
  }
}

void os::clear_event(const event_type& event_mask_to_clear)
{
  // Get a pointer to the control block of the task that is running.
  task_control_block* control_block_of_the_running_task = task_control_block::get_running_task_pointer();

  if(control_block_of_the_running_task != nullptr)
  {
    mcal::irq::disable_all();
    control_block_of_the_running_task->event &= event_type(~event_mask_to_clear);
    mcal::irq::enable_all();
  }
}
