#ifndef RECIPE_H
#define RECIPE_H

#include "config.hpp"

#include <vector>
#include <cstdio>
#include <string.h>
#include <sys/stat.h>

//Uses TinyXML2: https://github.com/leethomason/tinyxml2/
//Accessed 13 March 2023. Distributed under zlib license.
#include "tinyxml2.h"
using namespace tinyxml2;

class Recipe {

public:
    Recipe() {}

    bool readRecipe(const char *path) {
        status = GOOD;

        positions.clear();

        if(doc.LoadFile(path) != XML_SUCCESS) {
#ifdef DEBUG_MODE_RECIPE
            printf("[Recipe] Did not find valid xml file at path '%s'\n", path);
            fflush(stdout);
#endif
            status = ERROR;
            return false;
        }

#ifdef DEBUG_MODE_RECIPE
        printf("[Recipe] Found recipe at path '%s'\n", path);
        fflush(stdout);
#endif

        XMLElement *root = doc.FirstChildElement("recipe");

        if(!root) {
#ifdef DEBUG_MODE_RECIPE
            printf("[Recipe] Did not find root 'recipe' node\n");
            fflush(stdout);
#endif
            status = ERROR;
            return false;
        }

        readFloatElement(root, "wafer-size", &waferSize, true);
        readFloatElement(root, "exposure-time", &exposureTime, true);
        readPathElement(root, "pattern", &patternPath, true);
        readPathElement(root, "alignment-mark", &markPath, true);
        readPointsListElement(root, positions, true);

        if(status == GOOD) {
#ifdef DEBUG_MODE_RECIPE
            printf("[Recipe] Recipe parsed successfully.\n");
            fflush(stdout);
            displayData();
#endif
            return true;
        } else {
#ifdef DEBUG_MODE_RECIPE
            printf("[Recipe] Failed to parse recipe.\n");
            fflush(stdout);
#endif
            return false;
        }
    }

private:
    float waferSize;

    //later, encapsulate these in a separate object called a "Design"
    float exposureTime;
    const char *patternPath;
    const char *markPath;

    struct Point {
        float x;
        float y;
    };

    std::vector<Point> positions;

    XMLDocument doc;

    enum Status {
        GOOD,
        ERROR
    } status = GOOD;

    //returns true if found and correct, false otherwise
    bool readFloatElement(XMLElement *parent, const char elementName[64], float *dest, bool required) {
        XMLElement *floatElem = parent->FirstChildElement(elementName);

        if(!floatElem) {
            if(required) {
#ifdef DEBUG_MODE_RECIPE
                char msg[256] = "element '";
                strncat(msg, elementName, strlen(elementName));
                const char *suffix = "' was never specified.";
                strncat(msg, suffix, strlen(suffix));
                printf("[Recipe] %s\n", msg);
                fflush(stdout);
#endif
                status = ERROR;
            }
            return false;
        }

        if(floatElem->QueryFloatText(dest) != XML_SUCCESS) {
#ifdef DEBUG_MODE_RECIPE
            char msg[256] = "element '";
            strncat(msg, elementName, strlen(elementName));
            const char *suffix = "' does not have floating-point value.";
            strncat(msg, suffix, strlen(suffix));
            printf("[Recipe] %s\n", msg);
            fflush(stdout);
#endif
            status = ERROR;
            return false;
        }

        return true;
    }

    //returns true if found and correct, false otherwise
    bool readPathElement(XMLElement *parent, const char elementName[64], const char **dest, bool required) {
        XMLElement *pathElem = parent->FirstChildElement(elementName);

        if(!pathElem) {
            if(required) {
#ifdef DEBUG_MODE_RECIPE
                char msg[256] = "element '";
                strncat(msg, elementName, strlen(elementName));
                const char *suffix = "' was never specified.";
                strncat(msg, suffix, strlen(suffix));
                printf("[Recipe] %s\n", msg);
                fflush(stdout);
#endif
                status = ERROR;
                return false;
            }
        }

        const char *elemText = pathElem->GetText();

        if(!elemText) {
#ifdef DEBUG_MODE_RECIPE
            char msg[256] = "element '";
            strncat(msg, elementName, strlen(elementName));
            const char *suffix = "' does not have string value.";
            strncat(msg, suffix, strlen(suffix));
            printf("[Recipe] %s\n", msg);
            fflush(stdout);
#endif
            status = ERROR;
            return false;
        }

        struct stat fileInfo;

        //check if path exists and refers to a readable file
        if(stat(elemText, &fileInfo) != 0 || !S_ISREG(fileInfo.st_mode)) {
#ifdef DEBUG_MODE_RECIPE
            char msg[256] = "element '";
            strncat(msg, elementName, strlen(elementName));
            const char *suffix = "' has an invalid file path.";
            strncat(msg, suffix, strlen(suffix));
            printf("[Recipe] %s\n", msg);
            fflush(stdout);
#endif
            status = ERROR;
            return false;
        }

        *dest = elemText;
        return true;
    }

    //returns true if found and correct, false otherwise
    bool readPointElement(XMLElement *parent, XMLElement **child, Point &pt) {
        const char *elementName = "point";

        if(*child == 0) {
            *child = parent->FirstChildElement(elementName);
        }
        else {
            *child = (*child)->NextSiblingElement(elementName);
        }

        if(!*child) {
            return false;
        }

        if((*child)->QueryFloatAttribute("x", &pt.x) != XML_SUCCESS ||
           (*child)->QueryFloatAttribute("y", &pt.y) != XML_SUCCESS) {
#ifdef DEBUG_MODE_RECIPE
            char msg[256] = "element '";
            strncat(msg, elementName, strlen(elementName));
            const char *suffix;
            suffix = "' is missing 'x' and/or 'y' floating-point attributes.";
            strncat(msg, suffix, strlen(suffix));
            printf("[Recipe] %s\n", msg);
            fflush(stdout);
#endif
            status = ERROR;
            return false;
        }

        return true;
    }

    int readPointsListElement(XMLElement *parent, std::vector<Point> &pts, bool required) {
        const char *elementName = "positions";
        XMLElement *posElem = parent->FirstChildElement(elementName);

        if(!posElem) {
            if(required) {
#ifdef DEBUG_MODE_RECIPE
                char msg[256] = "element '";
                strncat(msg, elementName, strlen(elementName));
                const char *suffix;
                suffix = "' was never specified.";
                strncat(msg, suffix, strlen(suffix));
                printf("[Recipe] %s\n", msg);
                fflush(stdout);
#endif
                status = ERROR;
            }
            return 0;
        }

        XMLElement *child = 0;
        Point tmp;

        while(readPointElement(posElem, &child, tmp)) {
            pts.push_back(tmp);
        }

        if(status == ERROR) return 0;
        else return pts.size();
    }

    void displayData() {
#ifdef DEBUG_MODE_RECIPE
        printf("[Recipe] Recipe info:\n");
        printf("  Wafer size is %f millimeters.\n", waferSize);
        printf("  Exposure time is %f seconds.\n", exposureTime);
        printf("  Pattern image is located at %s.\n", patternPath);
        printf("  Alignment mark image is located at %s.\n", markPath);
        printf("  Die positions:\n");

        for(int i = 0; i < (int) positions.size(); i++) {
            printf("    #%02d: %f, %f\n", i, positions[i].x, positions[i].y);
        }

        fflush(stdout);
#endif
    }
};

#endif // RECIPE_H


