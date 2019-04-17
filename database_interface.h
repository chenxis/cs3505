/*
*   This interface is intended to be used by a spreadsheet server
*   for processing tasks to and from a database.
*
*/

#include <string>
#include <vector>
#include <array>

#ifndef DATABASE_INTERFACE_H
#define DATABASE_INTERFACE_H


class database_interface 
{

    private:

    std::string fix_history(std::string spreadsheet_name, std::string cell_name, std::string old_content);

    public:

        database_interface();

        /*  
        *   Adds a new user and password to the user table.
        *   returns 0 if added -1 if add fails
        */
        int register_user(std::string user_name, std::string password);

        std::string get_user_password(std::string username);

        /*  
        *   changes the password of an existing user
        *   returns 0 if changed -1 if password change fails
        */
        int change_password(std::string user_name, std::string new_password);
        

        /*  
        *   retrieves spreadsheet id when provided a name
        *   returns the number if found -1 if not found
        */
        int get_spreadsheet_id(std::string spreadsheet_name);

        /*
        *   Creates a spreadsheet, if spreadsheet exists returns -1, 0 if created.
        *
        */
        int create_spreadsheet(std::string spreadsheet_name);

        /*
        *   Returns a vector containing all spreadsheet names in the table
        */
        std::vector<std::string> get_spreadsheets();

        /*
        *   Returns a vector containing all cells
        *   array[0] = Cell Name (Ex: "A2")
        *   array[1] = Contents
        */
        std::vector<std::array<std::string,2>> open_spreadsheet(std::string spreadsheet_name);

        /*
        *   Returns a 0 if edit was made, -1 if there was an error.
        */
        int make_edit(std::string spreadsheet_name, std::string cell_name, std::string cell_content);

        /*
        *   When provided with a cell name and spreadsheet name, 
        *   returns the previous cell contents.
        */
        std::string revert_cell(std::string spreadsheet_name, std::string cell_name);

        /*
        *   When provided with a cell name and spreadsheet name, 
        *   returns the cells contents.
        */
        std::string get_contents(std::string spreadsheet_name, std::string cell_name);

        /*
        *   Returns an array containing cell name and cell contents of last edit.
        */
        std::array<std::string,2> undo_change(std::string spreadsheet_name);

        void delete_spreadsheet(std::string spreadsheet_name);

        

};

#endif