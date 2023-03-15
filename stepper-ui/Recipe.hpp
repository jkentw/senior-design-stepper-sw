#ifndef RECIPE_H
#define RECIPE_H

#include <QWidget>
#include <QFileInfo>

//Uses TinyXML2: https://github.com/leethomason/tinyxml2/
//Accessed 13 March 2023. Distributed under zlib license.
#include "tinyxml2.h"
using namespace tinyxml2;

#include <iostream>
#include <string.h>

static const bool DEBUG_OUTPUT = true;

class Recipe : public QObject {
    Q_OBJECT

public:
    Recipe(QObject *parent = nullptr);

    Q_INVOKABLE
    bool readRecipe(QString path) {
        status = GOOD;

        if(DEBUG_OUTPUT) std::cout << "reading recipe..." << std::endl;
        XMLDocument doc;

        if(doc.LoadFile(path.toStdString().c_str()) != XML_SUCCESS) {
            if(DEBUG_OUTPUT) std::cout << "did not find valid xml file" << std::endl;
            status = ERROR;
            return false;
        }

        XMLElement *root = doc.FirstChildElement("recipe");

        if(!root) {
            if(DEBUG_OUTPUT) std::cout << "no root 'recipe' found" << std::endl;
            status = ERROR;
            return false;
        }

        readFloatElement(root, "wafer-size", &waferSize, true);
        readFloatElement(root, "exposure-time", &exposureTime, true);
        readPathElement(root, "pattern", &patternPath, true);
        readPathElement(root, "alignment-mark", &markPath, true);
        count = readPointsListElement(root, x, y, 16, true);

        if(status == GOOD) {
            if(DEBUG_OUTPUT) std::cout << "Recipe parsed successfully." << std::endl;
            displayData();
            return true;
        } else {
            if(DEBUG_OUTPUT) std::cout << "Recipe parsing failed." << std::endl;
            return false;
        }
    }

private:
    float waferSize;

    //later, encapsulate these in a separate object called a "Design"
    float exposureTime;
    QString patternPath;
    QString markPath;

    //temporary
    float x[16];
    float y[16];
    int count;

    enum Status {
        GOOD,
        ERROR
    } status = GOOD;

    //returns true if found and correct, false otherwise
    bool readFloatElement(XMLElement *parent, const char elementName[64], float *dest, bool required) {
        XMLElement *floatElem = parent->FirstChildElement(elementName);

        char msg[256] = "element '";
        strncat(msg, elementName, strlen(elementName));
        const char *suffix;

        if(!floatElem) {
            if(required) {
                if(DEBUG_OUTPUT) {
                    suffix = "' was never specified.";
                    strncat(msg, suffix, strlen(suffix));
                    std::cout << msg << std::endl;
                }
                status = ERROR;
            }
            return false;
        }

        if(floatElem->QueryFloatText(dest) != XML_SUCCESS) {
            if(DEBUG_OUTPUT) {
                suffix = "' does not have floating-point value.";
                strncat(msg, suffix, strlen(suffix));
                std::cout << msg << std::endl;
            }
            status = ERROR;
            return false;
        }

        return true;
    }

    //returns true if found and correct, false otherwise
    bool readPathElement(XMLElement *parent, const char elementName[64], QString *dest, bool required) {
        XMLElement *pathElem = parent->FirstChildElement(elementName);

        char msg[256] = "element '";
        strncat(msg, elementName, strlen(elementName));
        const char *suffix;

        if(!pathElem) {
            if(required) {
                if(DEBUG_OUTPUT) {
                    suffix = "' was never specified.";
                    strncat(msg, suffix, strlen(suffix));
                    std::cout << msg << std::endl;
                }
                status = ERROR;
            }
        }

        const char *elemText = pathElem->GetText();

        if(!elemText) {
            if(DEBUG_OUTPUT) {
                suffix = "' does not have string value.";
                strncat(msg, suffix, strlen(suffix));
                std::cout << msg << std::endl;
            }
            status = ERROR;
            return false;
        }

        QString elemStr(elemText);
        QFileInfo qfi(elemStr);

        if(!QFileInfo::exists(elemStr) || !qfi.isFile()) {
            if(DEBUG_OUTPUT) {
                suffix = "' has an invalid file path.";
                strncat(msg, suffix, strlen(suffix));
                std::cout << msg << std::endl;
            }
            status = ERROR;
            return false;
        }

        *dest = elemStr;
        return true;
    }

    //returns true if found and correct, false otherwise
    bool readPointElement(XMLElement *parent, XMLElement **child, float *x, float *y) {
        const char *elementName = "point";

        if(*child == 0) {
            *child = parent->FirstChildElement(elementName);
        }
        else {
            *child = (*child)->NextSiblingElement(elementName);
        }

        char msg[256] = "element '";
        strncat(msg, elementName, strlen(elementName));
        const char *suffix;

        if(!*child) {
            return false;
        }

        if((*child)->QueryFloatAttribute("x", x) != XML_SUCCESS ||
           (*child)->QueryFloatAttribute("y", y) != XML_SUCCESS) {
            if(DEBUG_OUTPUT) {
                suffix = "' is missing 'x' and/or 'y' floating-point attributes.";
                strncat(msg, suffix, strlen(suffix));
                std::cout << msg << std::endl;
            }
            status = ERROR;
            return false;
        }

        return true;
    }

    int readPointsListElement(XMLElement *parent, float *xArray, float *yArray, int maxCount, bool required) {
        const char *elementName = "positions";
        XMLElement *posElem = parent->FirstChildElement(elementName);

        char msg[256] = "element '";
        strncat(msg, elementName, strlen(elementName));
        const char *suffix;

        int count = 0;

        if(!posElem) {
            if(required) {
                if(DEBUG_OUTPUT) {
                    suffix = "' was never specified.";
                    strncat(msg, suffix, strlen(suffix));
                    std::cout << msg << std::endl;
                }
                status = ERROR;
            }
            return count;
        }

        XMLElement *child = 0;

        for(count = 0; count < maxCount
            && readPointElement(posElem, &child, &xArray[count], &yArray[count]); count++);

        if(status == ERROR) return 0;
        else return count;
    }

    void displayData() {
        std::cout << "Wafer size is " << waferSize << " millimeters." << std::endl;
        std::cout << "Exposure time is " << exposureTime << " seconds." << std::endl;
        std::cout << "Pattern image is located at " << patternPath.toStdString() << "." << std::endl;
        std::cout << "Alignment mark image is located at " << markPath.toStdString() << "." << std::endl;
        std::cout << "Die positions:" << std::endl;

        for(int i = 0; i < count; i++) {
            std::cout << "  #" << i << ": " << x[i] << ", " << y[i] << std::endl;
        }
    }
};

inline Recipe::Recipe(QObject *parent) : QObject(parent)
{}

#endif // RECIPE_H


