#!/usr/bin/env python
#
# Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
#

# ------------------------------------------------------------ #
# Simple example of how to communicate with Fmb using the   #
# org.blade.Fmb D-BUS interface.                             #
#                                                              #
# Fmb must be compiled with D-BUS support for this to work. #
# ------------------------------------------------------------ #

import gtk
import dbus
import dbus.service
if getattr(dbus, 'version', (0,0,0)) >= (0,41,0):
  import dbus.glib

# acquire a reference to the Fmb object
bus = dbus.SessionBus()
fmb_object = bus.get_object('org.blade.Fmb', '/org/blade/FileManager')
fmb = dbus.Interface(fmb_object, 'org.blade.Fmb')

# You can now invoke methods on the fmb object, for
# example, to terminate a running Fmb instance (just
# like Fmb -q), you can use:
#
# fmb.Terminate()
#
# or, if you want to open the bulk rename dialog in the
# standalone version with an empty file list and /tmp
# as default folder for the "Add Files" dialog, use:
#
# fmb.BulkRename('/tmp', [], True, '', '')
#
# See the fmb-dbus-service-infos.xml file for the exact
# interface definition.
#

# We just popup the bulk rename dialog to
# demonstrate that it works. ;-)
fmb.BulkRename('/tmp', [], True, '', '')
