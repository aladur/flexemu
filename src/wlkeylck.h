/*                                                                              
    wckeylck.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2023  W. Schwotzer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_WAYLAND
#include <wayland-client.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif

struct wayland_data
{
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_seat *seat;
    struct wl_keyboard *keyboard;
};

int wayland_init(struct wayland_data *data);
int wayland_shutdown(struct wayland_data *data);

#ifdef  __cplusplus
}
#endif 

