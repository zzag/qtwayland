# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from hardwareintegration.pro.

if(TARGET Qt::WaylandClient)
    add_subdirectory(client)
endif()
if(TARGET Qt::WaylandCompositor)
    add_subdirectory(compositor)
endif()
