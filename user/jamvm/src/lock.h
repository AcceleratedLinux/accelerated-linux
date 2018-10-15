/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008
 * Robert Lougher <rob@lougher.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "thread.h"

extern void monitorInit(Monitor *mon);
extern void monitorLock(Monitor *mon, Thread *self);
extern void monitorUnlock(Monitor *mon, Thread *self);
extern int monitorWait0(Monitor *mon, Thread *self, long long ms, int ns,
                        int blocked, int interruptible);
extern int monitorNotify(Monitor *mon, Thread *self);
extern int monitorNotifyAll(Monitor *mon, Thread *self);

#define monitorWait(mon, self, ms, ns) \
    monitorWait0(mon, self, ms, ns, FALSE, TRUE)

extern void objectLock(Object *ob);
extern void objectUnlock(Object *ob);
extern void objectNotify(Object *ob);
extern void objectNotifyAll(Object *ob);
extern void objectWait0(Object *ob, long long ms, int ns, int interruptible);
extern int objectLockedByCurrent(Object *ob);
extern void threadMonitorCache();

#define objectWait(ob, ms, ns) \
    objectWait0(ob, ms, ns, TRUE)
