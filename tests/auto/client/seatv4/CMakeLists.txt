# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from seatv4.pro.

#####################################################################
## tst_seatv4 Test:
#####################################################################

qt_internal_add_test(tst_seatv4
    SOURCES
        tst_seatv4.cpp
    LIBRARIES
        SharedClientTest
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_seatv4 CONDITION QT_FEATURE_cursor
    LIBRARIES
        Qt::GuiPrivate
        Wayland::Cursor
)
