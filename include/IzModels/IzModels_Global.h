#pragma once

#include <QtCore/qglobal.h>

#if defined(IZMODELS_LIBRARY)
#define IZMODELSSHARED_EXPORT Q_DECL_EXPORT
#else
#define IZMODELSSHARED_EXPORT Q_DECL_IMPORT
#endif
