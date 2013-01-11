#include <iostream>
#define GLCOREARB_PROTOTYPES
/*
#define GL_VERSION_1_3 1
#define GL_VERSION_1_4 1
#define GL_VERSION_1_5 1
#define GL_VERSION_2_0 1
#define GL_VERSION_2_1 1
#define GL_VERSION_3_0 1
#define GL_VERSION_3_1 1
#define GL_VERSION_3_2 1
#define GL_VERSION_3_3 1
#define GL_VERSION_4_0 1
#define GL_VERSION_4_1 1
#define GL_VERSION_4_2 1
#define GL_VERSION_4_3 1
*/
//#include <GL/glcorearb.h>
//for png
//#include <cstdio>
#include <png.h>

#include <GL/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <fontconfig/fontconfig.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include "font.hpp"

static unsigned char internal_clz_lut[256] = {
  8, 7, 6, 6, 5, 5, 5, 5,
  4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

void write_png_file(const char* file_name, const std::vector<unsigned char>& pixels, size_t width, size_t height)
{
  png_byte color_type = PNG_COLOR_TYPE_RGB_ALPHA;
  png_byte bit_depth = 8;

  png_structp png_ptr;
  png_infop info_ptr;

  png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(size_t j = 0; j < height; ++j){
    size_t col = j*width;
    row_pointers[j] = (png_bytep)malloc(sizeof(png_byte) * width * 4);
    for(size_t i= 0; i < width; ++i){
      size_t indexDst = i*4;
      size_t indexSrc = (col+i)*4;
      row_pointers[j][indexDst + 0] = pixels[indexSrc + 0];
      row_pointers[j][indexDst + 1] = pixels[indexSrc + 1];
      row_pointers[j][indexDst + 2] = pixels[indexSrc + 2];
      row_pointers[j][indexDst + 3] = pixels[indexSrc + 3];
    }
  }

  FILE *fp = fopen(file_name, "w");
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  png_init_io(png_ptr, fp);
  png_set_IHDR(png_ptr, info_ptr, width, height,
	       bit_depth, color_type, PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, NULL);
  for (size_t y = 0; y < height; ++y)
    free(row_pointers[y]);
  free(row_pointers);

  fclose(fp);
}

static unsigned int internal_clz(unsigned int value)
{
  unsigned int accum = 0;
  accum +=                 internal_clz_lut[ value >> 24        ];
  accum += (accum ==  8) ? internal_clz_lut[(value >> 16) & 0xFF] : 0;
  accum += (accum == 16) ? internal_clz_lut[(value >>  8) & 0xFF] : 0;
  accum += (accum == 24) ? internal_clz_lut[ value        & 0xFF] : 0;
  return accum;
}

struct FontData
{
  FT_Face m_Face;
  std::string m_FontPath;
  int m_FontWidth, m_FontHeight;
  int m_FontWidthPad, m_FontHeightPad;
  int m_NumRows, m_NumColumns, m_MaxNumGlyphs; //numGlyphs = m_NumRows*m_NumColumns
  Coord m_currentPlacementPos;
  static FT_Library m_Library;
  static int m_LibraryRefs;
  const static int m_AtlasTextureWidth = 512;
  const static int m_AtlasTextureHeight = 512;
  unsigned int m_AtlasTextureID;
  std::map<unsigned int, GlyphInfo> m_Ginfo; //coding point as key
  std::vector<unsigned char> m_AtlasTexture;
};

FT_Library FontData::m_Library;
int FontData::m_LibraryRefs = 0;

static bool GlyphInfoCompByPriority
(const std::map<unsigned int, GlyphInfo>::iterator& a,
 const std::map<unsigned int, GlyphInfo>::iterator& b)
{
  return a->second.m_Priority < b->second.m_Priority;
}

std::map<unsigned int, GlyphInfo>::iterator FontRenderer::getFreeEntry(wchar_t ch)
{
  std::map<unsigned int, GlyphInfo>::iterator it;
  std::vector<std::map<unsigned int, GlyphInfo>::iterator> info_v;
  std::pair<std::map<unsigned int, GlyphInfo>::iterator, bool> ret;
  GlyphInfo info;
  info.advanceX = 0;
  info.width = 0;
  info.height = 0;
  info.top = 0;
  info.left = 0;
  info.sMin = info.sMax = info.tMin = info.tMax = 0;
  info.m_Priority = 0;
  info.m_GlyphIndex = ch;
  info.m_IsDirty = false;
  info.m_IsLocked = false;

  if(!atlasFull()){
    info.m_Pos = getPlacementPos();
    updateInsertionPosition();
    ret = m_pimpl->m_Ginfo.insert(std::make_pair((unsigned int)ch, info));
    return ret.first;
  } else {
    /* Find entry with lowest priority */
    info_v.reserve(m_pimpl->m_Ginfo.size());
    for(it = m_pimpl->m_Ginfo.begin(); it != m_pimpl->m_Ginfo.end(); ++it)
      info_v.push_back(it);
    std::sort(info_v.begin(), info_v.end(), GlyphInfoCompByPriority);
    for(int i = 0; i < info_v.size(); ++i){
      if(!info_v[i]->second.m_IsLocked){
	/* erase entry we don't need, create new entry with new key */
	GlyphInfo& info_r = info_v[i]->second;
	info.m_Pos = info_r.m_Pos;
	m_pimpl->m_Ginfo.erase(info_v[i]);	
	ret = m_pimpl->m_Ginfo.insert(std::make_pair((unsigned int)ch, info));
	return ret.first;
      }
    }
    /* Anything beyond this line should never happen */
    return m_pimpl->m_Ginfo.end();
  }
  return m_pimpl->m_Ginfo.end();
}

void FontRenderer::updateFrequency(wchar_t ch)
{
  std::map<unsigned int, GlyphInfo>::iterator it;
  it = m_pimpl->m_Ginfo.find(ch);
  if(it == m_pimpl->m_Ginfo.end()) return;
  GlyphInfo& gl = it->second;
  ++gl.m_Priority;
}

void FontRenderer::updateInsertionPosition()
{
  ++m_pimpl->m_currentPlacementPos.x;
  if(m_pimpl->m_currentPlacementPos.x >= m_pimpl->m_NumRows){
    m_pimpl->m_currentPlacementPos.x = 0;
    ++m_pimpl->m_currentPlacementPos.y;
  }
}

Coord FontRenderer::getPlacementPos() const
{
  return m_pimpl->m_currentPlacementPos;
}

bool FontRenderer::atlasFull() const
{
  return m_pimpl->m_Ginfo.size() == m_pimpl->m_MaxNumGlyphs;
}

FontRenderer::FontRenderer
(const std::string& family,
 const std::string language,
 bool bold,
 bool italic,
 bool monospace,
 int fontWidth, int fontHeight)
  : m_pimpl(new FontData)
{
  int error;

  if(!FontData::m_LibraryRefs){
    error = FT_Init_FreeType(&FontData::m_Library);
    if(error)
      throw std::runtime_error("Failed to load FT2 lib");
  }
  /* Reference count for library handle */
  FontData::m_LibraryRefs++;
  /* On Ubuntu/Debian, ask fontconfig to find a matching font.
     On Windows, use GetFontData() */
  std::string path = findFontPath(family, language, bold, italic, monospace);
  #ifdef DEBUG
  std::cout << "Loading font at path " << path << std::endl;
  #endif
  m_pimpl->m_FontPath = path;
  m_pimpl->m_FontWidth = fontWidth;
  m_pimpl->m_FontHeight = fontHeight;
  /* Insertion (x,y) point in the texture atlas.
     0 <= m_pimpl->m_currentPlacementPos.x < m_NumRows
     0 <= m_pimpl->m_currentPlacementPos.y < m_NumColumns
  */
  m_pimpl->m_currentPlacementPos = Coord(0,0);
  /* Round up to nearest power of two */
  unsigned hbW = 1<<(31-internal_clz(fontWidth));
  unsigned hbH = 1<<(31-internal_clz(fontHeight));

  std::cout << "rounded-up font size: " << hbW << "/" << hbH << std::endl;

  /* width and height of each glyph, rounded up to nearest POT */
  m_pimpl->m_FontWidthPad = (hbW == fontWidth) ? fontWidth : (hbW << 1);
  m_pimpl->m_FontHeightPad = (hbH == fontHeight) ? fontHeight : (hbH << 1);

  /* TODO: On Windows, use FT_Open_Face/FT_Open_Args instead of FT_New_Face, 
     because there is no way to get the font path there. Instead, we can get
     the font data directly from GetFontData(), and FT_Open_Face() consumes that */
  error = FT_New_Face(FontData::m_Library, path.c_str(), 0, &m_pimpl->m_Face);
  if(error) throw std::runtime_error("Failed to load FT2 face");
  error = FT_Set_Pixel_Sizes(m_pimpl->m_Face, fontWidth, fontHeight);
  if(error) throw std::runtime_error("Failed to set glyph size");

  //m_pimpl->m_AtlasTexture = new unsigned char[FontData::m_AtlasTextureWidth * FontData::m_AtlasTextureHeight];
  m_pimpl->m_AtlasTexture.resize(FontData::m_AtlasTextureWidth * FontData::m_AtlasTextureHeight * 4);
  std::fill(m_pimpl->m_AtlasTexture.begin(), m_pimpl->m_AtlasTexture.end(), 0);
  m_pimpl->m_NumRows = FontData::m_AtlasTextureWidth / m_pimpl->m_FontWidthPad;
  m_pimpl->m_NumColumns = FontData::m_AtlasTextureHeight / m_pimpl->m_FontHeightPad;
  /* Total number of glyphs that can fit into the atlas */
  m_pimpl->m_MaxNumGlyphs = m_pimpl->m_NumRows * m_pimpl->m_NumColumns;

  glEnable(GL_TEXTURE_2D);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures(1, &m_pimpl->m_AtlasTextureID);
  glBindTexture(GL_TEXTURE_2D, m_pimpl->m_AtlasTextureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8 /*GL_R8*/,
	       FontData::m_AtlasTextureWidth,
	       FontData::m_AtlasTextureHeight,
	       0, GL_RGBA /* GL_RED */, GL_UNSIGNED_BYTE, &m_pimpl->m_AtlasTexture[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

FontRenderer::~FontRenderer()
{
  FT_Done_Face(m_pimpl->m_Face);
  --FontData::m_LibraryRefs;
  if(!FontData::m_LibraryRefs){
    FT_Done_FreeType(FontData::m_Library);
  }
  delete m_pimpl;
}

/* 
   Precaches glyphs / populates the cache atlas texture, but does
   not lock glyphs.
   Glyps to be precached should be without duplicates
*/
void FontRenderer::loadGlyph(wchar_t ch)
{
  int glyph_index, error;
  FT_Glyph glyph;
  FT_BitmapGlyph glyph_bitmap;
  FT_Bitmap bitmap;
  FT_GlyphSlot slot;
  unsigned int charcode;
  unsigned int xOffset, yOffset;
  unsigned int colSrc, colDst;

  charcode = (unsigned int)ch;  
  slot = m_pimpl->m_Face->glyph;
  glyph_index = FT_Get_Char_Index(m_pimpl->m_Face, charcode);
  error = FT_Load_Glyph(m_pimpl->m_Face, glyph_index, FT_LOAD_DEFAULT);
  if(error)
    throw std::runtime_error("Failed to load char");
  error = FT_Get_Glyph(m_pimpl->m_Face->glyph, &glyph);
  if(error)
    throw std::runtime_error("Failed to get glyph");
  if(glyph->format != FT_GLYPH_FORMAT_BITMAP){
    error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1 );
    if(error)
      throw std::runtime_error("Failed to convert glyph");
  }

  glyph_bitmap = (FT_BitmapGlyph)glyph;
  bitmap = glyph_bitmap->bitmap;

  /* If it doesn't exist, create a new one. Updates atlas insertion position*/
  std::map<unsigned int, GlyphInfo>::iterator info_it = FontRenderer::getFreeEntry(charcode);
  /* should never happen */
  if(info_it == m_pimpl->m_Ginfo.end()) return;

  GlyphInfo& info = info_it->second;
  info.advanceX = glyph->advance.x >> 16;
  info.width = bitmap.width;
  info.height = bitmap.rows;
  info.top = glyph_bitmap->top;
  info.left = glyph_bitmap->left;
  info.sMin = float(info.m_Pos.x * m_pimpl->m_FontWidthPad) / float(FontData::m_AtlasTextureWidth);
  info.sMax = float(info.m_Pos.x * m_pimpl->m_FontWidthPad + bitmap.width) / float(FontData::m_AtlasTextureWidth);
  info.tMin = float(info.m_Pos.y * m_pimpl->m_FontHeightPad) / float(FontData::m_AtlasTextureHeight);
  info.tMax = float(info.m_Pos.y * m_pimpl->m_FontHeightPad + bitmap.rows) / float(FontData::m_AtlasTextureHeight);
  info.m_Priority = 1;
  info.m_GlyphIndex = ch;
  info.m_IsDirty = true;
  info.m_IsLocked = false;

  for(unsigned int y = 0; y < info.height; ++y){
    yOffset = info.m_Pos.y * m_pimpl->m_FontHeightPad;
    colDst = (y+yOffset)*FontData::m_AtlasTextureWidth; //*bitmap.pitch;
    colSrc = y*bitmap.pitch;
    for(unsigned int x = 0; x < info.width; ++x){
      xOffset = info.m_Pos.x * m_pimpl->m_FontWidthPad;
      unsigned char color = bitmap.buffer[x + colSrc];
      unsigned int index = (colDst + x + xOffset)*4;
      m_pimpl->m_AtlasTexture[index + 0] = color;
      m_pimpl->m_AtlasTexture[index + 1] = color;
      m_pimpl->m_AtlasTexture[index + 2] = color;
      m_pimpl->m_AtlasTexture[index + 3] = color;
    }
  }
  FT_Done_Glyph(glyph);
}

void FontRenderer::refreshClientAtlas()
{
  glEnable(GL_TEXTURE_2D);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, FontData::m_AtlasTextureWidth);
  glBindTexture(GL_TEXTURE_2D, m_pimpl->m_AtlasTextureID);
#if 0
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8 /* GL_R8 */,
	       FontData::m_AtlasTextureWidth,
	       FontData::m_AtlasTextureHeight,
	       0, GL_RGBA /* GL_RED */, GL_UNSIGNED_BYTE, &m_pimpl->m_AtlasTexture[0]);
#endif
#if 1
  for(std::map<unsigned int, GlyphInfo>::iterator it = m_pimpl->m_Ginfo.begin();
      it != m_pimpl->m_Ginfo.end(); ++it){
    GlyphInfo& info = it->second;
    if(info.m_IsDirty){
      info.m_IsDirty = false;
      int xOffs = info.m_Pos.x * m_pimpl->m_FontWidthPad;
      int yOffs = info.m_Pos.y * m_pimpl->m_FontHeightPad;
      int srcOffs = (xOffs + yOffs * FontData::m_AtlasTextureWidth) * 4;
      glTexSubImage2D(GL_TEXTURE_2D, 0, xOffs, yOffs, m_pimpl->m_FontWidthPad, m_pimpl->m_FontHeightPad,
		      GL_RGBA, GL_UNSIGNED_BYTE, &m_pimpl->m_AtlasTexture[srcOffs]);
    }
  }
#endif
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void FontRenderer::print(const std::wstring& str, int x, int y)
{
  std::map<unsigned int, GlyphInfo>::iterator it;
  glPushMatrix();
  glTranslatef(x, y, 0);
  unsigned int xAccum = 0;
  for(int i = 0; i < str.length(); ++i){
    wchar_t ch = str[i];
    it = m_pimpl->m_Ginfo.find(ch);
    if(it == m_pimpl->m_Ginfo.end()){
      loadGlyph(ch);
      refreshClientAtlas();
      it = m_pimpl->m_Ginfo.find(ch);
    }
    GlyphInfo& gl = it->second;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_pimpl->m_AtlasTextureID); 

    float x0 = gl.left + xAccum;
    float y0 = gl.top;
    float x1 = gl.left + xAccum + gl.width;
    float y1 = gl.top - gl.height;

    glBegin(GL_QUADS);
    glTexCoord2f(gl.sMin, gl.tMin); glVertex2f(x0, y0);
    glTexCoord2f(gl.sMin, gl.tMax); glVertex2f(x0, y1);
    glTexCoord2f(gl.sMax, gl.tMax); glVertex2f(x1, y1);
    glTexCoord2f(gl.sMax, gl.tMin); glVertex2f(x1, y0);
    glEnd();
    xAccum += gl.advanceX;
  }
  glPopMatrix();
}

std::string FontRenderer::findFontPath(
			   const std::string& family,
			   const std::string language,
			   bool bold,
			   bool italic,
			   bool monospace)
{
  const FcChar8	*format = (const FcChar8*)"%{file}";
  FcObjectSet* os;
  FcPattern* pat;
  FcPattern* match;
  FcLangSet* ls;
  FcResult result;

  std::string ret;
  int pat_weight = bold ? FC_WEIGHT_BOLD : FC_WEIGHT_REGULAR; /*FC_WEIGHT_REGULAR or FC_WEIGHT_BOOK = normal */
  int pat_slant = italic ? FC_SLANT_ITALIC : FC_SLANT_ROMAN; /* FC_SLANT_ITALIC = italic, FC_SLANT_ROMAN = normal */
  int pat_spacing = monospace ? FC_MONO : FC_PROPORTIONAL; /* FC_PROPORTIONAL = normal, FC_MONO = monospace */

  /* Maybe initialize libfontconfig just once? */
  if(!FcInit()){
    std::runtime_error e("Failed to initialize fontconfig.");
    throw e;
  }

  pat = FcPatternCreate();
  ls = FcLangSetCreate();

  if(!language.empty()){
    FcLangSetAdd(ls, (const FcChar8*)language.c_str());
  }
  /* Ask for language/charset support */
  FcPatternAddLangSet(pat, FC_LANG, ls);
  /* We only want truetype fonts, for scaling */
  FcPatternAddBool(pat, FC_OUTLINE, FcTrue);
  FcPatternAddBool(pat, FC_SCALABLE, FcTrue);
  /* Search for a specific font name, if specified by the user. This is entirely optional */
  if(!family.empty()) FcPatternAddString(pat, FC_FAMILY, (const FcChar8*)family.c_str());
  /* Specify we want either bold or regular weight */
  FcPatternAddInteger(pat, FC_WEIGHT, pat_weight);
  /* Specify we want either italic or regular slant */
  FcPatternAddInteger(pat, FC_SLANT, pat_slant);
  /* Specify we want variable space between glyphs or monospace */
  FcPatternAddInteger(pat, FC_SPACING, pat_spacing);
  /* We always want antialiasing. Since it's put last, it doesn't get prioritized */
  FcPatternAddBool(pat, FC_ANTIALIAS, FcTrue);

  FcConfigSubstitute(0, pat, FcMatchPattern);
  FcDefaultSubstitute (pat);
  match = FcFontMatch(0, pat, &result);
  if(!match){
    std::runtime_error e("Failed to find an appropriate font.");
    throw e;
  }
  //FcFontSetAdd(fs, match);
  FcPatternDestroy(pat);
  FcLangSetDestroy(ls);
  //FcChar8* path = FcPatternFormat(fs->fonts[0], format);
  FcChar8* path = FcPatternFormat(match, (const FcChar8*)"%{file}");
  ret = std::string((const char*)path);
  FcStrFree(path);
  //FcFontSetDestroy(fs);
  FcPatternDestroy(match); //either this or destroy fs
  FcFini();

  if(ret.empty()){
    std::runtime_error e("Found font, but failed to get its path!");
    throw e;
  }
  return ret;
}

void FontRenderer::dumpFontAtlas() const
{
  write_png_file("test.png", m_pimpl->m_AtlasTexture,
		 FontData::m_AtlasTextureWidth, FontData::m_AtlasTextureHeight);
}

