#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_Image.h>
#include <SDL/SDL_ttf.h>
#include <chipmunk/chipmunk.h>

#define SLEEP_TICKS 16
#define XSIZE 480  
#define YSIZE 1000
#define XMID XSIZE/2
#define YMID YSIZE/2

int ticks = 0;
int some_value = 42;
int backgroundX = 0;
int backgroundY = 0;
cpSpace *space;
cpBody *lander;
cpBody *missile;
cpBody *missile2;
cpBody *explosion;
SDL_Surface *screen;	     
SDL_Surface *missile_image;
SDL_Surface *explosion_image;
SDL_Surface *lander_image;  
SDL_Surface *temp;	       
SDL_Surface *background;   
SDL_Rect src, dest;	      
int running=0;         
int randomize(int limit) {
int divisor = RAND_MAX/(limit+1);
int rand_val;
do {
rand_val = rand() / divisor;
} while (rand_val > limit);
return rand_val;
}

SDL_Surface *loadImage(char *name)
{
	SDL_Surface *temp = IMG_Load(name);
	SDL_Surface *image;
	SDL_Surface *background = IMG_Load(name);
	if (temp == NULL && background == NULL)
	{
		printf("Cant load image %s\n", name);
		return NULL;
	}	
	SDL_SetColorKey(temp, (SDL_SRCCOLORKEY|SDL_RLEACCEL), SDL_MapRGB(temp->format, 0, 0, 0));	
	image = SDL_DisplayFormat(temp);	
	SDL_FreeSurface(temp);	
	if (image == NULL)
	{
		printf("Failed to convert image %s to native format\n", name);
		return NULL;
	}	
	return image;
}

void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination)
{
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    SDL_BlitSurface(source,NULL,destination,&offset );
}

void update(int ticks)
{
    int i;
	int steps = 2;
	cpFloat dt = 1.0/60.0/(cpFloat)steps;
	for(i=0; i<steps; i++)
    {
		cpSpaceStep(space, dt);
	}
}

static void postStepRemove(cpSpace *space,cpShape *shape,void *unused) 
{
       cpSpaceRemoveBody(space, shape->body);
       cpBodyFree(shape->body);
       cpSpaceRemoveShape(space, shape);
       cpShapeFree(shape);
}

static int begin(cpArbiter *arb, cpSpace *space,void *ignore) 
{
       cpShape *a,*b;
       cpArbiterGetShapes(arb, &a, &b);
       cpSpaceAddPostStepCallback(space,(cpPostStepFunc)postStepRemove,b,NULL);
       return 0;
}

static int collFunc(cpShape *a, cpShape *b, cpContact *contacts, int numContacts, cpFloat normal_coef, void *data)
{
	int *some_ptr = (int *)data;
	return 1;
}

void init(void)
{
    int i;
    unsigned int iseed = (unsigned int)time(NULL);
  
    cpFloat radius = 10;
	cpFloat lander_mass = 0.2;
	cpFloat missile_mass = 0.2;
	cpFloat missile_radius = 10;
	cpFloat missile_mass = 0.4;
	cpFloat missile_radius = 20;
	cpFloat x,y;
	cpVect gravity = cpv(0, 0.25);
	cpResetShapeIdCounter();
	space = cpSpaceNew();
	cpSpaceSetGravity(space, gravity);
	cpShape *shape;
	srand(iseed);
	
    lander = cpBodyNew(lander_mass, cpMomentForCircle(lander_mass, 0.0, radius, cpvzero));
    x=XMID-200+(cpFloat)randomize(200);
    y=900;   
    lander->p = cpv(x,y);
    cpSpaceAddBody(space, lander);
    shape = cpCircleShapeNew(lander, radius, cpvzero);
    shape->e = 0.0; shape->u = 2.5;
    shape->data=(cpDataPointer)0;
    shape->collision_type = 1;
    cpSpaceAddShape(space, shape);
	cpSpaceAddCollisionHandler(space,1,0,begin,NULL,NULL,NULL,NULL);

	missile = cpBodyNew(missile_mass, cpMomentForCircle(missile_mass, 0.0, missile_radius, cpvzero));
    missile->p = cpv(100,300);
    cpSpaceAddBody(space, missile);
    shape = cpCircleShapeNew(missile, missile_radius, cpvzero);
    shape->data=(cpDataPointer)2;
    shape->e = 0.0; shape->u = 0.0;
    shape->collision_type = 1;
    cpSpaceAddShape(space, shape);

	missile2 = cpBodyNew(missile2_mass, cpMomentForCircle(missile2_mass, 0.0, missile2_radius, cpvzero));
    missile2->p = cpv(100,300);
    cpSpaceAddBody(space, missile2);
    shape = cpCircleShapeNew(missile2, missile2_radius, cpvzero);
    shape->data=(cpDataPointer)2;
    shape->e = 0.0; shape->u = 0.0;
    shape->collision_type = 1;
    cpSpaceAddShape(space, shape);
    
    explosion = cpBodyNew(missile_mass, cpMomentForCircle(missile_mass, 0.0, missile_radius, cpvzero));
    explosion->p = cpv(120,300);
    cpSpaceAddBody(space, explosion);
    shape = cpCircleShapeNew(explosion, missile_radius, cpvzero);
    shape->data=(cpDataPointer)3;
    shape->e = 0.0; shape->u = 0.0;
    shape->collision_type = 1;
    cpSpaceAddShape(space, shape);
}

static void drawShapes(cpShape *shape)
{
	cpBody *body = shape->body;
	cpCircleShape *circle = (cpCircleShape *)shape;
	cpVect c = cpvadd(body->p, cpvrotate(circle->c, body->rot));
	render(c.x, c.y, circle->r, body->a,shape);
}

static void render(cpFloat x, cpFloat y, cpFloat r, cpFloat a,cpShape *shape)
{
       SDL_Rect dest;
       int i;
       if(shape->data==(cpDataPointer)2){
             dest.x = x;
             dest.y = y;
             dest.w = missile_image->w;
             dest.h = missile_image->h;
             SDL_BlitSurface(cannonball_image, NULL, screen, &dest);
       }
       
       if(hit>0){
             
             for(i=0;i<=5;i++){
             dest.x = hit;
             dest.y = 420-i*5;
             dest.w = explosion_image[i]->w;
             dest.h = explosion_image[i]->h;
             SDL_BlitSurface(explosion_image[i], NULL, screen, &dest);
           }
       }      
}
void moveUp()
{
     cpVect main_rocket = cpv(0, -0.1);
     cpVect center_of_gravity=cpv(0,0);
     cpBodyApplyImpulse(lander, main_rocket,center_of_gravity);
}
void moveLeft()
{
    cpVect main_rocket = cpv(-0.1,0);
     cpVect center_of_gravity = cpv (0,0);
     cpBodyApplyImpulse(lander, main_rocket, center_of_gravity);
     }
void moveRight()
{
     cpVect main_rocket = cpv(0.1,0);
     cpVect center_of_gravity = cpv (0,0);
     cpBodyApplyImpulse(lander, main_rocket, center_of_gravity);
     }
void moveDown()
{
   cpVect main_rocket = cpv(0,0.1);
     cpVect center_of_gravity = cpv (0,0);
     cpBodyApplyImpulse(lander, main_rocket, center_of_gravity);
     }
void fire()
{
     missile->p = lander->p;   
     missile->v =(100,100);  
	 cpSpaceAddCollisionHandler(space,1,0,begin,NULL,NULL,NULL,NULL);
}
void destroy(void)
{
	cpSpaceFree(space);	
}
static void drawLanderShape(cpShape *shape)
{
	cpBody *body = shape->body;
	cpCircleShape *circle = (cpCircleShape *)shape;
	cpVect c = cpvadd(body->p, cpvrotate(circle->c, body->rot));
	renderLander(c.x, c.y, circle->r, body->a,shape);
}
static void drawLander(void *ptr, void *unused)
{
	cpShape *shape = (cpShape *)ptr;
	drawLanderShape(shape);
}
int main(int argc, char* argv[])
{
int i;
cpBody *body;
cpShape *shape;
SDL_Event  event;
running  = 1;
cpInitChipmunk();
init();
if (SDL_Init(SDL_INIT_VIDEO) != 0) {
	printf("Unable to initialize SDL: %s\n", SDL_GetError());
	return 1;
}

atexit(SDL_Quit);

screen = SDL_SetVideoMode(XSIZE, YSIZE, 16, SDL_DOUBLEBUF);
if (screen == NULL) {
	printf("Unable to set video mode: %s\n", SDL_GetError());
	return 1;
}

if (TTF_Init() != 0) {
  printf("Unable to initialize SDL_ttf: %s \n", TTF_GetError());
  return 1;
}
TTF_Font *fntCourier = TTF_OpenFont( "cour.ttf",24);
SDL_Color clrFg = {255,255,255,0};
SDL_Surface *textSurface = TTF_RenderText_Solid(fntCourier, "R-type prototype", clrFg);
SDL_Rect textDest = {10,0,0};

temp = loadImage("cannon/explosion_0.bmp");
explosion_image = SDL_DisplayFormatAlpha(temp);
temp =loadImage("space_ship.jpg");
background = loadImage("background.jpg");
lander_image = SDL_DisplayFormatAlpha(temp);
temp=loadImage("cannon/cannonball.bmp");
missile_image = SDL_DisplayFormatAlpha(temp);
SDL_FreeSurface(temp);
SDL_FreeSurface(temp);

while (1) {
    while(SDL_PollEvent(&event)){
    if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
        running = 0;
    } 
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP)
    {
        moveUp();           
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT) 
    {
        moveLeft();
    }   
     if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT) 
        {
        moveRight();
        } 
     if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN)
           {
        moveDown();
        }
     if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
           {
        fire();
    
        }
     backgroundX -= 2;
     if(backgroundX <= -background->w)
        {
          backgroundX = 0;
        }
        apply_surface(backgroundX,backgroundY,background,screen);
        apply_surface(backgroundY+background->w,backgroundY,background,screen);
    if (running == 0)
      break;
    SDL_FillRect(screen, NULL, 0);
    cpSpaceEachShape(space,(cpSpaceShapeIteratorFunc)drawLander,NULL);
    SDL_BlitSurface(background,NULL, screen,NULL);
    SDL_BlitSurface(textSurface,NULL, screen,&textDest);
    SDL_Flip(screen);
    ticks++;
   update(ticks);
  } 
SDL_FreeSurface(textSurface);
SDL_FreeSurface(lander_image);
TTF_CloseFont(fntCourier);
SDL_Quit();
destroy();
return 0;
}
