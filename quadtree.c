#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include<time.h>

#include<SDL2/SDL.h>
#include<math.h>

#define QT_NODE_CAPACITY (4)
#define MAX_ARRAY_SIZE (1024)

SDL_Window *mywindow=0;
SDL_Renderer *myrenderer=0;

void draw_line(int x1, int y1, int x2, int y2){
  SDL_SetRenderDrawColor(myrenderer,0,0,0,SDL_ALPHA_OPAQUE);
  SDL_RenderDrawLine(myrenderer,x1,y1,x2,y2);
}

void draw_point(int x, int y){
  SDL_SetRenderDrawColor(myrenderer,0,0,0,SDL_ALPHA_OPAQUE);
  SDL_RenderDrawPoint(myrenderer,x,y);
}

int randint(int min, int max){
  int r = (rand() % (max - min + 1) + min);
  return r;
}

typedef struct Point {
    float x;
    float y;
} Point;

typedef struct AABB {
    Point *center;
    float halfDimension;
} AABB;

typedef struct QuadTree {
    AABB *boundry;
    Point **points;

    struct QuadTree* NW;
    struct QuadTree* NE;
    struct QuadTree* SW;
    struct QuadTree* SE;

} QuadTree;


Point *Point_new(float x, float y) {
    Point *p = (Point *)malloc(sizeof(Point));
    p->x = x;
    p->y = y;
    return p;
}


Point *Point_origin() {
    Point *p = (Point *)malloc(sizeof(Point));
    p->x = 0.;
    p->y = 0.;
    return p;
}

void Point_print(Point *point) {
    printf("(%2.2f, %2.2f)\n", point->x, point->y);
}

void Point_draw(Point *point){
  int x = point->x;
  int y = point->y;
  draw_point(x,y);
}

AABB *AABB_new(Point *center, float halfDimension) {
    AABB *aabb = (AABB *)malloc(sizeof(AABB));
    aabb->center = center;
    aabb->halfDimension = halfDimension;
    return aabb;
}

void AABB_draw(AABB *box){
  int x1 = box->center->x - box->halfDimension;
  int y1 = box->center->y - box->halfDimension;
  int x2 = box->center->x + box->halfDimension;
  int y2 = box->center->y + box->halfDimension;
  draw_line(x1,y1,x1,y2);
  draw_line(x1,y1,x2,y1);
  draw_line(x1,y2,x2,y2);
  draw_line(x2,y1,x2,y2);
}

bool AABB_cotains_point(AABB *boundry, Point *point) {
    if (point->x < boundry->center->x - boundry->halfDimension || point->x > boundry->center->x + boundry->halfDimension) {
        return false;
    }

    if (point->y < boundry->center->y - boundry->halfDimension || point->y > boundry->center->y + boundry->halfDimension) {
        return false;
    }

    return true;
}

bool AABB_intersects_AABB(AABB *self, AABB *other) {
    if (self->center->x + self->halfDimension > other->center->x - other->halfDimension) {
        return true;
    }

    if (self->center->x - self->halfDimension < other->center->x + other->halfDimension) {
        return true;
    }

    if (self->center->y + self->halfDimension > other->center->y - other->halfDimension) {
        return true;
    }

    if (self->center->y - self->halfDimension < other->center->y + other->halfDimension) {
        return true;
    }

    return false;
}

QuadTree *QuadTree_new(AABB *boundry) {
    QuadTree *qt = (QuadTree *)malloc(sizeof(QuadTree));
    qt->NE = NULL;
    qt->NW = NULL;
    qt->SE = NULL;
    qt->SW = NULL;

    qt->boundry = boundry;

    qt->points = (Point **)malloc(sizeof(Point*) * QT_NODE_CAPACITY);

    for (size_t i = 0; i < QT_NODE_CAPACITY; i++)
    {
        qt->points[i] = NULL;
    }

    return qt;
}

size_t QuadTree_points_size(Point *points[]) {
    size_t i;
    for (i = 0; i < QT_NODE_CAPACITY; i++)
    {
        if (points[i] == NULL) {
            return i;
        }
    }

    return i;
}

QuadTree *QuadTree_subdivide(QuadTree *root) {
    float halfDim = root->boundry->halfDimension / 2;
    // North West
    Point *nw_p = Point_new(root->boundry->center->x - halfDim, root->boundry->center->y + halfDim);
    root->NW = QuadTree_new(AABB_new(nw_p, halfDim));

    // North East
    Point *ne_p = Point_new(root->boundry->center->x + halfDim, root->boundry->center->y + halfDim);
    root->NE = QuadTree_new(AABB_new(ne_p, halfDim));

    // South West
    Point *sw_p = Point_new(root->boundry->center->x - halfDim, root->boundry->center->y - halfDim);
    root->SW = QuadTree_new(AABB_new(sw_p, halfDim));

    // South East
    Point *se_p = Point_new(root->boundry->center->x + halfDim, root->boundry->center->y - halfDim);
    root->SE = QuadTree_new(AABB_new(se_p, halfDim));

    return root;
}


bool QuadTree_insert(QuadTree *root, Point *point) {
    if (!AABB_cotains_point(root->boundry, point)) {
        return false;
    }

    size_t points_size = QuadTree_points_size(root->points);

    if (points_size < QT_NODE_CAPACITY && root->NW == NULL) {
        root->points[points_size] = point;
        return true;
    }

    if (root->NW == NULL) {
        QuadTree_subdivide(root);
    }

    if (QuadTree_insert(root->NW, point)) return true;
    if (QuadTree_insert(root->NE, point)) return true;
    if (QuadTree_insert(root->SW, point)) return true;
    if (QuadTree_insert(root->SE, point)) return true;

    return false;
}

Point **QuadTree_query_range(QuadTree *root, AABB *range) {
    Point **result;
    result = (Point **)malloc(sizeof(Point *) * MAX_ARRAY_SIZE);

    size_t index = 0;
    for (size_t i = 0; i < MAX_ARRAY_SIZE; i++) {
        result[i] = NULL;
    }

    if (!AABB_intersects_AABB(root->boundry, range)) {
        return result;
    }

    size_t points_size = QuadTree_points_size(root->points);
    for (size_t i = 0; i < points_size; i++)
    {
        if (AABB_cotains_point(range, root->points[i])) {
            result[index++] = root->points[i];
        }
    }

    if (root->NW == NULL) {
        return result;
    }

    size_t i;

    i = 0;
    Point **nw_r = QuadTree_query_range(root->NW, range);
    while (nw_r[i] != NULL && i < MAX_ARRAY_SIZE) {
        result[index++] = nw_r[i];
    }

    i = 0;
    Point **ne_r = QuadTree_query_range(root->NE, range);
    while (ne_r[i] != NULL && i < MAX_ARRAY_SIZE) {
        result[index++] = ne_r[i];
    }

    i = 0;
    Point **sw_r = QuadTree_query_range(root->SW, range);
    while (sw_r[i] != NULL && i < MAX_ARRAY_SIZE) {
        result[index++] = sw_r[i];
    }

    i = 0;
    Point **se_r = QuadTree_query_range(root->SE, range);
    while (se_r[i] != NULL && i < MAX_ARRAY_SIZE) {
        result[index++] = se_r[i];
    }

    return result;
}

void QuadTree_draw(QuadTree *root){
  //draw root boundry
  AABB_draw(root->boundry);

  //recurse for all subdivs if not NULL
  if (root->NW != NULL){
    QuadTree_draw(root->NW);
  }
  if (root->NE != NULL){
    QuadTree_draw(root->NE);
  }
  if (root->SW != NULL){
    QuadTree_draw(root->SW);
  }
  if (root->SE != NULL){
    QuadTree_draw(root->SE);
  }
}

int draw_stuff(){
  SDL_Event myevent;
  if(SDL_Init(SDL_INIT_EVERYTHING)>=0){
    mywindow=SDL_CreateWindow("qtvis",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 640,480,SDL_WINDOW_SHOWN);
    if(mywindow!=0){
      myrenderer=SDL_CreateRenderer(mywindow,-1,0);
    }
  }
  else{
    return 1;
  }
  while(1){

    if(SDL_PollEvent(&myevent) && myevent.type == SDL_QUIT){
      break;
    }
    SDL_SetRenderDrawColor(myrenderer, 255, 255, 255, 255);
    SDL_RenderClear(myrenderer);



    SDL_RenderPresent(myrenderer);
  }
  SDL_Quit();
  return 0;
}

int main(int argc, char const *argv[]) {

  srand(time(0));

  Point *points[1000];
  AABB *big_box = AABB_new(Point_new(320,320),320);
  QuadTree *qtroot = QuadTree_new(big_box);


  //insert 1000 random points
  for(int i=0;i<1000;i++){
    if (i%2 == 0){
      points[i] = Point_new((float)randint(240,640),(float)randint(240,640));
    }
    else{
      points[i] = Point_new((float)randint(0,640),(float)randint(0,640));
    }
  }

  for(int i=0;i<1000;i++){
    Point_print(points[i]);
    QuadTree_insert(qtroot,points[i]);
  }








  //draw_stuff();
  SDL_Event myevent;
  if(SDL_Init(SDL_INIT_EVERYTHING)>=0){
    mywindow=SDL_CreateWindow("qtvis",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, 640,640,SDL_WINDOW_SHOWN);
    if(mywindow!=0){
      myrenderer=SDL_CreateRenderer(mywindow,-1,0);
    }
  }
  else{
    return 1;
  }
  while(1){

    if(SDL_PollEvent(&myevent) && myevent.type == SDL_QUIT){
      break;
    }
    SDL_SetRenderDrawColor(myrenderer, 255, 255, 255, 255);
    SDL_RenderClear(myrenderer);

    //draw here
    for(int i=0;i<1000;i++){
      Point_draw(points[i]);
    }

    QuadTree_draw(qtroot);


    SDL_RenderPresent(myrenderer);
  }
  SDL_Quit();
  return 0;
}
