#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_Image.h>
#include <SDL/SDL_ttf.h>
#include <chipmunk/chipmunk.h>


#define SLEEP_TICKS 16
#define XSIZE 1200
#define YSIZE 1000
#define XMID XSIZE/2
#define YMID YSIZE/2


int ticks = 0;
int some_value = 42;
cpSpace *space;
cpBody *lander;

SDL_Surface *screen;	//This pointer will reference the backbuffer
SDL_Surface *lander_image; //This pointer will reference our bitmap sprite
SDL_Surface *temp;	    //This pointer will temporarily reference our bitmap sprite
SDL_Rect src, dest;	    //These rectangles will describe the source and destination regions of our blit

int running=0;         //Is the game running? 

/* return a pseudo-random number between 0 and limit inclusive.
*/
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
	/* Load the image using SDL Image */
	SDL_Surface *temp = IMG_Load(name);
	SDL_Surface *image;
	if (temp == NULL)
	{
		printf("Failed to load image %s\n", name);
		return NULL;
	}	
	/* Make the background transparent */
	SDL_SetColorKey(temp, (SDL_SRCCOLORKEY|SDL_RLEACCEL), SDL_MapRGB(temp->format, 0, 0, 0));	
	/* Convert the image to the screen's native format */
	image = SDL_DisplayFormat(temp);	
	SDL_FreeSurface(temp);	
	if (image == NULL)
	{
		printf("Failed to convert image %s to native format\n", name);
		return NULL;
	}	
	/* Return the processed image */
	return image;
}

void update(int ticks)
{
    int i;
	int steps = 2;
	cpFloat dt = 1.0/60.0/(cpFloat)steps;
	for(i=0; i<steps; i++){
		cpSpaceStep(space, dt);
	}
}

static void postStepRemove(cpSpace *space,cpShape *shape,void *unused) {
       cpSpaceRemoveBody(space, shape->body);
       cpBodyFree(shape->body);
       cpSpaceRemoveShape(space, shape);
       cpShapeFree(shape);
}

static int begin(cpArbiter *arb, cpSpace *space,void *ignore) {
       cpShape *a,*b;
       cpArbiterGetShapes(arb, &a, &b);
       cpSpaceAddPostStepCallback(space,(cpPostStepFunc)postStepRemove,b,NULL);
       return 0;
}

static int collFunc(cpShape *a, cpShape *b, cpContact *contacts, int numContacts, cpFloat normal_coef, void *data)
{
	int *some_ptr = (int *)data;

// Do various things with the contact information. 
// Make particle effects, estimate the impact damage from the relative velocities, etc.
//	for(int i=0; i<numContacts; i++)
//		printf("Collision at %s. (%d - %d) %d\n", cpvstr(contacts[i].p), a->collision_type, b->collision_type, *some_ptr);
	
// Returning 0 will cause the collision to be discarded. This allows you to do conditional collisions.
	return 1;
}

void init(void)
{
    int i;
    unsigned int iseed = (unsigned int)time(NULL);
  
    cpFloat radius = 10;
	cpFloat lander_mass = 0.2;
	cpFloat x,y;
	cpVect gravity = cpv(0, 0.25);
	// Optional. Read the docs to see what this really does.
	cpResetShapeIdCounter();
	
	// Create a space and adjust some of it's parameters.
	space = cpSpaceNew();
	cpSpaceSetGravity(space, gravity);
	
    // Create a shape pointer 
	cpShape *shape;

	
	srand(iseed);
	
	// Create a lander

      lander = cpBodyNew(lander_mass, cpMomentForCircle(lander_mass, 0.0, radius, cpvzero));
        x=XMID-200+(cpFloat)randomize(200);
        y=600;
        
        lander->p = cpv(x,y);
 
        cpSpaceAddBody(space, lander);
        shape = cpCircleShapeNew(lander, radius, cpvzero);
        shape->e = 0.0; shape->u = 2.5;
        shape->data=(cpDataPointer)0;
        //shape->collision_type = 1;
        cpSpaceAddShape(space, shape);
    // Add a collision callback (begin).
	cpSpaceAddCollisionHandler(space,1,0,begin,NULL,NULL,NULL,NULL);
	
}

void thrust()
{
     cpVect main_rocket = cpv(0, -0.01);
     cpVect center_of_gravity=cpv(0,0);
     cpBodyApplyImpulse(lander, main_rocket,center_of_gravity);
}

void destroy(void)
{
	cpSpaceFree(space);	
}
static void renderLander(cpFloat x, cpFloat y, cpFloat r, cpFloat a,cpShape *shape)
{
       SDL_Rect dest;
       /* Set the blitting rectangle to the size of the src image */
       dest.x = x;
       dest.y = y;
       dest.w = lander_image->w;
       dest.h = lander_image->h;
       /* Blit the entire image onto the screen at coordinates x and y */
       SDL_BlitSurface(lander_image, NULL, screen, &dest);
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
SDL_Event  event; // SDL events

running  = 1;

//Initialise the physics engine
cpInitChipmunk();

//Initialise the stuff we need for marble physics
init();

//We must first initialize the SDL video component, and check for success
if (SDL_Init(SDL_INIT_VIDEO) != 0) {
	printf("Unable to initialize SDL: %s\n", SDL_GetError());
	return 1;
}
//When this program exits, SDL_Quit must be called
atexit(SDL_Quit);

//Set the video mode to 640x480 with 16bit colour and double-buffering
screen = SDL_SetVideoMode(XSIZE, YSIZE, 16, SDL_DOUBLEBUF);
if (screen == NULL) {
	printf("Unable to set video mode: %s\n", SDL_GetError());
	return 1;
}

//Set up printing
if (TTF_Init() != 0) {
  printf("Unable to initialize SDL_ttf: %s \n", TTF_GetError());
  return 1;
}
TTF_Font *fntCourier = TTF_OpenFont( "cour.ttf",24);
SDL_Color clrFg = {255,255,255,0};  // White ("Fg" is foreground)
SDL_Surface *textSurface = TTF_RenderText_Solid(fntCourier, "Lunar Lander Example", clrFg);
SDL_Rect textDest = {10,0,0};


//Load a particle image
temp =loadImage("space_ship.jpg");
//Convert the surface to the appropriate display format
lander_image = SDL_DisplayFormatAlpha(temp);

//Release the temporary surface
SDL_FreeSurface(temp);

while (1) {
    while(SDL_PollEvent(&event)){
    if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
        running = 0;
    } 
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE){
        thrust();           
    }
    
    
    // while (handling events)
    if (running == 0)
      break;
      
    //Blank the background
    SDL_FillRect(screen, NULL, 0);
         
    //Draw the particles
    cpSpaceEachShape(space,(cpSpaceShapeIteratorFunc)drawLander,NULL);
    //Draw the text
    SDL_BlitSurface(textSurface,NULL, screen,&textDest);
    
    //Double buffer
    SDL_Flip(screen);
    ticks++;
   update(ticks);
  } // while (main loop)
//Release the surfaces
SDL_FreeSurface(textSurface);
SDL_FreeSurface(lander_image);
TTF_CloseFont(fntCourier);
//Return success!
SDL_Quit();
destroy();
return 0;
}
