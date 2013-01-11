#include <stdexcept>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "font.hpp"

int width = 1280;
int height = 512;

class MainLoop
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
  
  
  /* Asking for Arial, with support for Norwegian, Bold, not Italic and variably spaced. */  
  MainLoop() : fntrender("Arial", "no", true, false, false, 32, 32)
  {
    
  }
  
  void operator()()
  {
    static bool once = true;
    const std::wstring str(L"Cras a egestas mi. Praesent non auctor lorem. Pellentesque pretium vestibulum suscipit. Duis dui augue, imperdiet eget porta egestas, viverra ut enim. Suspendisse neque dui, faucibus et cursus sit amet, viverra ac orci. Pellentesque mollis, lectus eu molestie sagittis, risus neque blandit turpis, ac hendrerit ligula ipsum in justo. Nunc eleifend nisi ac neque bibendum elementum. Integer mattis arcu nec leo tincidunt vitae hendrerit nunc aliquet. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Nulla pellentesque lectus ac sapien tincidunt semper. Sed euismod auctor sagittis. Pellentesque in elit et orci cursus auctor. Phasellus in malesuada justo. Vivamus tempus orci et nibh laoreet ac varius nibh sagittis. Quisque dictum fermentum euismod. Aliquam laoreet, lectus rutrum sagittis molestie, turpis lacus ornare nisl, ac blandit orci massa a augue. abcdefghijklmnopqrstuvwxyzæøåABCDEFGHIJKLMNOPQRSTUVWXYZÆØÅ1234567890<>,;.:-_'*~^|§!#¤%&/()=?+\\`abcdefghijklmnopqrstuvwxyzæøåABCDEFGHIJKLMNOPQRSTUVWXYZÆØÅ1234567890<>,;.:-_'*~^|§!#¤%&/()=?+\\`");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, width, 0, height, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT);
    int fsize = 32;
    int lineWidth = 80;
    for(int i=512-fsize, j=0; i >= 0; i-=fsize, j++){
      std::wstring sub;
      int len = str.length() - j*lineWidth;
      if((len - lineWidth) < 0) break;
      if(len > lineWidth) len = lineWidth;
      sub = str.substr(j*lineWidth, len);
      fntrender.print(sub, 0, i);
    }
    if(once){
      once = false;
      fntrender.dumpFontAtlas();
    }
    SDL_GL_SwapBuffers();
  }
private:
  FontRenderer fntrender;
};



int main(int argc, char* argv[])
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_SetVideoMode(width, height, 32, SDL_OPENGL);
  SDL_WM_SetCaption("MechCore.net Font Test", NULL);
  bool running = true;
  SDL_Event event;

  try {
    MainLoop loop;
    while(running){
      while(SDL_PollEvent(&event)){
	switch(event.type){
	case SDL_KEYDOWN:
	  if(event.key.keysym.sym == SDLK_ESCAPE){
	    running = false;
	  }
	  break;
	case SDL_QUIT:
	  running = false;
	  break;
	}
      }
      loop();
    }
    SDL_Quit();
    return 0;
  } catch(std::runtime_error& e){
    printf("Exception caught: %s\n", e.what());
  } catch(std::exception& e){
    printf("Exception caught: %s\n", e.what());
  }
}
