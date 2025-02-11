// Copyright (C) 2018 Toitware ApS.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; version
// 2.1 only.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// The license can be found in the file `LICENSE` in the top level
// directory of this repository.

#include "timer.h"

#include "../objects_inline.h"
#include "../utils.h"

namespace toit {

TimerEventSource* TimerEventSource::instance_ = null;

TimerEventSource::TimerEventSource()
    : EventSource("Timer")
    , Thread("Timer")
    , timer_changed_(OS::allocate_condition_variable(mutex()))
    , stop_(false) {
  ASSERT(instance_ == null);
  instance_ = this;
  spawn();
}

TimerEventSource::~TimerEventSource() {
  {
    // Stop the main thread.
    Locker locker(mutex());
    stop_ = true;

    OS::signal(timer_changed_);
  }

  join();

  ASSERT(timers_.is_empty());

  OS::dispose(timer_changed_);

  instance_ = null;
}

void TimerEventSource::arm(Timer* timer, int64_t timeout) {
  Locker locker(mutex());
  bool is_linked = timers_.is_linked(timer);
  if (is_linked && timer->timeout() == timeout) {
    return;
  }

  // Get current timeout, if any.
  auto head = timers_.first();
  int64_t old_timeout = head ? head->timeout() : timeout + 1;

  // Remove in case it was already enqueued.
  if (is_linked) {
    timers_.unlink(timer);
  }

  // Clear and install timer.
  timer->set_state(0);
  timer->set_timeout(timeout);

  timers_.insert_before(timer, [&timer](Timer* t) { return timer->timeout() < t->timeout(); });

  if (timeout < old_timeout) {
    // Signal if new timeout is less the the old.
    // This means we don't re-arm even if the first timer
    // was removed. This simply means we avoid waking up NOW, but instead
    // delays the wakeup to the already scheduled time. The result
    // is overall at maximum the same number of wakeups, but most likely
    // much less.
    OS::signal(timer_changed_);
  }
}

void TimerEventSource::on_unregister_resource(Locker& locker, Resource* r) {
  ASSERT(is_locked());
  Timer* timer = r->as<Timer*>();

  Timer* first = timers_.first();
  if (timers_.is_linked(timer)) {
    timers_.unlink(timer);
    if (first == timer) {
      // Signal if the first one changes.
      OS::signal(timer_changed_);
    }
  }
}

void TimerEventSource::entry() {
  Locker locker(mutex());
  HeapTagScope scope(ITERATE_CUSTOM_TAGS + EVENT_SOURCE_MALLOC_TAG);

  while (!stop_) {
    int64 time = OS::get_monotonic_time();

    int64 delay_us = 0;
    while (!timers_.is_empty()) {
      if (time >= timers_.first()->timeout()) {
        Timer* timer = timers_.remove_first();
        dispatch(locker, timer, 0);
      } else {
        delay_us = timers_.first()->timeout() - time;
        break;
      }
    }
    if (delay_us > 0) {
      OS::wait_us(timer_changed_, delay_us);
    } else {
      OS::wait(timer_changed_);
    }
  }
}

} // namespace toit
