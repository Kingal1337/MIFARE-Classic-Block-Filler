

#include "block_filler_app.h"

#include <block_filler_icons.h>

/**
 * Calcuates the BCC of a string of bytes*/
int calculate_bcc(FuriString* bytes) {
    if(bytes == NULL || furi_string_size(bytes) == 0) {
        return 0;
    }
    FuriString* current_hex = furi_string_alloc();
    FuriString* right_bytes = furi_string_alloc_set(bytes);

    int result_bcc = 0;

    bool has_more = true;
    do {
        int index = furi_string_search_str(right_bytes, " ");
        if(index == FURI_STRING_FAILURE) {
            has_more = false;
        }
        furi_string_set_n(current_hex, right_bytes, 0, index);
        furi_string_right(right_bytes, index + 1);

        result_bcc = result_bcc ^ (int)strtol(furi_string_get_cstr(current_hex), NULL, 16);
    } while(has_more);

    furi_string_free(right_bytes);
    furi_string_free(current_hex);

    return result_bcc;
}

/**
 * 
 * Fills in every sector with generic data (including block 0)
 * fill_in_block_0 should be called after this
 * NOTE: THIS WILL OVERWRITE DATA!!!
 * 
 * - Fills in every data block with 0
 * - All keys will be FF
 * - All access keys will be FF 07 80 69
 * keys A and B being FF and Access Keys With FF 07 80 69
 * 
 * @param file_path the file that will be modified
*/
void fill_in_sectors(FuriString* file_path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    FuriString* key = furi_string_alloc();
    bool key_exists = false;
    int counter = 0;
    bool file_exists = flipper_format_file_open_existing(ff, furi_string_get_cstr(file_path));
    if(file_exists) {
        do {
            furi_string_printf(key, "Block %d", counter);
            const char* key_str = furi_string_get_cstr(key);
            key_exists = flipper_format_key_exist(ff, key_str);
            bool is_sector_trailer = (counter != 0 && ((counter + 1) % 4) == 0);
            counter++;

            // flipper_format_rewind(ff);

            if(key_exists) {
                if(is_sector_trailer) { //sector trailer block
                    flipper_format_update_string_cstr(
                        ff, key_str, "FF FF FF FF FF FF FF 07 80 69 FF FF FF FF FF FF");

                } else { //just a regular data block
                    flipper_format_update_string_cstr(
                        ff, key_str, "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
                }
            }
        } while(key_exists);
    }

    furi_string_free(key);

    flipper_format_file_close(ff);

    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
}

void fill_in_block_0(FuriString* file_path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    bool file_exists = flipper_format_file_open_existing(ff, furi_string_get_cstr(file_path));
    if(file_exists) {
        FuriString* uid = furi_string_alloc();
        FuriString* atqa = furi_string_alloc();
        FuriString* new_block_0 = furi_string_alloc();

        flipper_format_read_string(ff, "UID", uid);
        flipper_format_read_string(ff, "ATQA", atqa);

        int bcc = calculate_bcc(uid);
        furi_string_printf(
            new_block_0,
            "%s %02X %s 00 00 00 00 00 00 00 00 00",
            furi_string_get_cstr(uid),
            bcc,
            furi_string_get_cstr(atqa));

        flipper_format_update_string(ff, "Block 0", new_block_0);

        furi_string_free(uid);
        furi_string_free(atqa);
        furi_string_free(new_block_0);
    }
    flipper_format_file_close(ff);

    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
}

void create_cloned_file() {
    FuriString* file_path = furi_string_alloc();
    FuriString* new_file_path = furi_string_alloc();
    bool file_selected = select_file(NFC_FOLDER, file_path, &I_Nfc_10px, ".nfc");

    if(file_selected) {
        bool valid_name = get_incremented_filename(file_path, ".nfc", new_file_path);
        if(valid_name) {
            bool clone_success = clone_file(file_path, new_file_path);
            if(clone_success) {
                fill_in_sectors(new_file_path);
                fill_in_block_0(new_file_path);
            }
        }
    }

    furi_string_free(file_path);
    furi_string_free(new_file_path);
}

uint32_t exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

uint32_t mainmenu_previous_callback(void* context) {
    UNUSED(context);
    return MainMenuSubmenuView;
}

void mainmenu_callback(void* context, uint32_t index) {
    BlockFillerApp* app = context;

    if(index == SelectFileIndex) {
        create_cloned_file();
    } else if(index == AboutIndex) {
        view_dispatcher_switch_to_view(app->view_dispatcher, AboutView);
    }
}

void free_app(BlockFillerApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, MainMenuSubmenuView);
    submenu_free(app->main_menu);

    //free the dispatcher
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    free(app);
}

int32_t block_filler_main(void* p) {
    UNUSED(p);
    BlockFillerApp* app = malloc(sizeof(BlockFillerApp));

    // Register view port in GUI
    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->main_menu = submenu_alloc();
    submenu_add_item(app->main_menu, "Select File", SelectFileIndex, mainmenu_callback, app);
    submenu_add_item(app->main_menu, "About", AboutIndex, mainmenu_callback, app);
    view_set_previous_callback(submenu_get_view(app->main_menu), exit_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, MainMenuSubmenuView, submenu_get_view(app->main_menu));
    view_dispatcher_switch_to_view(app->view_dispatcher, MainMenuSubmenuView);

    app->about_widget = widget_alloc();
    widget_add_text_scroll_element(
        app->about_widget,
        0,
        0,
        128,
        64,
        "This is a sample application.\n---\nReplace code and message\nwith your content!\n\nauthor: @codeallnight\nhttps://discord.com/invite/NsjCvqwPAd\nhttps://youtube.com/@MrDerekJamison");
    view_set_previous_callback(widget_get_view(app->about_widget), mainmenu_previous_callback);
    view_dispatcher_add_view(app->view_dispatcher, AboutView, widget_get_view(app->about_widget));

    view_dispatcher_run(app->view_dispatcher);

    free_app(app);

    return 0;
}