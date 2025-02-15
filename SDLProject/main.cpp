/**
* Author: Isabelle Larson
* Assignment: Simple 2D Scene
* Date due: 2025-02-15, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"


enum AppStatus { RUNNING, TERMINATED };


constexpr int WINDOW_WIDTH  = 640 * 1.75,
              WINDOW_HEIGHT = 480 * 1.75;

constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr GLint NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL    = 0;
constexpr GLint TEXTURE_BORDER     = 0;

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr char BOB_SPRITE_FILEPATH[]    = "bob.png";
constexpr char JOAN_SPRITE_FILEPATH[]   = "joan.png";
constexpr char GUITAR_SPRITE_FILEPATH[] = "guitar.png";


constexpr glm::vec3 INIT_SCALE       = glm::vec3(1.66f, 1.99f, 0.0f),
                    INIT_POS_BOB    = glm::vec3(1.0f, 0.0f, 0.0f),
                    INIT_POS_JOAN = glm::vec3(-1.0f, 0.0f, 0.0f),
                    INIT_POS_GUITAR = glm::vec3(0.0f,0.0f,0.0f);


constexpr float ROT_INCREMENT = 1.0f;

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

glm::mat4 g_view_matrix, g_bob_matrix, g_projection_matrix, g_joan_matrix, g_guitar_matrix;

float g_previous_ticks = 0.0f;
float g_guitar_scale = 1.0f;
bool g_guitar_scale_up = true;

glm::vec3 g_rotation_bob = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_rotation_joan = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_translation_bob = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_translation_joan = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_rotation_guitar = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_scale_guitar = glm::vec3(0.0f,0.0f,0.0f);

GLuint g_bob_texture_id;
GLuint g_joan_texture_id;
GLuint g_guitar_texture_id;



void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

float g_bob_x = 0.0f;
float g_bob_y = 0.0f;
float g_joan_x = 0.0f;
float g_joan_y = 0.0f;

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Joan and Bob",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);


    if (g_display_window == nullptr) shutdown();

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_bob_matrix       = glm::mat4(1.0f);
    g_joan_matrix     = glm::mat4(1.0f);
    g_guitar_matrix = glm::mat4(1.0f);
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_bob_texture_id    = load_texture(BOB_SPRITE_FILEPATH);
    g_joan_texture_id = load_texture(JOAN_SPRITE_FILEPATH);
    g_guitar_texture_id = load_texture(GUITAR_SPRITE_FILEPATH);
 


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }
    
}

void update()
{
    // delta time calculation s
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    // game logic
    
    
    g_bob_x += 1.0f * delta_time;
    g_bob_y += 1.0f * delta_time;
    g_joan_x += 1.0f * delta_time;
    g_joan_y += 1.0f * delta_time;
    g_rotation_guitar.x += ROT_INCREMENT * delta_time;

    
    
    // model matrix reset
    
    g_bob_matrix = glm::mat4(1.0f);
    g_joan_matrix = glm::mat4(1.0f);
    g_guitar_matrix = glm::mat4(1.0f);
    
    if (g_guitar_scale_up) {
         g_guitar_scale += 0.5f * delta_time;
         if (g_guitar_scale >= 2.0f) {
             g_guitar_scale_up = false;
         }
     } else {
         g_guitar_scale -= 0.5f * delta_time;
         if (g_guitar_scale <= 0.5f) {
             g_guitar_scale_up = true;
         }
     }
    
    // transformations
    g_bob_matrix = glm::translate(g_bob_matrix, INIT_POS_BOB);
    g_bob_matrix = glm::scale(g_bob_matrix, INIT_SCALE);
    g_bob_matrix = glm::translate(g_joan_matrix, glm::vec3(cos(g_bob_x), sin(g_bob_y), 0.0f));
    
    g_joan_matrix = glm::translate(g_joan_matrix, INIT_POS_JOAN);
    g_joan_matrix = glm::scale(g_joan_matrix, INIT_SCALE);
    g_joan_matrix = glm::translate(g_joan_matrix, glm::vec3(cos(g_joan_x), sin(g_joan_y), 0.0f));
    
    g_guitar_matrix = glm::translate(g_guitar_matrix, INIT_POS_GUITAR);
    g_guitar_matrix = glm::scale(g_guitar_matrix, INIT_SCALE);
    g_guitar_matrix = glm::rotate(g_guitar_matrix, g_rotation_guitar.x, glm::vec3(0.0f, 1.0f, 0.0f));
    g_guitar_matrix = glm::scale(g_guitar_matrix, glm::vec3(g_guitar_scale));
    



    
}

void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so use 6, not 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_bob_matrix, g_bob_texture_id);
    draw_object(g_joan_matrix, g_joan_texture_id);
    draw_object(g_guitar_matrix, g_guitar_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}

