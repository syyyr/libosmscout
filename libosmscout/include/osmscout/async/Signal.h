#ifndef LIBOSMSCOUT_SIGNAL_H
#define LIBOSMSCOUT_SIGNAL_H

/*
 This source is part of the libosmscout library
 Copyright (C) 2023 Lukas Karas

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include <osmscout/lib/CoreImportExport.h>

#include <functional>
#include <mutex>
#include <set>

namespace osmscout {

  template<typename... Args>
  class Slot;

  template<typename... Args>
  class Signal
  {
  private:
    std::mutex mutex;
    std::set<Slot<Args...>*> connections;

  public:
    friend class Slot<Args...>;

    Signal() = default;
    Signal(const Signal&) = delete;
    Signal(Signal&&) = delete;

    Signal& operator=(const Signal&) = delete;
    Signal& operator=(Signal&&) = delete;

    virtual ~Signal() = default;

    void Emit(const Args&... args);

    void Connect(Slot<Args...> &slot);
  };

  template<typename... Args>
  class Slot
  {
  private:
    std::mutex mutex;
    const std::function<void(const Args&...)> callback;
    std::set<Signal<Args...>*> connections;

  public:
    friend class Signal<Args...>;

    explicit Slot(const std::function<void(const Args&...)> &callback);

    Slot(const Slot&) = delete;
    Slot(Slot&&) = delete;

    Slot& operator=(const Slot&) = delete;
    Slot& operator=(Slot&&) = delete;

    virtual ~Slot() = default;

  private:
    void Call(const Args&... args) const;
    void Connect(Signal<Args...> *signal);
  };

  template<typename... Args>
  Slot<Args...>::Slot(const std::function<void(const Args&...)> &callback):
    callback(callback)
  {}

  template<typename... Args>
  void Slot<Args...>::Call(const Args&... args) const
  {
    callback(args...);
  }

  template<typename... Args>
  void Slot<Args...>::Connect(Signal<Args...> *signal)
  {
    std::unique_lock lock(mutex);
    connections.insert(signal);
  }

  template<typename... Args>
  void Signal<Args...>::Emit(const Args&... args)
  {
    std::unique_lock lock(mutex);
    for (const auto &slot: connections) {
      slot->Call(args...);
    }
  }

  template<typename... Args>
  void Signal<Args...>::Connect(Slot<Args...> &slot)
  {
    std::unique_lock lock(mutex);
    connections.insert(&slot);
    slot.Connect(this);
  }

} // namespace

#endif //LIBOSMSCOUT_SIGNAL_H
