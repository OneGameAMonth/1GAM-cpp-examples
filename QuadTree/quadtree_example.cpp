/*
Copyright (c) 2012, Mads Elvheim
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions 
are met:

  Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the 
  documentation and/or other materials provided with the distribution.
  
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <iostream>
#include <cstdlib> //only for rand() in the example
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "quadtree.hpp"
/*******************************************************************************************
********************************** Example usage *******************************************
*********************************************************************************************/

class ConcreteObject : public Object
{
public:
  ConcreteObject(const Point& position, const Rectangle& rect) :
    m_Position(position), m_BoundingBox(rect), m_Node(0){}
protected:
  AQTreeNode* vtGetNodeParent()
  {
    return m_Node;
  }
  void vtSetNodeParent(AQTreeNode* node)
  {
    m_Node = node;
  }
  Point vtGetPosition() const
  {
    return m_Position;
  }
  Rectangle vtGetBoundingBox() const
  {
    return m_BoundingBox;
  }
private:
  Point m_Position;
  Rectangle m_BoundingBox;
  AQTreeNode* m_Node;
};

static ConcreteObject* createConcreteObject(const std::list<Object*>& objList, unsigned int worldWidth, unsigned worldHeight)
{
  const unsigned int horisontalOffset = 64;
  const unsigned int verticalOffset = 64;
  bool overlap;
  ConcreteObject* obj = NULL;
  srand(time(NULL));
  for(;;){
    overlap = false;
    unsigned int x = horisontalOffset + rand() % (worldWidth - horisontalOffset);
    unsigned int y = verticalOffset + rand() % (worldHeight - verticalOffset);
    unsigned int w = (8 + rand() % 60)>>1;
    unsigned int h = (8 + rand() % 60)>>1;
    Rectangle rect(x-w, x+w, y-h, y+h);
    Point pos(x,y);
    if( rect.getMinX() < 0 || rect.getMinX() >= worldWidth ||
        rect.getMaxX() < 0 || rect.getMaxX() >= worldWidth ||
        rect.getMinY() < 0 || rect.getMinY() >= worldHeight ||
        rect.getMaxY() < 0 || rect.getMaxY() >= worldHeight )
        continue;
    for(std::list<Object*>::const_iterator it = objList.begin(); it != objList.end(); it++){
      if((*it)->getBoundingBox().isOverlapping(rect) ||
        (*it)->getBoundingBox().isInside(rect) ||
        rect.isInside((*it)->getBoundingBox()) ||
        rect.isOverlapping((*it)->getBoundingBox())){
          overlap = true;
          break;
        }
    }
    if(!overlap){
      obj = new ConcreteObject(pos, rect);
      break;
    }
  }
  return obj;
}


static void renderTreeGrid(AQTreeNode* node)
{
  unsigned int depth = node->getDepth();
  float intensity = 0.16f * (float)(depth+1);
  
  if(depth){
    std::vector<AQTreeNode*> children = node->getChildren();
    for(int i = 0; i < 4; ++i)
      if(children[i])
        renderTreeGrid(children[i]);
  }
  Rectangle rect = node->getRect();

  glColor3f(intensity,intensity,intensity);
  glLineWidth(1.2f * (float)depth);
  glBegin(GL_LINE_LOOP);
    glVertex2i(rect.getMinX(), rect.getMaxY());
    glVertex2i(rect.getMinX(), rect.getMinY());
    glVertex2i(rect.getMaxX(), rect.getMinY());
    glVertex2i(rect.getMaxX(), rect.getMaxY());
  glEnd();
}

static void render(Point cursor, AQTreeNode* qt,
  unsigned int vpsize,
  unsigned int worldWidth,
  unsigned int worldHeight)
{
  /* Coder colors to color objects based on the level in the tree
    where they are stored */
  unsigned char colors[6][3] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 255, 0},
    {0, 255, 255},
    {255, 0, 255},
  };
  int x1 = cursor.x - (vpsize >> 1);
  int x2 = cursor.x + (vpsize >> 1);
  int y1 = cursor.y - (vpsize >> 1);
  int y2 = cursor.y + (vpsize >> 1);
  if(x1 < 0) x1 = 0;
  if(x2 >= worldWidth) x2 = worldWidth-1;
  if(y1 < 0) y1 = 0;
  if(y2 >= worldHeight) y2 = worldHeight-1;
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, worldWidth, 0, worldHeight, 0.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT);
  
  /*******************************************************************
  ************** Render everything "invisible" as dark ***************
  ********************************************************************/
  
  /* Recursively gets the whole object list */
  std::list<Object*> objs = qt->getObjects();
  for(std::list<Object*>::iterator it = objs.begin(); it != objs.end(); it++){
    Rectangle bbox = (*it)->getBoundingBox();
    /* Objects can access the node they reside in*/
    AQTreeNode* parent = (*it)->getNodeParent();
    /* Color based on the QuadTree Node depth the object
        belongs to. If you mess with the example and change the depth level,
        remember to add more colors to the array. */
    unsigned int colorIndex = parent->getDepth();
    /* Color at 25% intensity */
    glColor3ub(colors[colorIndex][0]>>2, colors[colorIndex][1]>>2, colors[colorIndex][2]>>2);
    glBegin(GL_QUADS);
      glVertex2f(bbox.getMinX(), bbox.getMaxY());
      glVertex2f(bbox.getMinX(), bbox.getMinY());
      glVertex2f(bbox.getMaxX(), bbox.getMinY());
      glVertex2f(bbox.getMaxX(), bbox.getMaxY());
    glEnd();
  }
  
  Rectangle vp(x1, x2, y1, y2);
  /* Queries the quadtree for all the contents inside a rectangle,
    which we can move around with the cursor */
  std::list<Object*> culledObjs = qt->getObjectsInsideRect(vp); //qt->getObjects();
  
  std::cout << "Rendering " << culledObjs.size()  << " out of " << objs.size() << " objects." << std::endl;
  
  /* Restrict rendering to our box */
  glEnable(GL_SCISSOR_TEST);
  glScissor(x1, y1, x2-x1, y2-y1);

  /* Clear the box contents and re-render everything at full intensity */
  glClear(GL_COLOR_BUFFER_BIT);  
  for(std::list<Object*>::iterator it = culledObjs.begin(); it != culledObjs.end(); it++){
    Rectangle bbox = (*it)->getBoundingBox();
    /* Objects can access their parent QuadTree Node */
    AQTreeNode* parent = (*it)->getNodeParent();
    /* Color based on the QuadTree Node depth the object
        belongs to */
    unsigned int colorIndex = parent->getDepth();
    glColor3ub(colors[colorIndex][0], colors[colorIndex][1], colors[colorIndex][2]);
    glBegin(GL_QUADS);
      glVertex2i(bbox.getMinX(), bbox.getMaxY());
      glVertex2i(bbox.getMinX(), bbox.getMinY());
      glVertex2i(bbox.getMaxX(), bbox.getMinY());
      glVertex2i(bbox.getMaxX(), bbox.getMaxY());
    glEnd();
    /* Also add an outline */
    glColor3ub(255, 255, 255);
    glBegin(GL_LINE_LOOP);
      glVertex2i(bbox.getMinX(), bbox.getMaxY());
      glVertex2i(bbox.getMinX(), bbox.getMinY());
      glVertex2i(bbox.getMaxX(), bbox.getMinY());
      glVertex2i(bbox.getMaxX(), bbox.getMaxY());
    glEnd();
  }
  glDisable(GL_SCISSOR_TEST);

  /* Render the tree grid to visualize the nodes */
  renderTreeGrid(qt);
  glLineWidth(1.0f);
  SDL_GL_SwapBuffers();
}

/* Depth-first-traversal, printing depth, rects and owned objects*/
static void dumpTreeInfo(AQTreeNode* node)
{
  using std::cout;
  using std::endl;

  unsigned int depth = node->getDepth();

  if(depth){
    std::vector<AQTreeNode*> children = node->getChildren();
    for(int i = 0; i < 4; ++i)
      if(children[i])
        dumpTreeInfo(children[i]);
  }
  Rectangle rect = node->getRect();
  std::list<Object*> objects = node->getObjects();
  
  cout << "Leaf depth: " << depth << endl;
  cout << "Rect bounds:"
    << " XMin: " << rect.getMinX() << " XMax: " << rect.getMaxX()
    << " YMin: " << rect.getMinY() << " YMax: " << rect.getMaxY() << endl;
  cout << "Owner of " << objects.size() << " objects." << endl;
  cout << "Object rects: " << endl;
  for(std::list<Object*>::iterator it = objects.begin(); it != objects.end(); it++){
    Rectangle objRect = (*it)->getBoundingBox();
    cout << "Object ptr: " << std::hex << (*it) << std::dec << endl;
    cout << "Rect bounds:"
      << " XMin: " << objRect.getMinX() << " XMax: " << objRect.getMaxX()
      << " YMin: " << objRect.getMinY() << " YMax: " << objRect.getMaxY() << endl;    
  }
}
int main()
{
  const unsigned int NUM_OBJECTS = 80;
  const unsigned int WorldWidth = 640;
  const unsigned int WorldHeight = 640;
  const unsigned int ViewPortSize = 192;
  /* 640 320 160 80 40 20 */
  try {
    std::list<Object*> objects;
    AQTreeNode* quadTree = new QTNode(Rectangle(0,WorldWidth,0,WorldHeight), 5);
    for(unsigned int i = 0; i < NUM_OBJECTS; ++i){
      Object* ob = createConcreteObject(objects, WorldWidth, WorldHeight);
      quadTree->addObject(ob);
      objects.insert(objects.begin(), ob);
    }
    #ifdef DEBUG
    dumpTreeInfo(quadTree);
    #endif
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_SetVideoMode(WorldWidth, WorldHeight, 32, SDL_OPENGL);
    bool running = true;
    Point cursorPos = Point(0,0);
    SDL_Event event;
    while(running){
      while(SDL_PollEvent(&event)){
        switch(event.type){
          case SDL_QUIT:
            running = false;
            break;
          case SDL_KEYDOWN:
            running = false;
            break;
          case SDL_MOUSEMOTION:
            cursorPos.x = event.motion.x;
            cursorPos.y = WorldHeight - event.motion.y;
            break;
        }
      }
      render(cursorPos, quadTree, ViewPortSize, WorldWidth, WorldHeight);
    }
    SDL_Quit();
    for(std::list<Object*>::iterator it = objects.begin(); it != objects.end(); it++){
      delete *it;
    }
  } catch(std::bad_alloc& e){
    std::cout << "Out-of-memory exception: " << e.what() << std::endl;
  } catch(std::exception& e){
    std::cout << "Exception: " << e.what() << std::endl;
  }
}


