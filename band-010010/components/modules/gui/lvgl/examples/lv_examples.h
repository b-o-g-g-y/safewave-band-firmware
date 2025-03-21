/**
 * @file lv_examples.h
 *
 */

#ifndef LV_EXAMPLES_H
#define LV_EXAMPLES_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lvgl.h"

#include "styles/lv_example_style.h"
#include "get_started/lv_example_get_started.h"
#include "widgets/lv_example_widgets.h"
#include "layouts/lv_example_layout.h"
#include "scroll/lv_example_scroll.h"

/*********************
 *      DEFINES
 *********************/
#define LV_BUILD_EXAMPLES       1
#define LV_USE_METER            1

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_EXAMPLES_H*/
