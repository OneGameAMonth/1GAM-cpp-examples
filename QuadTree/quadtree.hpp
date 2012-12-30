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

#ifndef ONEGAM_QUADTREE_HPP_GUARD
#define ONEGAM_QUADTREE_HPP_GUARD

#ifdef DEBUG
#include <iostream> //debug
#endif
#include <list>
#include <vector>
#include <new>
#include <stdexcept>
#include <exception>
#include <cmath>

struct Point
{
  Point(int i, int j) : x(i), y(j){}
  int x,y;
};

static bool insideInterval(int v, int start, int end)
{
  return start <= v && v <= end;
}

class Rectangle
{
public:
  Rectangle() : m_Min(0,0), m_Max(0,0){}
  Rectangle(int x1, int x2, int y1, int y2) :
    m_Min(x1, y1), m_Max(x2, y2){}
  Rectangle(const Point& minP,
            const Point& maxP) :
    m_Min(minP), m_Max(maxP){}
  int getMinX() const { return m_Min.x; }
  int getMaxX() const { return m_Max.x; }
  int getMinY() const { return m_Min.y; }
  int getMaxY() const { return m_Max.y; }
  Point getMin() const { return m_Min; }
  Point getMax() const { return m_Max; }
  int getWidth() const { return std::abs(m_Min.x - m_Max.x); }
  int getHeight() const { return std::abs(m_Min.y - m_Max.y); }
  
  bool isInside(Point p) const
  {
    return (m_Min.x <= p.x && m_Max.x >= p.x &&
            m_Min.y <= p.y && m_Max.y >= p.y);
  }
  /* rect/rhs is inside this/lhs rectangle */
  bool isInside(const Rectangle& rect) const
  {
    return
      isInside(Point(rect.getMinX(), rect.getMinY())) &&
      isInside(Point(rect.getMinX(), rect.getMaxY())) &&
      isInside(Point(rect.getMaxX(), rect.getMinY())) &&
      isInside(Point(rect.getMaxX(), rect.getMaxY()));
  }
  bool isOverlapping(const Rectangle& rect) const
  {
    /* Ugly, but correct. FIXME */
    return
      (insideInterval(rect.getMinX(), getMinX(), getMaxX()) ||
       insideInterval(rect.getMaxX(), getMinX(), getMaxX()) ||
       insideInterval(getMinX(), rect.getMinX(), rect.getMaxX()) ||
       insideInterval(getMaxX(), rect.getMinX(), rect.getMaxX())) &&
      (insideInterval(rect.getMinY(), getMinY(), getMaxY()) ||
       insideInterval(rect.getMaxY(), getMinY(), getMaxY()) ||
       insideInterval(getMinY(), rect.getMinY(), rect.getMaxY()) ||
       insideInterval(getMaxY(), rect.getMinY(), rect.getMaxY()));
  }
  Rectangle getIntersection(const Rectangle& rect) const
  {
    return Rectangle(
      std::max(getMinX(), rect.getMinX()),
      std::min(getMaxX(), rect.getMaxX()),
      std::max(getMinY(), rect.getMinY()),
      std::min(getMaxY(), rect.getMaxY()));
  }
  /* Get the rectangle which is big enough to hold both rectangles */
  Rectangle getUnion(const Rectangle& rect) const
  {
    return Rectangle(
      std::min(getMinX(), rect.getMinX()),
      std::max(getMaxX(), rect.getMaxX()),
      std::min(getMinY(), rect.getMinY()),
      std::max(getMaxY(), rect.getMaxY()));  
  }
private:
  Point m_Min, m_Max;
};

class AQTreeNode;

class Object
{
public:
  virtual ~Object(){}

  AQTreeNode* getNodeParent()
  {
    return vtGetNodeParent();
  }
  void setNodeParent(AQTreeNode* node)
  {
    vtSetNodeParent(node);
  }
  Point getPosition() const
  {
    return vtGetPosition();
  }
  Rectangle getBoundingBox() const
  {
    return vtGetBoundingBox();
  }
protected:
  virtual AQTreeNode* vtGetNodeParent() = 0;
  virtual void vtSetNodeParent(AQTreeNode* node) = 0;
  virtual Point vtGetPosition() const = 0;
  virtual Rectangle vtGetBoundingBox() const = 0;
};

class AQTreeNode
{
public:
  virtual ~AQTreeNode(){};

  Rectangle getRect() const
  {
    return vtGetRect();
  }
  std::list<Object*> getObjectsInsideRect(const Rectangle& rect)
  {
    return vtGetObjectsInsideRect(rect);
  }
  std::list<Object*> getObjects()
  {
    return vtGetObjects();
  }
  void addObject(Object* obj)
  {
    vtAddObject(obj);
  }
  void eraseObject(Object* obj)
  {
    vtEraseObject(obj);
  }
  std::vector<AQTreeNode*> getChildren()
  {
    return vtGetChildren();
  }
  bool isEmpty() const
  {
    return vtIsEmpty();
  }
  unsigned int getDepth() const
  {
    return vtGetDepth();
  }
  void optimize()
  {
    return vtOptimize();
  }
protected:
    virtual Rectangle vtGetRect() const = 0;
    virtual std::list<Object*> vtGetObjectsInsideRect(const Rectangle& rect) = 0;
    virtual std::list<Object*> vtGetObjects() = 0;
    virtual void vtAddObject(Object* obj) = 0;
    virtual void vtEraseObject(Object* obj) = 0;
    virtual std::vector<AQTreeNode*> vtGetChildren() = 0;
    virtual bool vtIsEmpty() const = 0;
    virtual unsigned int vtGetDepth() const = 0;
    virtual void vtOptimize() = 0;
};

/* Leaves in the quadtree */
class QTLeaf : public AQTreeNode
{
public:
  QTLeaf(const Rectangle& bounds, unsigned int depth) : m_Rect(bounds), m_Depth(depth)
  {
    
  }
protected:
  Rectangle vtGetRect() const
  {
    return m_Rect;
  }
  std::list<Object*> vtGetObjectsInsideRect(const Rectangle& rect)
  {
    std::list<Object*> objList;
    
    for(std::list<Object*>::iterator it = m_Objects.begin(); it != m_Objects.end(); it++){
      if(rect.isOverlapping((*it)->getBoundingBox()))
        objList.insert(objList.begin(), *it);
    }
    return objList;
  }
  std::list<Object*> vtGetObjects()
  {
    return m_Objects;
  }
  void vtAddObject(Object* obj)
  {                    
    /* No children for leaves, so add directly */
    obj->setNodeParent(this);
    m_Objects.insert(m_Objects.begin(), obj);
  }
  void vtEraseObject(Object* obj)
  {
    for(std::list<Object*>::iterator it = m_Objects.begin(); it != m_Objects.end(); it++){
      if(*it == obj){
        m_Objects.erase(it);
        break;
      }
    }
  }
  std::vector<AQTreeNode*> vtGetChildren()
  {
    std::vector<AQTreeNode*> emptyVec;
    return emptyVec;
  }
  bool vtIsEmpty() const
  {
    return m_Objects.empty();
  }
  unsigned int vtGetDepth() const
  {
    return m_Depth;
  }
  void vtOptimize()
  {
    /* Leaves have no children, so nothing to optimize */
    return;
  }
private:
  Rectangle m_Rect;
  /* Leafs have no children
  QTNode* children[4]; */
  std::list<Object*> m_Objects;
  unsigned int m_Depth;
};

/* A QuadTree Node or Root */
class QTNode : public AQTreeNode
{
public:
  /* Unless max depth is reached, recursively add children nodes */
  QTNode(const Rectangle& bounds, unsigned int depth) : m_Rect(bounds), m_Depth(depth)
  {
    for(int i = 0; i < 4; ++i)
      m_Children[i] = 0;
  }
private:
  Rectangle getChildRect(unsigned int which)
  {
    /*  1 = upper left,
        2 = upper right,
        3 = bottom left,
        4 = bottom right
    */
    
    /* Rectangle ranges are inclusive, so carefully add 1 to middle.x and middle.y
        for the rightmost and bottommost rectangles. None of the nodes should overlap. */
        
    Point middle = Point(
      m_Rect.getMinX() + (m_Rect.getMaxX() - m_Rect.getMinX()) / 2,
      m_Rect.getMinY() + (m_Rect.getMaxY() - m_Rect.getMinY()) / 2);
    Rectangle rect;  
    switch(which){
      case 0:
        rect = Rectangle(m_Rect.getMinX(), middle.x, m_Rect.getMinY(), middle.y);
        break;
      case 1:
        rect = Rectangle(middle.x + 1, m_Rect.getMaxX(), m_Rect.getMinY(), middle.y);
        break;
      case 2:
          rect = Rectangle(m_Rect.getMinX(), middle.x, middle.y + 1, m_Rect.getMaxY());
        break;
      case 3:
        rect = Rectangle(middle.x + 1, m_Rect.getMaxX(), middle.y + 1, m_Rect.getMaxY());
        break;
    }
    return rect;
  }
protected:
  Rectangle vtGetRect() const
  {
    return m_Rect;
  }
  /* Recursively adds all the objects in the tree which overlaps a rectangle */
  std::list<Object*> vtGetObjectsInsideRect(const Rectangle& rect)
  {
    std::list<Object*> objList;
    
    for(int i = 0; i < 4; ++i){
      if(!m_Children[i]) continue;
      if(m_Children[i]->getRect().isInside(rect)){
        /* rect is completely inside a child */
        std::list<Object*> objs = m_Children[i]->getObjectsInsideRect(rect);
        objList.insert(objList.begin(), objs.begin(), objs.end());
        break;
      } else if(m_Children[i]->getRect().isOverlapping(rect)){
        /* rect partially covers several children */
        Rectangle partialRect = rect.getIntersection(m_Children[i]->getRect());
        std::list<Object*> objs = m_Children[i]->getObjectsInsideRect(partialRect);
        objList.insert(objList.begin(), objs.begin(), objs.end());
      }
    }
    /* For the node itself, only add its object if it is inside the rect */
    for(std::list<Object*>::iterator it = m_Objects.begin(); it != m_Objects.end(); it++){
      if(rect.isOverlapping((*it)->getBoundingBox()))
        objList.insert(objList.begin(), *it);
    }
    return objList;  
  }
  
  std::list<Object*> vtGetObjects()
  {
    std::list<Object*> li = m_Objects;
    for(int i = 0; i < 4; ++i){
      if(!m_Children[i]) continue;
      std::list<Object*> liOb = m_Children[i]->getObjects();
      li.insert(li.begin(), liOb.begin(), liOb.end());
    }
    return li;
  }
  
  void vtAddObject(Object* obj)
  {
    Rectangle bbox = obj->getBoundingBox();
    if(!m_Rect.isInside(bbox)){
      std::runtime_error e("QTNode: Invalid use of library. Object rect out of bounds.");
      throw e;    
    }
    
    bool belongsToChild = false;
    for(int i = 0; i < 4; ++i){
      Rectangle chRect = getChildRect(i);
      if(chRect.isInside(bbox)){
        if(!m_Children[i]){
          if(m_Depth > 1) m_Children[i] = new QTNode(chRect, m_Depth - 1);
          else m_Children[i] = new QTLeaf(chRect, 0);
        }
        m_Children[i]->addObject(obj);
        belongsToChild = true;
        break;
      }
    }

    /* Only add at this level if it doesn't belong to any of the children
      This means that if an object overlaps two nodes, it belongs to the parent node.
      It's not allowed for an object to overlap the root rectangle, so every legal object
      is guaranteed to be at least parented by the root. */

    if(!belongsToChild){
      obj->setNodeParent(this);
      m_Objects.insert(m_Objects.begin(), obj);
    }
  }
  void vtEraseObject(Object* obj)
  {

    for(std::list<Object*>::iterator it = m_Objects.begin(); it != m_Objects.end(); it++){
      if(*it == obj){
        m_Objects.erase(it);
        return;
      }
    }
    /* If we didn't find it here, ask our children */
    for(int i = 0; i < 4; ++i){
      if(m_Children[i])
        m_Children[i]->eraseObject(obj);
    }
    return;
  }
  std::vector<AQTreeNode*> vtGetChildren()
  {
    std::vector<AQTreeNode*> n;
    n.push_back(m_Children[0]);
    n.push_back(m_Children[1]);
    n.push_back(m_Children[2]);
    n.push_back(m_Children[3]);

    return n;
  }
  bool vtIsEmpty() const
  {
    bool e1, e2, e3, e4;
    if(!m_Children[0]) e1 = true;
    else e1 = m_Children[0]->isEmpty();
    if(!m_Children[1]) e2 = true;
    else e2 = m_Children[1]->isEmpty();
    if(!m_Children[2]) e3 = true;
    else e3 = m_Children[2]->isEmpty();
    if(!m_Children[3]) e4 = true;
    else e4 = m_Children[3]->isEmpty();
    
    return m_Objects.empty() && e1 && e2 && e3 && e4;
  }
  unsigned int vtGetDepth() const
  {
    return m_Depth;
  }
  void vtOptimize()
  {
    for(int i=0; i<4; ++i){
      if(!m_Children[i]) continue;
      if(m_Children[i]->isEmpty()){
        delete m_Children[i];
        m_Children[i] = 0;
      }
    }
  }
private:
  Rectangle m_Rect;
  AQTreeNode* m_Children[4];
  std::list<Object*> m_Objects;
  unsigned int m_Depth;
};

#endif

