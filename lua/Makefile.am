# xoreos - A reimplementation of BioWare's Aurora engine
#
# xoreos is the legal property of its developers, whose names
# can be found in the AUTHORS file distributed with this source
# distribution.
#
# xoreos is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# xoreos is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with xoreos. If not, see <http://www.gnu.org/licenses/>.

# Lua 5.0.3 (<http://www.lua.org/>).

include $(top_srcdir)/Makefile.common

noinst_LTLIBRARIES = liblua.la

noinst_HEADERS = \
                 lapi.h \
                 lauxlib.h \
                 lcode.h \
                 ldebug.h \
                 ldo.h \
                 lfunc.h \
                 lgc.h \
                 llex.h \
                 llimits.h \
                 lmem.h \
                 lobject.h \
                 lopcodes.h \
                 lparser.h \
                 lstate.h \
                 lstring.h \
                 ltable.h \
                 ltm.h \
                 lua.h \
                 lualib.h \
                 lundump.h \
                 lvm.h \
                 lzio.h \
                 $(EMPTY)

liblua_la_SOURCES = \
                    lapi.cpp \
                    lauxlib.cpp \
                    lbaselib.cpp \
                    lcode.cpp \
                    ldblib.cpp \
                    ldebug.cpp \
                    ldo.cpp \
                    ldump.cpp \
                    lfunc.cpp \
                    lgc.cpp \
                    liolib.cpp \
                    llex.cpp \
                    lmathlib.cpp \
                    lmem.cpp \
                    loadlib.cpp \
                    lobject.cpp \
                    lopcodes.cpp \
                    lparser.cpp \
                    lstate.cpp \
                    lstring.cpp \
                    lstrlib.cpp \
                    ltable.cpp \
                    ltablib.cpp \
                    ltests.cpp \
                    ltm.cpp \
                    lundump.cpp \
                    lvm.cpp \
                    lzio.cpp \
                    $(EMPTY)

EXTRA_DIST = \
             README.xoreos \
             COPYRIGHT \
             $(EMPTY)
