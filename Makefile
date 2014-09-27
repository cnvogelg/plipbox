#
# Makefile - main plipbox Makefile
#
# Written by
#  Christian Vogelgsang <chris@vogelgsang.org>
#
# This file is part of plipbox.
# See README for copyright notice.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
#  02111-1307  USA.
#

include version.mk

SUBDIRS := avr/src amiga/src doc/src
DISTFILES := Makefile README.md ChangeLog.md COPYING contrib doc avr amiga hardware
PROJECT := plipbox

REVSION := $(shell git log -1 --pretty=format:%h)
DATE := $(shell date '+%Y%m%d')
DIST_NAME := $(PROJECT)-$(VERSION)
SNAP_NAME := $(PROJECT)-pre$(VERSION)-$(REVSION)-$(DATE)

help:
	@echo "clean   clean project"
	@echo "dist    build release"
	@echo "snap    build snapshot"

clean:
	@for a in $(SUBDIRS) ; do $(MAKE) -C $$a clean_dist ; done

dist:
	@$(MAKE) pack PACK_NAME=$(DIST_NAME)

snap:
	@$(MAKE) pack PACK_NAME=$(SNAP_NAME)

pack:
	@echo "packing $(PACK_NAME)"
	@for a in $(SUBDIRS) ; do $(MAKE) -C $$a dist ; done
	@rm -rf $(PACK_NAME) $(PACK_NAME).zip
	@mkdir $(PACK_NAME)
	@cp -a $(DISTFILES) $(PACK_NAME)
	@zip -r $(PACK_NAME).zip $(PACK_NAME)
	@rm -rf $(PACK_NAME)
