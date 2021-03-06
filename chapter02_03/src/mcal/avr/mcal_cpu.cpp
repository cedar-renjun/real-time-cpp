///////////////////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2007 - 2013.
//  Distributed under the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <mcal_osc.h>
#include <mcal_port.h>
#include <mcal_wdg.h>

extern "C" void mcal_cpu_init()
{
  mcal::port::init(nullptr);
  mcal::wdg::init(nullptr);
  mcal::osc::init(nullptr);
}
