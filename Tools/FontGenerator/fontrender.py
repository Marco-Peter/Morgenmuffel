import freetype
import sys
import codecs


class FontRenderError(Exception):
    def __init__(self, msg=None):
        if msg is None:
            msg = "Error on FontRender object"
        super(FontRenderError, self).__init__(msg)


class FontRender:
    def __init__(self, fontname, size):
        self.size = size
        self.face = freetype.Face(fontname)
        self.face.set_pixel_sizes(0, size)

    def loadGlyph(self, character):
        ''' loads a glyph as bitmap '''
        self.face.load_char(character,
                            freetype.FT_LOAD_RENDER |
                            freetype.FT_LOAD_TARGET_MONO)

    def familyName(self):
        return self.face.family_name.decode()

    def ascender(self):
        return (self.face.size.ascender) // 64

    def descender(self):
        return (self.face.size.descender) // 64

    def width(self):
        return (self.face.glyph.metrics.width) // 64

    def height(self):
        return self.face.glyph.bitmap.rows

    def bearingX(self):
        return (self.face.glyph.metrics.horiBearingX) // 64

    def bearingY(self):
        return (self.face.glyph.metrics.horiBearingY) // 64

    def advance(self):
        return (self.face.glyph.metrics.horiAdvance) // 64

    def bitmapWidth(self):
        return self.face.glyph.bitmap.width

    def bitmapRows(self):
        return self.face.glyph.bitmap.rows

    def pitch(self):
        return self.face.glyph.bitmap.pitch

    def getPixelValue(self, x, y):
        ''' Get the value of a pixel within the bitmap '''
        if x > self.width():
            raise FontRenderError(
                str("x value {:d} exceeds glyph width".format(x)))
        if y > self.size:
            raise FontRenderError(
                str("y value {:d} exceeds glyph height".format(y)))

        if self.height() > (self.bearingY() - self.descender()):
            rowOffset = self.size - self.height()
        else:
            rowOffset = self.size - (self.bearingY() - self.descender())

        if rowOffset < 0:
            rowOffset = 0

        if (y >= rowOffset) and (y < rowOffset + self.height()):
            valByte = (y - rowOffset) * self.face.glyph.bitmap.pitch + x // 8
            valBit = x % 8
            try:
                return bool(self.face.glyph.bitmap.buffer[valByte] & (0x80 >> valBit))
            except:
                print("x = {}".format(x))
                print("y = {}".format(y))
                print("rowOffset = {}".format(rowOffset))
                print("valByte = {}".format(valByte))
                print("Bitmap width: {}".format(self.bitmapWidth()))
                print("Bitmap height: {}".format(self.bitmapRows()))
                print("buffer length: {}".format(
                    len(self.face.glyph.bitmap.buffer)))
                raise
        else:
            return False

    def drawGlyph(self):
        ''' Draw a glyph  '''
        print(" * ")
        print(" * ")

        for row in range(self.size):
            # Build the string of a row
            gRow = str(" * ")
            for column in range(self.face.glyph.bitmap.width):
                if self.getPixelValue(column, row):
                    gRow = gRow + "*"
                else:
                    gRow = gRow + " "
            if row == 0:
                gRow = gRow + " *** Top ***"
            if row == self.size - 1 + self.descender():
                gRow = gRow + " *** Origin ***"
            if row == self.size - 1:
                gRow = gRow + " *** Bottom ***"

            print(gRow)
        print(" * ")
        print(" * ")

    def transformGlyph(self):
        ''' Returns the current glyph in display format '''

        PAGESIZE = 8

        # Calculate the number of pages to be used
        pages = (self.size + PAGESIZE - 1) // PAGESIZE

        # Calculate the origin height
        origin = 0x0001 << (pages * PAGESIZE - 1 + self.descender())
        print(" * Origin: 0x{:X}".format(origin))

        # allocate the target bytearray
        # We need to fill up all used pages completely
        output = bytearray(self.face.glyph.bitmap.width *
                           pages * PAGESIZE // 8)

        # The number of pages to write
        for column in range(self.face.glyph.bitmap.width):
            colByte = 0
            for row in range(self.size):
                if self.getPixelValue(column, row):
                    colByte = colByte | (0x1 << row)
            # Cut the glyph into page sized pieces
            for page in range(pages):
                output[self.bitmapWidth() * page * PAGESIZE // 8 +
                       column] = colByte & 0xFF
                colByte = colByte >> PAGESIZE
        return output


if __name__ == '__main__':
    if len(sys.argv) < 4:
        print("Usage: python fontrender.py <fontfilename> <fontsize> <characterfile>")
        print("Example: python fontrender.py FreeSans.ttf 16 characters_FreeSans_16.txt")
        sys.exit()

    # characters can be entered as is or as unicode code points
    # newline and carriage return characters are ignored
    characters = ""

    with open(sys.argv[3], 'r') as charFile:
        for ln in charFile:
            line = ln.strip()
            if line[:2] == "U+":
                # Unicode code point mode
                characters = characters + (chr(int(line[2:], 16)))
            elif line != "":
                # Read given characters directly
                characters = characters + line

    print("Characters: " + characters)
    fr = FontRender(sys.argv[1], int(sys.argv[2]))

    outFile = codecs.open("font_" + fr.familyName() +
                          "_" + sys.argv[2] + ".c",
                          'w',
                          "UTF-8")

    # Write the file header
    outFile.write(
        "/******************************************************************************\n" +
        "* Font data for " + fr.familyName() + " " + sys.argv[2] + "px height.\n" +
        "*\n" +
        "* Automatically generated\n" +
        "*\n" +
        "******************************************************************************/\n" +
        "\n" +
        "#include \"font.h\"\n" +
        "\n" +
        "static const uint8_t glyphData[];\n" +
        "static const Glyph glyphs[];\n" +
        "\n" +
        "const Font font" + fr.familyName() + sys.argv[2] + " = {\n" +
        u"\t" + sys.argv[2] + ",\n" +
        u"\tglyphs\n" +
        u"};\n\n" +
        u"static const Glyph glyphs[] = {\n")

    glyphData = []
    glyphDataPos = 0

    for character in characters:
        fr.loadGlyph(character)

        print(u"/* Character: {}".format(character))
        print(u" * Width: {:d}".format(fr.width()))
        print(u" * Height: {:d}".format(fr.height()))
        print(u" * Bearing X {:d}".format(fr.bearingX()))
        print(u" * Bearing Y {:d}".format(fr.bearingY()))
        print(u" * Advance {:d}".format(fr.advance()))
        print(u" * Ascender {:d}".format(fr.ascender()))
        print(u" * Descender {:d}".format(fr.descender()))
        print(u" * Pitch {:d}".format(fr.pitch()))

        outFile.write(
            u"\t{\n" +
            u"\t\tL\'" + ('\\' if character == '\'' else '') + character + u"\',\t\t\t\t\t// character\n" +
            u"\t\t{\t\t\t\t\t\t// struct desc\n" +
            u"\t\t\tglyphData + 0x{:04x},\t// data --> offset against glyphData\n".format(glyphDataPos) +
            u"\t\t\t{:d},\t\t\t\t\t// width\n".format(fr.bitmapWidth()) +
            u"\t\t\t{:d},\t\t\t\t\t// height\n".format(fr.bitmapRows()) +
            u"\t\t\t{:d},\t\t\t\t\t// bearingX\n".format(fr.bearingX()) +
            u"\t\t\t{:d},\t\t\t\t\t// bearingY\n".format(fr.bearingY()) +
            u"\t\t\t{:d}\t\t\t\t\t// advance\n".format(fr.advance()) +
            u"\t\t}\n" +
            u"\t},\n")

        fr.drawGlyph()
        vals = fr.transformGlyph()
        print(u" * Number of bits: {:d}".format(len(vals) * 8))
        charData = (character, [])
        for val in vals:
            print(u"0x{:X}".format(val))
            charData[1].append(val)

        glyphData.append(charData)
        glyphDataPos = glyphDataPos + len(vals)

    # Append a last one with zero values
    outFile.write(
        u"\t/* *** Array End Marker *** */\n" +
        u"\t{\n" +
        u"\t\t0,\n" +
        u"\t\t{\n" +
        u"\t\t\t0x0000,\n" +
        u"\t\t\t0,\n" +
        u"\t\t\t0,\n" +
        u"\t\t\t0,\n" +
        u"\t\t\t0,\n" +
        u"\t\t\t0\n" +
        u"\t\t}\n" +
        u"\t}\n" +
        u"};\n\n")

    # Write the glyph data container
    outFile.write(
        u"static const uint8_t glyphData[] = {")

    firstCharacter = True
    for character in glyphData:
        if not firstCharacter:
            outFile.write(u",")
        firstCharacter = False
        outFile.write(u"\n/* " + character[0] + u" */\t")
        firstByte = True
        for pos in character[1]:
            if not firstByte:
                outFile.write(u", ")
            firstByte = False
            outFile.write(u"0x{:02x}".format(pos))
        if character[1] == []:
            firstCharacter = True
    outFile.write(u"\n};\n\n")

    outFile.close()
