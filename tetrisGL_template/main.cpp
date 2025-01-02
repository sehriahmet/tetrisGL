//
// Author: Ahmet Oguz Akyuz
// 
// This is a sample code that draws a single block piece at the center
// of the window. It does many boilerplate work for you -- but no
// guarantees are given about the optimality and bug-freeness of the
// code. You can use it to get started or you can simply ignore its
// presence. You can modify it in any way that you like.
//
//


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <ft2build.h>
#include FT_FREETYPE_H

#include <deque>


#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;
void checkAndClearRows();
void spawnNewBlock();
float cameraAngle = 0.0f; // Current camera rotation angle in degrees
float targetCameraAngle = 0.0f; // Target angle for smooth camera rotation

GLuint gProgram[3];
int gWidth = 600, gHeight = 1000;
GLuint gVertexAttribBuffer, gTextVBO, gIndexBuffer;
GLuint gTex2D;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;
int gTriangleIndexDataSizeInBytes, gLineIndexDataSizeInBytes;

GLint modelingMatrixLoc[2];
GLint viewingMatrixLoc[2];
GLint projectionMatrixLoc[2];
GLint eyePosLoc[2];
GLint lightPosLoc[2];
GLint kdLoc[2];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix = glm::translate(glm::mat4(1.f), glm::vec3(-0.5, -0.5, -0.5));
// glm::mat4 modelingMatrix = glm::translate(glm::mat4(1.f), glm::vec3(-100, -100, -0.5));
glm::vec3 eyePos = glm::vec3(0, 9, 32);
glm::vec3 lightPos = glm::vec3(0, 9, 32);


glm::vec3 kdGround(0.334, 0.288, 0.635); // this is the ground color in the demo
glm::vec3 kdCubes(0.86, 0.11, 0.31);

bool isRotating = false; 
float currentRotationTime = 0.0f; 
const float rotationDuration = 0.3f; // bura hizlandirilabilir. 


int activeProgramIndex = 0;

// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

// For reading GLSL files
bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

GLuint createVS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

	return fs;
}

void initFonts(int windowWidth, int windowHeight)
{
    // Set OpenGL options
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[2]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 0, &face))
    //if (FT_New_Face(ft, "/usr/share/fonts/truetype/gentium-basic/GenBkBasR.ttf", 0, &face)) // you can use different fonts
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (GLuint) face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //
    // Configure VBO for texture quads
    //
    glGenBuffers(1, &gTextVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initShaders()
{
	// Create the programs

    gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();
	gProgram[2] = glCreateProgram();

	// Create the shaders for both programs

    GLuint vs1 = createVS("vert.glsl"); // for cube shading
    GLuint fs1 = createFS("frag.glsl");

	GLuint vs2 = createVS("vert2.glsl"); // for border shading
	GLuint fs2 = createFS("frag2.glsl");

	GLuint vs3 = createVS("vert_text.glsl");  // for text shading
	GLuint fs3 = createFS("frag_text.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	glAttachShader(gProgram[2], vs3);
	glAttachShader(gProgram[2], fs3);

	// Link the programs

    for (int i = 0; i < 3; ++i)
    {
        glLinkProgram(gProgram[i]);
        GLint status;
        glGetProgramiv(gProgram[i], GL_LINK_STATUS, &status);

        if (status != GL_TRUE)
        {
            cout << "Program link failed: " << i << endl;
            exit(-1);
        }
    }


	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 2; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
		lightPosLoc[i] = glGetUniformLocation(gProgram[i], "lightPos");
		kdLoc[i] = glGetUniformLocation(gProgram[i], "kd");

        glUseProgram(gProgram[i]);
        glUniformMatrix4fv(modelingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
        glUniform3fv(eyePosLoc[i], 1, glm::value_ptr(eyePos));
        glUniform3fv(lightPosLoc[i], 1, glm::value_ptr(lightPos));
        glUniform3fv(kdLoc[i], 1, glm::value_ptr(kdGround));
	}
}

// VBO setup for drawing a cube and its borders
void initVBO()
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &gVertexAttribBuffer);
	glGenBuffers(1, &gIndexBuffer);

	assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    GLuint indices[] = {
        0, 1, 2, // front
        3, 0, 2, // front
        4, 7, 6, // back
        5, 4, 6, // back
        0, 3, 4, // left
        3, 7, 4, // left
        2, 1, 5, // right
        6, 2, 5, // right
        3, 2, 7, // top
        2, 6, 7, // top
        0, 4, 1, // bottom
        4, 5, 1,  // bottom
        /* 0+8, 1+8, 2+8, // front
        3+8, 0+8, 2+8, // front
        4+8, 7+8, 6+8, // back
        5+8, 4+8, 6+8, // back
        0+8, 3+8, 4+8, // left
        3+8, 7+8, 4+8, // left
        2+8, 1+8, 5+8, // right
        6+8, 2+8, 5+8, // right
        3+8, 2+8, 7+8, // top
        2+8, 6+8, 7+8, // top
        0+8, 4+8, 1+8, // bottom
        4+8, 5+8, 1+8, */

    };

    GLuint indicesLines[] = {
        7, 3, 2, 6, // top
        4, 5, 1, 0, // bottom
        2, 1, 5, 6, // right
        5, 4, 7, 6, // back
        0, 1, 2, 3, // front
        0, 3, 7, 4, // left
        /* 15, 11, 10, 14,
        12, 13, 9, 8,
        10, 9, 13, 14,
        13, 12, 15, 14,
        8, 9, 10, 11,
        8, 11, 15, 12, */
    };

    GLfloat vertexPos[] = {
        0, 0, 1, // 0: bottom-left-front
        1, 0, 1, // 1: bottom-right-front
        1, 1, 1, // 2: top-right-front
        0, 1, 1, // 3: top-left-front
        0, 0, 0, // 0: bottom-left-back
        1, 0, 0, // 1: bottom-right-back
        1, 1, 0, // 2: top-right-back
        0, 1, 0, // 3: top-left-back
        /* 1, 0, 1,
        2, 0, 1,
        2, 1, 1,
        1, 1, 1,
        1, 0, 0,
        2, 0, 0,
        2, 1, 0,
        1, 1, 0, */

    };

    GLfloat vertexNor[] = {
         1.0,  1.0,  1.0, // 0: unused
         0.0, -1.0,  0.0, // 1: bottom
         0.0,  0.0,  1.0, // 2: front
         1.0,  1.0,  1.0, // 3: unused
        -1.0,  0.0,  0.0, // 4: left
         1.0,  0.0,  0.0, // 5: right
         0.0,  0.0, -1.0, // 6: back 
         0.0,  1.0,  0.0, // 7: top
        /* 1.0,  1.0,  1.0, // 0: unused
         0.0, -1.0,  0.0, // 1: bottom
         0.0,  0.0,  1.0, // 2: front
         1.0,  1.0,  1.0, // 3: unused
        -1.0,  0.0,  0.0, // 4: left
         1.0,  0.0,  0.0, // 5: right
         0.0,  0.0, -1.0, // 6: back 
         0.0,  1.0,  0.0, // 7: top */
    };

	gVertexDataSizeInBytes = sizeof(vertexPos);
	gNormalDataSizeInBytes = sizeof(vertexNor);
    gTriangleIndexDataSizeInBytes = sizeof(indices);
    gLineIndexDataSizeInBytes = sizeof(indicesLines);
    int allIndexSize = gTriangleIndexDataSizeInBytes + gLineIndexDataSizeInBytes;

	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexPos);
	glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, vertexNor);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndexSize, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, gTriangleIndexDataSizeInBytes, indices);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, gTriangleIndexDataSizeInBytes, gLineIndexDataSizeInBytes, indicesLines);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
}

void init() 
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // polygon offset is used to prevent z-fighting between the cube and its borders
    glPolygonOffset(0.5, 0.5);
    glEnable(GL_POLYGON_OFFSET_FILL);

    initShaders();
    initVBO();
    initFonts(gWidth, gHeight);
}

void drawCube(const glm::mat4& modelMatrix, glm::vec3 colorOfTheCube) {
    glUseProgram(gProgram[0]); 

    glUniformMatrix4fv(modelingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniform3fv(kdLoc[0], 1, glm::value_ptr(colorOfTheCube));

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}


void drawCubeEdges(const glm::mat4& modelMatrix) {
    glUseProgram(gProgram[1]); // Use the edge-drawing shader program

    glUniformMatrix4fv(modelingMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Set the color and line width
    glLineWidth(3);

    // Draw edges for each face of the cube
    for (int i = 0; i < 6; ++i) {
        glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, BUFFER_OFFSET(gTriangleIndexDataSizeInBytes + i * 4 * sizeof(GLuint)));
    }
}

void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    glUseProgram(gProgram[2]);
    glUniform3f(glGetUniformLocation(gProgram[2], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}


void reshape(GLFWwindow* window, int w, int h) {
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);

    // Use perspective projection
    float fovyRad = (float)(45.0 / 180.0) * M_PI;
    projectionMatrix = glm::perspective(fovyRad, gWidth / (float)gHeight, 1.0f, 100.0f);

    // std::cout<<eyePos.x<<std::endl;
    if (targetCameraAngle == 0) { // && eyePos == glm::vec3(4.5f,8.0f,35.0f)
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(targetCameraAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 rotatedEyePos = glm::vec3(rotationMatrix * glm::vec4(eyePos, 1.0f));
        viewingMatrix = glm::lookAt(rotatedEyePos, glm::vec3(0, 4.5, 0), glm::vec3(0, 1, 0));
    }

    for (int i = 0; i < 2; ++i) {
        glUseProgram(gProgram[i]);
        glUniformMatrix4fv(projectionMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        glUniformMatrix4fv(viewingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    }
}

void rotateEyePosition(float deltaTime) {
    if (!isRotating) return;

    currentRotationTime += deltaTime;

    float t = glm::clamp(currentRotationTime / rotationDuration, 0.0f, 1.0f);

    // float currentAngle = glm::mix(cameraAngle, targetCameraAngle, t);

    float currentAngle = 0 + (t * targetCameraAngle);

    // currentAngle +=cameraAngle;

    // std::cout<<  " cameraAngle:  " <<cameraAngle << " targetCameraAngle:  " << targetCameraAngle <<" currentangle: " <<currentAngle << std::endl;

    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(currentAngle), glm::vec3(0, 1.0f, 0));
    glm::vec3 rotatedEyePos = glm::vec3(rotationMatrix * glm::vec4(eyePos, 1.0f));
    viewingMatrix = glm::lookAt(rotatedEyePos, glm::vec3(0, 4.5, 0), glm::vec3(0, 1, 0));
    

    for (int i = 0; i < 2; ++i) {
        glUseProgram(gProgram[i]);
        lightPosLoc[i] = glGetUniformLocation(gProgram[i], "lightPos");
        glUniform3fv(lightPosLoc[i], 1, glm::value_ptr(rotatedEyePos));
        glUniformMatrix4fv(viewingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    }

    if (t >= 1.0f) {
        isRotating = false;
        currentRotationTime = 0.0f; 
        eyePos = rotatedEyePos;
        // startCameraAngle = targetCameraAngle; // Set new start angle

        if (cameraAngle == -90 || cameraAngle == 270 && targetCameraAngle == -90) { cameraAngle = 180.0f; }
        else if (cameraAngle == 0 && targetCameraAngle == -90) cameraAngle = 270.0f;
        else if (cameraAngle == 90 && targetCameraAngle == -90) cameraAngle = 0.0f;
        else if (cameraAngle == 180 && targetCameraAngle == -90) cameraAngle = 90.0f;
        

        else if (cameraAngle == -90 || cameraAngle == 270 && targetCameraAngle == 90) { cameraAngle = 0.0f; }
        else if (cameraAngle == 0 && targetCameraAngle == 90) cameraAngle = 90.0f;
        else if (cameraAngle == 90 && targetCameraAngle == 90) cameraAngle = 180.0f;
        else if (cameraAngle == 180 && targetCameraAngle == 90) cameraAngle = 270.0f;
        targetCameraAngle =0;
        // std::cout<<"after camera angle: "<<cameraAngle<<"  targetcameraangle:  "<<targetCameraAngle<<"\n"<<std::endl;
        // std::cout<<"eyepos.x:  "<<eyePos.x<<"  eyePos.z:  "<<eyePos.z<<"\n"<<std::endl;
    }
}

const int GRID_SIZE = 9;

std::deque<std::vector<std::vector<int>>> background(19, std::vector<std::vector<int>>(GRID_SIZE, std::vector<int>(GRID_SIZE, 0)));
glm::vec3 activeBlockPosition(3, 11 , 3); 
int score = 0;
bool gameOver = false;
float fallSpeed = 0.5f; 
float lastFallTime = 0.0f;


// bu fonksiyon kup dustukten sonra yerinde ciziyo 
void drawBlocks() {
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < 19; y++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                if (background[y][x][z]) {
                    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x-4.5, y, z-4.5));
                    // std::cout << "Drawing block at: (" << x << ", " << y << ", " << z << ")\n";
                    glUniformMatrix4fv(modelingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(modelMatrix));
                    drawCubeEdges(modelMatrix);
                    drawCube(modelMatrix, kdCubes);
                }
            }
        }
    }
}

void moveBlock(int dx, int dz) {
    glm::vec3 newPosition = activeBlockPosition + glm::vec3(dx, 0, dz);

    if (newPosition.x < 0 || newPosition.x + 2 >= GRID_SIZE ||
        newPosition.z < 0 || newPosition.z + 2 >= GRID_SIZE) {
        return; 
    }

    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                if (background[(int)newPosition.y + y][(int)newPosition.x + x][(int)newPosition.z + z] == 1) {
                    return; 
                }
            }
        }
    }

    activeBlockPosition = newPosition;
}

bool gameCont = false; 

void updateBlockFall(float currentTime) {
    if (!gameCont) return; 

    if (currentTime - lastFallTime > fallSpeed) {
        glm::vec3 newPosition = activeBlockPosition + glm::vec3(0, -1, 0); 
        if (newPosition.y < 1 || background[newPosition.y][newPosition.x][newPosition.z] || background[newPosition.y][newPosition.x + 2][newPosition.z] 
        || background[newPosition.y][newPosition.x][newPosition.z + 2] ) {
            for(int x = 0; x < 3; x++){
                for(int y = 0; y < 3 ; y++){
                    for(int z = 0; z < 3 ; z++){
                        background[activeBlockPosition.y + y][activeBlockPosition.x + x][activeBlockPosition.z + z] = 1;
                    }
                }
            }
            checkAndClearRows(); 
            spawnNewBlock(); 
        } else {
            activeBlockPosition = newPosition;
        }
        lastFallTime = currentTime;
    }
}

void spawnNewBlock() {
    
    if (background[13][3][3]) {
        gameOver = true;
    } else {
        activeBlockPosition = glm::vec3(3, 13, 3);
    }
}

void checkAndClearRows() {
    for (int y = 2; y < GRID_SIZE; y++) { // sonra ayarla 
        bool fullRow = true;
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                if (!background[y][x][z]) {
                    fullRow = false;
                    break;
                }
            }
            if (!fullRow) break;
        }
        if (fullRow) {
            for(int y=1; y<4;y++){
                for (int x=0;x<GRID_SIZE; x++) {
                    for (int z=0; z<GRID_SIZE; z++) {
                        score++;
                        background[y][x][z] = 0;
                    }
                }
            }

            for(int y=4; y<13;y++){
                for (int x=0;x<GRID_SIZE; x++) {
                    for (int z=0; z<GRID_SIZE; z++) {
                        if (background[y][x][z]) {
                            background[y][x][z] = 0;
                            background[y-3][x][z] = 1; 
                        } 
                    }
                }
            }

        }
    }
}

float textDisplayTime = 0.0f; 
std::string activeText = "";  
glm::vec3 textColor(1.0f, 0, 0); 
float textSize = 0.7f; 
float textX = 30.0f; 
float textY = 900.0f; 
float textDurationTime = 0.5f;

// Updated keyboard handling function
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (gameOver) return;


    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_A: 
                activeText = "A";
                textDisplayTime = glfwGetTime() + textDurationTime; 
                
                if (cameraAngle==0) moveBlock(-1, 0); 
                else if (cameraAngle==90) moveBlock(0,1);
                else if (cameraAngle==-90 || cameraAngle == 270) moveBlock(0,-1);
                else if (cameraAngle==180) moveBlock(1,0);

                
                break;

            case GLFW_KEY_D: 
                activeText = "D";
                textDisplayTime = glfwGetTime() + textDurationTime; 
                if (cameraAngle==0) moveBlock(1, 0); 
                else if (cameraAngle==90) moveBlock(0,-1);
                else if (cameraAngle==-90 || cameraAngle == 270) moveBlock(0,1);
                else if (cameraAngle==180) moveBlock(-1,0);
                
                
                
                break;

            case GLFW_KEY_S: 
                activeText = "S";
                textDisplayTime = glfwGetTime() + textDurationTime; 
                gameCont = true; 
                fallSpeed = std::max(0.1f, fallSpeed - 0.2f); 
                break;
            case GLFW_KEY_W: 
                activeText = "W";
                textDisplayTime = glfwGetTime() + textDurationTime; 
                fallSpeed = std::min(1.1f, fallSpeed + 0.2f); 
                if (fallSpeed == 1.1f) gameCont = false;
                break;
            case GLFW_KEY_H: 
                if (!isRotating) {
                    activeText = "H";
                    textDisplayTime = glfwGetTime() + textDurationTime; 

                    isRotating = true;
                    currentRotationTime = 0.0f;
                    targetCameraAngle = -90; 
                    // std::cout<<"before camera angle: "<<cameraAngle<<"  targetcameraangle:  "<<targetCameraAngle<<std::endl;
                    // rotateEyePosition();
                    
                }

                break;
            case GLFW_KEY_K: 
                if (!isRotating) {
                    activeText = "K";
                    textDisplayTime = glfwGetTime() + textDurationTime; 
                    targetCameraAngle = 90; 
                    // std::cout<<"before camera angle: "<<cameraAngle<<"  targetcameraangle:  "<<targetCameraAngle<<std::endl;

                    isRotating = true;
                    currentRotationTime = 0.0f;

                    // rotateEyePosition();
                    
                    // std::cout<<"after camera angle: "<<cameraAngle<<"  targetcameraangle:  "<<targetCameraAngle<<"\n"<<std::endl;
                }

                break;
        }
    }
}

// Updated display function
void display() {
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glm::mat4 blockModelMatrix;

    // bura zemini ciziyo
    for(int x = 0; x < GRID_SIZE; x++){
        for(int z = 0; z < GRID_SIZE ; z++){

            blockModelMatrix = glm::translate(glm::mat4(1.0f), (glm::vec3(x-4.5, 0.4f, z-4.5) ));
            // std::cout << activeBlockPosition.x << std::endl;
            // std::cout<<x << "  "<<z<<std::endl;
            blockModelMatrix = glm::scale(blockModelMatrix, glm::vec3(1.0f, 0.6f, 1.0f));
            glUniformMatrix4fv(modelingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(blockModelMatrix));
            drawCubeEdges(blockModelMatrix);
            drawCube(blockModelMatrix, kdGround);  
        }
    } 

    if (gameOver) {
        renderText("Game Over!", 100, 500, 1.5, glm::vec3(0.92, 0.20, 0.63));
        renderText("Score " + std::to_string(score), 200, 450, 1.0, glm::vec3(0.51, 0.20, 0.92));
        // return; 
    }


    // bura yerdeki bloklari ciziyo
    drawBlocks();
    
    for(int x = 0; x < 3; x++){
        for(int y = 0; y < 3 ; y++){
            for(int z = 0; z < 3 ; z++){
                blockModelMatrix = glm::translate(glm::mat4(1.0f), (activeBlockPosition + glm::vec3(x-4.5, y, z-4.5) ));
                // std::cout << activeBlockPosition.x << std::endl;
                glUniformMatrix4fv(modelingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(blockModelMatrix));
                drawCubeEdges(blockModelMatrix);
                drawCube(blockModelMatrix, kdCubes);
            }
        }
    } 

    // glUniformMatrix4fv(modelingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(blockModelMatrix));
    // drawCube();
    // glm::mat4 blockModelMatrix = glm::translate(glm::mat4(1.0f), (activeBlockPosition ));
    // blockModelMatrix = glm::scale(blockModelMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
    // glUniformMatrix4fv(modelingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(blockModelMatrix));
    if (cameraAngle == 0) renderText("Front", 30, 950, 0.50, glm::vec3(0, 1, 1));
    else if (cameraAngle == 90) renderText("Right", 30, 950, 0.50, glm::vec3(0, 1, 1));
    else if (cameraAngle == -90 || cameraAngle == 270 ) renderText("Left", 30, 950, 0.50, glm::vec3(0, 1, 1));
    else if (cameraAngle == 180) renderText("Back", 30, 950, 0.50, glm::vec3(0, 1, 1));
    
    if (score<100000) renderText("Point: " + std::to_string(score), 450, 950, 0.50, glm::vec3(0, 1, 1));
    else renderText("Point: " + std::to_string(score), 400, 950, 0.50, glm::vec3(0, 1, 1));

    if (glfwGetTime() < textDisplayTime) {
        renderText(activeText, textX, textY, textSize, textColor);
    }

}

void mainLoop(GLFWwindow* window) {
    float lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();

        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        rotateEyePosition(deltaTime);

        display();
        updateBlockFall(currentTime);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}


int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(gWidth, gHeight, "tetrisGL", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointersf
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();
    
    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, gWidth, gHeight); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
