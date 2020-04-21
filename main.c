#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

int main(int argc, char *args[])
{
SDL_Surface* background = NULL;

SDL_Init (SDL_INIT_EVERYTHING);
screen = SDL_SetVideoMode(640,480,32,SDL_SWSURFACE);
background = SDL_LoadIMG ("background.jpg");

SDL_BlitSurface(background,NULL, screen, NULL);
SDL_Flip(screen);
SDL_Delay(2000);

SDL_FreeSurface(background);
SDL_Quit();
return 0
}
