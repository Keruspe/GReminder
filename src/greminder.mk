# This file is part of GReminder.
#
# Copyright 2014 Marc-Antoine Perennou <Marc-Antoine@Perennou.com>
#
# GReminder is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# GReminder is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GReminder.  If not, see <http://www.gnu.org/licenses/>.

bin_PROGRAMS +=       \
	bin/greminder \
	$(NULL)

bin_greminder_SOURCES =                                   \
	src/greminder/greminder-macros.h                  \
	src/greminder/greminder-actions.h                 \
	src/greminder/greminder-db.h                      \
	src/greminder/greminder-item.h                    \
	src/greminder/greminder-keyword-widget.h          \
	src/greminder/greminder-keywords-widget.h         \
	src/greminder/greminder-list-window.h             \
	src/greminder/greminder-row.h                     \
	src/greminder/greminder-window.h                  \
	src/greminder/greminder-actions-private.h         \
	src/greminder/greminder-db-private.h              \
	src/greminder/greminder-item-private.h            \
	src/greminder/greminder-keyword-widget-private.h  \
	src/greminder/greminder-keywords-widget-private.h \
	src/greminder/greminder-list-window-private.h     \
	src/greminder/greminder-row-private.h             \
	src/greminder/greminder-window-private.h          \
	src/greminder/greminder-actions.c                 \
	src/greminder/greminder-db.c                      \
	src/greminder/greminder-item.c                    \
	src/greminder/greminder-keyword-widget.c          \
	src/greminder/greminder-keywords-widget.c         \
	src/greminder/greminder-list-window.c             \
	src/greminder/greminder-row.c                     \
	src/greminder/greminder-window.c                  \
	src/greminder/greminder.c                         \
	$(NULL)

bin_greminder_LDADD = \
	$(AM_LIBS)    \
	$(NULL)
