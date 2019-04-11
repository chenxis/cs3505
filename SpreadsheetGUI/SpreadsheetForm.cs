using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Text.RegularExpressions;
using SpreadsheetUtilities;
using SS;


//Author Chenxi Sun Uid u0455173


namespace SpreadsheetGUI
{

    public partial class SpreadsheetForm : Form
    {
        // Each spreadsheet form will contain a private spreadsheet object
        private Spreadsheet spreadsheet;
        // Keeps track of the location where the file is saved
        private String filename;

        // helper variables so that user doesn't get asked to save changes twice
        private bool passedFromClose;
        private bool saveCanceled;



        public SpreadsheetForm()
        {
            InitializeComponent();

            //constructor
            spreadsheet = new Spreadsheet(variablechecker, s => s.ToUpper(), "ps6");

        
            SpreadsheetUpdater();
            filename = null; // initialize to empty
            passedFromClose = false;
            saveCanceled = false;

            // Initially the status label will be null
            toolStripStatusLabel1.Text = "";

            // if the selected cell changes, update the display data
            spreadsheetPanel1.SelectionChanged += NewSelection;
           
            
        }

        /// <summary>
        /// Check if variable is valid
        /// </summary>
        /// <param name="name"></param>
        private Boolean variablechecker(String name)
        {
           
            if (Regex.IsMatch(name, @"^[a-zA-Z][1-9][0-9]?$"))
                return true;
            else return false;
        }

        /// <summary>
        /// get name of selected cell
        /// </summary>
        /// <returns>The name of the selected cell</returns>
        private String GetCellName()
        {
            // get the row and the column of selected cell from panel
            int col, row;
            spreadsheetPanel1.GetSelection(out col, out row);
      
            int cellRow = ++row;
      

            char cellCol = (char)('A' + col);
            return "" + cellCol + cellRow;
        }

 




/// <summary>
/// helper method that updates the cell value box and the cell name and the cell selected.
/// </summary>
        private void SpreadsheetUpdater()
        {
                  
            String CellNameText = GetCellName();
            Object cellContents = spreadsheet.GetCellContents(CellNameText);
            Object cellValue = spreadsheet.GetCellValue(CellNameText);

                    
            cellNameBox.Text = CellNameText;

          
            if (spreadsheet.GetCellContents(CellNameText) is Formula)
                cellContentsBox.Text = "=" + cellContents.ToString();
            else // otherwise just set the textbox equal to the content
                cellContentsBox.Text = cellContents.ToString();

         
            if (spreadsheet.GetCellValue(CellNameText) is FormulaError)
            {
               
                FormulaError error = (FormulaError)spreadsheet.GetCellValue(CellNameText);
                cellValueBox.Text = error.Reason;
            }

          
                cellValueBox.Text = cellValue.ToString();
        }

        /// <summary>
        /// This is a helper method that will update the spreadsheet when a new cell is selected
        /// </summary>

        private void NewSelection(SpreadsheetPanel ss)
        {
            SpreadsheetUpdater();
        }

        /// <summary>
        /// This is a helper method for the evaluate button that will evaluate the cell when pressed
        /// </summary>
        /// <param name="e">the event</param>
        /// <param name="sender">default</param>
        private void evaluateButton_Click(object sender, EventArgs e)
        {
            ISet<String> dependents = null; // will hold dependents of selected cell

            try
            {
              
                dependents = spreadsheet.SetContentsOfCell(cellNameBox.Text, cellContentsBox.Text);
            }
            catch (FormulaFormatException)
            {
              
                MessageBox.Show("Error: You have entered an invalid formula. Your changes will"
                + " be reverted and the formula will not be saved.", "Formula Format Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            catch (CircularException)
            {
              
                MessageBox.Show("Error: A circular reference occurred. Some change you have made"
                    + " to the currently selected cell was invalid. All changes will be reverted.", "Circular Reference Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

        
            SpreadsheetUpdater();

        
            if (ReferenceEquals(dependents, null))
                return;

            
            foreach (string d in dependents)
                UpdateCell(d);

          
            if (spreadsheet.Changed)
                toolStripStatusLabel1.Text = "File not saved";
        }

     /// <summary>
     /// 
     /// helper method that will make return the same as clicking on evaluate button
     /// </summary>
     /// <param name="sender"></param>
     /// <param name="e"></param>
        private void cellContentsBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            // if the key that was pressed is the return key "\r"

            if (e.KeyChar.Equals('\r')) // then evaluate the formula box
                evaluateButton_Click(sender, e);
      







        }

        /// <summary>
        /// Will update all of the cell and its dependents
        /// </summary>
        /// <param name="cellName"></param>
        private void UpdateCell(string cellName)
        {
            // data needed to update cell
            int cellColIndex;
            int cellRowIndex;
            string cellValue;

            // helper methods return the index of the row and column
            cellColIndex = GetColIndex(cellName);
            cellRowIndex = GetRowIndex(cellName);

            if ((spreadsheet.GetCellValue(cellName) is FormulaError))
            {
            
                FormulaError error = (FormulaError)spreadsheet.GetCellValue(cellName);
                cellValue = error.Reason;
            }
            else
                cellValue = spreadsheet.GetCellValue(cellName).ToString();

            // update the value of the cell
            spreadsheetPanel1.SetValue(cellColIndex, cellRowIndex, cellValue);
        }


        /// <summary>
        /// Private helper method to get the column index of a cell name
        /// </summary>
        /// <param name="cellName"></param>
        private int GetColIndex(string cellName)
        {
            int col;

            // get the column letter and convert it to the numerical index
            col = cellName.ToCharArray(0, 1)[0] - 'A';

            return col;

        }

        /// <summary>
        /// Private helper method to get the row index of a cell name
        /// </summary>
        /// <param name="cellName"></param>
        private int GetRowIndex(string cellName)
        {
            int row;

            // parse the remainder of the string as an int in order to 
            // get the row number and convert it to the numberical index            
            int.TryParse(cellName.Substring(1), out row);
            row--;

            return row;
        }

        /// <summary>
        /// This method is run when the 'New' button is clicked.  The method opens
        /// a new spreadsheet panel in a new window. 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
    
        private void newMenuItem_Click(object sender, EventArgs e)
        {
            SpreadsheetForm newForm = new SpreadsheetForm();
            int count = SpreadsheetApplicationContext.getAppContext().RunForm(newForm);
            newForm.Text = "Spreadsheet" + count; // iterate the number in the spreadsheet title

        }

        /// <summary>
        /// This method is run when the 'Open' button is clicked.  The method opens
        /// a saved spreadsheet from a file.        
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void openMenuItem_Click(object sender, EventArgs e)
        {
            // Referenced MSDN library for Filter syntax
            openFileDialog1.Filter = "Spreadsheet Files (*.sprd)|*.sprd|All Files (*.*)|*.*";
            openFileDialog1.DefaultExt = ".sprd";
            openFileDialog1.FileName = "";
            openFileDialog1.Title = "Open";
            openFileDialog1.ShowDialog();

            // get the name of the selected file
            string openname = openFileDialog1.FileName;
            
            if (openname == "")
                return;

         
            if (openFileDialog1.FilterIndex == 1)
                openFileDialog1.AddExtension = true;

           
            if (spreadsheet.Changed)
            {
                DialogResult result = MessageBox.Show("your spreadsheet is not saved do you want to overwrite existing file",
                    "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);

                // if 'No' then we don't want to open the file so return
                if (result == DialogResult.No)
                    return;
            }

            try // Try to load a spreadsheet from a file
            {
              



                foreach (string s in spreadsheet.GetNamesOfAllNonemptyCells())
                {
                    int CellColumnIndex = GetColIndex(s);
                    int CellRowIndex = GetRowIndex(s);
                    spreadsheetPanel1.SetValue(CellColumnIndex, CellRowIndex, "");
                }



                spreadsheet = new Spreadsheet(openname, variablechecker, s => s.ToUpper(), "ps6");
                filename = openname; // initialize to the filepath where opened
                this.Text = openname; // set title to filename            
                SpreadsheetUpdater(); // update the tool boxes                

                // Update each cell in the GUI to show the values in the spreadsheet
                foreach (string s in spreadsheet.GetNamesOfAllNonemptyCells())
                    UpdateCell(s);

                // Changed should now be false, so show that the spreadsheet has been saved
                if (!spreadsheet.Changed)
                    toolStripStatusLabel1.Text = "";


            }
            catch (SpreadsheetReadWriteException s)
            {
                MessageBox.Show("no such file exist, please select another one", "error", MessageBoxButtons.OK, MessageBoxIcon.Error); // display the message from the exception
            }

                 

        }

  /// <summary>
  /// 
  /// save the spreadsheet on click
  /// </summary>
  /// <param name="sender"></param>
  /// <param name="e"></param>
        private void saveMenuItem_Click(object sender, EventArgs e)
        {
            if (ReferenceEquals(filename, null)) // If no file name has been specificed                            
                saveAsToolStripMenuItem_Click(sender, e);
            // if the spreadsheet hasn't been saved yet, call the Save As method instead

            // if the file name is still empty, it means Save As was just canceled so return
            if (ReferenceEquals(filename, null))
            {
                saveCanceled = true;
                return;
            }


            // If the spreadsheet has been changed then save it
            if (spreadsheet.Changed)
                spreadsheet.Save(filename);

            // Changed should now be false, so show that the spreadsheet has been saved
            if (!spreadsheet.Changed)
                toolStripStatusLabel1.Text = "Saved";

            // if not, do nothing
        }

      /// <summary>
      /// 
      /// save as the spreadsheet at a specific location
      /// </summary>
      /// <param name="sender"></param>
      /// <param name="e"></param>
        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
        
            saveFileDialog1.Filter = "Spreadsheet Files (*.sprd)|*.sprd|All Files (*.*)|*.*";
            saveFileDialog1.DefaultExt = ".sprd";
            saveFileDialog1.Title = "Save";
            saveFileDialog1.ShowDialog();

            string savename = saveFileDialog1.FileName;

           
            if (savename == "")
                return;

       
            DialogResult result = MessageBox.Show("Saving to " + savename +
                    " will overwrite existing file. Save anyways?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);

            // if 'No' then we don't want to open the file so return
            if (result == DialogResult.No)
                return;

            this.Text = savename; // set the name of the document to the saved name

        //add extension of .sprd
            if (saveFileDialog1.FilterIndex == 1)
                saveFileDialog1.AddExtension = true;

            spreadsheet.Save(savename);

         
            filename = savename;
        }

        /// <summary>
        /// This method is run when the 'Close' button is clicked. If changes have been made
        /// it prompts the user with the option to save their changes.    
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void closeMenuItem_Click(object sender, EventArgs e)
        {
            if (!(spreadsheet.Changed)) // If no changes have been made just close the file
                Close();
            else
            {   
                DialogResult result = MessageBox.Show("Your spreadsheet has unsaved changes. Would you like to " +
                    "save your changes before closing?", "Save Before Exiting?", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning);

          
                if (result == DialogResult.Yes)
                {
                    saveMenuItem.PerformClick(); 
                    if (saveCanceled)  
                        return;
                }
                else if (result == DialogResult.Cancel)
                    return;

                
                passedFromClose = true;
                Close();
            }
        }

        /// <summary>
        /// This method addresses the case when a user clicks the red X in the top right
        /// of the form to close it. Prompts user with save option if changes have been made.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void SpreadsheetForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (spreadsheet.Changed & !passedFromClose)
            {  
                DialogResult result = MessageBox.Show("Your spreadsheet has unsaved changes. Would you like to " +
                    "save your changes before closing?", "Save Before Exiting?", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning);

                // if 'Yes' then save the file
                if (result == DialogResult.Yes)
                {
                    saveMenuItem.PerformClick(); // call the save method  
                    if (spreadsheet.Changed)     // if changed is still true (meaning user did not save the spreadsheet)
                        e.Cancel = true;         // then don't close the spreadsheet
                }
                else if (result == DialogResult.Cancel)
                    e.Cancel = true; 

              
            }
        }

        /// <summary>
        /// Help file that describe the functionality of the spreadsheet and some of the extra features that were implemented
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void helpMenu_Click(object sender, EventArgs e)
        {
            MessageBox.Show(
                "This spreadsheet will intake double string and formula with an =. You have to input the value into" +
                "the F(x) box and hit evaluate to input the value. The spreadsheet will also allow users to save their files in .sprd format" +
                "There is a cell name box in the far left that will show the name of each cell. The save feature can save the spreadsheet at the " +
                "current location or save it in a specific location in the save as menu. The spreadsheet has some extra feature in that you can" +
                "use the return key instead of clicking evaluate to input cell value. The name of spreadsheet will also increment as you open new " +
                "windows of spreadsheet.  "
                , "Help", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }


    }
}