#include <glut.h>
#include <vector>   
#include <string>   
#include <stack>
#include <iostream> 
#include <stdlib.h> 
#include <math.h>   

#define M_PI 3.14159265358979323846

enum TransformationMode {
    TRANSLATE,
    ROTATE,
    SCALE,
    SHEAR,
    REFLECT
};

// Global transformation matrix using 2D array
float transformMatrix[4][4] = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};

// Global position, rotation, scale variables
float position[3] = { 0.0f, 0.0f, 0.0f };
float rotation[3] = { 0.0f, 0.0f, 0.0f };
float scale[3] = { 1.0f, 1.0f, 1.0f };
float shear[3] = { 0.0f, 0.0f, 0.0f };
bool reflection[3] = { false, false, false }; // For x, y, z planes

// Shape selection
enum Shape { CUBE, SPHERE, PYRAMID, CYLINDER };
Shape currentShape = CUBE;
TransformationMode currentMode = TRANSLATE;  // Default mode

float cameraPos[] = { 0.0f, 0.0f, 5.0f };  // Initial camera position
float cameraFront[] = { 0.0f, 0.0f, -1.0f };  // Camera direction
float cameraUp[] = { 0.0f, 1.0f, 0.0f };      // Camera up vector
bool cameraControlMode = false;                // Toggle for camera control
float cameraSpeed = 0.1f;                      // Camera movement speed

struct Button {
    float x, y, width, height;
    std::string label;
    bool isShapeButton;
    union {
        Shape shape;
        TransformationMode mode;
    };
    bool selected;

    // Constructor for shape buttons
    Button(float _x, float _y, float _w, float _h, const std::string& _label, bool _isShape, Shape _shape, bool _selected)
        : x(_x), y(_y), width(_w), height(_h), label(_label), isShapeButton(_isShape), selected(_selected) {
        shape = _shape;
    }

    // Constructor for transformation mode buttons
    Button(float _x, float _y, float _w, float _h, const std::string& _label, bool _isShape, TransformationMode _mode, bool _selected)
        : x(_x), y(_y), width(_w), height(_h), label(_label), isShapeButton(_isShape), selected(_selected) {
        mode = _mode;
    }

    Button(float _x, float _y, float _w, float _h, const std::string& _label, bool _selected = false)
        : x(_x), y(_y), width(_w), height(_h), label(_label), selected(_selected) {}
};

std::vector<Button> buttons;


void matrixIdentity(float matrix[4][4]) {

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }

}

void matrixMultiply(float a[4][4], float b[4][4], float result[4][4]) {
    float temp[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                temp[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i][j] = temp[i][j];
        }
    }
}


void createShearMatrix(float xy, float xz, float yx, float yz, float zx, float zy, float matrix[4][4]) {
    matrixIdentity(matrix);
    
    matrix[0][1] = xy;  
    matrix[0][2] = xz;  

    
    matrix[1][0] = yx;  
    matrix[1][2] = yz;  

    
    matrix[2][0] = zx;  
    matrix[2][1] = zy;  
}


void createReflectionMatrix(bool x, bool y, bool z, float matrix[4][4]) {

    matrixIdentity(matrix);
    
    matrix[0][0] = x ? -1.0f : 1.0f;  
    matrix[1][1] = y ? -1.0f : 1.0f;  
    matrix[2][2] = z ? -1.0f : 1.0f;  
    matrix[3][3] = 1.0f;  

}


void createTranslationMatrix(float x, float y, float z, float matrix[4][4]) {

    matrixIdentity(matrix);
    matrix[0][3] = x;
    matrix[1][3] = y;
    matrix[2][3] = z;

}

void createScaleMatrix(float x, float y, float z, float matrix[4][4]) {

    matrixIdentity(matrix);
    matrix[0][0] = x;
    matrix[1][1] = y;
    matrix[2][2] = z;

}

void createRotationXMatrix(float angle, float matrix[4][4]) {

    matrixIdentity(matrix);

    float rad = angle * M_PI / 180.0f;
    float cos_t = cos(rad);
    float sin_t = sin(rad);
    matrix[1][1] = cos_t;
    matrix[1][2] = -sin_t;
    matrix[2][1] = sin_t;
    matrix[2][2] = cos_t;

}

void createRotationYMatrix(float angle, float matrix[4][4]) {

    matrixIdentity(matrix);
    float rad = angle * M_PI / 180.0f;
    float cos_t = cos(rad);
    float sin_t = sin(rad);
    matrix[0][0] = cos_t;
    matrix[0][2] = sin_t;
    matrix[2][0] = -sin_t;
    matrix[2][2] = cos_t;


}

void createRotationZMatrix(float angle, float matrix[4][4]) {

    matrixIdentity(matrix);
    float rad = angle * M_PI / 180.0f;
    float cos_t = cos(rad);
    float sin_t = sin(rad);
    matrix[0][0] = cos_t;
    matrix[0][1] = -sin_t;
    matrix[1][0] = sin_t;
    matrix[1][1] = cos_t;


}


void updateTransformMatrix() {
    float translationMat[4][4], rotationXMat[4][4], rotationYMat[4][4],
        rotationZMat[4][4], scaleMat[4][4], shearMat[4][4],
        reflectionMat[4][4], tempMat[4][4];

    
    createTranslationMatrix(position[0], position[1], position[2], translationMat);
    createRotationXMatrix(rotation[0], rotationXMat);
    createRotationYMatrix(rotation[1], rotationYMat);
    createRotationZMatrix(rotation[2], rotationZMat);
    createScaleMatrix(scale[0], scale[1], scale[2], scaleMat);
    createShearMatrix(shear[0], shear[1], shear[0], shear[2], shear[1], shear[2], shearMat);
    createReflectionMatrix(reflection[0], reflection[1], reflection[2], reflectionMat);

    
    matrixIdentity(tempMat);
    matrixMultiply(tempMat, scaleMat, tempMat);
    
    matrixMultiply(tempMat, shearMat, tempMat);

    matrixMultiply(tempMat, reflectionMat, tempMat);
    
    matrixMultiply(tempMat, rotationZMat, tempMat);
    matrixMultiply(tempMat, rotationYMat, tempMat);
    matrixMultiply(tempMat, rotationXMat, tempMat);
    
    matrixMultiply(tempMat, translationMat, transformMatrix);
}

void applyTransformMatrix() {
    
    float glMatrix[16];
    int idx = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            
            glMatrix[idx++] = transformMatrix[j][i];
        }
    }
    glMultMatrixf(glMatrix);
}


void drawGrid() {
    glDisable(GL_LIGHTING);  // Disable lighting for grid lines
    glColor3f(0.3f, 0.3f, 0.3f);  // Grey color for grid
    glBegin(GL_LINES);

    // Draw grid lines along X axis with larger size (20x20)
    for (float i = -20; i <= 20; i += 1.0f) {
        glVertex3f(i, 0, -20);
        glVertex3f(i, 0, 20);
    }

    // Draw grid lines along Z axis
    for (float i = -20; i <= 20; i += 1.0f) {
        glVertex3f(-20, 0, i);
        glVertex3f(20, 0, i);
    }

    glEnd();

    // Draw coordinate axes with longer length
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // X axis - Red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(4, 0, 0);

    // Y axis - Green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 4, 0);

    // Z axis - Blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 4);
    glEnd();
    glLineWidth(1.0f);

    glEnable(GL_LIGHTING);
}

void renderText(float x, float y, const char* text) {
    // [Previous renderText function remains the same]
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 700);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);  // White text
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
}

// Add this function to display keyboard instructions
void renderInstructions() {
    char buffer[256];
    float startY = 150;  // Starting Y position for instructions

    // Common instructions header
    if (cameraControlMode) {
        renderText(10, startY, "Camera Control Mode Instructions:");
        renderText(10, startY - 20, "W/S: Move forward/backward");
        renderText(10, startY - 40, "A/D: Strafe left/right");
        renderText(10, startY - 60, "Q/E: Move up/down");
        renderText(10, startY - 80, "Right Mouse + Drag: Rotate camera");
        renderText(10, startY - 100, "Space: Reset camera position");
        renderText(10, startY - 120, "C: Toggle camera/object control");
    }
    else {
        // Mode-specific instructions
        switch (currentMode) {
        case TRANSLATE:
            renderText(10, startY, "Translation Mode Instructions:");
            renderText(10, startY - 20, "W/S: Move up/down (Y axis)");
            renderText(10, startY - 40, "A/D: Move left/right (X axis)");
            renderText(10, startY - 60, "Q/E: Move forward/backward (Z axis)");
            renderText(10, startY - 80, "Space: Reset position");
            renderText(10, startY - 100, "C: Toggle camera/object control");
            break;

        case ROTATE:
            renderText(10, startY, "Rotation Mode Instructions:");
            renderText(10, startY - 20, "W/S: Rotate around X axis");
            renderText(10, startY - 40, "A/D: Rotate around Y axis");
            renderText(10, startY - 60, "Q/E: Rotate around Z axis");
            renderText(10, startY - 80, "Space: Reset rotation");
            renderText(10, startY - 100, "C: Toggle camera/object control");
            break;

        case SCALE:
            renderText(10, startY, "Scale Mode Instructions:");
            renderText(10, startY - 20, "W/S: Scale up/down (Y axis)");
            renderText(10, startY - 40, "A/D: Scale left/right (X axis)");
            renderText(10, startY - 60, "Q/E: Scale forward/backward (Z axis)");
            renderText(10, startY - 80, "Space: Reset scale");
            renderText(10, startY - 100, "C: Toggle camera/object control");
            break;

        case SHEAR:
            renderText(10, startY, "Shear Mode Instructions:");
            renderText(10, startY - 20, "W/S: Shear in XY plane");
            renderText(10, startY - 40, "A/D: Shear in XZ plane");
            renderText(10, startY - 60, "Q/E: Shear in YZ plane");
            renderText(10, startY - 80, "Space: Reset shear");
            renderText(10, startY - 100, "C: Toggle camera/object control");
            break;

        case REFLECT:
            renderText(10, startY, "Reflection Mode Instructions:");
            renderText(10, startY - 20, "X: Toggle reflection across YZ plane");
            renderText(10, startY - 40, "Y: Toggle reflection across XZ plane");
            renderText(10, startY - 60, "Z: Toggle reflection across XY plane");
            renderText(10, startY - 80, "Space: Reset all reflections");
            renderText(10, startY - 100, "C: Toggle camera/object control");
            break;
        }
    }

    // Add general instructions at the bottom
    renderText(10, startY - 140, "ESC: Exit program");
}


// Keep only one drawButtons function and remove the duplicate
void drawButtons() {
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    for (const Button& btn : buttons) {
        // Draw button background
        if (btn.selected) {
            glColor3f(0.5f, 0.5f, 0.8f);  // Highlighted color for selected
        }
        else {
            glColor3f(0.3f, 0.3f, 0.3f);  // Normal color
        }

        glBegin(GL_QUADS);
        glVertex2f(btn.x, btn.y);
        glVertex2f(btn.x + btn.width, btn.y);
        glVertex2f(btn.x + btn.width, btn.y + btn.height);
        glVertex2f(btn.x, btn.y + btn.height);
        glEnd();

        // Draw button text
        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(btn.x + 35, btn.y + btn.height / 2);
        for (char c : btn.label) {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
        }
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
}



void drawCube() {
    glBegin(GL_QUADS);

    // Front face (Red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);

    // Back face (Green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);

    // Top face (Blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);

    // Bottom face (Yellow)
    glColor3f(1.0f, 1.0f, 0.0f);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);

    // Right face (Purple)
    glColor3f(1.0f, 0.0f, 1.0f);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);

    // Left face (Cyan)
    glColor3f(0.0f, 1.0f, 1.0f);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);

    glEnd();
}


void drawSphere() {
    const int stacks = 20;
    const int slices = 20;
    const float radius = 0.5f;

    // Draw sphere using triangle strips
    for (int i = 0; i < stacks; i++) {
        float phi1 = M_PI * float(i) / stacks;
        float phi2 = M_PI * float(i + 1) / stacks;

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; j++) {
            float theta = 2.0f * M_PI * float(j) / slices;

            // First vertex
            float x = radius * sin(phi1) * cos(theta);
            float y = radius * cos(phi1);
            float z = radius * sin(phi1) * sin(theta);
            glColor3f(1.0f, 0.0f, 0.0f);  // Red color
            glNormal3f(x / radius, y / radius, z / radius);
            glVertex3f(x, y, z);

            // Second vertex
            x = radius * sin(phi2) * cos(theta);
            y = radius * cos(phi2);
            z = radius * sin(phi2) * sin(theta);
            glNormal3f(x / radius, y / radius, z / radius);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
}

// Function to draw a pyramid
void drawPyramid() {
    glBegin(GL_TRIANGLES);
    // Front
    glColor3f(1.0f, 0.0f, 0.0f);
    glNormal3f(0.0f, 0.5f, 0.5f);
    glVertex3f(0.0f, 0.5f, 0.0f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);

    // Right
    glColor3f(0.0f, 1.0f, 0.0f);
    glNormal3f(0.5f, 0.5f, 0.0f);
    glVertex3f(0.0f, 0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);

    // Back
    glColor3f(0.0f, 0.0f, 1.0f);
    glNormal3f(0.0f, 0.5f, -0.5f);
    glVertex3f(0.0f, 0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f);

    // Left
    glColor3f(1.0f, 1.0f, 0.0f);
    glNormal3f(-0.5f, 0.5f, 0.0f);
    glVertex3f(0.0f, 0.5f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glEnd();

    // Bottom
    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 1.0f);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glEnd();
}

void drawCylinder() {
    const float radius = 0.5f;
    const float height = 1.0f;
    const int segments = 30;
    
    // Draw the sides
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / segments;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        
        glColor3f(0.0f, 0.0f, 1.0f);  // Cyan color
        
        // Normal for the side
        glNormal3f(cos(theta), 0.0f, sin(theta));
        
        // Top vertex
        glVertex3f(x, height/2, z);
        // Bottom vertex
        glVertex3f(x, -height/2, z);
    }
    glEnd();
    

    // Draw top and bottom circles
    for (int side = 0; side < 2; side++) {
        float y = (side == 0) ? height/2 : -height/2;
        float normal_y = (side == 0) ? 1.0f : -1.0f;
        glColor3f(0.0f, 1.0f, 0.0f);

        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, normal_y, 0.0f);
        glVertex3f(0.0f, y, 0.0f);  // Center point
        
        for (int i = 0; i <= segments; i++) {
            float theta = (side == 0) ? 
                         -2.0f * M_PI * float(i) / segments :  
                         2.0f * M_PI * float(i) / segments;    
            
            float x = radius * cos(theta);
            float z = radius * sin(theta);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
}


void drawShape() {
    switch (currentShape) {
    case CUBE:
        drawCube();
        break;
    case SPHERE:
        drawSphere();
        break;
    case PYRAMID:
        drawPyramid();
        break;
    case CYLINDER:
        drawCylinder();
        break;
    }
}




void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Set up light
    GLfloat light_position[] = { 10.0f, 10.0f, 10.0f, 1.0f };
    GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    
    GLfloat mat_ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[] = { 50.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 800.0f / 700.0f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    // Modified button layout
    float buttonWidth = 30;  // Increased width for better visibility
    float buttonHeight = 30;
    float padding = 10;

    // Shape selection buttons - right side
    float shapeStartX = 1550;
    float shapeStartY = 1000;

    // Transformation buttons - left side
    float transformStartX = 10;
    float transformStartY = 500;

    buttons.clear();

    // Shape selection buttons (right side)
    buttons.push_back(Button(shapeStartX, shapeStartY - 0 * (buttonHeight + padding), buttonWidth, buttonHeight, "Cube", true, CUBE, currentShape == CUBE));
    buttons.push_back(Button(shapeStartX, shapeStartY - 1 * (buttonHeight + padding), buttonWidth, buttonHeight, "Sphere", true, SPHERE, currentShape == SPHERE));
    buttons.push_back(Button(shapeStartX, shapeStartY - 2 * (buttonHeight + padding), buttonWidth, buttonHeight, "Pyramid", true, PYRAMID, currentShape == PYRAMID));
    buttons.push_back(Button(shapeStartX, shapeStartY - 3 * (buttonHeight + padding), buttonWidth, buttonHeight, "Cylinder", true, CYLINDER, currentShape == CYLINDER));

    // Transformation mode buttons (left side)
    buttons.push_back(Button(transformStartX, transformStartY - 0 * (buttonHeight + padding), buttonWidth, buttonHeight, "Translate", false, TRANSLATE, currentMode == TRANSLATE));
    buttons.push_back(Button(transformStartX, transformStartY - 1 * (buttonHeight + padding), buttonWidth, buttonHeight, "Rotate", false, ROTATE, currentMode == ROTATE));
    buttons.push_back(Button(transformStartX, transformStartY - 2 * (buttonHeight + padding), buttonWidth, buttonHeight, "Scale", false, SCALE, currentMode == SCALE));
    buttons.push_back(Button(transformStartX, transformStartY - 3 * (buttonHeight + padding), buttonWidth, buttonHeight, "Shear", false, SHEAR, currentMode == SHEAR));
    buttons.push_back(Button(transformStartX, transformStartY - 4 * (buttonHeight + padding), buttonWidth, buttonHeight, "Reflect", false, REFLECT, currentMode == REFLECT));

    // Add camera control button
    float cameraButtonX = 10;
    float cameraButtonY = 550;
    float buttonWidth2 = 30;  // Wider button for better visibility
    float buttonHeight2 = 30;

    // Add camera control button to buttons vector
    buttons.push_back(Button(cameraButtonX, cameraButtonY, buttonWidth2, buttonHeight2, "Camera Control", cameraControlMode));
}



void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    
    //Setup of Camera Position Using LookAt Function
    gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2],
        cameraPos[0] + cameraFront[0], cameraPos[1] + cameraFront[1], cameraPos[2] + cameraFront[2],
        cameraUp[0], cameraUp[1], cameraUp[2]);

    drawGrid();

    glLoadIdentity();
    gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2],
        cameraPos[0] + cameraFront[0], cameraPos[1] + cameraFront[1], cameraPos[2] + cameraFront[2],
        cameraUp[0], cameraUp[1], cameraUp[2]);

    applyTransformMatrix();
    drawShape();
    drawButtons();


    char buffer[256];

    // Position
    snprintf(buffer, sizeof(buffer), "Position: (%.1f, %.1f, %.1f)", position[0], position[1], position[2]);
    renderText(10, 670, buffer);

    // Rotation
    snprintf(buffer, sizeof(buffer), "Rotation: (%.1f, %.1f, %.1f)", rotation[0], rotation[1], rotation[2]);
    renderText(10, 650, buffer);

    // Scale
    snprintf(buffer, sizeof(buffer), "Scale: (%.1f, %.1f, %.1f)", scale[0], scale[1], scale[2]);
    renderText(10, 630, buffer);

    // Shear
    snprintf(buffer, sizeof(buffer), "Shear XY: %.1f  XZ: %.1f  YZ: %.1f", shear[0], shear[1], shear[2]);
    renderText(10, 610, buffer);

    // Reflection
    snprintf(buffer, sizeof(buffer), "Reflection: X: %s  Y: %s  Z: %s",
        reflection[0] ? "ON" : "OFF",
        reflection[1] ? "ON" : "OFF",
        reflection[2] ? "ON" : "OFF");
    renderText(10, 590, buffer);

    // Camera Control 
    snprintf(buffer, sizeof(buffer), "Camera Control: %s", cameraControlMode ? "ON" : "OFF");
    renderText(10, 570, buffer);

    renderText(700, 10, "Love Dewangan 500109339");

    
    const char* modeText;
    switch (currentMode) {
    case TRANSLATE: modeText = "TRANSLATE"; break;
    case ROTATE: modeText = "ROTATE"; break;
    case SCALE: modeText = "SCALE"; break;
    case SHEAR: modeText = "SHEAR"; break;
    case REFLECT: modeText = "REFLECT"; break;
    default: modeText = "UNKNOWN"; break;
    }

    snprintf(buffer, sizeof(buffer), "Current Mode: %s", modeText);
    renderText(10, 550, buffer);

    
    renderInstructions();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = (float)w / (float)h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //This set the window for Orthographic mode to perspective (Field of View angle, Aspect Ratio, Near Clipping Plane, Far Clipping Plane)
    gluPerspective(45.0f, ratio, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
}

void toggleCameraControl() {
    cameraControlMode = !cameraControlMode;
    // Update camera control button selection state
    for (Button& btn : buttons) {
        if (btn.label == "Camera Control") {
            btn.selected = cameraControlMode;
        }
    }
}


float lastX = 400, lastY = 350;  // Last mouse position
float yaw = -90.0f;   // Camera yaw angle (horizontal rotation)
float pitch = 0.0f;   // Camera pitch angle (vertical rotation)
bool firstMouse = true;  // Flag for first mouse movement
bool mouseRightDown = false;  // Track right mouse button state


void mouseMotion(int x, int y) {
    if (!mouseRightDown || !cameraControlMode) return;

    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
        return;
    }

    // Calculate mouse movement since last frame
    float xoffset = x - lastX;
    float yoffset = lastY - y;  // Reversed since y-coordinates go from bottom to top
    lastX = x;
    lastY = y;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Constrain pitch to avoid camera flipping
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Update camera front vector
    float radYaw = yaw * M_PI / 180.0f;
    float radPitch = pitch * M_PI / 180.0f;

    cameraFront[0] = cos(radYaw) * cos(radPitch);
    cameraFront[1] = sin(radPitch);
    cameraFront[2] = sin(radYaw) * cos(radPitch);

    // Normalize the front vector
    float length = sqrt(cameraFront[0] * cameraFront[0] +
        cameraFront[1] * cameraFront[1] +
        cameraFront[2] * cameraFront[2]);
    cameraFront[0] /= length;
    cameraFront[1] /= length;
    cameraFront[2] /= length;

    glutPostRedisplay();
}


void mouse(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON) {
        mouseRightDown = (state == GLUT_DOWN);
        if (state == GLUT_DOWN) {
            firstMouse = true;
        }
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        y = glutGet(GLUT_WINDOW_HEIGHT) - y;

        for (Button& btn : buttons) {
            if (x >= btn.x && x <= btn.x + btn.width &&
                y >= btn.y && y <= btn.y + btn.height) {

                if (btn.label == "Camera Control") {
                    toggleCameraControl();
                }
                else if (btn.isShapeButton) {
                    // Update all shape buttons
                    for (Button& shapeBtn : buttons) {
                        if (shapeBtn.isShapeButton) {
                            shapeBtn.selected = (shapeBtn.shape == btn.shape);
                        }
                    }
                    currentShape = btn.shape;
                }
                else {
                    // Only allow transformation mode changes when not in camera control mode
                    if (!cameraControlMode) {
                        // Update all transform buttons
                        for (Button& transformBtn : buttons) {
                            if (!transformBtn.isShapeButton) {
                                transformBtn.selected = (transformBtn.mode == btn.mode);
                            }
                        }
                        currentMode = btn.mode;
                    }
                }
                glutPostRedisplay();
                break;
            }
        }
    }
}


void keyboard(unsigned char key, int x, int y) {
    float moveSpeed = 0.1f;
    float rotateSpeed = 5.0f;
    float scaleSpeed = 0.1f;
    float shearSpeed = 0.1f;

    // Global controls that work in any mode
    switch (key) {
    case 'c':  // Toggle camera control mode
        toggleCameraControl();
        glutPostRedisplay();
        return;
    case 27:  // ESC key - exit program
        exit(0);
        return;
    }

    if (cameraControlMode) {
        // Camera controls
        switch (key) {
        case 'w':  // Move forward
            cameraPos[0] += cameraFront[0] * cameraSpeed;
            cameraPos[1] += cameraFront[1] * cameraSpeed;
            cameraPos[2] += cameraFront[2] * cameraSpeed;
            break;
        case 's':  // Move backward
            cameraPos[0] -= cameraFront[0] * cameraSpeed;
            cameraPos[1] -= cameraFront[1] * cameraSpeed;
            cameraPos[2] -= cameraFront[2] * cameraSpeed;
            break;
        case 'a':  // Strafe left
        {
            float right[] = {
                cameraFront[1] * cameraUp[2] - cameraFront[2] * cameraUp[1],
                cameraFront[2] * cameraUp[0] - cameraFront[0] * cameraUp[2],
                cameraFront[0] * cameraUp[1] - cameraFront[1] * cameraUp[0]
            };
            float length = sqrt(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
            right[0] /= length;
            right[1] /= length;
            right[2] /= length;
            cameraPos[0] -= right[0] * cameraSpeed;
            cameraPos[1] -= right[1] * cameraSpeed;
            cameraPos[2] -= right[2] * cameraSpeed;
        }
        break;
        case 'd':  // Strafe right
        {
            float right[] = {
                cameraFront[1] * cameraUp[2] - cameraFront[2] * cameraUp[1],
                cameraFront[2] * cameraUp[0] - cameraFront[0] * cameraUp[2],
                cameraFront[0] * cameraUp[1] - cameraFront[1] * cameraUp[0]
            };
            float length = sqrt(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
            right[0] /= length;
            right[1] /= length;
            right[2] /= length;
            cameraPos[0] += right[0] * cameraSpeed;
            cameraPos[1] += right[1] * cameraSpeed;
            cameraPos[2] += right[2] * cameraSpeed;
        }
        break;
        case 'q':  // Move up
            cameraPos[1] += cameraSpeed;
            break;
        case 'e':  // Move down
            cameraPos[1] -= cameraSpeed;
            break;
        case ' ':  // Reset camera position
            cameraPos[0] = 0.0f;
            cameraPos[1] = 0.0f;
            cameraPos[2] = 5.0f;
            cameraFront[0] = 0.0f;
            cameraFront[1] = 0.0f;
            cameraFront[2] = -1.0f;
            break;
        }
    }
    else {
        // Object transformation controls
        switch (currentMode) {
        case TRANSLATE:
            switch (key) {
            case 'w': position[1] += moveSpeed; break;
            case 's': position[1] -= moveSpeed; break;
            case 'a': position[0] -= moveSpeed; break;
            case 'd': position[0] += moveSpeed; break;
            case 'q': position[2] -= moveSpeed; break;
            case 'e': position[2] += moveSpeed; break;
            case ' ': position[0] = position[1] = position[2] = 0.0f; break;
            }
            break;

        case ROTATE:
            switch (key) {
            case 'w': rotation[0] += rotateSpeed; break;
            case 's': rotation[0] -= rotateSpeed; break;
            case 'a': rotation[1] -= rotateSpeed; break;
            case 'd': rotation[1] += rotateSpeed; break;
            case 'q': rotation[2] -= rotateSpeed; break;
            case 'e': rotation[2] += rotateSpeed; break;
            case ' ': rotation[0] = rotation[1] = rotation[2] = 0.0f; break;
            }
            break;

        case SCALE:
            switch (key) {
            case 'w': scale[1] += scaleSpeed; break;
            case 's': scale[1] = std::max(0.1f, scale[1] - scaleSpeed); break;
            case 'a': scale[0] = std::max(0.1f, scale[0] - scaleSpeed); break;
            case 'd': scale[0] += scaleSpeed; break;
            case 'q': scale[2] = std::max(0.1f, scale[2] - scaleSpeed); break;
            case 'e': scale[2] += scaleSpeed; break;
            case ' ': scale[0] = scale[1] = scale[2] = 1.0f; break;
            }
            break;

        case SHEAR:
            switch (key) {
            case 'w': shear[0] += shearSpeed; break;
            case 's': shear[0] -= shearSpeed; break;
            case 'a': shear[1] += shearSpeed; break;
            case 'd': shear[1] -= shearSpeed; break;
            case 'q': shear[2] += shearSpeed; break;
            case 'e': shear[2] -= shearSpeed; break;
            case ' ': shear[0] = shear[1] = shear[2] = 0.0f; break;
            }
            break;

        case REFLECT:
            switch (key) {
            case 'x': reflection[0] = !reflection[0]; break;
            case 'y': reflection[1] = !reflection[1]; break;
            case 'z': reflection[2] = !reflection[2]; break;
            case ' ': reflection[0] = reflection[1] = reflection[2] = false; break;
            }
            break;
        }
    }

    
    updateTransformMatrix();
    glutPostRedisplay();
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    
    int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);

    
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("3D Cube Transformation");

    
    glutFullScreen();

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}