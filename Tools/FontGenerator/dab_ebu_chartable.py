''' Erstellt die Zeichentabelle fuer den DAB EBU Zeichensatz '''

import sys
import codecs

characters = ""
with open(sys.argv[1], 'r') as charFile:
	for ln in charFile:
		line = ln.strip()
		if line == "":
			characters = characters + u"?"
		elif line[:2] == "U+":
			characters = characters + (unichr(int(line[2:], 16)))
		else:
			characters = characters + line.decode(sys.getfilesystemencoding())

print characters

outfile = codecs.open('dab_ebu_chartable.c', 'w', 'UTF-8')
outfile.write(
u"#include \"radio_handler.h\"\n" +
u"const wchar_t const ebuChars[] = {")

firstChar = True
for character in characters:
	if not firstChar:
		outfile.write(u",")
	firstChar = False
	outfile.write(u"\n\tL\'" + ('\\' if character == '\'' else '') + character + "\'")
outfile.write(u"\n};\n\n")
outfile.close()

