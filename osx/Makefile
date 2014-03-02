all:
	gtk-mac-bundler GReminder.bundle
	gsed -e 's/gtk-2/gtk-3/' -i ./GReminder.app/Contents/MacOS/GReminder
	./gen_dmg.sh GReminder 2
