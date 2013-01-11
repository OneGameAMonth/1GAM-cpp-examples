#ifndef FONT_HPP_GUARD
#define FONT_HPP_GUARD
#include <string>
#include <utility>
#include <map>

struct FontData;

struct Coord
{
  Coord(int a, int b) : x(a), y(b){}
  Coord() : x(0), y(0){}
  int x, y;
};

struct GlyphInfo
{
  int advanceX;
  int width; //width
  int height; //rows
  int top, left; //worldspace position offsets for rectangle
  float sMin,sMax,tMin,tMax; //rectangle in texture space

  // From old AtlasEntry
  int m_Priority;
  unsigned int m_GlyphIndex;
  Coord m_Pos; //in the atlas
  bool m_IsDirty;
  bool m_IsLocked;
};

class FontRenderer
{
public:
  /********************** Usage of FontRenderer's constructor: *********************************************************
  Font family name            : string
  Language/charset specifier  : string
  Font weight (bold)          : bool
  Font pitch  (italic)        : bool
  Font spacing (monspace)     : bool
  Font width                  : integer
  Font height                 : integer
  
  Family name are names like Arial, Georgia, Times New Roman, etc.
  
  Language codes should be specified if your application does internationalization, and
  needs support for certain charsets/codepoints. For example, if your application wants to
  display Chinese or Korean text, you need support for those glyphs in the font.
  The language codes follow the  ISO 639-1 specification. See http://www.mathguide.de/info/tools/languagecode.html
  for a list of available ones.
  
  Weight is implemented as a boolean setting. Either the text is bold, or it isn't.
  Pitch is implemented as a boolean setting. Either the text is italic, or it isn't.
  Spacing is implemented as a boolean setting. Either the text is monospace, or it has varying space between letters.
  
  Width and height: The number of pixels in width and height of the largest glyph.
  *********************************************************************************************************************/  
  
  FontRenderer
(const std::string& family,
 const std::string language,
 bool bold,
 bool italic,
 bool monospace,
 int fontWidth, int fontHeight);
 
  ~FontRenderer();
  int getGlyphWidth(wchar_t glyph) const;
  int getGlyphHeight(wchar_t glyph) const;
  /* Based on a fixed horisontal or vertical length, return the length in the missing
     axis required to fit the string into the rectangle. */
  int getStringHorisontalLengthWithConstraint(const std::wstring& str, int horiOffset);
  int getStringVerticalalLengthWithConstraint(const std::wstring& str, int vertOffset);
  void print(const std::wstring& str, int x, int y);
  void dumpFontAtlas() const;
private:
  /* 'family' is the font name, like "Arial", "Courier" or "Gothica"
     'language' is a two letter language code from the ISO639 specification.
     Examples:
     "ko" = Korean
     "no" = Norwegian
     "en" = English
     "ru" = Russian
     "es" = Spanish
     "de" = German
     See http://www.mathguide.de/info/tools/languagecode.html for a big list
  */
  std::string findFontPath(
			   const std::string& family,
			   const std::string language,
			   bool bold,
			   bool italic,
			   bool monospace);
  std::map<unsigned int, GlyphInfo>::iterator getFreeEntry(wchar_t ch);
  void updateFrequency(wchar_t ch);
  void updateInsertionPosition();
  Coord getPlacementPos() const;
  bool atlasFull() const;
  void loadGlyph(wchar_t glyph);
  void refreshClientAtlas();
  FontData* m_pimpl;
};

#endif
