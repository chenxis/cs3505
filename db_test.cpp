#include "database_interface.h"
#include <vector>
#include <array>
#include <iostream>

/* 
* Rememebr to add                 replace with xxxxxxx with cade login
* setenv LD_LIBRARY_PATH "${PATH}/home/xxxxxxx/database_interface/connector/lib64"
* to .cshrc file at ~
*/ 

int main(int argc, const char **argv) {
    database_interface db;

    //How to retreive a list of spreadsheets

    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "Let's see what spreadsheets there are:" << std::endl;
    std::vector<std::string> sheets = db.get_spreadsheets();
    
    for(auto const& value: sheets) {
        std::cout << "Spreadsheet Name: " << value << std::endl;
    }

    std::cout << std::endl;

    std::cout << "Creating Test Spreadsheet\n";
    std::cout << std::endl;
    db.create_spreadsheet("Test Spreadsheet");
    std::cout << "Now what spreadsheets are there:" << std::endl;
    sheets = db.get_spreadsheets();

    for(auto const& value: sheets) {
        std::cout << "Spreadsheet Name: " << value << std::endl;
    }

    std::cout << std::endl;

    std::cout << "Now Open Test Spreadsheet" << std::endl;
    std::vector<std::array<std::string,2>> cells = db.open_spreadsheet("Test Spreadsheet");

    for(auto const& value: cells) {
        std::cout << "Cell Name: " << value.at(0);
        std::cout << "| Cell Contents: " << value.at(1) << std::endl;
    }

    std::cout << std::endl;
    

    std::cout << "Inserting some values for open to show up" << std::endl;

    //create some values for open:
    db.make_edit("Test Spreadsheet", "A1", "10");
    db.make_edit("Test Spreadsheet", "A2", "20");
    db.make_edit("Test Spreadsheet", "A3", "=A1+A2");
    db.make_edit("Test Spreadsheet", "C3", "Hey There");
    
    std::cout << std::endl;
    
    //How to retreive a spreadsheet
    std::cout << "Now Open Test Spreadsheet" << std::endl;
    cells = db.open_spreadsheet("Test Spreadsheet");
    
    for(auto const& value: cells) {
        std::cout << "Cell Name: " << value.at(0);
        std::cout << "| Cell Contents: " << value.at(1) << std::endl;
    }

    std::cout << std::endl;

    std::cout << "Setting cell contents of A2 to 40" << std::endl;
    db.make_edit("Test Spreadsheet", "A2", "40");
    std::cout << "A2 is now: " << db.get_contents("Test SpreadSheet", "A2") << std::endl;

    std::cout << std::endl;

    std::cout << "Setting cell contents of A1 to 73" << std::endl;
    db.make_edit("Test Spreadsheet", "A1", "73");
    std::cout << "A1 is now: " << db.get_contents("Test SpreadSheet", "A1") << std::endl;

    std::cout << std::endl;

    std::cout << "Undoing" << std::endl;
    std::array<std::string,2> undo = db.undo_change("Test Spreadsheet");
    std::cout << "Cell: " << undo.at(0) << " is now " << undo.at(1) << std::endl;

    std::cout << std::endl;

    std::cout << "Reverting A2 to previous contents" << std::endl;
    std::string new_content = db.revert_cell("Test Spreadsheet", "A2");
    std::cout << "A2 After Revert: " << db.get_contents("Test SpreadSheet", "A2") << std::endl;

    std::cout << std::endl;

    //delete
    db.delete_spreadsheet("Test Spreadsheet");

} 