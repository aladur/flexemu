/*
    wckeylck.c


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

#include "wlkeylck.h"


#ifdef HAVE_WAYLAND
static void
registry_global(void *data, struct wl_registry *registry,
        uint32_t name, const char *interface, uint32_t version)
{
    struct wayland_data *global_data = (struct wayland_data *)data;
    (void)global_data;
    (void)registry;
    (void)name;
    (void)interface;
    (void)version;
}

static const struct wl_registry_listener wl_registry_listener = {
    registry_global,
    NULL,
};

int wayland_init(struct wayland_data *data)
{
    data->display = wl_display_connect("");
    data->registry = wl_display_get_registry(data->display);

    wl_registry_add_listener(data->registry, &wl_registry_listener, &data);
    wl_display_roundtrip(data->display);

    return 0;
}

int wayland_shutdown(struct wayland_data *data)
{
    wl_registry_destroy(data->registry);
    data->registry = NULL;

    return 0;
}

#endif // #ifdef HAVE_WAYLAND

