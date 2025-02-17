/*
 * Interface.cpp
 *
 *  Created on: 27 févr. 2012
 *      Author: Ben
 */

#include "interface.h"
#include "../utils/trace.h"
#include "../events/scheduler.h"

#define __STDC_FORMAT_MACROS

#include <cinttypes>

#define ID_SW_BUTTON_OPEN    1001
#define ID_SW_BUTTON_CLOSE    1002
#define ID_SW_BUTTON_SIZE    1004
#define ID_SW_SLD            1003
#define ID_SD_BUTTON_OPEN    2001
#define ID_SD_BUTTON_CLOSE    2002
#define ID_SD_INPUT            2004
#define ID_SD_SLD            2003

const int SLIDING_WINDOW_STANDARD_WIDTH = 400;
const int SLIDING_WINDOW_LARGE_WIDTH = 600;

GlutWindow::GlutWindow(GlutWindow *parent, GLuint pid, GLint px, GLint py,
                       GLint pw, GLint ph, const char *titreTexture)
        : id(pid) {
    if (parent) parent->addChild(this);
    isVisible = true;
    setGeometry(px, py, pw, ph);
    if (titreTexture) {
        if (pw == 0 || ph == 0) {
            idTexture = loadTexture(titreTexture, w, h);
        } else {
            int iw, ih;
            idTexture = loadTexture(titreTexture, iw, ih);
        }
    } else {
        idTexture = 0;
    }
}

GlutWindow::~GlutWindow() {
    clearChildren();
}

void GlutWindow::addChild(GlutWindow *child) {
    children.push_back(child);
}

void GlutWindow::clearChildren() {
    vector<GlutWindow *>::const_iterator cw = children.begin();
    while (cw != children.end()) {
        delete (*cw);
        cw++;
    }
    children.clear();
}

void GlutWindow::glDraw() {
    vector<GlutWindow *>::const_iterator cw = children.begin();
    while (cw != children.end()) {
        if ((*cw)->isVisible) {
            glPushMatrix();
            glTranslatef(x, y, 0);
            (*cw)->glDraw();
            glPopMatrix();
        }
        cw++;
    }
}

int GlutWindow::keyFunc(int charcode) {
    int id = 0;

    vector<GlutWindow *>::const_iterator cw = children.begin();
    while (cw != children.end()) {
        id = (*cw)->keyFunc(charcode);
        if (id != 0) return id;
        cw++;
    }
    return id;
}


int GlutWindow::mouseFunc(int button, int state, int mx, int my) {
    int id = 0;

    if (mx >= x && mx <= x + w && my >= y && my <= y + h) {
        vector<GlutWindow *>::const_iterator cw = children.begin();
        while (cw != children.end()) {
            id = (*cw)->mouseFunc(button, state, mx - x, my - y);
            if (id != 0) return id;
            cw++;
        }
        return id;
    }
    return 0;
}

bool GlutWindow::passiveMotionFunc(int mx, int my) {
    vector<GlutWindow *>::const_iterator cw = children.begin();
    while (cw != children.end()) {
        if ((*cw)->passiveMotionFunc(mx - x, my - y)) return true;
        cw++;
    }
    return false;
}


void GlutWindow::bindTexture() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, idTexture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/***************************************************************************************/
/** drawString(x,y,str,mode,height                                                     */
/** /param x,y : position of first line of the string                                  */
/** /param str : string, must be ended by \0                                           */
/** /param mode : glutBitmapCharacter decoration mode, default is GLUT_BITMAP_8_BY_13  */
/** /param height : height of lines for multiline texts, default is 13				   */
GLfloat GlutWindow::drawString(GLfloat x, GLfloat y, const char *str, void *mode, GLint height) {
    glRasterPos2f(x, y);
    while (*str) {
        if (*str == '\n') {
            y -= height;
            glRasterPos2f(x, y);
        } else {
            glutBitmapCharacter(mode, *str);
        }
        str++;
    }
    return y - height;
}

/***************************************************************************************/
/** drawString(x,y,str,TextMode                                                        */
/** /param x,y : position of first line of the string                                  */
/** /param str : string                                                                */
/** /param mode : character decoration mode                                            */
GLfloat GlutWindow::drawString(GLfloat x0, GLfloat y, const char *str, TextMode mode) {
    glRasterPos2f(x0, y);
    void *tm = (currentTextSize == TextSize::TEXTSIZE_STANDARD) ? GLUT_BITMAP_9_BY_15 : GLUT_BITMAP_TIMES_ROMAN_24;
    GLint height = (currentTextSize == TextSize::TEXTSIZE_STANDARD) ? 15 : 24;
    GLfloat x = x0;

    bool isBold = (mode == TextMode::TEXTMODE_BOLD || mode == TextMode::TEXTMODE_TITLE ||
                   mode == TextMode::TEXTMODE_POPUP);
    while (*str) {
        if (*str == '\n') {
            y -= height;
            glRasterPos2f(x0, y);
        } else {
            glRasterPos2d(x, y);
            glutBitmapCharacter(tm, *str);
            if (isBold) {
                glRasterPos2d(x + 1, y);
                glutBitmapCharacter(tm, *str);
            }
            x += glutBitmapWidth(tm, *str);
        }
        str++;
    }
    return y - height;
}

void GlutWindow::setTextSize(TextSize ts) {
    currentTextSize = ts;
}

/***************************************************************************************/
/* GlutSlidingMainWindow */
/***************************************************************************************/
GlutSlidingMainWindow::GlutSlidingMainWindow(GLint px, GLint py, GLint pw, GLint ph, const char *titreTexture) :
        GlutWindow(NULL, 1, px, py, pw, ph, titreTexture) {
    openningLevel = 0;
    buttonOpen = new GlutButton(this, ID_SW_BUTTON_OPEN, 5, 68, 32, 32,
                                "../../simulatorCore/resources/textures/UITextures/boutons_fg.tga");
    buttonClose = new GlutButton(this, ID_SW_BUTTON_CLOSE, 5, 26, 32, 32,
                                 "../../simulatorCore/resources/textures/UITextures/boutons_fd.tga", false);
    buttonSize = new GlutButton(this, ID_SW_BUTTON_SIZE, pw + SLIDING_WINDOW_STANDARD_WIDTH - 40, ph - 40, 32, 32,
                                "../../simulatorCore/resources/textures/UITextures/boutons_zoom.tga");
    slider = new GlutSlider(this, ID_SW_SLD, pw + SLIDING_WINDOW_STANDARD_WIDTH - 20, 5, ph - 60,
                            "../../simulatorCore/resources/textures/UITextures/slider.tga", (ph - 60) / 13);
    selectedGlBlock = NULL;
    slider->isVisible = false;
    buttonSize->isVisible = false;
}

GlutSlidingMainWindow::~GlutSlidingMainWindow() {
    // clean the map
    multimap<Time, BlockDebugData *>::iterator it = traces.begin();
    while (it != traces.end()) {
        delete (*it).second;
        ++it;
    }
    traces.clear();
}

void GlutSlidingMainWindow::glDraw() {
    if (!isVisible) return;
    // drawing of the tab
    bindTexture();
    glPushMatrix();
    glTranslatef(x, y, 0.0);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0.0);
    glVertex2i(0, 0);
    glTexCoord2f(0.3125, 0.);
    glVertex2i(40, 0);
    glTexCoord2f(0.3125, 1);
    glVertex2i(40, 128);
    glTexCoord2f(0, 1);
    glVertex2i(0, 128);
    glEnd();

    if (openningLevel) {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.25, 0.25, 0.25, 0.75);
        glBegin(GL_QUADS);
        //glTexCoord2f(0.3125,0.);
        glVertex2i(40, 0);
        //glTexCoord2f(1.0,0.);
        glVertex2i(w, 0);
        //glTexCoord2f(1.0,1.0);
        glVertex2i(w, h);
        //glTexCoord2f(0.3125,1.0);
        glVertex2i(40, h);
        glEnd();
        char str[256];
        Time t = BaseSimulator::getScheduler()->now();
        if (t < 10 * 1000 * 1000) { //10sec
            sprintf(str, "Current time : %d:%d ms", int(t / 1000), int((t % 1000))); // ms
        } else {
            sprintf(str, "Current time : %d:%d s", int(t / 1000000), int((t % 1000000) / 10000)); // s
        }
        glColor3f(1.0, 1.0, 0.0);
        drawString(42.0, h - 20.0, str, TextMode::TEXTMODE_TITLE);
        glColor3f(1.0, 1.0, 1.0);
        if (selectedGlBlock) {
            GLfloat posy = h - 40;
            int ss = 0; // index of slice start
            unsigned int str_length;
            sprintf(str, "Selected Block : %s", selectedGlBlock->getInfo().c_str());
            str_length = strlen(str);
            for (unsigned int i = 0; i < str_length + 1; i++) { // draw lines one at a time
                if (str[i] == '\n' || str[i] == '\0') {
                    str[i] = '\0';
                    posy = drawString(42.0, posy, str + ss, TextMode::TEXTMODE_TITLE);
                    ss = i + 1;
                }
            }
            multimap<Time, BlockDebugData *>::iterator it = traces.begin();
            //GLfloat posy = h-65;
            stringstream line;
            int pos = slider->getPosition();
            int s, cs;
            while (it != traces.end() && posy > 0) {
                if (((*it).second)->blockId == selectedGlBlock->blockId) {
                    if (pos) {
                        pos--;
                    } else {
                        ((*it).second)->color.glColor();
                        line.str("");
                        s = (*it).first / 1000;
                        cs = ((*it).first % 1000);
                        line << "[" << s << ":" << cs << "] " << ((*it).second)->str;
                        posy = drawString(42.0, posy, line.str().c_str(), TextMode::TEXTMODE_STANDARD);
                    }
                }
                ++it;
            }
        } else {
            sprintf(str, "Selected Block : None (use [Ctrl]+click)");
            drawString(42.0, h - 40.0, str, TextMode::TEXTMODE_TITLE);
            multimap<Time, BlockDebugData *>::iterator it = traces.begin();
            GLfloat posy = h - 65;
            stringstream line;
            int pos = slider->getPosition();
            while (it != traces.end() && pos--) {
                it++;
            };

            int s, cs;
            while (it != traces.end() && posy > 0) {
                ((*it).second)->color.glColor();
                line.str("");
                s = (*it).first / 1000;
                cs = ((*it).first % 1000);
                line << "[" << s << ":" << cs << "] #" << ((*it).second)->blockId << ":" << ((*it).second)->str;
                posy = drawString(42.0, posy, line.str().c_str(), TextMode::TEXTMODE_STANDARD);
                ++it;
            }
        }
    }
    glPopMatrix();
    GlutWindow::glDraw();
}

bool GlutSlidingMainWindow::passiveMotionFunc(int mx,int my) {
    bool showButtons = mx>x || openningLevel>0;
    if (showButtons!=isVisible) {
        isVisible=showButtons;
        return true;
    }

    return false;
}


int GlutSlidingMainWindow::mouseFunc(int button, int state, int mx, int my) {
    int n = GlutWindow::mouseFunc(button, state, mx, my);
    if (state == GLUT_UP) {
        switch (n) {
            case ID_SW_BUTTON_OPEN :
                setOpenCloseButtonPosition(true);
                break;
            case ID_SW_BUTTON_CLOSE :
                setOpenCloseButtonPosition(false);
                break;
            case ID_SW_BUTTON_SIZE :
                if (currentTextSize == TextSize::TEXTSIZE_LARGE) {
                    setTextSize(TextSize::TEXTSIZE_STANDARD);
                } else {
                    setTextSize(TextSize::TEXTSIZE_LARGE);
                }
                break;
        }
    }
    return n;
}

void GlutSlidingMainWindow::openClose() {
    if (openningLevel == 2) { // close if open twice
        setOpenCloseButtonPosition(false);
        setOpenCloseButtonPosition(false);
    } else {
        setOpenCloseButtonPosition(true);
    }
    isVisible=openningLevel>0;
}

void GlutSlidingMainWindow::setOpenCloseButtonPosition(bool openning) {
    int sliding = (currentTextSize == TextSize::TEXTSIZE_STANDARD) ? SLIDING_WINDOW_STANDARD_WIDTH
                                                                   : SLIDING_WINDOW_LARGE_WIDTH;
    if (openning) {
        openningLevel++;
        x -= sliding;
        w += sliding;
        if (openningLevel == 2) buttonOpen->activate(false);
        buttonClose->activate(true);
    } else {
        openningLevel--;
        x += sliding;
        w -= sliding;
        buttonOpen->activate(true);
        if (openningLevel == 0) buttonClose->activate(false);
    }
    updateSliderWindow();
}

void GlutSlidingMainWindow::reshapeFunc(int mx, int my, int mw, int mh) {
    int sz = openningLevel * ((currentTextSize == TextSize::TEXTSIZE_STANDARD) ? SLIDING_WINDOW_STANDARD_WIDTH
                                                                               : SLIDING_WINDOW_LARGE_WIDTH);
    w = 40 + sz;
    x = mx + mw - w;
    y = my;
    h = mh;
    updateSliderWindow();
}

void GlutSlidingMainWindow::updateSliderWindow() {
    slider->setGeometry(w - 20, 5, 11, h - 60);
    slider->isVisible = (openningLevel > 0);
    buttonSize->isVisible = (openningLevel > 0);
    buttonSize->setGeometry(w - 40, h - 40, 32, 32);
    slider->update();
}

void GlutSlidingMainWindow::addTrace(bID id, const string &str, const Color &color) {
    BlockDebugData *bdd = new BlockDebugData(id, str, color);
    traces.insert(pair<Time, BlockDebugData *>(BaseSimulator::getScheduler()->now(), bdd));
    if (selectedGlBlock) {
        if (selectedGlBlock->blockId == id) slider->incDataTextLines();
    } else {
        slider->incDataTextLines();
    }
}

void GlutSlidingMainWindow::select(GlBlock *sb) {
    selectedGlBlock = sb;
    if (selectedGlBlock) {
        int n = 0;
        multimap<Time, BlockDebugData *>::iterator it = traces.begin();
        while (it != traces.end()) {
            if (((*it).second)->blockId == selectedGlBlock->blockId) {
                n++;
            }
            ++it;
        }
        slider->setDataTextLines(n);
    } else {
        slider->setDataTextLines(traces.size());
    }
};

void GlutSlidingMainWindow::setTextSize(TextSize ts) {
    // first close the window
    int sliding = (currentTextSize == TextSize::TEXTSIZE_STANDARD) ? SLIDING_WINDOW_STANDARD_WIDTH
                                                                   : SLIDING_WINDOW_LARGE_WIDTH;
    x += sliding * openningLevel;
    w -= sliding * openningLevel;

    currentTextSize = ts;
    // reopen window
    sliding = (currentTextSize == TextSize::TEXTSIZE_STANDARD) ? SLIDING_WINDOW_STANDARD_WIDTH
                                                               : SLIDING_WINDOW_LARGE_WIDTH;
    x -= sliding * openningLevel;
    w += sliding * openningLevel;

    updateSliderWindow();
}


/***************************************************************************************/
/* GlutSlidingDebugWindow */
/***************************************************************************************/
GlutSlidingDebugWindow::GlutSlidingDebugWindow(GLint px, GLint py, GLint pw, GLint ph, const char *titreTexture) :
        GlutWindow(NULL, 2, px, py, pw, ph, titreTexture) {
    openningLevel = 0;
    buttonOpen = new GlutButton(this, ID_SD_BUTTON_OPEN, 5, 168, 32, 32,
                                "../../simulatorCore/resources/textures/UITextures/boutons_fg.tga");
    buttonClose = new GlutButton(this, ID_SD_BUTTON_CLOSE, 5, 126, 32, 32,
                                 "../../simulatorCore/resources/textures/UITextures/boutons_fd.tga", false);
    slider = new GlutSlider(this, ID_SD_SLD, pw + 400 - 20, 5, ph - 75,
                            "../../simulatorCore/resources/textures/UITextures/slider.tga", (ph - 60) / 13);
    input = new GlutInputWindow(this, ID_SD_INPUT, pw + 10, ph - 66, 380, 36);
    debugId = 1;
}

GlutSlidingDebugWindow::~GlutSlidingDebugWindow() {
    // clean the vector
    vector<BlockDebugData *>::iterator it = tabDebug.begin();
    while (it != tabDebug.end()) {
        delete (*it);
        ++it;
    }
    tabDebug.clear();
}

void GlutSlidingDebugWindow::glDraw() {
    // drawing of the tab
    bindTexture();
    glPushMatrix();
    glTranslatef(x, y, 0.0);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0.0);
    glVertex2i(0, 100);
    glTexCoord2f(0.3125, 0.);
    glVertex2i(40, 100);
    glTexCoord2f(0.3125, 1);
    glVertex2i(40, 228);
    glTexCoord2f(0, 1);
    glVertex2i(0, 228);
    glEnd();

    if (openningLevel) {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.7, 0.1, 0.1, 0.75);
        glBegin(GL_QUADS);
        glVertex2i(40, 0);
        glVertex2i(w, 0);
        glVertex2i(w, h);
        glVertex2i(40, h);
        glEnd();

        glColor3f(0.25, 0.25, 0.25);
        glBegin(GL_QUADS);
        glVertex2i(50, 5);
        glVertex2i(w - 25, 5);
        glVertex2i(w - 25, h - 70);
        glVertex2i(50, h - 70);
        glEnd();

        char str[256];
        Time t = BaseSimulator::getScheduler()->now();
        sprintf(str, "Current time : %d:%d", int(t / 1000), int((t % 1000)));
        glColor3f(1.0, 1.0, 0.0);
        drawString(45.0, h - 20.0, str);
        if (input->hasFocus) {
            drawString(w - 85, h - 20.0, "DEBUG MODE");
            char c[6];
            sprintf(c, "%" PRIu32, debugId);
            drawString(w / 2 - 45, h - 20.0, c);
        }

        GLfloat posy = h - 80;
        vector<BlockDebugData *>::reverse_iterator ci = tabDebug.rbegin();
        while (ci != tabDebug.rend() && posy > 0) {
            (*ci)->color.glColor();
            posy = drawString(55.0, posy, (*ci)->str.c_str());
            ci++;
        }
    }
    glPopMatrix();
    GlutWindow::glDraw();
}

int GlutSlidingDebugWindow::mouseFunc(int button, int state, int mx, int my) {
    int n = GlutWindow::mouseFunc(button, state, mx, my);
    switch (n) {
        case ID_SD_BUTTON_OPEN :
            openningLevel++;
            x -= 400;
            w += 400;
            buttonOpen->activate(false);
            buttonClose->activate(true);
            break;
        case ID_SD_BUTTON_CLOSE :
            openningLevel--;
            x += 400;
            w -= 400;
            buttonOpen->activate(true);
            buttonClose->activate(false);
            break;
        case ID_SD_INPUT :
            input->hasFocus = (getScheduler()->getMode() == SCHEDULER_MODE_DEBUG);
            break;
    }
    return n;
}

int GlutSlidingDebugWindow::keyFunc(int charcode) {
    bID id = 0;

    vector<GlutWindow *>::const_iterator cw = children.begin();
    while (cw != children.end()) {
        id = (*cw)->keyFunc(charcode);
        if (id == 1) return id;
        if (id == 2) { // validation
            string str = input->getTextAndClear();
            if (!str.empty()) {
                string result;
                input->hasFocus = getScheduler()->debug(str, debugId, result);
                BlockDebugData *bdd = new BlockDebugData(debugId, result, LIGHTBLUE);
                tabDebug.push_back(bdd);
                bdd = new BlockDebugData(debugId, str, YELLOW);
                tabDebug.push_back(bdd);
            }
        }
        cw++;
    }
    return id;
}

void GlutSlidingDebugWindow::reshapeFunc(int mx, int my, int mw, int mh) {
    int sz = 400 * openningLevel;
    setGeometry(mx - sz, my, mw + sz, mh);
    slider->setGeometry(mw + 400 - 20, 5, 11, mh - 75);
    slider->update();
    input->setGeometry(mw + 10, mh - 66, 380, 30);
}

/***************************************************************************************/
/* GlutButton */
/***************************************************************************************/

GlutButton::GlutButton(GlutWindow *parent, GLuint pid, GLint px, GLint py, GLint pw, GLint ph, const char *titreTexture,
                       bool pia) :
        GlutWindow(parent, pid, px, py, pw, ph, titreTexture) {
    isActive = pia;
    isDown = false;
    isHighlighted = false;
}

void GlutButton::addSubMenu(GlutWindow *child) {
    addChild(child);
    child->isVisible = false;
}


void GlutButton::glDraw() {
    GLfloat tx = 0, ty = 0.5;

    bindTexture();
    if (isActive) {
        if (isDown) {
            tx = 0.0;
            ty = 0.0;
        } else if (isHighlighted) {
            tx = 0.5;
            ty = 0.0;
        } else {
            tx = 0.5;
            ty = 0.5;
        }
    }
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(tx, ty);
    glVertex2i(0, 0);
    glTexCoord2f(tx + 0.5f, ty);
    glVertex2i(w, 0);
    glTexCoord2f(tx + 0.5f, ty + 0.5f);
    glVertex2i(w, h);
    glTexCoord2f(tx, ty + 0.5f);
    glVertex2i(0, h);
    glEnd();
    glPopMatrix();
}

int GlutButton::mouseFunc(int button, int state, int mx, int my) {
    isHighlighted = (mx > x && mx < x + w && my > y && my < y + h);
    isDown = false;
    if (isHighlighted) {
        isDown = (state == GLUT_DOWN);
        return (isActive && state == GLUT_UP) ? id : 0;
    }
    return 0;
}

bool GlutButton::passiveMotionFunc(int mx, int my) {
    isHighlighted = (mx > x && mx < x + w && my > y && my < y + h);
    return isHighlighted;
}

/***************************************************************************************/
/* GlutRotationButton */
/***************************************************************************************/

GlutRotationButton::GlutRotationButton(GlutWindow *parent, GLuint pid, GLint px, GLint py, GLint pw, GLint ph,
                                       const char *titreTexture, bool blue, uint8_t idSrc, uint8_t idDest,
                                       Cell3DPosition &pos, short orientation, float cw) : GlutWindow(parent, pid, px,
                                                                                                      py, pw, ph,
                                                                                                      titreTexture) {
    isBlue = blue;
    id0 = idSrc;
    id1 = idDest;
    isHighlighted = false;
    finalPosition = pos;
    finalOrientation = orientation;
    characterWidth = cw;
}

void GlutRotationButton::glDraw() {
    GLfloat tx = 0.5 * (isBlue) + 0.25 * (isHighlighted),
            ty = 0.0;

    // draw background
    bindTexture();
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(tx, ty);
    glVertex2i(0, 0);
    glTexCoord2f(tx + 0.25f, ty);
    glVertex2i(w, 0);
    glTexCoord2f(tx + 0.25f, ty + 0.5f);
    glVertex2i(w, h);
    glTexCoord2f(tx, ty + 0.5f);
    glVertex2i(0, h);
    glEnd();
    // draw X->Y
    glDisable(GL_DEPTH_TEST);
    // first id
    tx = id0 * characterWidth;
    ty = 0.5;
    glBegin(GL_QUADS);
    glTexCoord2f(tx, ty);
    glVertex2i(0.125 * w, 0);
    glTexCoord2f(tx + characterWidth, ty);
    glVertex2i(0.375 * w, 0);
    glTexCoord2f(tx + characterWidth, ty + 0.5f);
    glVertex2i(0.375 * w, h);
    glTexCoord2f(tx, ty + 0.5f);
    glVertex2i(0.125 * w, h);
    glEnd();
    // second id
    tx = id1 * characterWidth;
    glBegin(GL_QUADS);
    glTexCoord2f(tx, ty);
    glVertex2i(0.625 * w, 0);
    glTexCoord2f(tx + characterWidth, ty);
    glVertex2i(0.875 * w, 0);
    glTexCoord2f(tx + characterWidth, ty + 0.5f);
    glVertex2i(0.875 * w, h);
    glTexCoord2f(tx, ty + 0.5f);
    glVertex2i(0.625 * w, h);
    glEnd();

    glPopMatrix();
    glEnable(GL_DEPTH_TEST);

}

int GlutRotationButton::mouseFunc(int button, int state, int mx, int my) {
    isHighlighted = (mx > x && mx < x + w && my > y && my < y + h);
    return (isHighlighted && state == GLUT_UP) ? id : 0;
}

bool GlutRotationButton::passiveMotionFunc(int mx, int my) {
    isHighlighted = (mx > x && mx < x + w && my > y && my < y + h);
    return isHighlighted;
}

/***************************************************************************************/
/* GlutRBMotionButton */
/***************************************************************************************/

GlutRBMotionButton::GlutRBMotionButton(GlutWindow *parent, GLuint pid, GLint px, GLint py, GLint pw, GLint ph,
                                       const char *titreTexture, bool isRotation, uint8_t idDest, Cell3DPosition &pos,
                                       float cw) : GlutWindow(parent, pid, px, py, pw, ph, titreTexture) {
    isBlue = isRotation;
    actionID = 6 + isRotation;
    directionID = idDest;
    isHighlighted = false;
    finalPosition = pos;
    characterWidth = cw;
}

void GlutRBMotionButton::glDraw() {
    GLfloat tx = 0.5 * (isBlue) + 0.25 * (isHighlighted),
            ty = 0.0;

    // draw background
    bindTexture();
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(tx, ty);
    glVertex2i(0, 0);
    glTexCoord2f(tx + 0.25f, ty);
    glVertex2i(w, 0);
    glTexCoord2f(tx + 0.25f, ty + 0.5f);
    glVertex2i(w, h);
    glTexCoord2f(tx, ty + 0.5f);
    glVertex2i(0, h);
    glEnd();
    // draw -> X
    glDisable(GL_DEPTH_TEST);
    // first id
    tx = actionID * characterWidth;
    ty = 0.5;
    glBegin(GL_QUADS);
    glTexCoord2f(tx, ty);
    glVertex2i(0.125 * w, 0);
    glTexCoord2f(tx + characterWidth, ty);
    glVertex2i(0.375 * w, 0);
    glTexCoord2f(tx + characterWidth, ty + 0.5f);
    glVertex2i(0.375 * w, h);
    glTexCoord2f(tx, ty + 0.5f);
    glVertex2i(0.125 * w, h);
    glEnd();
    // second id
    tx = directionID * characterWidth;
    glBegin(GL_QUADS);
    glTexCoord2f(tx, ty);
    glVertex2i(0.625 * w, 0);
    glTexCoord2f(tx + characterWidth, ty);
    glVertex2i(0.875 * w, 0);
    glTexCoord2f(tx + characterWidth, ty + 0.5f);
    glVertex2i(0.875 * w, h);
    glTexCoord2f(tx, ty + 0.5f);
    glVertex2i(0.625 * w, h);
    glEnd();

    glPopMatrix();
    glEnable(GL_DEPTH_TEST);

}

int GlutRBMotionButton::mouseFunc(int button, int state, int mx, int my) {
    isHighlighted = (mx > x && mx < x + w && my > y && my < y + h);
    return (isHighlighted && state == GLUT_UP) ? id : 0;
}

bool GlutRBMotionButton::passiveMotionFunc(int mx, int my) {
    isHighlighted = (mx > x && mx < x + w && my > y && my < y + h);
    return isHighlighted;
}

/***************************************************************************************/
/* GlutRotation2DButton */
/***************************************************************************************/

GlutRotation2DButton::GlutRotation2DButton(GlutWindow *parent, GLuint pid, GLint px, GLint py, GLint pw, GLint ph,
                                           const char *titreTexture, bool blue, uint8_t idSrc, uint8_t idDest,
                                           Cell3DPosition &pos, short orientation, float cw) : GlutWindow(parent, pid,
                                                                                                          px, py, pw,
                                                                                                          ph,
                                                                                                          titreTexture) {
    isBlue = blue;
    id0 = idSrc;
    id1 = idDest;
    isHighlighted = false;
    finalPosition = pos;
    finalOrientation = orientation;
    characterWidth = cw;
}

void GlutRotation2DButton::glDraw() {
    GLfloat tx = 0.5 * (isBlue) + 0.25 * (isHighlighted),
            ty = 0.0;

    // draw background
    bindTexture();
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(tx, ty);
    glVertex2i(0, 0);
    glTexCoord2f(tx + 0.25f, ty);
    glVertex2i(w, 0);
    glTexCoord2f(tx + 0.25f, ty + 0.5f);
    glVertex2i(w, h);
    glTexCoord2f(tx, ty + 0.5f);
    glVertex2i(0, h);
    glEnd();
    // draw X->Y
    glDisable(GL_DEPTH_TEST);
    // first id
    tx = id0 * characterWidth;
    ty = 0.5;
    glBegin(GL_QUADS);
    glTexCoord2f(tx, ty);
    glVertex2i(0.125 * w, 0);
    glTexCoord2f(tx + characterWidth, ty);
    glVertex2i(0.375 * w, 0);
    glTexCoord2f(tx + characterWidth, ty + 0.5f);
    glVertex2i(0.375 * w, h);
    glTexCoord2f(tx, ty + 0.5f);
    glVertex2i(0.125 * w, h);
    glEnd();
    // second id
    tx = id1 * characterWidth;
    glBegin(GL_QUADS);
    glTexCoord2f(tx, ty);
    glVertex2i(0.625 * w, 0);
    glTexCoord2f(tx + characterWidth, ty);
    glVertex2i(0.875 * w, 0);
    glTexCoord2f(tx + characterWidth, ty + 0.5f);
    glVertex2i(0.875 * w, h);
    glTexCoord2f(tx, ty + 0.5f);
    glVertex2i(0.625 * w, h);
    glEnd();

    glPopMatrix();
    glEnable(GL_DEPTH_TEST);

}

int GlutRotation2DButton::mouseFunc(int button, int state, int mx, int my) {
    isHighlighted = (mx > x && mx < x + w && my > y && my < y + h);
    return (isHighlighted && state == GLUT_UP) ? id : 0;
}

bool GlutRotation2DButton::passiveMotionFunc(int mx, int my) {
    isHighlighted = (mx > x && mx < x + w && my > y && my < y + h);
    return isHighlighted;
}


/***************************************************************************************/
/* GlutPopupWindow */
/***************************************************************************************/

GlutPopupWindow::GlutPopupWindow(GlutWindow *parent, GLint px, GLint py, GLint pw, GLint ph)
        : GlutWindow(parent, 99, px, py, pw, ph, NULL) {
    isVisible = false;
}

void GlutPopupWindow::glDraw() {
    if (isVisible) {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor4f(1.0, 1.0, 0.0, 0.9);
        glPushMatrix();
        glTranslatef(x, y, 0);
        glBegin(GL_QUADS);
        glVertex2i(0, 0);
        glVertex2i(w, 0);
        glVertex2i(w, h);
        glVertex2i(0, h);
        glEnd();
        glColor4f(0.0, 0.0, 0.5, 1.0);
        drawString(5.0, h - 20.0, info.c_str(), TextMode::TEXTMODE_POPUP);
        glPopMatrix();
    }
}

/***************************************************************************************/
/* GlutPopupMenuWindow */
/***************************************************************************************/

GlutPopupMenuWindow::GlutPopupMenuWindow(GlutWindow *parent, GLint px, GLint py, GLint pw, GLint ph)
        : GlutWindow(parent, 49, px, py, pw, ph, NULL) {
    isVisible = false;
}

void GlutPopupMenuWindow::addButton(int i, const char *titre, GlutPopupMenuWindow *subMenuWindow) {
    int py = 0;
    std::vector<GlutWindow *>::const_iterator cb = children.begin();
    while (cb != children.end()) {
        if ((*cb)->isVisible) py += (*cb)->h + 5;
        cb++;
    }
    GlutButton *button = new GlutButton(this, i, 0, 0, 0, 0, titre);
    button->setGeometry(10, h - 5 - button->h / 2 - py, button->w / 2, button->h / 2);
    if (subMenuWindow) {
        button->addSubMenu(subMenuWindow);
    }
}

void GlutPopupMenuWindow::addButton(GlutWindow *button) {
    int py = 0;
    std::vector<GlutWindow *>::const_iterator cb = children.begin();
    while (cb != children.end()) {
        if ((*cb)->isVisible) py += (*cb)->h + 5;
        cb++;
    }
    addChild(button);
    button->setGeometry(10, h - 5 - button->h / 2 - py, button->w / 4, button->h / 2);
}


void GlutPopupMenuWindow::glDraw() {
    if (isVisible) {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor4f(1.0, 1.0, 0.0, 0.75);
        glPushMatrix();
        glTranslatef(x, y, 0);
        glBegin(GL_QUADS);
        glVertex2i(0, 0);
        glVertex2i(w, 0);
        glVertex2i(w, h);
        glVertex2i(0, h);
        glEnd();
        glPopMatrix();
        GlutWindow::glDraw();
    }
}

int GlutPopupMenuWindow::mouseFunc(int button, int state, int mx, int my) {
    if (!isVisible) return 0;
    int n = GlutWindow::mouseFunc(button, state, mx, my);
    return n;
}

void GlutPopupMenuWindow::activate(unsigned int id, bool value) {
    std::vector<GlutWindow *>::const_iterator cb = children.begin();
    while (cb != children.end() && (*cb)->id != id) {
        cb++;
    }
    if (cb != children.end()) {
        ((GlutButton *) (*cb))->activate(value);
    }
}

GlutWindow *GlutPopupMenuWindow::getButton(unsigned int id) {
    std::vector<GlutWindow *>::const_iterator cb = children.begin();
    while (cb != children.end() && (*cb)->id != id) {
        cb++;
    }
    if (cb != children.end()) {
        return (*cb);
    }
    return NULL;
}


/***************************************************************************************/
/* GlutHelpWindow */
/***************************************************************************************/
GlutHelpWindow::GlutHelpWindow(GlutWindow *parent, GLint px, GLint py, GLint pw, GLint ph, const char *textFile)
        : GlutWindow(parent, -1, px, py, pw, ph, NULL) {
    isVisible = false;
    text = NULL;

    //GlutButton *btn = new GlutButton(this, 999,pw-35,ph-35,32,32,"../../simulatorCore/resources/textures/UITextures/close.tga");

    ifstream fin(textFile);
    if (!fin) {
        cerr << "cannot open file " << textFile << endl;
    } else {
        stringstream out;
        out << fin.rdbuf();
        string strout = out.str();
        text = (unsigned char *) new char[strout.size() + 1];
        memcpy(text, strout.c_str(), strout.size());
        text[strout.size() - 1] = 0; // end of string
    }
}

GlutHelpWindow::~GlutHelpWindow() {
    delete[] text;
}

void GlutHelpWindow::glDraw() {
    if (isVisible) {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.5, 0.5, 0.5, 0.75);
        glPushMatrix();
        glTranslatef(x, y, 0);
        glBegin(GL_QUADS);
        glVertex2i(0, h - 40);
        glVertex2i(w, h - 40);
        glVertex2i(w, h);
        glVertex2i(0, h);
        glEnd();
        glColor4f(1.0, 1.0, 0.0, 0.75);
        glBegin(GL_QUADS);
        glVertex2i(0, 0);
        glVertex2i(w, 0);
        glVertex2i(w, h - 40);
        glVertex2i(0, h - 40);
        glEnd();
        glColor4f(0.0, 0.0, 0.0, 1.0);
        glRasterPos2f(10, h - 32);
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, text);
        glPopMatrix();
        GlutWindow::glDraw();
    }
}

int GlutHelpWindow::mouseFunc(int button, int state, int mx, int my) {
    if (!isVisible) return 0;
    int n = GlutWindow::mouseFunc(button, state, mx, my);
    switch (n) {
        case 999 :
            isVisible = false;
            break;
    }
    return n;
}

/***************************************************************************************/
/* GlutSlider */
/***************************************************************************************/
GlutSlider::GlutSlider(GlutWindow *parent, GLuint pid, GLint px, GLint py, GLint ph, const char *titreTexture, int ntl)
        : GlutWindow(parent, pid, px, py, 11, ph, titreTexture) {
    dataTextLines = 0;
    dataPosition = 0;
    nbreTextLines = ntl;
    update();
}

GlutSlider::~GlutSlider() {

}

void GlutSlider::update() {
    bool isFull = (dataTextLines == 0 || nbreTextLines >= dataTextLines);
    double s = isFull ? 1.0 : nbreTextLines / double(dataTextLines);
    buttonHeight = int(s * (h - 20));
    buttonY = isFull ? 10 : int((dataTextLines - dataPosition - nbreTextLines) * (h - 20) / double(dataTextLines)) + 10;
}

void GlutSlider::glDraw() {
    int byhy = buttonY + buttonHeight,
            byhy_2 = buttonY + buttonHeight / 2;
    glPushMatrix();
    glTranslatef(x, y, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(0.5, 0.5, 0.5);
    // free area inf
    glBegin(GL_QUADS);
    glVertex2i(0, 10);
    glVertex2i(w, 10);
    glVertex2i(w, buttonY);
    glVertex2i(0, buttonY);
    glEnd();
    // free area sup
    glBegin(GL_QUADS);
    glVertex2i(0, byhy);
    glVertex2i(w, byhy);
    glVertex2i(w, h - 10);
    glVertex2i(0, h - 10);
    glEnd();
    bindTexture();
    // button inf
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2i(0, 0);
    glTexCoord2f(1, 0);
    glVertex2i(w, 0);
    glTexCoord2f(1, 10.0 / 35.0);
    glVertex2i(w, 10);
    glTexCoord2f(0, 10.0 / 35.0);
    glVertex2i(0, 10);
    glEnd();
    // button sup
    glBegin(GL_QUADS);
    glTexCoord2f(0, 25.0 / 35.0);
    glVertex2i(0, h - 10);
    glTexCoord2f(1, 25.0 / 35.0);
    glVertex2i(w, h - 10);
    glTexCoord2f(1, 1);
    glVertex2i(w, h);
    glTexCoord2f(0, 1);
    glVertex2i(0, h);
    glEnd();

    glBegin(GL_QUAD_STRIP);
    glTexCoord2f(0, 11.0 / 35.0);
    glVertex2i(0, buttonY);
    glTexCoord2f(1, 11.0 / 35.0);
    glVertex2i(w, buttonY);
    glTexCoord2f(0, 12.0 / 35.0);
    glVertex2i(0, byhy_2 - 4);
    glTexCoord2f(1, 12.0 / 35.0);
    glVertex2i(w, byhy_2 - 4);
    glTexCoord2f(0, 23.0 / 35.0);
    glVertex2i(0, byhy_2 + 4);
    glTexCoord2f(1, 23.0 / 35.0);
    glVertex2i(w, byhy_2 + 4);
    glTexCoord2f(0, 24.0 / 35.0);
    glVertex2i(0, byhy);
    glTexCoord2f(1, 24.0 / 35.0);
    glVertex2i(w, byhy);
    glTexCoord2f(0, 25.0 / 35.0);
    glVertex2i(0, byhy);
    glTexCoord2f(1, 25.0 / 35.0);
    glVertex2i(w, byhy);
    glEnd();
    glPopMatrix();
}

int GlutSlider::mouseFunc(int button, int state, int mx, int my) {
    if (mouseDown) {
        bool isFull = (dataTextLines == 0 || nbreTextLines >= dataTextLines);
        if (!isFull) {
            buttonY += my - currentMousePos;
            currentMousePos = my;
            dataPosition = dataTextLines - nbreTextLines - int((buttonY - 10) * dataTextLines / (h - 20));
            if (dataPosition <= 0) {
                dataPosition = 0;
                buttonY = int((dataTextLines - nbreTextLines) * (h - 20) / double(dataTextLines)) + 10;;
            } else if (dataPosition > dataTextLines - nbreTextLines) {
                dataPosition = dataTextLines - nbreTextLines;
                buttonY = 10;
            }
        }
    } else {
        if (mx < x || mx > x + w || my < y || my > y + h) return 0;
        if (state == GLUT_DOWN) {
            mouseDown = true;
            if (my < 10) {
                // upper button
                dataPosition++;
                if (dataPosition > dataTextLines - nbreTextLines) dataPosition = dataTextLines - nbreTextLines;
                update();
            } else if (my > h - 10) {
                // lower button
                dataPosition--;
                if (dataPosition > dataTextLines - nbreTextLines) dataPosition = dataTextLines - nbreTextLines;
                update();
            } else if (my < y + buttonY) {
                // upper area
                dataPosition += 5;
                if (dataPosition > dataTextLines - nbreTextLines) dataPosition = dataTextLines - nbreTextLines;
                update();
            } else if (my > y + buttonY + buttonHeight) {
                // lower area
                dataPosition -= 5;
                if (dataPosition < 0) dataPosition = 0;
                update();
            } else {
                // button
                currentMousePos = my;
            }
        }
        return 1;
    }
    if (state == GLUT_UP) {
        mouseDown = false;
    }
    return 3;
}

/***************************************************************************************/
/* GlutInputWindow */
/***************************************************************************************/

GlutInputWindow::GlutInputWindow(GlutWindow *parent, GLuint pid, GLint px, GLint py, GLint pw, GLint ph) :
        GlutWindow(parent, pid, px, py, pw, ph, NULL) {
    hasFocus = false;
}

GlutInputWindow::~GlutInputWindow() {

}

void GlutInputWindow::glDraw() {
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(x, y, 0.0);
    if (hasFocus) {
        glColor3f(0.5, 0.5, 0.5);
    } else {
        glColor3f(0.25, 0.25, 0.25);
    }
    // petite fenetre de saisie
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(w, 0);
    glVertex2i(w, h);
    glVertex2i(0, h);
    glEnd();

    // tour de la fenetre
    glBegin(GL_LINES);
    glColor3f(0.0, 0.0, 0.0);
    glVertex2i(0, h);
    glVertex2i(0, 0);
    glVertex2i(w, h);
    glVertex2i(0, h);
    glColor3f(1.0, 1.0, 1.0);
    glVertex2i(w, 0);
    glVertex2i(w, h);
    glVertex2i(0, 0);
    glVertex2i(w, 0);
    glEnd();

    glColor3f(1.0f, 1.0f, 0);
    drawString(5, 8, text.c_str(), GLUT_BITMAP_9_BY_15);
    if (hasFocus) {
        int cx = text.length() * 9 + 5;
        glBegin(GL_QUADS);
        glVertex2i(cx, 6);
        glVertex2i(cx + 9, 6);
        glVertex2i(cx + 9, 21);
        glVertex2i(cx, 21);
        glEnd();

    }
    glPopMatrix();
    GlutWindow::glDraw();
}

int GlutInputWindow::mouseFunc(int button, int state, int mx, int my) {
    if (button == GLUT_DOWN && mx > x && mx < x + w && my > y && my < y + h) {
        hasFocus = true;
        return id;
    }
    return 0;
}

int GlutInputWindow::keyFunc(int keyCode) {
    if (hasFocus) {
        //cout << keyCode << endl;
        switch (keyCode) {
            case 8 :
                text = text.substr(0, text.length() - 1);
                break;
            case 13 :
                return 2;
                break;
            case 27 :
                hasFocus = false;
                break;
            default :
                text.append(1, keyCode);
        }

        return 1;
    }
    return 0;
}

string GlutInputWindow::getTextAndClear() {
    string v = text;
    text.clear();
    return v;
}
