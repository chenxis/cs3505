//Include our class header
#include "database_interface.h"

/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <boost/scoped_ptr.hpp>
#include "jdbc/mysql_connection.h"
#include "jdbc/mysql_driver.h"
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <vector>
#include <array>

//Define Connection Details
#define URI "tcp://24.11.85.102:3306"
#define USER "Squid"
#define PASS "Squid3505"
#define DB "spreadsheet"

//sql::Driver *driver;

database_interface::database_interface() {

}

std::string database_interface::get_user_password(std::string username) {
    sql::mysql::MySQL_Driver *driver;
    std::string result;

    try {
        driver = sql::mysql::get_driver_instance();

        boost::scoped_ptr< sql::Connection >
        con(driver->connect(URI, USER, PASS));
        con->setSchema(DB);

        boost::scoped_ptr< sql::PreparedStatement > pstmt(con->prepareStatement("select password from users where username = ?"));
        pstmt->setString(1,username);
        boost::scoped_ptr< sql::ResultSet >
            res(pstmt->executeQuery());
        
        while (res->next()) {
            result = res->getInt("password");
        }

    } catch (sql::SQLException &e) {
        return "";
    }

    return result;
}

int database_interface::register_user(std::string user_name, std::string password) {
    sql::mysql::MySQL_Driver *driver;

    try {
        driver = sql::mysql::get_driver_instance();

        boost::scoped_ptr< sql::Connection >
        con(driver->connect(URI, USER, PASS));
        con->setSchema(DB);

        boost::scoped_ptr< sql::PreparedStatement > pstmt(con->prepareStatement("INSERT INTO USERS(username,password) VALUES (?,?)"));
        pstmt->setString(1,user_name);
        pstmt->setString(2,password);
        pstmt->executeUpdate();
        

    } catch (sql::SQLException &e) {
        return -1;
    }

    return 0;
}

int database_interface::change_password(std::string user_name, std::string new_password) {
    sql::mysql::MySQL_Driver *driver;

    try {
        driver = sql::mysql::get_driver_instance();

        boost::scoped_ptr< sql::Connection >
        con(driver->connect(URI, USER, PASS));
        con->setSchema(DB);

        boost::scoped_ptr< sql::PreparedStatement > pstmt(con->prepareStatement("Update USERS set password = ? where username = ?"));
        pstmt->setString(2,user_name);
        pstmt->setString(1,new_password);
        pstmt->executeUpdate();
        

    } catch (sql::SQLException &e) {
        return -1;
    }

    return 0;
}

/*  
*   retrieves spreadsheet id when provided a name
*   returns the number if found -1 if not found
*/
int database_interface::get_spreadsheet_id(std::string spreadsheet_name) {
    sql::mysql::MySQL_Driver *driver;
    int id = 0;

    try {
        driver = sql::mysql::get_driver_instance();

        boost::scoped_ptr< sql::Connection >
        con(driver->connect(URI, USER, PASS));
        con->setSchema(DB);

        boost::scoped_ptr< sql::PreparedStatement > pstmt(con->prepareStatement("select id from spreadsheets where spreadsheet_name = ?"));
        pstmt->setString(1,spreadsheet_name);
        boost::scoped_ptr< sql::ResultSet >
            res(pstmt->executeQuery());
        
        while (res->next()) {
            id = res->getInt("spreadsheet_id");
        }

    } catch (sql::SQLException &e) {
        return -1;
    }

    if (id > 0) {
        return id;
    } else {
        return -1;
    }
}

/*
*   Creates a spreadsheet, if spreadsheet exists returns -1, 0 if created.
*
*/
int database_interface::create_spreadsheet(std::string spreadsheet_name) {
    sql::mysql::MySQL_Driver *driver;

    try {
        driver = sql::mysql::get_driver_instance();

        boost::scoped_ptr< sql::Connection >
        con(driver->connect(URI, USER, PASS));
        con->setSchema(DB);

        boost::scoped_ptr< sql::PreparedStatement > pstmt(con->prepareStatement("INSERT INTO spreadsheets(spreadsheet_name) VALUES (?)"));
        pstmt->setString(1,spreadsheet_name);
        pstmt->executeUpdate();
        

    } catch (sql::SQLException &e) {
        return -1;
    }

    return 0;
}

/*
*   Returns a vector containing all spreadsheet names in the table
*/
std::vector<std::string> database_interface::get_spreadsheets() {
    sql::mysql::MySQL_Driver *driver;
    std::vector<std::string> spreadsheets;

    try {
        driver = sql::mysql::get_driver_instance();

        boost::scoped_ptr< sql::Connection >
        con(driver->connect(URI, USER, PASS));
        con->setSchema(DB);

        boost::scoped_ptr< sql::PreparedStatement > pstmt(con->prepareStatement("select * from spreadsheets order by spreadsheet_name"));
        boost::scoped_ptr< sql::ResultSet >
            res(pstmt->executeQuery());
        
        while (res->next()) {
            std::string temp = res->getString("spreadsheet_name");

            spreadsheets.push_back(temp);

        }

    } catch (sql::SQLException &e) {
        return spreadsheets;
    }

    return spreadsheets;

}

/*
*   Returns a vector containing all cells
*   array[0] = Cell Name (Ex: "A2")
*   array[1] = Contents
*/
std::vector<std::array<std::string,2>> database_interface::open_spreadsheet(std::string spreadsheet_name) {
    sql::mysql::MySQL_Driver *driver;
    std::vector<std::array<std::string,2>> cells;

    try {
        driver = sql::mysql::get_driver_instance();

        boost::scoped_ptr< sql::Connection >
        con(driver->connect(URI, USER, PASS));
        con->setSchema(DB);

        boost::scoped_ptr< sql::PreparedStatement > pstmt(con->prepareStatement("select cell_name, cell_contents from cells where spreadsheet_name = ?"));
        pstmt->setString(1,spreadsheet_name);
        boost::scoped_ptr< sql::ResultSet >
            res(pstmt->executeQuery());
        
        while (res->next()) {
            std::array<std::string,2> temp;
            temp.at(0) = res->getString("cell_name");
            temp.at(1) = res->getString("cell_contents");

            cells.push_back(temp);

        }

    } catch (sql::SQLException &e) {
        return cells;
    }

    return cells;

}


/*
*   Returns a 0 if edit was made, -1 if there was an error.
*/
int database_interface::make_edit(std::string spreadsheet_name, std::string cell_name, std::string cell_contents) {
    
    //Is something already in the DB
    std::string result = this->get_contents(spreadsheet_name, cell_name);
        
    if(result == "") {
        //Insert into DB

        sql::mysql::MySQL_Driver *driver;

        try {

            driver = sql::mysql::get_driver_instance();

            //Initiate Connection
            boost::scoped_ptr< sql::Connection >
            con(driver->connect(URI, USER, PASS));
            con->setSchema(DB);

            //Prep Query
            boost::scoped_ptr< sql::PreparedStatement > 
                pstmt(con->prepareStatement("insert into cells (spreadsheet_name, cell_name, cell_contents) values(?,?,?)"));
            pstmt->setString(1,spreadsheet_name);
            pstmt->setString(2,cell_name);
            pstmt->setString(3,cell_contents);
            pstmt->executeUpdate();

        } catch (sql::SQLException &e) {
            return -1;
        }

        return 0;
    } else {        
        //Move To History
        sql::mysql::MySQL_Driver *driver;

        try {

            driver = sql::mysql::get_driver_instance();

            //Initiate Connection
            boost::scoped_ptr< sql::Connection >
            con(driver->connect(URI, USER, PASS));
            con->setSchema(DB);

            //Prep cell Query
            boost::scoped_ptr< sql::PreparedStatement > 
                pstmt2(con->prepareStatement("insert into cell_history (spreadsheet_name, cell_name, cell_contents, created_at) values(?,?,?, NOW())"));
            pstmt2->setString(1,spreadsheet_name);
            pstmt2->setString(2,cell_name);
            pstmt2->setString(3,result);
            pstmt2->executeUpdate();

            //Prep edit Query
            boost::scoped_ptr< sql::PreparedStatement > 
                pstmt3(con->prepareStatement("insert into edit_history (spreadsheet_name, cell_name, cell_contents, created_at) values(?,?,?, NOW())"));
            pstmt3->setString(1,spreadsheet_name);
            pstmt3->setString(2,cell_name);
            pstmt3->setString(3,result);
            pstmt3->executeUpdate();


        } catch (sql::SQLException &e) {
            return -1;
        }

        //Done with history
        //Time to add some fresh cell data

        try {

            driver = sql::mysql::get_driver_instance();

            //Initiate Connection
            boost::scoped_ptr< sql::Connection >
            con(driver->connect(URI, USER, PASS));
            con->setSchema(DB);

            //Prep Query
            boost::scoped_ptr< sql::PreparedStatement > 
                pstmt2(con->prepareStatement("update cells set cell_contents = ? where spreadsheet_name = ? and cell_name = ?"));
            pstmt2->setString(2,spreadsheet_name);
            pstmt2->setString(3,cell_name);
            pstmt2->setString(1,cell_contents);
            pstmt2->executeUpdate();

            return 0;
        } catch (sql::SQLException &e) {
            return -1;
        }

    }
}

/*
*   When provided with a cell name and spreadsheet name, 
*   returns the previous cell contents.
*/
std::string database_interface::revert_cell(std::string spreadsheet_name, std::string cell_name) {
    std::string result;

    result = this->get_contents(spreadsheet_name, cell_name);

    if(result != "") {
        return fix_history(spreadsheet_name, cell_name, result);
    } else {
        return "";    
    }
}

std::string database_interface::fix_history(std::string spreadsheet_name, std::string cell_name, std::string old_content) {

    std::string historic_content;
    int cell_hist_id;
    sql::mysql::MySQL_Driver *driver;

    //get history
    try {
        driver = sql::mysql::get_driver_instance();

        boost::scoped_ptr< sql::Connection >
        con(driver->connect(URI, USER, PASS));
        con->setSchema(DB);
        
        //Should return most rescent cell history
        boost::scoped_ptr< sql::PreparedStatement > 
            pstmt(con->prepareStatement("select cell_history_id, cell_contents from cell_history where spreadsheet_name = ? and cell_name = ? order by created_at desc limit 1"));
        pstmt->setString(1,spreadsheet_name);
        pstmt->setString(2,cell_name);
        boost::scoped_ptr< sql::ResultSet >
            res(pstmt->executeQuery());
        
        while (res->next()) {
            historic_content = res->getString("cell_contents");
            cell_hist_id = res->getInt("cell_history_id");
        }
    
        //if historic value found set cell to historic value, delete, and add to edits.


        //Change cell
        boost::scoped_ptr< sql::PreparedStatement > pstmt2(con->prepareStatement("update cells set cell_contents = ? where spreadsheet_name = ? and cell_name = ?"));
        pstmt2->setString(1,historic_content);
        pstmt2->setString(2,spreadsheet_name);
        pstmt2->setString(3,cell_name);
        pstmt2->executeUpdate();

        //Add to edits
        boost::scoped_ptr< sql::PreparedStatement > pstmt4(con->prepareStatement("insert into edit_history (spreadsheet_name, cell_name, cell_contents, created_at) values(?,?,?, NOW());"));
        pstmt4->setString(3,old_content);
        pstmt4->setString(1,spreadsheet_name);
        pstmt4->setString(2,cell_name);
        pstmt4->executeUpdate();

        //Delete
        boost::scoped_ptr< sql::PreparedStatement > pstmt3(con->prepareStatement("delete from cell_history where cell_history_id = ?"));
        pstmt3->setInt(1,cell_hist_id);
        pstmt3->executeUpdate();

        return historic_content;

    

    } catch (sql::SQLException &e) {
        return "";
    }

    //Shouldn't get here.
    return "";
}

/*
*   When provided with a cell name and spreadsheet name, 
*   returns the cells contents.
*/
std::string database_interface::get_contents(std::string spreadsheet_name, std::string cell_name) {
    sql::mysql::MySQL_Driver *driver;
    std::string result;

    try {
        driver = sql::mysql::get_driver_instance();

        boost::scoped_ptr< sql::Connection >
        con(driver->connect(URI, USER, PASS));
        con->setSchema(DB);

        boost::scoped_ptr< sql::PreparedStatement > pstmt(con->prepareStatement("select cell_contents from cells where spreadsheet_name = ? and cell_name = ?"));
        pstmt->setString(1,spreadsheet_name);
        pstmt->setString(2,cell_name);
        boost::scoped_ptr< sql::ResultSet >
            res(pstmt->executeQuery());
        
        while (res->next()) {
            result = res->getString("cell_contents");
        }

    } catch (sql::SQLException &e) {
        return "";
    }

    return result;
}

/*
*   Returns an array containing cell name and cell contents of last edit.
*/
std::array<std::string,2> database_interface::undo_change(std::string spreadsheet_name) {
    std::array<std::string,2> result;
    int edit_id;
    
    //get the last edit in edit_history
    sql::mysql::MySQL_Driver *driver;

    try {
        driver = sql::mysql::get_driver_instance();

        boost::scoped_ptr< sql::Connection >
        con(driver->connect(URI, USER, PASS));
        con->setSchema(DB);

        boost::scoped_ptr< sql::PreparedStatement > pstmt(con->prepareStatement("select edit_id, cell_name, cell_contents from edit_history where spreadsheet_name = ? order by created_at desc limit 1"));
        pstmt->setString(1,spreadsheet_name);
        boost::scoped_ptr< sql::ResultSet >
            res(pstmt->executeQuery());
        
        while (res->next()) {
            result.at(0) = res->getString("cell_name");
            result.at(1) = res->getString("cell_contents");
            edit_id = res->getInt("edit_id");
        }

        //update cells to old content
        boost::scoped_ptr< sql::PreparedStatement > pstmt2(con->prepareStatement("update cells set cell_contents = ? where spreadsheet_name = ? and cell_name = ?"));
        pstmt2->setString(1, result.at(1));
        pstmt2->setString(2,spreadsheet_name);
        pstmt2->setString(3,result.at(0));
        pstmt2->executeUpdate();

        //delete edit
        boost::scoped_ptr< sql::PreparedStatement > pstmt3(con->prepareStatement("delete from edit_history where edit_id = ?"));
        pstmt3->setInt(1, edit_id);
        pstmt3->executeUpdate();

    } catch (sql::SQLException &e) {
        return result;
    }


    return result;
}

void database_interface::delete_spreadsheet(std::string spreadsheet_name) {
    sql::mysql::MySQL_Driver *driver;

     try {
        driver = sql::mysql::get_driver_instance();

        boost::scoped_ptr< sql::Connection >
        con(driver->connect(URI, USER, PASS));
        con->setSchema(DB);

        //delete edit hist
        boost::scoped_ptr< sql::PreparedStatement > pstmt1(con->prepareStatement("delete from edit_history where spreadsheet_name = ?"));
        pstmt1->setString(1, spreadsheet_name);
        pstmt1->executeUpdate();

        //delete cell hist
        boost::scoped_ptr< sql::PreparedStatement > pstmt2(con->prepareStatement("delete from cell_history where spreadsheet_name = ?"));
        pstmt2->setString(1, spreadsheet_name);
        pstmt2->executeUpdate();

        //delete cells
        boost::scoped_ptr< sql::PreparedStatement > pstmt3(con->prepareStatement("delete from cells where spreadsheet_name = ?"));
        pstmt3->setString(1, spreadsheet_name);
        pstmt3->executeUpdate();

        //delete spreadsheet
        boost::scoped_ptr< sql::PreparedStatement > pstmt4(con->prepareStatement("delete from spreadsheets where spreadsheet_name = ?"));
        pstmt4->setString(1, spreadsheet_name);
        pstmt4->executeUpdate();

    } catch (sql::SQLException &e) {
        //do nothing
    }

}