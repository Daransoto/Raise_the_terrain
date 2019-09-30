#include <SDL.h>
#include <stdio.h>
#include <math.h>
double init_terrain(double terrain[8][8][2], double elevation[8][8],
	FILE * elevs, SDL_DisplayMode Display);
void draw(double terrain[8][8][2], double elevation[8][8], double inclination,
	double max, SDL_Renderer *renderer, SDL_DisplayMode Display, double basey);
void rotate(SDL_Event event, double terrain[8][8][2], double basex,
	double basey);
void error_message(const char *message);
/**
* main - Entry point.
* @argc: Argument counter.
* @argv: Argument vector.
* Return: 0 on success, 1 otherwise.
*/
int main(int argc, char *argv[])
{
	int width, height, i, j;
	double elevation[8][8], terrain[8][8][2] = { 0 }, basex, basey, x, y;
	double inclination = 0.7, max_elev = 0;
	FILE *elevs;
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_bool done = SDL_FALSE;
	SDL_DisplayMode Display;
	SDL_Event event;

	if (argc != 2)
		error_message("Usage: terrain.exe file");
	elevs = fopen(argv[1], "r");
	if (!elevs)
		error_message("Failed opening the file.");
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		error_message(SDL_GetError());
	if (SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer) < 0)
		error_message(SDL_GetError());
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	SDL_GetCurrentDisplayMode(0, &Display);
	max_elev = init_terrain(terrain, elevation, elevs, Display);
	fclose(elevs);
	basex = (terrain[0][3][0] + terrain[0][4][0]) / 2;
	basey = (terrain[3][0][1] + terrain[4][0][1]) / 2;
	while (!done)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
		draw(terrain, elevation, inclination, max_elev, renderer, Display, basey);
		SDL_RenderPresent(renderer);
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
				done = SDL_TRUE;
			else
				rotate(event, terrain, basex, basey);
		}
	}
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);
	SDL_Quit();
	return (0);

}
/**
* init_terrain - Initializes the terrain and elevations, optimizing screen.
* @terrain: Basic terrain.
* @elevation: Matrix that will save elevations in the input file.
* @elevs: File read.
* @Display: Matrix that will be displayed (isometric).
* Return: Max elevation in file.
*/
double init_terrain(double terrain[8][8][2], double elevation[8][8],
	FILE *elevs, SDL_DisplayMode Display)
{
	int i, j, width, height;
	double max_elev = 0, inclination = 0.7, sc_longest, sc_shortest, ratio;
	double tile_width, tile_long;

	width = Display.w;
	height = Display.h;
	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
		{
			fscanf(elevs, "%lf", &elevation[i][j]);
			if (max_elev < fabs(elevation[i][j]))
				max_elev = fabs(elevation[i][j]);
		}
	if (height <= width)
	{
		sc_longest = width;
		sc_shortest = height;
	}
	else
	{
		sc_longest = height;
		sc_shortest = width;
	}
	ratio = sc_longest / sc_shortest;
	if (inclination >= 0.5)
		tile_long = 2 * inclination / ratio;
	else
		tile_long = 2 * (1 - inclination) / ratio;
	tile_width = sc_shortest / (7 * tile_long);
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			terrain[i][j][0] = tile_width * j;
			terrain[i][j][1] = tile_width * i;
		}
	}
	return (max_elev);
}
/**
* draw - Draws the terrain on screen.
* @terrain: Matrix without isometric transform.
* @elevation: Matrix of elevations of terrain.
* @inclination: Inclination of the isometric visualization.
* @max: Maximum elevation on file.
* @renderer: Renderer for the window.
* @Display: Display mode of program.
* @basey: Calculation of vertical base of rotation of terrain matrix.
*/
void draw(double terrain[8][8][2], double elevation[8][8], double inclination,
	double max, SDL_Renderer *renderer, SDL_DisplayMode Display, double basey)
{
	double width = Display.w, x, display[8][8][2];
	double height = Display.h, y, z, center;
	int i, j;

	max = (max + height - 4 * (1 - inclination) * basey) / 2;
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			x = terrain[i][j][0];
			y = terrain[i][j][1];
			z = elevation[i][j];
			display[i][j][0] = inclination * x - inclination * y + width / 2;
			display[i][j][1] = (1 - inclination) * x + (1 - inclination) * y - z + max;
		}
	}
	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
		{
			if (j < 7)
			{
				SDL_RenderDrawLine(renderer,
					(int)round(display[i][j][0]), (int)round(display[i][j][1]),
					(int)round(display[i][j + 1][0]), (int)round(display[i][j + 1][1]));
			}
			if (i < 7)
			{
				SDL_RenderDrawLine(renderer,
					(int)round(display[i][j][0]), (int)round(display[i][j][1]),
					(int)round(display[i + 1][j][0]), (int)round(display[i + 1][j][1]));
			}
		}
}
/**
* rotate - Rotates the terrain.
* @event: Event on the program (key pressed, etc).
* @terrain: Terrain matrix.
* @basex: Calculation of horizontal base of rotation of terrain matrix.
* @basey: Calculation of vertical base of rotation of terrain matrix.
*/
void rotate(SDL_Event event, double terrain[8][8][2], double basex,
	double basey)
{
	int i, j;
	double x, y;

	if (event.key.keysym.sym == SDLK_RIGHT)
	{
		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < 8; j++)
			{
				x = terrain[i][j][0], y = terrain[i][j][1];
				terrain[i][j][0] = (x - basex) * cos(M_PI / 180) -
					(y - basey) * sin(M_PI / 180) + basex;
				terrain[i][j][1] = (x - basex) * sin(M_PI / 180) +
					(y - basey) * cos(M_PI / 180) + basey;
			}
		}
	}
	else if (event.key.keysym.sym == SDLK_LEFT)
	{
		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < 8; j++)
			{
				x = terrain[i][j][0], y = terrain[i][j][1];
				terrain[i][j][0] = (x - basex) * cos(-M_PI / 180) -
					(y - basey) * sin(-M_PI / 180) + basex;
				terrain[i][j][1] = (x - basex) * sin(-M_PI / 180) +
					(y - basey) * cos(-M_PI / 180) + basey;
			}
		}
	}
}
/**
* error_message - Function to handle error messages.
* @message: Message to display on error.
*/
void error_message(const char *message)
{
	fprintf(stderr, "%s\n", message);
	exit(1);
}