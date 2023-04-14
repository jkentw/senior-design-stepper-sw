#ifndef PROJECTORMODULE_HPP
#define PROJECTORMODULE_HPP

#include "DynamicImage.h"

#ifdef DEBUG_MODE_GLOBAL
#define DEBUG_MODE_PROJECTOR
#endif

namespace projector_module {

extern DynamicImage *projectedImage;

extern bool isOpen();
extern bool openProjector();
extern void closeProjector();
extern void setPattern(QImage *image);
extern void show();
extern void hide();

}

#endif // PROJECTORMODULE_HPP
