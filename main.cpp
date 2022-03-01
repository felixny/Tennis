#ifdef __APPLE__
/* Defined before OpenGL and GLUT includes to avoid deprecation messages */
#define GL_SILENCE_DEPRECATION
#endif

#include "GL/glut.h"
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

#define WINDOW_WIDTH (1280)
#define WINDOW_HEIGHT (720)

#define GROUND_Y (WINDOW_HEIGHT * 3 / 4)

#define BALL_POSITION_MAX (60 * 3)

enum { PLAYER_1, PLAYER_2, PLAYER_NONE, PLAYER_MAX = PLAYER_NONE };

typedef struct {
  glm::vec2 positions[BALL_POSITION_MAX];
  glm::vec2 velocity;
} BALL;

glm::vec2 mousePosition;
BALL ball;
glm::vec2 netPositions[2]{{WINDOW_WIDTH / 2, GROUND_Y},
                          {WINDOW_WIDTH / 2, GROUND_Y - 200}};

glm::vec2 servePosition[PLAYER_MAX] = {
    {100, GROUND_Y - 100},                // p1
    {WINDOW_WIDTH - 100, GROUND_Y - 100}  // p2
};

float cross(glm::vec2 _v0, glm::vec2 _v1) {
  return _v0.x * _v1.y - _v1.x * _v0.y;
}

bool IntersectLineSegment(glm::vec2 _v0Start, glm::vec2 _v0End,
                          glm::vec2 _v1Start, glm::vec2 _v1End) {
  glm::vec2 v0 = _v0End - _v0Start;
  glm::vec2 v0StartToV1Start = _v1Start - _v0Start;
  glm::vec2 v0StartToV1End = _v1End - _v0Start;
  float cross0 = cross(v0, v0StartToV1Start);
  float cross1 = cross(v0, v0StartToV1End);
  if (cross0 * cross1 < 0) {
    glm::vec2 v1 = _v1End - _v1Start;
    glm::vec2 v1StartToVStart = _v0Start - _v1Start;
    glm::vec2 v1StartToV0End = _v0End - _v1Start;
    float cross2 = cross(v1, v1StartToVStart);
    float cross3 = cross(v1, v1StartToV0End);
    if (cross2 * cross3 < 0) return true;
  }
  return false;
}

void Display() {
  glClearColor(0.25f, 0.25f, 0.25f, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  GLfloat range[2];
  glGetFloatv(GL_LINE_WIDTH_RANGE, range);
  glLineWidth(range[1]);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_BLEND);

  glColor3ub(0x00, 0xff, 0xff);
  glBegin(GL_LINES);
  {
    glVertex2f(0, GROUND_Y);
    glVertex2f(WINDOW_WIDTH, GROUND_Y);

    /* if (IntersectLineSegment(mousePosition, ball.positions[0],
      netPositions[0], netPositions[1])) glColor3ub(0xff, 0xff, 0x00); */
    glVertex2fv((GLfloat*)&netPositions[0]);
    glVertex2fv((GLfloat*)&netPositions[1]);
  }
  glEnd();

  glColor3ub(0xff, 0x00, 0x00);
  glBegin(GL_LINES);
  {
    glVertex2fv((GLfloat*)&mousePosition);
    glVertex2fv((GLfloat*)&ball.positions[0]);
  }
  glEnd();

  glBegin(GL_LINE_STRIP);
  for (int i = 0; i < BALL_POSITION_MAX; i++) {
    float a = (float)(BALL_POSITION_MAX - i) / BALL_POSITION_MAX * 0.5;
    glColor4f(0, 1, 1, a);
    glVertex2fv((GLfloat*)&ball.positions[i]);
  }

  glEnd();

  glPushMatrix();
  {
    glTranslatef(ball.positions[0].x, ball.positions[0].y, 0);
    glBegin(GL_TRIANGLE_FAN);
    {
      glColor3ub(0x00, 0xff, 0xff);
      glVertex2f(0, 0);
      int n = 32;
      glColor4ub(0x00, 0xff, 0xff, 0x00);
      for (int i = 0; i <= n; i++) {
        float r = glm::pi<float>() * 2 * i / n;
        glm::vec2 v = glm::vec2(cosf(r), sinf(r)) * 15.0f;
        glVertex2fv((GLfloat*)&v);
      }
    }
    glEnd();
  }
  glPopMatrix();

  glutSwapBuffers();
}

void Idle() {
  for (int i = BALL_POSITION_MAX - 1; i > 0; i--)
    ball.positions[i] = ball.positions[i - 1];

  glm::vec2 lastBallPosition = ball.positions[0];

  ball.velocity.y += 0.02f;
  ball.positions[0] += ball.velocity;

  if (ball.positions[0].y > GROUND_Y) {
    ball.positions[0].y = GROUND_Y;
    ball.velocity.y *= -0.8f;
  }

  {
    if (IntersectLineSegment(lastBallPosition, ball.positions[0],
                             netPositions[0], netPositions[1])) {
      ball.positions[0].x = lastBallPosition.x;
      ball.velocity.x *= -0.5f;
    }
  }
  glutPostRedisplay();
}

void PassiveMotion(int _x, int _y) { mousePosition = {_x, _y}; }

void Mouse(int, int _mode, int, int) {
  if (_mode != GLUT_DOWN) return;

  glm::vec2 v = ball.positions[0] - mousePosition;
  v = glm::normalize(v);
  ball.velocity = v * 5.0f;
}

void Init() {
  for (int i = 0; i < BALL_POSITION_MAX; i++)
    ball.positions[i] = servePosition[PLAYER_1];
  ball.velocity = {};
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE);
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  glutCreateWindow("Tennis");
  glutDisplayFunc(Display);
  glutIdleFunc(Idle);
  glutPassiveMotionFunc(PassiveMotion);
  glutMouseFunc(Mouse);
  Init();
  glutMainLoop();
}