// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>
#include <sstream>
//#include <gl/glut.h>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>
#include <math.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

const int window_width = 1024, window_height = 768;

typedef struct Vertex {
    float Position[4];
    float Color[4];
    float Normal[3];
    void SetPosition(float *coords) {
        Position[0] = coords[0];
        Position[1] = coords[1];
        Position[2] = coords[2];
        Position[3] = 1.0;
    }
    void SetColor(float *color) {
        Color[0] = color[0];
        Color[1] = color[1];
        Color[2] = color[2];
        Color[3] = color[3];
    }
    void SetNormal(float *coords) {
        Normal[0] = coords[0];
        Normal[1] = coords[1];
        Normal[2] = coords[2];
    }
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void loadObject(char*, glm::vec4, Vertex * &, GLushort* &, int);
void createVAOs(Vertex[], GLushort[], int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);

// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

const GLuint NumObjects = 11;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects] = { 0 };
GLuint VertexBufferId[NumObjects] = { 0 };
GLuint IndexBufferId[NumObjects] = { 0 };

size_t NumIndices[NumObjects] = { 0 };
size_t VertexBufferSize[NumObjects] = { 0 };
size_t IndexBufferSize[NumObjects] = { 0 };

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint LightID;

GLint gX = 0.0;
GLint gZ = 0.0;

// animation control
bool animation = false;
GLfloat phi = 0.0;

////////////////
//Color
float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float normal[3] = {0.0f, 0.0f, 1.0f};

//Task2
double Pi = 3.1415926;
int keyCListen, keyUpListen, keyDownListen, keyLeftListen, keyRightListen = 0; //key reaction listen

GLfloat up = 1.0f;
glm::vec3 cameraPos = glm::vec3(10.0f, 10.0f,  10.0f);// note down the position of camera
glm::vec3 cameraLook = glm::vec3(0.0f, 0.0f,  0.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, up,  0.0f);

GLfloat rotHoriz = 0; //horizonal move count
GLfloat rotVerti = 0; // vertical move count
double Radius = 17.32;// radius of the default ball


//Task3
Vertex* Verts3; Vertex* Verts4; Vertex* Verts5; Vertex* Verts6; Vertex* Verts7; Vertex* Verts8; Vertex* Verts9; Vertex* Verts10;
GLushort* Idcs3; GLushort* Idcs4; GLushort* Idcs5; GLushort* Idcs6; GLushort* Idcs7; GLushort* Idcs8; GLushort* Idcs9; GLushort* Idcs10;
int markCreate = 1;




glm::mat4 objModelMatrix[7] = {glm::mat4(1.0)}; //basic matrix for each object
vec3 rotateCenter[3]; // rotation center of Arm1, Arm2 and Pen
vec3 penAxis;    // Axis of the pen
vec3 penHorizontalAxis = vec3(0.0, 1.0, 0.0);
vec3 penVerticalAxis = vec3(0.0, 0.0, 1.0);
GLuint highlightID;
int highlightObj=0;
int keyBListen, keyTListen, key1Listen, key2Listen, keyPListen, holdingShift = 0;
GLint direction = 0;
GLfloat delta = Pi/18;
int inputMode = 0;

int moveListen = 0;


// Task 6
glm::vec3 jumpOriStartPoint;
float t;
mat4 jumpModel;
glm::vec4 jumpStartPoint;
glm::vec4 jumpPenOriginPoint;
float jumpXZAngle;
glm::vec3 jumpInitSpeed;







void loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLushort* &out_Indices, int ObjectId)
{
    // Read our .obj file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool res = loadOBJ(file, vertices, normals);
    
    std::vector<GLushort> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);
    
    const size_t vertCount = indexed_vertices.size();
    const size_t idxCount = indices.size();
    
    // populate output arrays
    out_Vertices = new Vertex[vertCount];
    for (int i = 0; i < vertCount; i++) {
        out_Vertices[i].SetPosition(&indexed_vertices[i].x);
        out_Vertices[i].SetNormal(&indexed_normals[i].x);
        out_Vertices[i].SetColor(&color[0]);
    }
    out_Indices = new GLushort[idxCount];
    for (int i = 0; i < idxCount; i++) {
        out_Indices[i] = indices[i];
    }
    
    // set global variables!!
    NumIndices[ObjectId] = idxCount;
    VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
    IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
}



void createObjects(void)
{
    //-- COORDINATE AXES --//
    Vertex CoordVerts[] =
    {
        { { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },// Origin Red
        { { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },// X Red
        { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },// Origin Green
        { { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },// Y Green
        { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },// Origin Blue
        { { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },// Z Blue
    };
    
    VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
    createVAOs(CoordVerts, NULL, 0);
    
    
    
    
    
    
    // ATTN: create your grid vertices here!
    //-- GRID --//
    
    Vertex CoordGridZ[22];
    int countM = -5;
    for(int i = 0; i<21; i=i+2){
        if (countM < 6){
            CoordGridZ[i].Position[0] = -5;
            CoordGridZ[i].Position[1] = 0;
            CoordGridZ[i].Position[2] = countM;
            CoordGridZ[i].Position[3] = 1.0;
            CoordGridZ[i].SetColor(white);
            CoordGridZ[i].SetNormal(normal);
            
        }
        countM++;
    }
    countM = -5;
    for(int i = 1; i<22; i=i+2){
        if (countM < 6){
            CoordGridZ[i].Position[0] = 5;
            CoordGridZ[i].Position[1] = 0;
            CoordGridZ[i].Position[2] = countM;
            CoordGridZ[i].Position[3] = 1.0;
            CoordGridZ[i].SetColor(white);
            CoordGridZ[i].SetNormal(normal);
        }
        countM++;
    }
    VertexBufferSize[1] = sizeof(CoordGridZ);
    createVAOs(CoordGridZ, NULL, 1);
    
    Vertex CoordGridX[22];
    int countN = -5;
    for(int i = 0; i<21; i=i+2){
        if (countN < 6){
            CoordGridX[i].Position[0] = countN;
            CoordGridX[i].Position[1] = 0;
            CoordGridX[i].Position[2] = 5;
            CoordGridX[i].Position[3] = 1.0;
            CoordGridX[i].SetColor(white);
            CoordGridX[i].SetNormal(normal);
            
        }
        countN++;
    }
    countN = -5;
    for(int i = 1; i<22; i=i+2){
        if (countN < 6){
            CoordGridX[i].Position[0] = countN;
            CoordGridX[i].Position[1] = 0;
            CoordGridX[i].Position[2] = -5;
            CoordGridX[i].Position[3] = 1.0;
            CoordGridX[i].SetColor(white);
            CoordGridX[i].SetNormal(normal);
        }
        countN++;
        
    }
    VertexBufferSize[2] = sizeof(CoordGridX);
    createVAOs(CoordGridX, NULL, 2);
    
    
    //-- .OBJs --//
    
    // ATTN: load your models here
    //Vertex* Verts;
    //GLushort* Idcs;
    //loadObject("models/base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, ObjectID);
    //createVAOs(Verts, Idcs, ObjectID);
    
    //    Vertex* Verts;
    //    GLushort* Idcs;
    
    if (highlightObj == 3) {
        loadObject("base.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts3, Idcs3, 3);
        createVAOs(Verts3, Idcs3, 3);
    }
    else{
        loadObject("base.obj", glm::vec4(1.0, 0.0, 1.0, 1.0), Verts3, Idcs3, 3);
        createVAOs(Verts3, Idcs3, 3);
    }
    
    if (highlightObj == 4) {
        loadObject("top.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts4, Idcs4, 4);
        createVAOs(Verts4, Idcs4, 4);
    }
    else{
        loadObject("top.obj", glm::vec4(1.0, 0.3, 0.2, 1.0), Verts4, Idcs4, 4);
        createVAOs(Verts4, Idcs4, 4);
    }
    
    if (highlightObj == 5) {
        loadObject("arm1.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts5, Idcs5, 5);
        createVAOs(Verts5, Idcs5, 5);
    }
    else{
        loadObject("arm1.obj", glm::vec4(0.5, 1.0, 0.3, 1.0), Verts5, Idcs5, 5);
        createVAOs(Verts5, Idcs5, 5);
    }    rotateCenter[0] = vec3(0.0, 1.7, 0.0);
    
    if (highlightObj == 6) {
        loadObject("joint.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts6, Idcs6, 6);
        createVAOs(Verts6, Idcs6, 6);
    }
    else{
        loadObject("joint.obj", glm::vec4(0.5, 0.0, 1.0, 1.0), Verts6, Idcs6, 6);
        createVAOs(Verts6, Idcs6, 6);
    }
    
    if (highlightObj == 7) {
        loadObject("arm2.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts7, Idcs7, 7);
        createVAOs(Verts7, Idcs7, 7);
    }
    else{
        loadObject("arm2.obj", glm::vec4(0.8, 0.6, 1.0, 1.0), Verts7, Idcs7, 7);
        createVAOs(Verts7, Idcs7, 7);
    }    rotateCenter[1] = vec3(2.0, 1.85, 0.0);
    
    if (highlightObj == 8) {
        loadObject("pen.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts8, Idcs8, 8);
        createVAOs(Verts8, Idcs8, 8);
    }
    else{
        loadObject("pen.obj", glm::vec4(0.8, 0.4, 0.4, 1.0), Verts8, Idcs8, 8);
        createVAOs(Verts8, Idcs8, 8);
    }
    rotateCenter[2] = vec3(2.0, 0.65, 0.0);
    penAxis = vec3(1.0, 0.0, 0.0);
    
    if (highlightObj == 9) {
        loadObject("button.obj", glm::vec4(1.0, 1.0, 1.0, 1.0), Verts9, Idcs9, 9);
        createVAOs(Verts9, Idcs9, 9);
    }
    else{
        loadObject("button.obj", glm::vec4(1.0, 0.7, 1.0, 1.0), Verts9, Idcs9, 9);
        createVAOs(Verts9, Idcs9, 9);
    }
    // Jumping ball
    loadObject("ball.obj", glm::vec4(1.0), Verts10, Idcs10, 10);
    createVAOs(Verts10, Idcs10, 10);
    jumpOriStartPoint = glm::vec3(3.35,0.75,0.0);
    
    
    markCreate = 0;
    
}

int jump( float delta ) {
    if (t == 0) {
        jumpStartPoint = vec4(jumpOriStartPoint,1.0);
        jumpPenOriginPoint = vec4(rotateCenter[2], 1.0);
        for (int i=6; i>=0; i--) {
            jumpStartPoint = objModelMatrix[i] * jumpStartPoint;
            jumpPenOriginPoint = objModelMatrix[i] * jumpPenOriginPoint;
        }
        
        jumpXZAngle = atan((jumpStartPoint.z-jumpPenOriginPoint.z)/(jumpStartPoint.x-jumpPenOriginPoint.x));
        
        jumpStartPoint = jumpStartPoint - jumpPenOriginPoint;
        //        jumpStartPoint = glm::rotate(jumpStartPoint, jumpXZAngle, vec3(0.0,1.0,0.0));
        jumpInitSpeed = normalize(vec3(jumpStartPoint)) * 2.0f;
    }
    glm::vec3 movingVector = vec3(jumpStartPoint);
    movingVector.x += jumpInitSpeed.x * t;
    movingVector.y += jumpInitSpeed.y * t -0.5 * 9.8 * t * t;
    
    //    movingVector = glm::rotate(movingVector, -jumpXZAngle, vec3(0.0,1.0,0.0));
    movingVector = movingVector + vec3(jumpPenOriginPoint);
    
    jumpModel = glm::translate(mat4(1.0), movingVector);
    
    if (movingVector.y <= 0) {
        t = 0;
        
        if (abs(movingVector.x) > 5.0 || abs(movingVector.z) > 5.0)
            return 1;
        //        baseMoveSum = vec3(movingVector.x,0.0, movingVector.z);
        objModelMatrix[0] = glm::translate(mat4(1.0), vec3(movingVector.x,0.0, movingVector.z));
        return 1;
    }
    
    t = t+delta;
    return 0;
}

void moveObj(){
    
    if (keyBListen == 1){// move base
        if (direction == GLFW_KEY_LEFT || direction == GLFW_KEY_RIGHT ) {
            glm::vec3 movingVector = glm::vec3 (1.0, 0.0, 0.0);
            
            if (direction == GLFW_KEY_LEFT){
                objModelMatrix[0] = glm::translate(objModelMatrix[0], -glm::vec3(1,0,0));
                moveListen = 1;
            }
            else{
                objModelMatrix[0] = glm::translate(objModelMatrix[0], glm::vec3(1,0,0));
                moveListen = 1;
            }
        }
        if (direction == GLFW_KEY_UP || direction == GLFW_KEY_DOWN ) {
            glm::vec3 movingVector = glm::vec3 (0.0, 0.0, 1.0);
            if (direction == GLFW_KEY_UP){
                objModelMatrix[0] = glm::translate(objModelMatrix[0], -movingVector);
                moveListen = 1;
            }
            else{
                objModelMatrix[0] = glm::translate(objModelMatrix[0], movingVector);
                moveListen = 1;
            }
        }
    }
    if (keyTListen == 1) {// move top
        if (direction == GLFW_KEY_LEFT){
            objModelMatrix[1] = glm::rotate(objModelMatrix[1], delta, glm::vec3(0.0, 1.0, 0.0));
            moveListen = 1;
        }
        else if (direction == GLFW_KEY_RIGHT){
            objModelMatrix[1] = glm::rotate(objModelMatrix[1], -delta, glm::vec3(0.0, 1.0, 0.0));
            moveListen = 1;
        }
        else return;
    }
    
    
    if (key1Listen == 1) {// move arm1
        if (direction != GLFW_KEY_UP && direction != GLFW_KEY_DOWN)
            return;
        if (direction == GLFW_KEY_UP) {
            objModelMatrix[2] = glm::translate(objModelMatrix[2], rotateCenter[0]);
            objModelMatrix[2] = glm::rotate(objModelMatrix[2], delta, glm::vec3(0.0, 0.0, 1.0));
            objModelMatrix[2] = glm::translate(objModelMatrix[2], -rotateCenter[0]);
            moveListen = 1;
        }
        else if (direction == GLFW_KEY_DOWN) {
            objModelMatrix[2] = glm::translate(objModelMatrix[2], rotateCenter[0]);
            objModelMatrix[2] = glm::rotate(objModelMatrix[2], -delta, glm::vec3(0.0, 0.0, 1.0));
            objModelMatrix[2] = glm::translate(objModelMatrix[2], -rotateCenter[0]);
            moveListen = 1;
        }
    }
    
    if (key2Listen == 1) {// move arm2
        if (direction != GLFW_KEY_UP && direction != GLFW_KEY_DOWN)
            return;
        if (direction == GLFW_KEY_UP) {
            objModelMatrix[4] = glm::translate(objModelMatrix[4], rotateCenter[1]);
            objModelMatrix[4] = glm::rotate(objModelMatrix[4], delta, glm::vec3(0.0, 0.0, 1.0));
            objModelMatrix[4] = glm::translate(objModelMatrix[4], -rotateCenter[1]);
            moveListen = 1;
        }
        else if (direction == GLFW_KEY_DOWN) {
            objModelMatrix[4] = glm::translate(objModelMatrix[4], rotateCenter[1]);
            objModelMatrix[4] = glm::rotate(objModelMatrix[4], -delta, glm::vec3(0.0, 0.0, 1.0));
            objModelMatrix[4] = glm::translate(objModelMatrix[4], -rotateCenter[1]);
            moveListen = 1;
        }
    }
    
    if (keyPListen == 1) {// move pen
        if (holdingShift) {
            if (direction != GLFW_KEY_LEFT && direction != GLFW_KEY_RIGHT)
                return;
            if (direction == GLFW_KEY_LEFT) {
                objModelMatrix[5] = glm::translate(objModelMatrix[5], rotateCenter[2]);
                objModelMatrix[5] = glm::rotate(objModelMatrix[5], delta, penAxis);
                objModelMatrix[5] = glm::translate(objModelMatrix[5], -rotateCenter[2]);
                moveListen = 1;
            }
            else if (direction == GLFW_KEY_RIGHT) {
                objModelMatrix[5] = glm::translate(objModelMatrix[5], rotateCenter[2]);
                objModelMatrix[5] = glm::rotate(objModelMatrix[5], -delta, penAxis);
                objModelMatrix[5] = glm::translate(objModelMatrix[5], -rotateCenter[2]);
                moveListen = 1;
            }
        }
        else {
            switch (direction) {
                case GLFW_KEY_LEFT:
                    objModelMatrix[5] = glm::translate(objModelMatrix[5], rotateCenter[2]);
                    objModelMatrix[5] = glm::rotate(objModelMatrix[5], -delta, penHorizontalAxis);
                    objModelMatrix[5] = glm::translate(objModelMatrix[5], -rotateCenter[2]);
                    moveListen = 1;
                    break;
                case GLFW_KEY_RIGHT:
                    objModelMatrix[5] = glm::translate(objModelMatrix[5], rotateCenter[2]);
                    objModelMatrix[5] = glm::rotate(objModelMatrix[5], delta, penHorizontalAxis);
                    objModelMatrix[5] = glm::translate(objModelMatrix[5], -rotateCenter[2]);
                    moveListen = 1;
                    break;
                    
                case GLFW_KEY_UP:
                    objModelMatrix[5] = glm::translate(objModelMatrix[5], rotateCenter[2]);
                    objModelMatrix[5] = glm::rotate(objModelMatrix[5], delta, penVerticalAxis);
                    objModelMatrix[5] = glm::translate(objModelMatrix[5], -rotateCenter[2]);
                    moveListen = 1;
                    break;
                case GLFW_KEY_DOWN:
                    objModelMatrix[5] = glm::translate(objModelMatrix[5], rotateCenter[2]);
                    ;
                    objModelMatrix[5] = glm::rotate(objModelMatrix[5], -delta, penVerticalAxis);
                    objModelMatrix[5] = glm::translate(objModelMatrix[5], -rotateCenter[2]);
                    moveListen = 1;
                    break;
                    
                default:
                    return;
            }
            
        }
    }
    
}



void moveCamera(void){
    if(keyCListen == 1){
        GLfloat arcHoriz = 45*Pi/180+rotHoriz;// angle with x-y plane
        GLfloat arcVerti = 35.26*Pi/180+rotVerti;// angle with x-z plane
        GLfloat viewX = 10.0f;
        GLfloat viewY = 10.0f;
        GLfloat viewZ = 10.0f;
        
        cameraPos   = glm::vec3(viewX, viewY, viewZ);
        
        
        if (arcVerti > 2*Pi) arcVerti = arcVerti - 2*Pi;
        if (arcVerti < 0) arcVerti = arcVerti +2*Pi;
        if(((0 < arcVerti < (Pi/2)) && ((3*Pi)/2) < arcVerti < 2*Pi ) ){
            viewX = cos(arcHoriz)*(cos(arcVerti)*Radius);
            viewZ = sin(arcHoriz)*(cos(arcVerti)*Radius);
            viewY = sin(arcVerti)*Radius;
            
            cameraPos   = glm::vec3(viewX, viewY, viewZ);
            
            up = 1.0f;
            gViewMatrix = glm::lookAt(cameraPos, cameraLook, cameraUp);
            
            
        }
        else if((Pi/2) < arcVerti < ((3*Pi)/2)){
            viewX = cos(arcHoriz)*(cos(arcVerti)*Radius);
            viewZ = sin(arcHoriz)*(cos(arcVerti)*Radius);
            viewY = sin(arcVerti)*Radius;
            
            cameraPos   = glm::vec3(viewX, viewY, viewZ);
            up = -1.0f;
            gViewMatrix = glm::lookAt(cameraPos, cameraLook, cameraUp);
            
        }
    }
    
}

void renderScene(void)
{
    //ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!
    
    
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
    // Re-clear the screen for real rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    
    
    
    glUseProgram(programID);
    {
        glm::vec3 lightPos = glm::vec3(0, 4, 4);
        glm::mat4x4 ModelMatrix = glm::mat4(1.0);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
        glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        
        
        glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
        glDrawArrays(GL_LINES, 0, 6);
        
        glBindVertexArray(VertexArrayId[1]);	// draw CoordGridZ
        glDrawArrays(GL_LINES, 0, 22);
        
        glBindVertexArray(VertexArrayId[2]);	// draw CoordGridX
        glDrawArrays(GL_LINES, 0, 22);
        
        
        for (int i=3; i<10; i++) {
            ModelMatrix = ModelMatrix * objModelMatrix[i-3];
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
            glBindVertexArray(VertexArrayId[i]);
            if (highlightObj==i) {
                glUniform1d(highlightID, 1);
                glDrawElements(GL_TRIANGLES, VertexBufferSize[i], GL_UNSIGNED_SHORT, NULL);
                glUniform1d(highlightID, 0);
            }
            else
                glDrawElements(GL_TRIANGLES, VertexBufferSize[i], GL_UNSIGNED_SHORT, NULL);
        }
        
        if (inputMode == GLFW_KEY_J) {
            markCreate = 1;
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &jumpModel[0][0]);
            glBindVertexArray(VertexArrayId[10]);
            glDrawElements(GL_TRIANGLES, VertexBufferSize[10], GL_UNSIGNED_SHORT, NULL);
        }
        
        glBindVertexArray(0);
        
    }
    glUseProgram(0);
    // Draw GUI
    TwDraw();
    
    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
    moveCamera();
}

void pickObject(void)
{
    // Clear the screen in white
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(pickingProgramID);
    {
        glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
        glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
        
        // Send our transformation to the currently bound shader, in the "MVP" uniform
        glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
        
        // ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
        glBindVertexArray(0);
        
    }
    glUseProgram(0);
    // Wait until all the pending drawing commands are really done.
    // Ultra-mega-over slow !
    // There are usually a long time between glDrawElements() and
    // all the fragments completely rasterized.
    glFlush();
    glFinish();
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    // Read the pixel at the center of the screen.
    // You can also use glfwGetMousePos().
    // Ultra-mega-over slow too, even for 1 pixel,
    // because the framebuffer is on the GPU.
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    unsigned char data[4];
    glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top
    
    // Convert the color back to an integer ID
    gPickedIndex = int(data[0]);
    
    if (gPickedIndex == 255){ // Full white, must be the background !
        gMessage = "background";
    }
    else {
        std::ostringstream oss;
        oss << "point " << gPickedIndex;
        gMessage = oss.str();
    }
    
    // Uncomment these lines to see the picking shader in effect
    //glfwSwapBuffers(window);
    //continue; // skips the normal rendering
}

int initWindow(void)
{
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // Open a window and create its OpenGL context
    window = glfwCreateWindow(window_width, window_height, "Wang,Zun(6151-0196)", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    
    // Initialize the GUI
    TwInit(TW_OPENGL_CORE, NULL);
    TwWindowSize(window_width, window_height);
    TwBar * GUI = TwNewBar("Picking");
    TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
    TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);
    
    // Set up inputs
    glfwSetCursorPos(window, window_width / 2, window_height / 2);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    
    return 0;
}

void initOpenGL(void)
{
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);
    
    // Projection matrix : 45?Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    //gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates
    
    
    gViewMatrix = glm::lookAt(cameraPos, cameraLook, cameraUp);
    
    
    
    
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
    pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");
    
    // Get a handle for our "MVP" uniform
    MatrixID = glGetUniformLocation(programID, "MVP");
    ModelMatrixID = glGetUniformLocation(programID, "M");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    ProjMatrixID = glGetUniformLocation(programID, "P");
    
    PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
    // Get a handle for our "pickingColorID" uniform
    pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
    // Get a handle for our "LightPosition" uniform
    LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    
    //    createObjects();
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {
    
    GLenum ErrorCheckValue = glGetError();
    const size_t VertexSize = sizeof(Vertices[0]);
    const size_t RgbOffset = sizeof(Vertices[0].Position);
    const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;
    
    // Create Vertex Array Object
    glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
    glBindVertexArray(VertexArrayId[ObjectId]);		//
    
    
    
    // Create Buffer for vertex data
    glGenBuffers(1, &VertexBufferId[ObjectId]);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
    glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);
    
    
    
    // Create Buffer for indices
    if (Indices != NULL) {
        glGenBuffers(1, &IndexBufferId[ObjectId]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
    }
    
    
    // Assign vertex attributes
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);
    
    glEnableVertexAttribArray(0);	// position
    glEnableVertexAttribArray(1);	// color
    glEnableVertexAttribArray(2);	// normal
    
    // Disable our Vertex Buffer Object
    glBindVertexArray(0);
    
    ErrorCheckValue = glGetError();
    if (ErrorCheckValue != GL_NO_ERROR)
    {
        fprintf(
                stderr,
                "ERROR: Could not create a VBO: %s \n",
                gluErrorString(ErrorCheckValue)
                );
    }
}

void cleanup(void)
{
    // Cleanup VBO and shader
    for (int i = 0; i < NumObjects; i++) {
        glDeleteBuffers(1, &VertexBufferId[i]);
        glDeleteBuffers(1, &IndexBufferId[i]);
        glDeleteVertexArrays(1, &VertexArrayId[i]);
    }
    glDeleteProgram(programID);
    glDeleteProgram(pickingProgramID);
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // ATTN: MODIFY AS APPROPRIATE
    //    if (action == GLFW_PRESS) {
    //        switch (key)
    //        {
    //
    //
    //            case GLFW_KEY_A:
    //                break;
    //            case GLFW_KEY_D:
    //                break;
    //            case GLFW_KEY_W:
    //                break;
    //            case GLFW_KEY_S:
    //                break;
    //            case GLFW_KEY_SPACE:
    //                break;
    //            default:
    //                break;
    //        }
    //    }
    
    if(key == GLFW_KEY_C && action == GLFW_PRESS){
        keyCListen = 1;
        keyBListen = keyTListen = key1Listen = key2Listen = keyPListen = 0;
    }
    
    if(keyCListen == 1){
        if (action == GLFW_PRESS) {
            switch (key)
            {
                case GLFW_KEY_LEFT:
                    rotHoriz = rotHoriz-0.1;
                    break;
                case GLFW_KEY_RIGHT:
                    rotHoriz = rotHoriz+0.1;
                    break;
                case GLFW_KEY_UP:
                    rotVerti = rotVerti+0.1;
                    break;
                case GLFW_KEY_DOWN:
                    rotVerti = rotVerti-0.1;
                    break;
                default:
                    break;
            }
        }
    }
    
    if(key == GLFW_KEY_B && action == GLFW_PRESS){
        keyBListen = 1;
        keyCListen = keyTListen = key1Listen = key2Listen = keyPListen = 0;
        highlightObj = 3;
        markCreate = 1;
    }
    
    if(keyBListen == 1){
        if (action == GLFW_PRESS) {
            switch (key)
            {
                case GLFW_KEY_LEFT:
                    direction = GLFW_KEY_LEFT;
                    moveListen = 0;
                    break;
                case GLFW_KEY_RIGHT:
                    direction = GLFW_KEY_RIGHT;
                    moveListen = 0;
                    break;
                case GLFW_KEY_UP:
                    direction = GLFW_KEY_UP;
                    moveListen = 0;
                    break;
                case GLFW_KEY_DOWN:
                    direction = GLFW_KEY_DOWN;
                    moveListen = 0;
                    break;
                default:
                    break;
            }
        }
    }
    
    if(key == GLFW_KEY_T && action == GLFW_PRESS){
        keyTListen = 1;
        keyBListen = keyCListen = key1Listen = key2Listen = keyPListen = 0;
        highlightObj = 4;
        markCreate = 1;
        
    }
    if(keyTListen == 1){
        if (action == GLFW_PRESS) {
            switch (key)
            {
                case GLFW_KEY_LEFT:
                    direction = GLFW_KEY_LEFT;
                    moveListen = 0;
                    break;
                case GLFW_KEY_RIGHT:
                    direction = GLFW_KEY_RIGHT;
                    moveListen = 0;
                    break;
                default:
                    break;
            }
        }
    }
    
    if(key == GLFW_KEY_1 && action == GLFW_PRESS){
        key1Listen = 1;
        keyCListen = keyBListen = keyTListen = key2Listen = keyPListen = 0;
        highlightObj = 5;
        markCreate = 1;
        
    }
    
    if(key1Listen == 1){
        if (action == GLFW_PRESS) {
            switch (key)
            {
                case GLFW_KEY_UP:
                    direction = GLFW_KEY_UP;
                    moveListen = 0;
                    break;
                case GLFW_KEY_DOWN:
                    direction = GLFW_KEY_DOWN;
                    moveListen = 0;
                    break;
                default:
                    break;
            }
        }
    }
    
    if(key == GLFW_KEY_2 && action == GLFW_PRESS){
        key2Listen = 1;
        keyCListen = keyBListen = keyTListen = key1Listen = keyPListen = 0;
        highlightObj = 7;
        markCreate = 1;
        markCreate = 1;
        
    }
    
    if(key2Listen == 1){
        if (action == GLFW_PRESS) {
            switch (key)
            {
                case GLFW_KEY_UP:
                    direction = GLFW_KEY_UP;
                    moveListen = 0;
                    break;
                case GLFW_KEY_DOWN:
                    direction = GLFW_KEY_DOWN;
                    moveListen = 0;
                    break;
                default:
                    break;
            }
        }
    }
    if (action == GLFW_RELEASE) {
        direction = 0;
        holdingShift = 0;
        return;
    }
    if (mods == GLFW_MOD_SHIFT) {
        holdingShift = 1;
        highlightObj = 9;
        markCreate = 1;
        
    }
    else {
        holdingShift = false;
        direction=key;
    }
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
        holdingShift = 1;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        keyPListen = 1;
        keyCListen = keyTListen = keyBListen = key1Listen = key2Listen = 0;
        highlightObj = 8;
        markCreate = 1;
        
    }
    if(keyPListen == 1){
        if (action == GLFW_PRESS) {
            switch (key)
            {
                case GLFW_KEY_LEFT:
                    direction = GLFW_KEY_LEFT;
                    moveListen = 0;
                    break;
                case GLFW_KEY_RIGHT:
                    direction = GLFW_KEY_RIGHT;
                    moveListen = 0;
                    break;
                case GLFW_KEY_UP:
                    direction = GLFW_KEY_UP;
                    moveListen = 0;
                    break;
                case GLFW_KEY_DOWN:
                    direction = GLFW_KEY_DOWN;
                    moveListen = 0;
                    break;
                default:
                    break;
            }
        }
    }
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        pickObject();
    }
}

int main(void)
{
    // initialize window
    int errorCode = initWindow();
    if (errorCode != 0)
        return errorCode;
    
    // initialize OpenGL pipeline
    initOpenGL();
    
    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;
    do {
        //// Measure speed
        //double currentTime = glfwGetTime();
        //nbFrames++;
        //if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
        //	// printf and reset
        //	printf("%f ms/frame\n", 1000.0 / double(nbFrames));
        //	nbFrames = 0;
        //	lastTime += 1.0;
        //}
        if (markCreate == 1) {
            createObjects();
        }
        
        if (animation){
            phi += 0.01;
            if (phi > 360)
                phi -= 360;
        }
        if (moveListen == 0) {
            moveObj();
        }
        if (inputMode == GLFW_KEY_J) {
            while (!jump(0.01))
                renderScene();
            inputMode = 0;
        }
        // DRAWING POINTS
        renderScene();
        
        
    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);
    
    cleanup();
    
    return 0;
}
