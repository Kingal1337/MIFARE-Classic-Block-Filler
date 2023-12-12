#include "file_utils.h"

bool select_file(
    const char base_path[],
    FuriString* file_path,
    const Icon* icon,
    const char extension[]) {
    //
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, extension, icon);
    browser_options.base_path = base_path;
    furi_string_set(file_path, base_path);

    bool file_selected = dialog_file_browser_show(dialogs, file_path, file_path, &browser_options);

    furi_record_close(RECORD_DIALOGS);

    return file_selected;
}

bool does_file_exist(const char file_path[]) {
    if(file_path == NULL) {
        return false;
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);

    bool file_exists = flipper_format_file_open_existing(ff, file_path);
    flipper_format_file_close(ff);

    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);

    return file_exists;
}

bool get_incremented_filename(
    FuriString* file_path,
    const char extension[],
    FuriString* new_file_path) {
    int max_tries = 20;

    bool valid_file_name = false;

    // bool ends_with_extension = furi_string_end_with(file_path, extension);
    // if(ends_with_extension) {
    FuriString* file_path_no_extension = furi_string_alloc_set(file_path);
    FuriString* temp_new_file_path = furi_string_alloc(); //the file path of the new file

    //remove file extension
    furi_string_left(file_path_no_extension, furi_string_size(file_path) - 4);
    int counter = 1;
    do {
        furi_string_reset(temp_new_file_path);
        furi_string_cat(temp_new_file_path, file_path_no_extension);
        furi_string_cat_printf(temp_new_file_path, "_%d%s", counter, extension);

        //try and open a new file with the incremented file name
        valid_file_name = !does_file_exist(furi_string_get_cstr(temp_new_file_path));
        FURI_LOG_D(
            "Block Zero App",
            "Current File name [%s] %x",
            furi_string_get_cstr(temp_new_file_path),
            valid_file_name);

        counter++;
        if(counter > max_tries) {
            FURI_LOG_E("Block Zero App", "Too Many Tries");
            break;
        }
    } while(!valid_file_name);

    if(!valid_file_name) { //called if there has been too many increments or the file name is just invalid, most likely too many increments
        FURI_LOG_E("Block Zero App", "Could not create new file, too many copies");
        //show error to user
    } else {
        FURI_LOG_E(
            "Block Zero App", "Valid file name %s", furi_string_get_cstr(temp_new_file_path));
    }

    furi_string_set(new_file_path, temp_new_file_path);

    furi_string_free(file_path_no_extension);
    furi_string_free(temp_new_file_path);

    // } else {
    //     FURI_LOG_E(
    //         "Block Zero App",
    //         "The file you selected does not match the file extension you provided (%s)",
    //         extension);
    // }
    return valid_file_name;
}

bool clone_file(FuriString* source_file_path, FuriString* destination_file_path) {
    if(source_file_path == NULL || destination_file_path == NULL) {
        return false;
    }
    if(furi_string_equal(source_file_path, destination_file_path)) { //cannot be the same file
        return false;
    }
    if(!does_file_exist(furi_string_get_cstr(source_file_path))) { //original file must exist
        return false;
    }
    if(does_file_exist(
           furi_string_get_cstr(destination_file_path))) { //destination file cannot exist
        return false;
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);

    bool open_success =
        flipper_format_buffered_file_open_existing(ff, furi_string_get_cstr(source_file_path));

    if(open_success) {
        Stream* stream = flipper_format_get_raw_stream(ff);

        stream_save_to_file(
            stream, storage, furi_string_get_cstr(destination_file_path), FSOM_CREATE_NEW);
    }

    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);

    return open_success;
}

bool remove_extension(FuriString* file_path) {
    // null or empty string has no extension
    if(file_path == NULL || furi_string_size(file_path) <= 0) {
        return false;
    }

    size_t slash_index = furi_string_search_rchar(file_path, '/', 0);
    //If there is a slash at the end of the filepath
    if(slash_index != FURI_STRING_FAILURE && slash_index >= (furi_string_size(file_path) - 1)) {
        return false;
    }

    size_t extension_index = furi_string_search_rchar(file_path, '.', 0);

    //could not find . for extension, meaning the extension has been "removed"
    if(extension_index == FURI_STRING_FAILURE) {
        return true;
    }

    //if this is called, it found an extension not in the file name, meaning the filename does not have an extension
    if(slash_index != FURI_STRING_FAILURE && slash_index > extension_index) {
        return true;
    }

    furi_string_left(file_path, extension_index);

    return true;
}

bool get_file_name_from_path(
    FuriString* file_path,
    FuriString* result_file_name,
    bool should_remove_extension) {
    if(file_path == NULL || result_file_name == NULL) {
        return false;
    }
    size_t slash_index = furi_string_search_rchar(file_path, '/', 0);
    if(slash_index == FURI_STRING_FAILURE || slash_index >= (furi_string_size(file_path) - 1)) {
        return false;
    }

    furi_string_set(result_file_name, file_path);
    furi_string_right(result_file_name, slash_index + 1);
    if(should_remove_extension) {
        remove_extension(result_file_name);
    }

    return true;
}
/*
*******************
Remove Extension test cases
*******************
        FuriString* remove_ext_test_1 = furi_string_alloc_set_str("/ext/nfc/test.nfc");
        bool result_1 = remove_extension(remove_ext_test_1);

        FuriString* remove_ext_test_2 = furi_string_alloc_set_str("/ext/tools/test.sdf/file");
        bool result_2 = remove_extension(remove_ext_test_2);

        FuriString* remove_ext_test_3 = furi_string_alloc_set_str("/ext/apps/.test");
        bool result_3 = remove_extension(remove_ext_test_3);

        FuriString* remove_ext_test_4 = furi_string_alloc_set_str("/ext/rfid/no_extension");
        bool result_4 = remove_extension(remove_ext_test_4);

        FURI_LOG_I(
            "Block Zero App Test 1",
            "Result %x; String %s",
            result_1,
            furi_string_get_cstr(remove_ext_test_1));

        FURI_LOG_I(
            "Block Zero App Test 2",
            "Result %x; String %s",
            result_2,
            furi_string_get_cstr(remove_ext_test_2));

        FURI_LOG_I(
            "Block Zero App Test 3",
            "Result %x; String %s",
            result_3,
            furi_string_get_cstr(remove_ext_test_3));

        FURI_LOG_I(
            "Block Zero App Test 4",
            "Result %x; String %s",
            result_4,
            furi_string_get_cstr(remove_ext_test_4));

        furi_string_free(remove_ext_test_1);
        furi_string_free(remove_ext_test_2);
        furi_string_free(remove_ext_test_3);
        furi_string_free(remove_ext_test_4);




*******************
get_file_name_from_path Test cases
*******************


        FuriString* get_file_name_path_test_1 = furi_string_alloc_set_str("/ext/nfc/test.nfc");
        FuriString* get_file_name_result_test_1 = furi_string_alloc();
        bool result_1 =
            get_file_name_from_path(get_file_name_path_test_1, get_file_name_result_test_1, true);

        FuriString* get_file_name_path_test_2 =
            furi_string_alloc_set_str("/ext/tools/test.sdf/file");
        FuriString* get_file_name_result_test_2 = furi_string_alloc();
        bool result_2 =
            get_file_name_from_path(get_file_name_path_test_2, get_file_name_result_test_2, true);

        FuriString* get_file_name_path_test_3 = furi_string_alloc_set_str("/ext/apps/.test");
        FuriString* get_file_name_result_test_3 = furi_string_alloc();
        bool result_3 =
            get_file_name_from_path(get_file_name_path_test_3, get_file_name_result_test_3, true);

        FuriString* get_file_name_path_test_4 =
            furi_string_alloc_set_str("/ext/rfid/no_extension");
        FuriString* get_file_name_result_test_4 = furi_string_alloc();
        bool result_4 =
            get_file_name_from_path(get_file_name_path_test_4, get_file_name_result_test_4, true);

        FURI_LOG_I(
            "Block Zero App Test 1",
            "Result %x; String %s",
            result_1,
            furi_string_get_cstr(get_file_name_result_test_1));

        FURI_LOG_I(
            "Block Zero App Test 2",
            "Result %x; String %s",
            result_2,
            furi_string_get_cstr(get_file_name_result_test_2));

        FURI_LOG_I(
            "Block Zero App Test 3",
            "Result %x; String %s",
            result_3,
            furi_string_get_cstr(get_file_name_result_test_3));

        FURI_LOG_I(
            "Block Zero App Test 4",
            "Result %x; String %s",
            result_4,
            furi_string_get_cstr(get_file_name_result_test_4));

        furi_string_free(get_file_name_path_test_1);
        furi_string_free(get_file_name_path_test_2);
        furi_string_free(get_file_name_path_test_3);
        furi_string_free(get_file_name_path_test_4);
        furi_string_free(get_file_name_result_test_1);
        furi_string_free(get_file_name_result_test_2);
        furi_string_free(get_file_name_result_test_3);
        furi_string_free(get_file_name_result_test_4);

*/
