#pragma once
#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>
#include <dialogs/dialogs.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>

#include <storage/storage.h>
#include <assets_icons.h>
#include "file_utils.h"

#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>

#define NFC_FOLDER EXT_PATH("nfc")

typedef struct BlockFillerApp BlockFillerApp;

struct BlockFillerApp {
    Submenu* main_menu;
    ViewDispatcher* view_dispatcher;
    Gui* gui;

    Widget* about_widget;
};

enum SubmenuIndex { SelectFileIndex, AboutIndex };

enum Views { MainMenuSubmenuView, AboutView };

int32_t block_filler_main(void* p);

uint32_t exit_callback(void* context);

void free_app(BlockFillerApp* app);