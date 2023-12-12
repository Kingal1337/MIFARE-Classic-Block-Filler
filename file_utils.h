#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <gui/icon.h>
#include <dialogs/dialogs.h>
#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>

/**
 * Opens a file selector dialog box
 * @param base_path the base path of the file selector Ex: for NFC files you can have "/ext/nfc"
 * @param file_path the selected file path
 * @param icon file icon pointer, can be NULL
 * @param extension types of files to be shown Ex: "nfc" for NFC files
*/
bool select_file(
    const char base_path[],
    FuriString* file_path,
    const Icon* icon,
    const char extension[]);

/**
 * Checks if a file exists or not
 * @param file_path the full file path
 * @return true if the file exists, otherwise return false
*/
bool does_file_exist(const char file_path[]);

/**
 * Takes a filename and attempts to increment it
 * Ex: 
 * /ext/nfc/some_file.nfc -> /ext/nfc/some_file_1.nfc
 * /ext/nfc/work_id.nfc -> /ext/nfc/work_id_1.nfc
 * /ext/nfc/work_id.nfc -> /ext/nfc/work_id_2.nfc
 * /ext/nfc/work_id.nfc -> /ext/nfc/work_id_3.nfc
 * @param file_path the full file path of the original file
 * @param extension the extension of the original file must include dot Ex: .nfc
 * @param new_file_path where the new file_path will be stored. if no filename is found, the original file name will be here instead
 * @return true if incremented filename is valid, else false,
*/
bool get_incremented_filename(
    FuriString* file_path,
    const char extension[],
    FuriString* new_file_path);

/**
 * Takes the contents of one file and copies it to another file
 * @param source_file_path the path of the file you want to clone
 * @param destination_file_path the path of where you want the file to be. Must be a file that doesn't exist
 * @return true if cloning is successful, otherwise false
*/
bool clone_file(FuriString* source_file_path, FuriString* destination_file_path);

/**
 * Removes the extension from the file_name
 * @param file_path the file_path or file name. Note: the extension will be removed from this FuriString
 * @return true if the extension has been removed. if the extension was not there to begin with, will also return true
*/
bool remove_extension(FuriString* file_path);

/**
 * Gets the file name from a file path
 * @param file_path  the file path
 * @param file_name  the FuriString to store the file name
 * @param should_remove_extension  true if the extension should be removed, otherwise false
*/
bool get_file_name_from_path(
    FuriString* file_path,
    FuriString* result_file_name,
    bool should_remove_extension);